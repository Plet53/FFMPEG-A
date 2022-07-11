#!/bin/bash

git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg-5ss

if [ -z "$ANDROID_NDK" ]; then
  echo "Please set ANDROID_NDK to the Android NDK folder"
  exit 1
fi

#Change to your local machine's architecture
HOST_OS_ARCH=windows-x86_64

configure_ffmpeg () {

  ABI=$1
  PLATFORM_VERSION=$2
  TOOLCHAIN_PATH=$ANDROID_NDK/toolchains/llvm/prebuilt/${HOST_OS_ARCH}/bin
  local STRIP_COMMAND

  # Determine the architecture specific options to use
  case ${ABI} in
  armeabi-v7a)
    TOOLCHAIN_PREFIX=armv7a-linux-androideabi
    ARCH=armv7-a
    EXTRA_CONFIG="--disable-neon  --disable-asm"
    STRIP_COMMAND=arm-linux-androideabi-strip
    ;;
  arm64-v8a)
    TOOLCHAIN_PREFIX=aarch64-linux-android
    ARCH=aarch64
    ;;
  x86)
    TOOLCHAIN_PREFIX=i686-linux-android
    ARCH=x86
    EXTRA_CONFIG="--disable-asm"
    ;;
  x86_64)
    TOOLCHAIN_PREFIX=x86_64-linux-android
    ARCH=x86_64
    EXTRA_CONFIG="--disable-asm"
    ;;
  esac

  if [ -z ${STRIP_COMMAND} ]; then
    STRIP_COMMAND=${TOOLCHAIN_PREFIX}-strip
  fi

  echo "Configuring FFmpeg build for ${ABI}"

  ./configure \
  --prefix=app/jni/${ABI} \
  --target-os=android \
  --arch=${ARCH} \
  --cc=${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}${PLATFORM_VERSION}-clang \
  --strip=${TOOLCHAIN_PATH}/${STRIP_COMMAND} \
  --extra-cflags="-w" \
  --x86asmexe=yasm \
  --enable-gpl \
  --enable-cross-compile \
  --disable-network \
  --disable-programs \
  --disable-hwaccels \
  --disable-devices \
  --disable-doc \
  --enable-shared \
  ${EXTRA_CONFIG} \
  --disable-protocol=concat \
  --disable-protocol=concatf \
  --disable-protocol=ftp \
  --disable-protocol=file \
  --disable-protocol=subfile \
  --disable-protocol=tee \
  --enable-protocol=async \
  
  return $?
}

build_ffmpeg () {

  configure_ffmpeg $1 $2

  if [ $? -eq 0 ]
  then
          make clean
          make -j6
          make install
  else
          echo "FFmpeg configuration failed, please check the error log."
  fi
}

build_ffmpeg armeabi-v7a 21
build_ffmpeg arm64-v8a 21
build_ffmpeg x86 21
build_ffmpeg x86_64 21