#include "stdlib.h"
#include "string.h"
#include <jni.h>

JNIEXPORT jstring JNICALL
Java_com_fantastico_1softworks_ffmpeg_1a_ui_main_FragmentSimple_hello(JNIEnv *env, jobject thiz) {
  jstring d = (*env)->NewStringUTF(env, "Hello, World!");
  return d;
}