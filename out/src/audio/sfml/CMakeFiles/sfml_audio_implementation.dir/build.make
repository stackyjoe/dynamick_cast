# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /usr/home/joe/coding/DynamickCast/dynamick_cast

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /usr/home/joe/coding/DynamickCast/dynamick_cast/out

# Include any dependencies generated for this target.
include src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/compiler_depend.make

# Include the progress variables for this target.
include src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/progress.make

# Include the compile flags for this target's objects.
include src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/flags.make

src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.o: src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/flags.make
src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.o: ../src/audio/sfml/sfml_wrapper.cpp
src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.o: src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/usr/home/joe/coding/DynamickCast/dynamick_cast/out/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.o"
	cd /usr/home/joe/coding/DynamickCast/dynamick_cast/out/src/audio/sfml && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.o -MF CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.o.d -o CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.o -c /usr/home/joe/coding/DynamickCast/dynamick_cast/src/audio/sfml/sfml_wrapper.cpp

src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.i"
	cd /usr/home/joe/coding/DynamickCast/dynamick_cast/out/src/audio/sfml && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /usr/home/joe/coding/DynamickCast/dynamick_cast/src/audio/sfml/sfml_wrapper.cpp > CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.i

src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.s"
	cd /usr/home/joe/coding/DynamickCast/dynamick_cast/out/src/audio/sfml && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /usr/home/joe/coding/DynamickCast/dynamick_cast/src/audio/sfml/sfml_wrapper.cpp -o CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.s

sfml_audio_implementation: src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/sfml_wrapper.cpp.o
sfml_audio_implementation: src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/build.make
.PHONY : sfml_audio_implementation

# Rule to build all files generated by this target.
src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/build: sfml_audio_implementation
.PHONY : src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/build

src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/clean:
	cd /usr/home/joe/coding/DynamickCast/dynamick_cast/out/src/audio/sfml && $(CMAKE_COMMAND) -P CMakeFiles/sfml_audio_implementation.dir/cmake_clean.cmake
.PHONY : src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/clean

src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/depend:
	cd /usr/home/joe/coding/DynamickCast/dynamick_cast/out && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /usr/home/joe/coding/DynamickCast/dynamick_cast /usr/home/joe/coding/DynamickCast/dynamick_cast/src/audio/sfml /usr/home/joe/coding/DynamickCast/dynamick_cast/out /usr/home/joe/coding/DynamickCast/dynamick_cast/out/src/audio/sfml /usr/home/joe/coding/DynamickCast/dynamick_cast/out/src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/audio/sfml/CMakeFiles/sfml_audio_implementation.dir/depend
