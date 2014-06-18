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

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

IF(D7AOSS_HAL_DRIVER STREQUAL "msp430")
	SET(MCU_FLAG  "-mmcu=msp430f5172")
#	SET(MCU_FLAG  "-mmcu=msp430f5438")
ELSEIF(D7AOSS_HAL_DRIVER STREQUAL "cc430")
	SET(MCU_FLAG "-mmcu=cc430f6137")
ENDIF()

#SET(CMAKE_C_FLAGS "-W -mmcu=cc430f6137 -fdata-sections -ffunction-sections -Os -O2")
SET(CMAKE_C_FLAGS "-W ${MCU_FLAG} -fdata-sections -ffunction-sections -Ofast")
SET(CMAKE_SHARED_LINKER_FLAGS "${MCU_FLAG} -Wl,--gc-sections -Wl,--print-gc-sections")
SET(CMAKE_EXE_LINKER_FLAGS "${MCU_FLAG} -Wl,--gc-sections")
