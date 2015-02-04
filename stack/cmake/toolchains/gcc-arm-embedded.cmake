#######################################
# Toolchain setup gcc-arm-embedded
#######################################

include(CMakeForceCompiler)

CMAKE_FORCE_C_COMPILER(arm-none-eabi-gcc GNU)
CMAKE_FORCE_CXX_COMPILER(arm-none-eabi-g++ GNU)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_CROSSCOMPILING 1)


SET(TOOLCHAIN_DIR "" CACHE PATH "The directory containing all the cross compilation tools. (Compilation will fail if this is not set correctly)")
LIST(APPEND CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_DIR}")

# where is the target environment 
#SET(CMAKE_FIND_ROOT_PATH  /opt/toolchains/gcc-arm-none-eabi-4_8-2014q3/)
#SET(CMAKE_FIND_ROOT_PATH  /Applications/SimplicityStudio_v2/developer/toolchains/gnu_arm/4.8_2013q4/)
#SET(CMAKE_OBJCOPY "${CMAKE_FIND_ROOT_PATH}/bin/arm-none-eabi-objcopy")
#SET(CMAKE_SIZE "${CMAKE_FIND_ROOT_PATH}/bin/arm-none-eabi-size")

MESSAGE(STATUS "Cross-compiling using gcc-arm-embedded toolchain")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#Override link command to allow additional flags to be inserted
SET(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" CACHE INTERNAL "")
SET(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" CACHE INTERNAL "")
