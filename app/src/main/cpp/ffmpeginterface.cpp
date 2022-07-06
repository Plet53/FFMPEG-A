#include <jni.h>
#include <android/log.h>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
extern "C" {
  #include "libavutil/dict.h"
  #include "libavformat/avformat.h"
}

#define BUF_SIZE 4096

jclass istream, ostream;
jobject instream, outstream;
jmethodID avail, marksup, read, reset, skip, write;
JNIEnv *tenv;
unsigned char *natinbuf, *natoutbuf, *inbuffer, *outbuffer;
jbyteArray jinbuffer, joutbuffer;

void android_av_log(void *avcl, int level, const char* format, va_list args);

int loglevels[8] =
  {ANDROID_LOG_FATAL, ANDROID_LOG_FATAL, ANDROID_LOG_ERROR,
   ANDROID_LOG_WARN, ANDROID_LOG_INFO, ANDROID_LOG_VERBOSE,
   ANDROID_LOG_DEBUG, ANDROID_LOG_DEBUG};

extern "C"
JNIEXPORT void JNICALL
Java_com_fantastico_1softworks_ffmpeg_1a_TranscodeActivity_00024Companion_nativeInit
  (JNIEnv *env, jobject thiz) {
  // Set up logging system to work with android's logging environment.
  av_log_set_callback(&android_av_log);
  // Get some info from the java environ.
  istream = env->FindClass("java/io/InputStream");
  ostream = env->FindClass("java/io/OutputStream");
  avail = env->GetMethodID(istream, "available", "()I");
  marksup = env->GetMethodID(istream, "markSupported", "()Z");
  reset = env->GetMethodID(istream, "reset", "()V");
  skip = env->GetMethodID(istream, "skip", "(J)J");
  read = env->GetMethodID(istream, "read", "([BII)I");
  write = env->GetMethodID(ostream, "write", "([BII)V");
}

// Forward av_log messages to the android log library
void android_av_log(void *avcl, int level, const char* format, va_list args) {
  if (level == AV_LOG_QUIET) return;
  int prio;
  if ((level > AV_LOG_TRACE) || ((level % 8) != 0)) prio = ANDROID_LOG_UNKNOWN;
  else prio = loglevels[level / 8];
  AVClass* avc = avcl ? *(AVClass **) avcl : nullptr;
  __android_log_vprint(prio, avc ? avc->class_name : "av_log", format, args);
  return;
}

// Implement Seeking for Java Input Stream
int64_t jiseek(void *opaque, int64_t offset, int whence) {
  if (offset < 0) {
    tenv->CallVoidMethod(instream, reset);
  }
  int64_t off = offset + ((offset < 0) ? whence : 0);
  int64_t r = tenv->CallLongMethod(instream, skip, off);
  return r;
}

// Implement Reading for Java Input Stream
int jiread(void *opaque, uint8_t *buf, int buf_size) {
  // Wondering why I have to gate this, personally.
  buf_size = FFMIN(buf_size, BUF_SIZE);
  int r = tenv->CallIntMethod(instream, read, jinbuffer, 0, buf_size);
  // The only possible exception is IOError, which, we can't really handle. Thus, Exit.
  if (tenv->ExceptionCheck()) {
    tenv->ExceptionClear();
    return AVERROR_EXIT;
  }
  if (r < 1) return AVERROR_EOF;
  // JNI requires manual updates in order to transmit info between layers.
  tenv->GetByteArrayRegion(jinbuffer, 0, r, reinterpret_cast<jbyte *>(buf));
  return r;
}

// Implement Writing for Java Output Stream
int jiwrite(void *opaque, uint8_t *buf, int buf_size) {
  buf_size = FFMIN(buf_size, BUF_SIZE);
  tenv->SetByteArrayRegion(joutbuffer, 0, buf_size, reinterpret_cast<jbyte *>(buf));
  tenv->CallVoidMethod(outstream, write, joutbuffer, 0, buf_size);
  if (tenv->ExceptionCheck()) {
    tenv->ExceptionClear();
    return AVERROR_EXIT;
  }
  return buf_size;
}

// Cleanup the varying resources used.
void cleanup() {
  if (jinbuffer) {
    tenv->ReleaseByteArrayElements(jinbuffer, reinterpret_cast<jbyte *>(natinbuf), 0);
  }
  if (joutbuffer) {
    tenv->ReleaseByteArrayElements(joutbuffer, reinterpret_cast<jbyte *>(natoutbuf), 0);
  }
  free(natinbuf); free(natoutbuf);
  natinbuf = natoutbuf = nullptr;
  jinbuffer = joutbuffer = nullptr;
  tenv = nullptr;
  return;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_fantastico_1softworks_ffmpeg_1a_TranscodeActivity_00024Companion_probeVid
  (JNIEnv *env, jobject thiz, jobject IOStream) {
  // JNI Busywork.
  tenv = env;
  jinbuffer = tenv->NewByteArray(BUF_SIZE);
  inbuffer = reinterpret_cast<unsigned char *>(tenv->GetByteArrayElements(jinbuffer, JNI_FALSE));
  natinbuf = static_cast<unsigned char *>(malloc(BUF_SIZE));
  instream = tenv->NewGlobalRef(IOStream);
  // Get return ready.
  jclass o = tenv->FindClass("com/fantastico_softworks/ffmpeg_a/TranscodeActivity$VidMeta");
  jobject r = tenv->AllocObject(o);
  jfieldID d = tenv->GetFieldID(o, "dur", "J");
  jfieldID w = tenv->GetFieldID(o, "wid", "I");
  jfieldID h = tenv->GetFieldID(o, "hei", "I");
  // Make sure we've got RAM.
  if (inbuffer == nullptr) {
    tenv->DeleteGlobalRef(instream);
    tenv = nullptr;
    return r;
  }
  // In order to work with android's file security standards, I need to implement a custom
  // AVIOContext that reads packets from a java managed InputStream.
  // HOH BOY HERE I GO ON MY NONSENSE.
  AVFormatContext *fmt_ctx = avformat_alloc_context();
  fmt_ctx->probesize = BUF_SIZE * 1028;
  bool seekable = tenv->CallBooleanMethod(IOStream, marksup);
  fmt_ctx->pb = avio_alloc_context(
                  natinbuf,
                  BUF_SIZE,
                  0,
                  nullptr,
                  &jiread,
                  nullptr,
                  seekable ? &jiseek : nullptr);
  // Guard against unworkable states.
  jlong e = avformat_open_input(&fmt_ctx, "", nullptr, nullptr);
  if (e < 0) {
    tenv->DeleteGlobalRef(instream);
    avformat_close_input(&fmt_ctx);
    tenv->SetLongField(r, d, e);
    cleanup();
    return r;
  }
  e = avformat_find_stream_info(fmt_ctx, nullptr);
  if (e < 0) {
    tenv->DeleteGlobalRef(instream);
    avformat_close_input(&fmt_ctx);
    tenv->SetLongField(r, d, e);
    cleanup();
    return r;
  }
  // Free resources
  tenv->ReleaseByteArrayElements(jinbuffer, reinterpret_cast<jbyte *>(inbuffer), 0);
  tenv->SetLongField(r, d, fmt_ctx->duration);
  for (int i = 0; i < fmt_ctx->nb_streams; i++) {
    AVStream *stream = fmt_ctx->streams[i];
    __android_log_print(ANDROID_LOG_INFO, "res", "%dx%d", stream->codecpar->height, stream->codecpar->width);
    tenv->SetIntField(r, h, FFMAX(tenv->GetIntField(r, h), stream->codecpar->height));
    tenv->SetIntField(r, w, FFMAX(tenv->GetIntField(r, w), stream->codecpar->width));
  }
  avformat_close_input(&fmt_ctx);
  tenv->DeleteGlobalRef(instream);
  cleanup();
  // Return duration, which will be used to determine bitrate in managed code.
  return r;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_fantastico_1softworks_ffmpeg_1a_TranscodeActivity_00024Companion_transcode
  (JNIEnv *env, jobject thiz, jobject inputstream, jobject outputstream, jint type, jint bitrate,
   jint height, jint width) {
  // TODO: implement transcode()
  // JNI Busywork
  tenv = env;
  jinbuffer = tenv->NewByteArray(BUF_SIZE);
  inbuffer = reinterpret_cast<unsigned char *>(tenv->GetByteArrayElements(jinbuffer, JNI_FALSE));
  natinbuf = static_cast<unsigned char *>(malloc(BUF_SIZE));
  instream = tenv->NewGlobalRef(inputstream);
  // First buffer fail chance.
  if (natinbuf == nullptr) {
    tenv->DeleteGlobalRef(instream);
    cleanup();
    return;
  }
  joutbuffer = tenv->NewByteArray(BUF_SIZE);
  outbuffer = reinterpret_cast<unsigned char *>(tenv->GetByteArrayElements(jinbuffer, JNI_FALSE));
  natoutbuf = static_cast<unsigned char *>(malloc(BUF_SIZE));
  outstream = tenv->NewGlobalRef(outputstream);
  // Second buffer fail chance.
  if (natoutbuf == nullptr) {
    tenv->DeleteGlobalRef(instream);
    tenv->DeleteGlobalRef(outstream);
    cleanup();
    return;
  }
  /* The flow must go as follows:
   * OPEN INPUT VIDEO, GATHER RELEVANT INFORMATION
   * SET UP OUTPUT VIDEO USING RELEVANT INFORMATION FROM MANAGED CODE
   * DEMUX, DECODE, MUX, ENCODE
   * CLEAN UP
   */
  // Setup I/O
  bool seekable = tenv->CallBooleanMethod(inputstream, marksup);
  AVFormatContext *infmt_ctx = avformat_alloc_context(), *outfmt_ctx = avformat_alloc_context();
  infmt_ctx->pb = avio_alloc_context(
      natinbuf,
      BUF_SIZE,
      0,
      nullptr,
      &jiread,
      nullptr,
      seekable ? &jiseek : nullptr);
  outfmt_ctx->pb = avio_alloc_context(
      natoutbuf,
      BUF_SIZE,
      1,
      nullptr,
      nullptr,
      &jiwrite,
      nullptr);
  // Gather information about input video
  int e = avformat_open_input(&infmt_ctx, "", nullptr, nullptr);
  if (e < 0) {
    __android_log_print(ANDROID_LOG_ERROR, "avfmt_open", "%s", av_err2str(e));
    tenv->DeleteGlobalRef(instream);
    tenv->DeleteGlobalRef(outstream);
    cleanup();
    return;
  }
  e = avformat_find_stream_info(infmt_ctx, nullptr);
  if (e < 0) {
    __android_log_print(ANDROID_LOG_ERROR, "avfmt_open", "%s", av_err2str(e));
    tenv->DeleteGlobalRef(instream);
    tenv->DeleteGlobalRef(outstream);
    cleanup();
    return;
  }
  // Prepare output video
  
}