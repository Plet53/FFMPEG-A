
# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("ffmpeg_a")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library(
        hello
        SHARED
        src/main/cpp/hello.cpp
)

#Add the FFMPEG Libraries

#AVUtil depends only on itself
add_library(
        avutil
        SHARED
        IMPORTED
)

set_target_properties(
        avutil
        PROPERTIES IMPORTED_LOCATION
        D:/FFMPEG-A/app/jni/${ANDROID_ABI}/lib/libavutil.so
)

#AVCodec depends on Util
add_library(
        avcodec
        SHARED
        IMPORTED
)

set_target_properties(
        avcodec
        PROPERTIES IMPORTED_LOCATION
        D:/FFMPEG-A/app/jni/${ANDROID_ABI}/lib/libavcodec.so
)

target_link_libraries(
        avcodec
        INTERFACE avutil
)

#AVFilter depends on Util
add_library(
        avfilter
        SHARED
        IMPORTED
)

set_target_properties(
        avfilter
        PROPERTIES IMPORTED_LOCATION
        D:/FFMPEG-A/app/jni/${ANDROID_ABI}/lib/libavfilter.so
)

target_link_libraries(
        avfilter
        INTERFACE avutil
)

#AVFormat depends on Util and Codec
add_library(
        avformat
        SHARED
        IMPORTED
)

set_target_properties(
        avformat
        PROPERTIES IMPORTED_LOCATION
        D:/FFMPEG-A/app/jni/${ANDROID_ABI}/lib/libavformat.so
)

target_link_libraries(
        avformat
        INTERFACE avcodec
        INTERFACE avutil
)

#PostProc depends only on itself
add_library(
        postproc
        SHARED
        IMPORTED
)

set_target_properties(
        postproc
        PROPERTIES IMPORTED_LOCATION
        D:/FFMPEG-A/app/jni/${ANDROID_ABI}/lib/libpostproc.so
)

#SWResample depends on Util
add_library(
        swresample
        SHARED
        IMPORTED
)

set_target_properties(
        swresample
        PROPERTIES IMPORTED_LOCATION
        D:/FFMPEG-A/app/jni/${ANDROID_ABI}/lib/libswresample.so
)

target_link_libraries(
        swresample
        INTERFACE avutil
)

#SWScale depends on Util
add_library(
        swscale
        SHARED
        IMPORTED
)

set_target_properties(
        swscale
        PROPERTIES IMPORTED_LOCATION
        D:/FFMPEG-A/app/jni/${ANDROID_ABI}/lib/libswscale.so
)

target_link_libraries(
        swscale
        INTERFACE avutil
)

# And the Main Interface Library depends on All Of Them
add_library(
        ffmpeginterface
        SHARED
        src/main/cpp/ffmpeginterface.cpp
)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
find_library( # Sets the name of the path variable.
        log-lib
        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log )

target_link_libraries(
        ffmpeginterface
        avutil
        #avcodec
        #avfilter
        avformat
        #postproc
        #swresample
        #swscale
        ${log-lib}
)

include_directories(jni/${CMAKE_ANDROID_ARCH_ABI}/include/)
