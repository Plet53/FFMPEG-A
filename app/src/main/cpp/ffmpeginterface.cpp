#include <jni.h>
#include <android/log.h>
#include <cstdlib>
#include <cstring>
#include <cstdarg>

extern "C" {
  #include "libavutil/dict.h"
  #include "libavformat/avformat.h"
  #include "libavutil/avutil.h"
  #include "libswresample/swresample.h"
  #include "libswscale/swscale.h"
  #include "libavutil/channel_layout.h"
  #include "libavutil/frame.h"
  #include "libavutil/samplefmt.h"
  #include <libavutil/opt.h>
  #include "libavcodec/avcodec.h"
  #include "libavutil/channel_layout.h"
}

#define DEBUG 1
#define BUF_SIZE 409600
#define AUD_RATE 96000

jclass istream, ostream;
jobject instream, outstream;
jmethodID marksup, read, reset, skip, write;
JNIEnv *tenv;
unsigned char *natinbuf, *natoutbuf;
jbyte *inbuffer, *outbuffer;
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
  marksup = env->GetMethodID(istream, "markSupported", "()Z");
  reset = env->GetMethodID(istream, "reset", "()V");
  skip = env->GetMethodID(istream, "skip", "(J)J");
  read = env->GetMethodID(istream, "read", "([BII)I");
  write = env->GetMethodID(ostream, "write", "([BII)V");
}

// Aquire output format info from chosen preset.
const AVOutputFormat *get_Preset(int id) {
  const AVOutputFormat *of;
  char type[8], mime[16];
  strcpy(mime, "video/");
  switch (id) {
    case 1:
      strcpy(reinterpret_cast<char *const>(type), "mp4");
      break;
    case 2:
      strcpy(reinterpret_cast<char *const>(type), "avi");
      break;
    // This will cover 0, the most common case.
    // Part of why I'm making this is to help Webm proliferate, after all.
    default:
      strcpy(reinterpret_cast<char *const>(type), "webm");
      break;
  }
  strcat(mime, type);
  // Without intimate knowledge of video formats,
  // I can't reliably create my own custom video formats.
  of = av_guess_format(type, nullptr, mime);
  return of;
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

extern "C"
JNIEXPORT jobject JNICALL
Java_com_fantastico_1softworks_ffmpeg_1a_TranscodeActivity_probeVid
  (JNIEnv *env, jobject thiz, jobject IOStream) {
  // JNI Busywork.
  tenv = env;
  jinbuffer = tenv->NewByteArray(BUF_SIZE);
  inbuffer = tenv->GetByteArrayElements(jinbuffer, JNI_FALSE);
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
  fmt_ctx->probesize = static_cast<int64_t>(BUF_SIZE * 1028);
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
    tenv = nullptr;
    return r;
  }
  e = avformat_find_stream_info(fmt_ctx, nullptr);
  if (e < 0) {
    tenv->DeleteGlobalRef(instream);
    avformat_close_input(&fmt_ctx);
    tenv->SetLongField(r, d, e);
    tenv = nullptr;
    return r;
  }
  tenv->SetLongField(r, d, fmt_ctx->duration);
#ifdef DEBUG
  AVDictionaryEntry *tag = nullptr;
  while (tag = av_dict_get(fmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)) {
    __android_log_print(ANDROID_LOG_INFO, "file", "%s:%s", tag->key, tag->value);
  }
#endif
  for (int i = 0; i < fmt_ctx->nb_streams; i++) {
    AVStream *stream = fmt_ctx->streams[i];
#ifdef DEBUG
    while (tag = av_dict_get(stream->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)) {
        __android_log_print(ANDROID_LOG_INFO, "stream", "%s:%s", tag->key, tag->value);
    }
#endif
    tenv->SetIntField(r, h, FFMAX(tenv->GetIntField(r, h), stream->codecpar->height));
    tenv->SetIntField(r, w, FFMAX(tenv->GetIntField(r, w), stream->codecpar->width));
  }
  avformat_close_input(&fmt_ctx);
  tenv->DeleteGlobalRef(instream);
  tenv = nullptr;
  // Return duration and res, which will be used to determine bitrate in managed code.
  return r;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_fantastico_1softworks_ffmpeg_1a_TranscodeActivity_transcode
  (JNIEnv *env, jobject thiz, jobject inputstream, jobject outputstream,
   jint type, jint height, jint width, jint bitrate, jboolean audio) {
  // Fields we expect to use.
  int aud, e, audid;
  jboolean out = JNI_FALSE;
  int64_t vidpos = 0, audpos = 0;
  AVStream *ovidstream, *oaudstream, *ividstream, *iaudstream;
  const AVOutputFormat *ofmt;
  AVFormatContext *infmt_ctx, *outfmt_ctx;
  const AVCodec *inaudcod, *invidcod, *outaudcod, *outvidcod;
  //const AVCodecDescriptor *avcd;
  AVCodecContext *ividcod, *iaudcod, *ovidcod, *oaudcod;
  SwrContext *swr_ctx = nullptr;
  SwsContext *sws_ctx = nullptr;
  AVPacket *pkt = nullptr;
  AVFrame *dec, *enc;
  // JNI Busywork
  tenv = env;
  bool seekable = tenv->CallBooleanMethod(inputstream, marksup);
  jinbuffer = tenv->NewByteArray(BUF_SIZE);
  inbuffer = tenv->GetByteArrayElements(jinbuffer, JNI_FALSE);
  natinbuf = static_cast<unsigned char *>(malloc(BUF_SIZE));
  instream = tenv->NewGlobalRef(inputstream);
  outstream = tenv->NewGlobalRef(outputstream);
  __android_log_print(ANDROID_LOG_INFO, "native", "%s", "debug");
  // First buffer fail chance.
  if (!natinbuf) goto final;
  joutbuffer = tenv->NewByteArray(BUF_SIZE);
  outbuffer = tenv->GetByteArrayElements(jinbuffer, JNI_FALSE);
  natoutbuf = static_cast<unsigned char *>(malloc(BUF_SIZE));
  // Second buffer fail chance.
  if (!natoutbuf) goto final;
  /* The flow must go as follows:
   * OPEN INPUT VIDEO, GATHER RELEVANT INFORMATION
   * SET UP OUTPUT VIDEO USING RELEVANT INFORMATION FROM MANAGED CODE AND CODEC STRUCTURES
   * DEMUX, MUX, DECODE, ENCODE
   * CLEAN UP
   */
  // Setup I/O
  infmt_ctx = avformat_alloc_context();
  infmt_ctx->pb = avio_alloc_context(
    natinbuf,
    BUF_SIZE,
    0,
    nullptr,
    &jiread,
    nullptr,
    seekable ? &jiseek : nullptr);
  if (!(infmt_ctx->pb)) {
    avformat_free_context(infmt_ctx);
    goto final; }
  // Gather information about input video
  e = avformat_open_input(&infmt_ctx, "", nullptr, nullptr);
  if (e < 0) {
    __android_log_print(ANDROID_LOG_ERROR, "avfmt_open", "%s", av_err2str(e));
    goto final;
  }
  e = avformat_find_stream_info(infmt_ctx, nullptr);
  if (e < 0) {
    __android_log_print(ANDROID_LOG_ERROR, "avfmt_open", "%s", av_err2str(e));
    goto final;
  }
  // Prepare output video
  ofmt = get_Preset(type);
  if (!ofmt) {
    avformat_free_context(infmt_ctx);
    __android_log_print(ANDROID_LOG_ERROR, "avfmt_open", "%s", "could not aqcuire format");
    goto final;
  }
  e = avformat_alloc_output_context2(&outfmt_ctx, ofmt, nullptr, nullptr);
  if (e < 0) {
    avformat_free_context(infmt_ctx);
    __android_log_print(ANDROID_LOG_ERROR, "avfmt_open", "%s", av_err2str(e));
    goto final;
  }
  outfmt_ctx->pb = avio_alloc_context(
    natoutbuf,
    BUF_SIZE,
    1,
    nullptr,
    nullptr,
    &jiwrite,
    nullptr);
  if (!(outfmt_ctx->pb)) goto cutcontext;
  ovidstream = avformat_new_stream(outfmt_ctx, nullptr);
  if (!ovidstream) goto cutcontext;
  for (int i = 0; i < infmt_ctx->nb_streams; i++) {
    if (infmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      ividstream = infmt_ctx->streams[i];
      invidcod = avcodec_find_decoder(infmt_ctx->streams[i]->codecpar->codec_id);
      if (!invidcod) { __android_log_print(ANDROID_LOG_ERROR, "avstm_open", "%s", "could not find video decoder");
        goto cutcontext; }
    }
    else if ((infmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) && audio == JNI_TRUE) {
      iaudstream = infmt_ctx->streams[i];
      audid = i;
      inaudcod = avcodec_find_decoder(infmt_ctx->streams[i]->codecpar->codec_id);
      if (!inaudcod) { __android_log_print(ANDROID_LOG_ERROR, "avstm_open", "%s", "could not find audio decoder");
        goto cutcontext; }
    }
  }
  // Being explicit about this, !! may be implementation dependant
  aud = iaudstream ? 1 : 0;
  __android_log_print(ANDROID_LOG_INFO, "native", "%s", "opening aud codecs");
  if (aud) {
    oaudstream = avformat_new_stream(outfmt_ctx, nullptr);
    if (!oaudstream) goto cutcontext;
    iaudcod = avcodec_alloc_context3(inaudcod);
    iaudcod->strict_std_compliance = -2;
    iaudcod->time_base = iaudstream->time_base;
    iaudcod->ch_layout = iaudstream->codecpar->ch_layout;
    iaudcod->sample_rate = iaudstream->codecpar->sample_rate;
    // Legally they could use a union type here. Factually, no real reason.
    iaudcod->sample_fmt = static_cast<AVSampleFormat>(iaudstream->codecpar->format);
    e = avcodec_open2(iaudcod, inaudcod, nullptr);
    if (e < 0) {
      __android_log_print(ANDROID_LOG_ERROR, "avcdc_open_ad", "%s", av_err2str(e));
      avcodec_free_context(&iaudcod);
      goto cutcontext;
    }
    outaudcod = avcodec_find_encoder(ofmt->audio_codec);
    if (!outaudcod) { avcodec_free_context(&iaudcod);
      goto cutcontext; }
    oaudcod = avcodec_alloc_context3(outaudcod);
    if (!oaudcod) { avcodec_free_context(&iaudcod);
      goto cutcontext; }
    oaudcod->strict_std_compliance = -2;
    oaudcod->time_base = iaudstream->time_base;
    oaudcod->bit_rate = AUD_RATE;
    oaudcod->sample_fmt = outaudcod->sample_fmts[0];
    // Listen to me. For the majority of what you're doing with audio you don't need more than 48 Khz
    oaudcod->sample_rate = FFMIN(iaudcod->sample_rate, 48000);
    // Fallback to Stereo Audio in the event that the Ch_Layouts structure isn't initialized.
    &(outaudcod->ch_layouts[0]) ? av_channel_layout_copy(&(oaudcod->ch_layout), &(outaudcod->ch_layouts[0])) :
    av_channel_layout_from_mask(&(oaudcod->ch_layout), AV_CH_LAYOUT_STEREO);
    e = avcodec_open2(oaudcod, outaudcod, nullptr);
    if (e < 0) {
      __android_log_print(ANDROID_LOG_ERROR, "avcdc_open_ae", "%s", av_err2str(e));
      goto justsound;
    }
    // Audio will need to be resampled
    e = swr_alloc_set_opts2(&swr_ctx, &oaudcod->ch_layout, oaudcod->sample_fmt, oaudcod->sample_rate,
      &iaudcod->ch_layout, iaudcod->sample_fmt, iaudcod->sample_rate, 0, nullptr);
    if (e < 0) { __android_log_print(ANDROID_LOG_ERROR, "swr_alloc", "%s", av_err2str(e));
      goto justsound; }
    swr_init(swr_ctx);
    oaudstream->codecpar->bit_rate = AUD_RATE;
    oaudstream->codecpar->sample_rate = FFMIN(iaudstream->codecpar->sample_rate, 48000);
  }
  __android_log_print(ANDROID_LOG_INFO, "native", "%s", "opening vid codecs");
  ividcod = avcodec_alloc_context3(invidcod);
  if (!ividcod) goto justsound;
  // In order to use experimental codecs, Strict Standard Compilance must be reduced.
  // Experimentals in this case being, Software VP8. Thanks Android Devs.
  ividcod->strict_std_compliance = -2;
  ividcod->time_base = ividstream->time_base;
  ividcod->height = ividstream->codecpar->height;
  ividcod->width = ividstream->codecpar->width;
  ividcod->pix_fmt = static_cast<AVPixelFormat>(ividstream->codecpar->format);
  e = avcodec_open2(ividcod, invidcod, nullptr);
  if (e < 0) {
    __android_log_print(ANDROID_LOG_ERROR, "avcdc_open_vd", "%s", av_err2str(e));
    avcodec_free_context(&ividcod);
    goto justsound;
  }
  outvidcod = avcodec_find_encoder(ofmt->video_codec);
  ovidcod = avcodec_alloc_context3(outvidcod);
  if (!ovidcod) { avcodec_free_context(&ividcod);
    goto justsound; }
  ovidcod->strict_std_compliance = -2;
  ovidcod->time_base = ividstream->time_base;
  // When we calculate the overall bitrate for the file, that's both video and audio, due to fileless constraints.
  ovidcod->bit_rate = bitrate - (AUD_RATE * aud);
  ovidcod->pix_fmt = &(outvidcod->pix_fmts[0]) ? outvidcod->pix_fmts[0] :
    AV_PIX_FMT_YUV420P;
  ovidcod->width = width;
  ovidcod->height = height;
  // TODO: Android SELinux simply does not allow device access, so this, can't work.
  e = avcodec_open2(ovidcod, outvidcod, nullptr);
  if (e < 0) {
    __android_log_print(ANDROID_LOG_ERROR, "avcdc_open_ve", "%s", av_err2str(e));
    avcodec_free_context(&ividcod);
    goto justsound;
  }
  ovidstream->codecpar->height = height;
  ovidstream->codecpar->width = width;
  ovidstream->codecpar->bit_rate = bitrate - (aud * AUD_RATE);
  if (ividcod->width != width ||
      ividcod->height != height ||
      ividcod->pix_fmt != ovidcod->pix_fmt ||
      infmt_ctx->video_codec_id != outfmt_ctx->video_codec_id) {
    sws_ctx = sws_getContext(
        ividstream->codecpar->width, ividstream->codecpar->height,
        ividcod->pix_fmt,
        width, height, oaudcod->pix_fmt, SWS_FAST_BILINEAR,
        nullptr, nullptr, nullptr);
    if (!sws_ctx) goto codecs;
  }
  __android_log_print(ANDROID_LOG_INFO, "native", "%s", "begin transcode");
  // Finally, we begin encoding.
  e = avformat_write_header(outfmt_ctx, nullptr);
  if (e < 0) {
    __android_log_print(ANDROID_LOG_ERROR, "avfmt_write", "%s", av_err2str(e));
    goto transform;
  }
  pkt = av_packet_alloc(); enc = av_frame_alloc(); dec = av_frame_alloc();
  if (!pkt) goto transform; if (!enc) goto finish; if (!dec) goto finish;
  // Until EOF (audpos only matters if we're dealing with audio)
  while (vidpos < infmt_ctx->duration && audpos < (infmt_ctx->duration * aud)) {
    // Grab the next available frame
    av_read_frame(infmt_ctx, pkt);
    // Make sure we grabbed something
    if (!(pkt->data)) {
      // If audio
      if (aud && pkt->stream_index == audid) {
        // Resample if needed
        if (swr_ctx) {
          avcodec_send_packet(iaudcod, pkt);
          avcodec_receive_frame(iaudcod, dec);
          // Initialize Output Audio Frame (channel layout, sample rate, format)
          enc->ch_layout = oaudcod->ch_layout;
          enc->sample_rate = oaudcod->sample_rate;
          enc->format = oaudcod->sample_fmt;
          e = swr_convert_frame(swr_ctx, enc, dec);
          if (e < 0) {
            __android_log_print(ANDROID_LOG_ERROR, "avfmt_write", "%s", av_err2str(e));
            goto finish;
          }
          audpos += pkt->duration;
          avcodec_send_frame(oaudcod, enc);
          avcodec_receive_packet(oaudcod, pkt);
        }
        pkt->stream_index = 1;
        // Otherwise, video
      } else {
        // Rescale if needed
        if (sws_ctx) {
          avcodec_send_packet(ividcod, pkt);
          avcodec_receive_frame(ividcod, dec);
          e = sws_scale_frame(sws_ctx, enc, dec);
          if (e < 0) {
            __android_log_print(ANDROID_LOG_ERROR, "avfmt_write", "%s", av_err2str(e));
            goto finish;
          }
          vidpos += pkt->duration;
          avcodec_send_frame(ovidcod, enc);
          avcodec_receive_packet(ovidcod, pkt);
        }
        pkt->stream_index = 0;
      }
      // Write frame to file
      av_interleaved_write_frame(outfmt_ctx, pkt);
      if (e < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "avfmt_write", "%s", av_err2str(e));
        goto finish;
      }
    } else {
      __android_log_print(ANDROID_LOG_ERROR, "avfmt_read", "%s", "Could not read data packet");
      goto finish;
    }
  }
  // Write trailer.
  e = av_write_trailer(outfmt_ctx);
  if (e < 0) { __android_log_print(ANDROID_LOG_ERROR, "avfmt_write", "%s", av_err2str(e)); goto finish;}
  // Mark that we have finished the job properly.
  out = JNI_TRUE;
  // GOTO Block for cleanup, keeps function calls off the stack.
  finish:
  if (dec) av_frame_free(&dec);
  if (enc) av_frame_free(&enc);
  av_packet_free(&pkt);
  transform:
  if (aud) swr_free(&swr_ctx);
  sws_freeContext(sws_ctx);
  codecs:
  avcodec_free_context(&ividcod);
  avcodec_free_context(&ovidcod);
  justsound:
  if (aud) {
    avcodec_free_context(&iaudcod);
    avcodec_free_context(&oaudcod);
  }
  cutcontext:
  // Avformat cleans up my own buffers for me! How kind.
  avformat_free_context(infmt_ctx);
  avformat_free_context(outfmt_ctx);
  final:
  tenv->DeleteGlobalRef(instream);
  tenv->DeleteGlobalRef(outstream);
  tenv = nullptr;
  return out;
}