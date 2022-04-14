#include <jni.h>
#include <android/log.h>
#include <cstdlib>
#include <cstring>
extern "C" {
  #include "libavutil/dict.h"
  #include "libavformat/avformat.h"
}

#define BUF_SIZE 4096

jclass istream, ostream;
jobject instream, outstream;
jmethodID avail, marksup, read, reset, skip;
JNIEnv *tenv;
AVDictionaryEntry *tag;
unsigned char *natbuf;
char dest[16];
jbyte *buffer;
jbyteArray jbuffer;

extern "C"
JNIEXPORT void JNICALL
Java_com_fantastico_1softworks_ffmpeg_1a_TranscodeActivity_00024Companion_nativeInit
  (JNIEnv *env, jobject thiz) {
  istream = env->FindClass("java/io/InputStream");
  ostream = env->FindClass("java/io/OutputStream");
  avail = env->GetMethodID(istream, "available", "()I");
  marksup = env->GetMethodID(istream, "markSupported", "()Z");
  reset = env->GetMethodID(istream, "reset", "()V");
  skip = env->GetMethodID(istream, "skip", "(J)J");
  read = env->GetMethodID(istream, "read", "([BII)I");
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
  // Wondering why I have to gate this myself, personally.
  buf_size = FFMIN(buf_size, BUF_SIZE);
  int r = tenv->CallIntMethod(instream, read, jbuffer, 0, buf_size);
  if (r < 1) return AVERROR_EOF;
  memcpy(buf, reinterpret_cast<unsigned char *>(buffer), r);
  return r;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_fantastico_1softworks_ffmpeg_1a_TranscodeActivity_00024Companion_probeVid
  (JNIEnv *env, jobject thiz, jobject IOStream) {
  tenv = env;
  jbuffer = tenv->NewByteArray(BUF_SIZE);
  buffer = tenv->GetByteArrayElements(jbuffer, JNI_FALSE);
  natbuf = static_cast<unsigned char *>(malloc(BUF_SIZE));
  instream = IOStream;
  const char* dur = "duration";
  const char* bit = "bitrate";
  // Get the return ready.
  jclass o = tenv->FindClass("com/fantastico_softworks/ffmpeg_a/TranscodeActivity$VidMeta");
  jobject r = tenv->AllocObject(o);
  jfieldID d = tenv->GetFieldID(o, "dur", "Ljava/lang/String;");
  jfieldID v = tenv->GetFieldID(o, "vidbitrate", "I");
  if (buffer == nullptr) {
    tenv->SetObjectField(r, d, tenv->NewStringUTF("Could not allocate buffer."));
    tenv->SetIntField(r, v, 0);
    tenv->ReleaseByteArrayElements(jbuffer, buffer, 0);
    tenv = nullptr;
    return r;
  }
  // In order to work with android's file security standards, I need to implement a custom
  // AVIOContext that reads packets from a java managed InputStream.
  // HOH BOY HERE I GO ON MY NONSENSE.
  AVFormatContext *fmt_ctx = avformat_alloc_context();
  bool seekable = tenv->CallBooleanMethod(IOStream, marksup);
  fmt_ctx->pb = avio_alloc_context(
                  natbuf,
                  BUF_SIZE,
                  0,
                  nullptr,
                  &jiread,
                  nullptr,
                  seekable ? &jiseek : nullptr);
  // Guard against unworkable states.
  // TODO: Invalid Data? Look into.
  int e = avformat_open_input(&fmt_ctx, "", nullptr, nullptr);
  sprintf(dest, "%d", e);
  __android_log_write(ANDROID_LOG_INFO, "native", dest);
  if (e < 0) {
    tenv->SetObjectField(r, d, tenv->NewStringUTF(av_err2str(e)));
    tenv->SetIntField(r, v, 0);
    tenv->ReleaseByteArrayElements(jbuffer, buffer, 0);
    tenv = nullptr;
    avformat_free_context(fmt_ctx);
    return r;
  }
  sprintf(dest, "%p", fmt_ctx->metadata);
  __android_log_write(ANDROID_LOG_INFO, "native", dest);
  e = avformat_find_stream_info(fmt_ctx, nullptr);
  if (e < 0) {
    tenv->SetObjectField(r, d, tenv->NewStringUTF(av_err2str(e)));
    tenv->SetIntField(r, v, 0);
    tenv->ReleaseByteArrayElements(jbuffer, buffer, 0);
    tenv = nullptr;
    avformat_free_context(fmt_ctx);
    return r;
  }
  // Grab the tags we're interested in, store at noted pointers.
  tag = av_dict_get(fmt_ctx->metadata, dur, nullptr, AV_DICT_IGNORE_SUFFIX);
  jstring dura = tenv->NewStringUTF(tag->value);
  tag = av_dict_get(fmt_ctx->metadata, bit, nullptr, AV_DICT_IGNORE_SUFFIX);
  int vbit = strtol(tag->value, nullptr, 10);
  // Set fields
  tenv->SetObjectField(r, d, dura);
  tenv->SetIntField(r, v, vbit);
  // Free resources
  tenv->ReleaseByteArrayElements(jbuffer, buffer, 0);
  avformat_free_context(fmt_ctx);
  tenv = nullptr;
  // Return
  return r;
}
