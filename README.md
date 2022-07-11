## Let me begin with a story.
 One day, I was walking my dogs at the local park. But I was also having a conversation via the Internet, on my smartphone. While I was out, I decided to take a brief video, and send it to my fellows, as it was relevant to the conversation. Now, some specifics. [The video in question is available here](https://youtube.com/watch?v=awFmpBUAU8U), and we're going to play a quick game. 
 <!--Anyone else have extreme distaste for the way youtube is handling Shorts? The link here will take you to the normal interface, but I didn't opt into this.-->
## Guess the filesize!
 Here, I'm going to provide some metadata about the video, provided by the always excellent **FFMPEG**, a critical tool in the belt of anyone working with *any* kind of video or audio. VID_20220127_111558711.mp4, was recorded using the default Android Camera App, has the following attributes:
 - Duration: 19.53 Seconds
 - Codec: H264 (High), AAC (LC)
 - Resolution: 1920x1080
 - Framerate: 30fps
 - Pixel Format: YUVJ420P
 - Sample Format: Float
 - Sample Rate: 48000hz

 Now, go ahread and write down your guess, maybe show a little work. Think about the assumptions you're making, and write them down. I'm going to include the full filesize in a markdown comment, and don't you dare peek until you've tried to work it out. 
 <!--42,280,692 Bytes, or, 40.3MB.-->
## But why is that relevant?
 The particular platform that this conversation was taking place on has a reasonable upload limit for all files shared to it. And the camera, by default, took a file that was significantly larger than that limit. At the time, the thought that I'd had was 
 >"*Ah well, I'm going to have to go home, and use FFMPEG to compress it down, and by then the conversation will be over.*"
<!--I am aware that there are web-based compressors available. It's the principle of the thing!-->
 And then I'd had another.
 >"*But FFMPEG is an open source library, so there's no reason I couldn't make a simple interface for it, and put it on my own phone.*"

# FFMPEG-A
### An (attempted) Android Wrapper for FFMPEG
### [Video Demonstration](https://www.youtube.com/watch?v=bNkymx6QDQc)

 This project was written and created using Android Studio, and thus that is the reccommended IDE for viewing the contents herein.
## app/src/main/layout
 Contained herein are XML Layout files for the planned layouts. Main is the container for all fragments that would be implemented in the full release, Simple is fully featured and implemented, the others, not at all. Android wants Constraint Layouts, or, layouts primarily created relationally from other elements within the same layout. This allows for layouts that inflate themselves to fill variable screen areas. The dominant challenge in designing these is the same in all UX design, creating intuitive interfaces that display relevant information cleanly and clearly to the end user. [A detailed screenshot of the main interface is included here]().
## app/src/main/menu
 Contained herein is the single Menu asset I created. It contains 5 options, which map to the 5 available presets. For keyboard users, while the menu is open, Keys 1-5 map to the options.
## app/src/main/cpp
 Contained herein are C++ files. This is the dominant Hard Execution of the application. Android relies on the Java Native Interface to allow managed execution of C Code. More on that later.
### hello.cpp
 Returns `Hello, World!`, as a Java String. Created to begin exploration of the JNI.
### ffmpeginterface.cpp
 Manages calls between managed Java Code and the varying libraries that make up FFMPEG. Defines several callbacks necessary for smooth interfacing. Function names given here are unmangled, more on that in the Java section.
#### nativeInit()
 Gathers a small amount of necessary information from the Java Environment for the sake of the JNI during execution.
#### get_Preset(int id)
 Returns an FFMPEG OutputFormat selected via an integer id, by use of mime types. 0 maps to Webm, 1 maps to MP4, and 2 maps to AVI.
#### android_av_log(void \*avcl, int level, const char\* format, va_list args)
 Logging callback that forwards all messages written by av_log to android's logcat library.
#### jiseek(void \*opaque, int64_t offset, int whence)
 Seeks input stream by amount given in offset, from position given in whence. Only callable if inputstream is seekable, as reported by java. Returns the amount of bytes skipped in the process, may be higher than offset.
#### jiread(void \*opaque, uint8_t \*buf, int buf_size)
 Reads information from a Java managed InputStream, stores at the buffer pointer given by *buf. Returns the amount of information read, or AVERROR_EXIT on IOError.
#### jiwrite(void \*opaque, uint8_t \*buf, int buf_size)
 Writes information from the pointer at *buf, to a Java managed OutputStream. Returns the amount of information written, or AVERROR_EXIT on IOError.
#### probeVid(InputStream IOStream)
 Probes a video using the FFMPEG libraries. Returns a VidMeta Object (Java POCO) which contains the Duration of the video in milliseconds, the Width in pixels, and the Height in pixels. If an AVERROR of any kind occurs, the number is stored in the Duration field of the return. For Debug builds, All metadata about the file is logged to the attached logcat process.
#### transcode(InputStream InputStream, OutputStream OutputStream, int type, int width, int height, int bitrate, bool audio) **Not Working**
 Transcodes a video file, accessible via InputStream, to type Type (conversion described in get_Preset), to width Width, and height Height, using bitrate Bitrate. If Audio is set to false, audio is not processed at all. This process does not preserve any metadata from the original video, meaning Location, Recording Device, Time Stamps, are not included in the new file. This is entirely intentional. In the modern age of digital footprinting and tracking, I intend to frustrate that process by whatever means I have, even as small as this. Returns true if the process went off perfectly, false otherwise.

 You may notice that this function does not actually, well, function. In order for FFMPEG to encode full frames of video, it needs to find a valid video device and driver, which requires direct access to (in this case) `/dev` on the device it's running on. For most Android installs, SELinux is configured to prevent this. More on this in the Android section. Without direct root permissions, this app cannot function. A debuggable .apk is available, try it yourself if you have a rooted device and can give it permission. I promise I'm not doing anything to any files you don't explicitly hand me. I actually can't, but I'll get to that when I talk about what Android *is*, and why that stands in the way of this project.
<!--Enterprise code filestructure. orz-->
 ## app/src/main/java/com/fantastico_softworks/ffmpeg_a
  Contained herein are Kotlin Script Files. These determine how the app itself launches, defines 2 contracts to receive files from and store files to external storage, and a separate object designed to run off the main thread at all times.
### MainActivity.kt
 This is the file that the app launches into. This inflates the Main View, and if not launching into a saved instance from being placed in the background, the Simple Fragment into the container within the Main View. This activity also handles the ChooseFile and SaveVideo contracts, updating the viewmodel accordingly.
### ChooseFile.kt, SaveVideo.kt
 These files represent Documents Contracts. In this case, they both create Intents that the Android OS can respond to be allowing selection of an external app that handles that given Intent, in this case, Opening a Video File, and Saving a Video File. Video files can only be saved to the Downloads Folder, due to constraints created by Google. In both cases, the object returned is a MediaStore URI, that is to say, a reference to the file header that does not tell me where on the device (in terms of the File Tree) the video actually is.
### TranscodeActivity.kt
 This class holds references to the native functions probeVid and transcode, implemented in ffmpeginterface.cpp. It calls nativeInit on creation of the object. Make sure these are called off of the main UI thread, as these are the Intensive parts of the application. In this application, they are called to the IO threads, because, that's what the IO threads are for.

## app/src/main/java/com/fantastico_softworks/ffmpeg_a/ui/main
 Contained herein are Kotlin Script Files that describe the Simple UI Fragment, as well as the shared View Model. The View Model is a singleton that Android provides a certain degree of primacy to in it's implementation, as it's designed to hold shared data between all fragments of the app. Given that the Simple fragment is the only implemented one, this view model only interacts with that. But it is designed to be more granularly controlled.
### FragmentSimple.kt
 The vast majority of the execution of this file is in lambdas in onCreateView. In onCreateView, the functionality of all of the simple fragment's buttons are defined as Lambdas (anonymous functions defined inline with their immediate purpose), and this is one of the things Kotlin is the best at. Further details are included in the UI diagram.
### MainViewModel.kt
 This file is where the main calls to the Transcode activity are located, and where data gathered from those calls lives. This also holds the configurations for the output video set by the user, or in the current case, by the preset resolver. Any other data the user might need would also live here for the other interfaces.

 There are three important build files I need to highlight, two in the app/ folder and one just above it.
### build.gradle
 Android apps use the Gradle build system to deliver the main codebase for simple apps. It is almost entirely opaque, and by default uses a syntax calling itself Groovy. The specific modifications I've had to make to this file are to include the ffmpeg libraries, which are compiled separately in my instance because, I'm still here on Windows 7 where bash isn't really natively happening. Also, compiling it separately makes the build process for re-building the app *significantly* faster which helps testing a lot. I'm not distributing binaries via github, it's somewhat poor form but more importantly, avcodec compiles to just over 100 MB, which is the Actual Limit for files on github.com
### CMakeLists.txt
 Common file to the CMake build system. In this case, imports the external ffmpeg libraries and links them against ffmpeginterface.so (the shared, compiled version of ffmpeginterface.cpp).
### ffbuild.sh
 FFMPEG uses a Bash Configure script to configure itself before being built using Make. This script downloads the latest source for ffmpeg, and compiles it for all 4 of Android's supported ABIs, configured in a specific way. Programs are disabled as, we can't actually run those on Android, Network and Devices disabled because that's beyond the scope of the project, and HWAccelerators disabled because most to all of those require third party libraries that may not be available on Android. Several specific protocols are also disabled due to their use of FILE pointers or stdio, more on that in the Android section. This is a modified form of [Don Turner's Build Script](https://gist.github.com/dturner/11fe5c8e420c2a73e15537274aafbd3a#file-ffmpeg-build-sh), who reportedly works for Google's London Branch. This is deeply funny to me, but not the reason I chose this path.

## ...but it doesn't work?
 It doesn't work. At the most, it doesn't perform its stated purpose. As I'd mentioned in the description of the transcode function, SELinux on Android is configured to deny access to `/dev`, more specifically, `/` which causes opendir() to fail entirely, and set errno to EACCES. To explain what all I mean by that, and why certain things had to be implemented in certain ways, we need to talk about
<!--Below here is opinion and commentary, but also, a detailed explanation of why things are they way they are.-->
# Android
 Android is a Low Trust platform.

 Android is Linux, but with stdio stripped out (no bash console in or out), and a heavy layer of SELinux locking down direct access to system resources. And, frankly, given the track record of Google, the history of the kinds of companies that like to develop for the platform (Facebook, Twitter), the kind of thing the platform gets used for, and its users, this makes perfect sense! Unrestricted access to the entire filesystem of an Android system would give an attacker very, *very* simple access to Personal information, Stored Credit Card numbers, all kinds of sensitive documents, et ecetera.

 But I can't help but feel that, blanket denial to the root directory, and thus, all hardware devices, *can't* be the best solution to the problem. And there are no permissions you can request from within Android since API 5 to gain that. But I shouldn't *need* to be a superuser to access the video hardware. The primary concern for SELinux should be protecting the end user, their device, and their personal information from Malicious Parties (not *just* Malicious *Third* Parties, Google), while still allowing them to use their device to it's fullest extent. Informed Users, allowing Honest Developers (rare as they are) to improve their day with simple services! Using code written in a sensible language, with sensible assumptions made! Because Android isn't just heavily secured linux. Android is heavily secured linux where Java is a First Class Citizen.

# Java, and the Java Native Interface
 Java is a land of contrasts. It's a pure object-oriented language, where basic types are Objects with entire backing libraries. It's a garbage collected language that has, possibly the worst garbage collection on the market. It's the language for Enterprise code, and for embedded systems, but only the ones designed to implement it. There's a major Open implementation, but also a major Corporate implementation, licensed and controlled by Oracle. Google had to win a lawsuit to keep it as a first class citizen on their platform. And they did! But all this is besides the point, as, for this project, I have managed to write 0 Java Code. <!--The dream.--> Technically. All of the code I've written for Java has been done in Kotlin. The simplest way to describe Kotlin, is by Metaphor.
### Kotlin is to Java as Python is to C.
 The relationship that Python and C have is near perfect interoperability. C, with python libraries, can call python libraries freely, and send information through to a python environment. Python is built on top of C libraries, can call them freely, and send information through to a C environment. Kotlin and Java have an equivalent relationship, being able to pass information between one another and call upon one another near perfectly. Writing code in the Higher Level, More Abstract languages (Kotlin and Python) is easier to do, and sacrifices nothing from writing your code well in the Lower Level languages. The higher level framework handles the busywork, and smooths the process of using code written in lower level languages.

 But why talk about this now? I bring this up as a point of contrast to the relationship between C and Java. Java has interoperability with C as defined by the Java Native Interface. Except, instead of the prior relationships where the higher level language does the difficult work, the Java Native Interface is built around requiring all of the work to be done in the C environment. [The full specification for the Java Native Interface can be read here.](https://docs.oracle.com/javase/7/docs/technotes/guides/jni/spec/jniTOC.html) Observant readers will note that this feature was implemented as of Java 7 (Standard Edition) and has not been touched since. Functions that are exposed to the java environment have mangled (their words, not mine) names, to match an exact function definition within the stated class in Java. Transcode, described above, has the actual name within C++ of `Java_com_fantastico_1softworks_ffmpeg_1a_TranscodeActivity_transcode`. <!--The _1 here expands to a full underscore.-->Without an explicit declaration within Java of the C functions that are available, and C functions being declared in a very specific way that links them to that Java declaration, there is no inter-operation in C, from Java. The process of calling a public Java function from C is much simpler, and goes as follows.
 1. Find the java class from the java environment, using a string.
 2. Find the method ID from that java class object, using a Method ID string, and a function signature string.
 3. Call the specific Call\[ReturnType\]Method function, on an object of the class referenced in 1, the method ID returned from 2, with arguments mapping to the function signature referenced in 2.
 4. Check for exceptions, handling if possible. Clear any exceptions found if execution must continue.

 Setting and getting fields of java objects follows the same pattern, but with finding Field IDs instead. All of these things exist on the JavaENV function table, a pointer to which is the first argument of all functions exposed via the Java Native Interface. And a simple wrapper function that deals in native strings could easily handle this, getting handles to Java IDs exactly as needed. Unfortunately, passing information between Java and C code is considered expensive on many fronts, and should be done as little as possible. Primitives have equivalent types, with JTypes taking as much width as possible. to serve their purpose. And this makes enough sense, Java primitives expect to cover a wide range of use cases, rather than expecting developers to be stingy with memory. But then, memory management is a classic, widely known failing of Java in runtime. Java leaks memory after any considerable length of running, and during garbage collection locks the entire thread until completion. Garbage collection is done via runtime evaluation of the heap, which is more expensive the more complex an application is. All of this is to say, I'd rather keep as much code as possible in C, calling to the Java Runtime strictly as neccesary, doing my own management, because, even as a journeyman, I'm better at it than trusting Java to do it. To me, Good Software is software that does it's job quickly, using few resources to do so. Concerns that are more salient on a platform that is classically run on shoestring hardware to begin with.

 I'm going to take a final moment to highlight Gradle as, an odd duck in terms of a build system. It's used here for downloading entire external libraries from webservers for internal use of code, and a lot of it's actual use is opaque to the enduser. Google does a lot to keep their secret sauce secret, and this is a key tool for them in making that happen. 

# Finally, the nature of Documentation, or, why this took so long to make.
 Android's app developer documentation is bad. Opening pages to sections are loose stories about how to do things, often topped with a link to a newer library that handles the exact same thing but in a slightly different way. It's not explained how, exactly to obtain the feature in question, whether it's an androidx plugin that has to be included in the dependancies of your gradle build file, or a feature that needs to be enabled in the buildFeatures section of your gradle file. The class specifications are entirely available via Android Studio. There are several options that are kept out of public documentation. And the website for the documentation itself is the strangest way of providing this I've ever seen. Oftentimes, on reloading the tab, Opera will crash the tab temporarily, citing ERR_TOO_MANY_REDIRECTS. From what I can guess, Google is tracking and analyzing the usage of their public documentation, for... Think critically for a second. What *would* you do with that information? Knowing exactly what path people take through your documentation, what would you actually do with that? When I was actively developing my Unity applications<!--Hex Maker Tactics, UnnamedMobileTwinStickShooterExperiment they're on my github go look at it-->, they sent me a 5 minute survey about their documentation. I imagine a general aggregation of that to be much, much easier to deal with than what Google is trying to do. But hey, I'm not the multi-billion dollar global corporation with hands in every industry.

# Closing Thoughts
 The first time I wrote anything for Android, it was a toy made in Unity. A simple 10 day app designed to test whether a certain method of control would feel good or not in the hands of players. It does. Engines and frameworks, when well built and well documented, make development fun and simple, regardless of the target. But, these things often limit, or at least direct development, towards a specific minute purpose. Through Code, all things are possible, but that's not the point here. I'd wanted to make this tool accessible to the Average Android User, in the hopes of spreading true ownership of personal data. Of knowing what a video really *is*. But, Android security standards mean that this is gated for Power Users only, as usual. Such is life. I intend to come back to this app at some point in the future, hopefully when the standards for who owns their devices are properly cleared up. Until then, naught can be done. 