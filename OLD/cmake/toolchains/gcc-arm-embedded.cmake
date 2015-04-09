#######################################
# Toolchain setup
#######################################

include(CMakeForceCompiler)


CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(arm-none-eabi-g++ GNU)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_CROSSCOMPILING 1)

# where is the target environment 
SET(CMAKE_FIND_ROOT_PATH  /opt/toolchains/gcc-arm-none-eabi-4_8-2014q3/)

MESSAGE(STATUS "Cross-compiling using gcc-arm-embedded toolchain")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

