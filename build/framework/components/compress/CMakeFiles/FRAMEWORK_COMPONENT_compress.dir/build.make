# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /media/sf_Virtualbox-shared/oss-7/stack

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /media/sf_Virtualbox-shared/oss-7/build

# Include any dependencies generated for this target.
include framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/depend.make

# Include the progress variables for this target.
include framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/progress.make

# Include the compile flags for this target's objects.
include framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/flags.make

framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj: framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/flags.make
framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj: /media/sf_Virtualbox-shared/oss-7/stack/framework/components/compress/compress.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/media/sf_Virtualbox-shared/oss-7/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj"
	cd /media/sf_Virtualbox-shared/oss-7/build/framework/components/compress && /home/liam/Documents/toolchain/gcc-arm-none-eabi-8-2018-q4-major/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj   -c /media/sf_Virtualbox-shared/oss-7/stack/framework/components/compress/compress.c

framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.i"
	cd /media/sf_Virtualbox-shared/oss-7/build/framework/components/compress && /home/liam/Documents/toolchain/gcc-arm-none-eabi-8-2018-q4-major/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /media/sf_Virtualbox-shared/oss-7/stack/framework/components/compress/compress.c > CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.i

framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.s"
	cd /media/sf_Virtualbox-shared/oss-7/build/framework/components/compress && /home/liam/Documents/toolchain/gcc-arm-none-eabi-8-2018-q4-major/bin/arm-none-eabi-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /media/sf_Virtualbox-shared/oss-7/stack/framework/components/compress/compress.c -o CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.s

framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj.requires:

.PHONY : framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj.requires

framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj.provides: framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj.requires
	$(MAKE) -f framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/build.make framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj.provides.build
.PHONY : framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj.provides

framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj.provides.build: framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj


FRAMEWORK_COMPONENT_compress: framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj
FRAMEWORK_COMPONENT_compress: framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/build.make

.PHONY : FRAMEWORK_COMPONENT_compress

# Rule to build all files generated by this target.
framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/build: FRAMEWORK_COMPONENT_compress

.PHONY : framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/build

framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/requires: framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/compress.c.obj.requires

.PHONY : framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/requires

framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/clean:
	cd /media/sf_Virtualbox-shared/oss-7/build/framework/components/compress && $(CMAKE_COMMAND) -P CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/cmake_clean.cmake
.PHONY : framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/clean

framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/depend:
	cd /media/sf_Virtualbox-shared/oss-7/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /media/sf_Virtualbox-shared/oss-7/stack /media/sf_Virtualbox-shared/oss-7/stack/framework/components/compress /media/sf_Virtualbox-shared/oss-7/build /media/sf_Virtualbox-shared/oss-7/build/framework/components/compress /media/sf_Virtualbox-shared/oss-7/build/framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : framework/components/compress/CMakeFiles/FRAMEWORK_COMPONENT_compress.dir/depend

