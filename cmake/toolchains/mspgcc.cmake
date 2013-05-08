#######################################
# Toolchain setup
#######################################
SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER msp430-gcc)
SET(CMAKE_LINKER msp430-gcc)
SET(CMAKE_AR msp430-ar)
SET(CMAKE_RANLIB msp430-ranlib)
# where is the target environment 
SET(CMAKE_FIND_ROOT_PATH  /opt/toolchains/mspgcc)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#SET(CMAKE_C_FLAGS "-W -mmcu=cc430f6137 -fdata-sections -ffunction-sections -Os -O2")
SET(CMAKE_C_FLAGS "-W -mmcu=cc430f6137 -fdata-sections -ffunction-sections -Ofast")
SET(CMAKE_SHARED_LINKER_FLAGS "-mmcu=cc430f6137 -Wl,--gc-sections -Wl,--print-gc-sections")
SET(CMAKE_EXE_LINKER_FLAGS "-mmcu=cc430f6137 -Wl,--gc-sections")
