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

# where is the target environment 
LIST(APPEND CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_DIR}")

MESSAGE(STATUS "Cross-compiling using gcc-arm-embedded toolchain")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#Override link command to allow additional flags to be inserted
SET_GLOBAL(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
SET_GLOBAL(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
