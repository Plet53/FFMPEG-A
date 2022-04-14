#include "jni.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_fantastico_1softworks_ffmpeg_1a_ui_main_FragmentSimple_hello(JNIEnv *env, jobject thiz) {
  jstring d = (*env).NewStringUTF("Hello, World!");
  return d;
}