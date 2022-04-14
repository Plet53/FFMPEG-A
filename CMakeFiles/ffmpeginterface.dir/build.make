# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake.exe

# The command to remove a file.
RM = /usr/bin/cmake.exe -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /d/ffmpeg-a/app

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /d/ffmpeg-a

# Include any dependencies generated for this target.
include CMakeFiles/ffmpeginterface.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/ffmpeginterface.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/ffmpeginterface.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ffmpeginterface.dir/flags.make

CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.o: CMakeFiles/ffmpeginterface.dir/flags.make
CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.o: app/src/main/cpp/ffmpeginterface.cpp
CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.o: CMakeFiles/ffmpeginterface.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/d/ffmpeg-a/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.o"
	/usr/bin/c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.o -MF CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.o.d -o CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.o -c /d/ffmpeg-a/app/src/main/cpp/ffmpeginterface.cpp

CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.i"
	/usr/bin/c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /d/ffmpeg-a/app/src/main/cpp/ffmpeginterface.cpp > CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.i

CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.s"
	/usr/bin/c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /d/ffmpeg-a/app/src/main/cpp/ffmpeginterface.cpp -o CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.s

# Object files for target ffmpeginterface
ffmpeginterface_OBJECTS = \
"CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.o"

# External object files for target ffmpeginterface
ffmpeginterface_EXTERNAL_OBJECTS =

msys-ffmpeginterface.dll: CMakeFiles/ffmpeginterface.dir/src/main/cpp/ffmpeginterface.cpp.o
msys-ffmpeginterface.dll: CMakeFiles/ffmpeginterface.dir/build.make
msys-ffmpeginterface.dll: avfilter-NOTFOUND
msys-ffmpeginterface.dll: avformat-NOTFOUND
msys-ffmpeginterface.dll: postproc-NOTFOUND
msys-ffmpeginterface.dll: swresample-NOTFOUND
msys-ffmpeginterface.dll: swscale-NOTFOUND
msys-ffmpeginterface.dll: avcodec-NOTFOUND
msys-ffmpeginterface.dll: avutil-NOTFOUND
msys-ffmpeginterface.dll: CMakeFiles/ffmpeginterface.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/d/ffmpeg-a/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library msys-ffmpeginterface.dll"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ffmpeginterface.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ffmpeginterface.dir/build: msys-ffmpeginterface.dll
.PHONY : CMakeFiles/ffmpeginterface.dir/build

CMakeFiles/ffmpeginterface.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ffmpeginterface.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ffmpeginterface.dir/clean

CMakeFiles/ffmpeginterface.dir/depend:
	cd /d/ffmpeg-a && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /d/ffmpeg-a/app /d/ffmpeg-a/app /d/ffmpeg-a /d/ffmpeg-a /d/ffmpeg-a/CMakeFiles/ffmpeginterface.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ffmpeginterface.dir/depend

