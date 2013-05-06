CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (D7AOSS)

#######################################
# Toolchain setup
#######################################
SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER msp430-gcc)
SET(CMAKE_AR msp430-ar)
SET(CMAKE_RANLIB msp430-ranlib)
# where is the target environment 
SET(CMAKE_FIND_ROOT_PATH  /usr/local/msp430/)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

#SET(CMAKE_C_FLAGS "-W -mmcu=cc430f6137 -fdata-sections -ffunction-sections -Os -O2")
SET(CMAKE_C_FLAGS "-W -mmcu=cc430f6137 -fdata-sections -ffunction-sections -Ofast")
SET(CMAKE_SHARED_LINKER_FLAGS "-mmcu=cc430f6137 -Wl,--gc-sections -Wl,--print-gc-sections")
SET(CMAKE_EXE_LINKER_FLAGS "-mmcu=cc430f6137 -Wl,--gc-sections")


#######################################
# Modules
#######################################
ADD_SUBDIRECTORY(d7aoss)
ADD_SUBDIRECTORY(examples)

#######################################
# Configuration
#######################################
SET(PhyRadioDriver "CC430" CACHE STRING "Radio chip driver used by PHY")
SET_PROPERTY(CACHE PhyRadioDriver PROPERTY STRINGS CC430 STUB)