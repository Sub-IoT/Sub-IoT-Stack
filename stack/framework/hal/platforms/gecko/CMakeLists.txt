#GET_PROPERTY(__includes DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#MESSAGE(AUTHOR_WARNING "Includes@${CMAKE_CURRENT_SOURCE_DIR}: ${__includes}")


#Check that the correct toolchain for the platform is being used
REQUIRE_TOOLCHAIN(gcc-arm-embedded)

#Define platform specific options
PLATFORM_PARAM(${PLATFORM_PREFIX}_MCU "EFM32GG990F1024" STRING "The MCU of the Gecko board")
PLATFORM_PARAM(${PLATFORM_PREFIX}_RADIO "none" STRING "The radio chip connected to the Gecko board")
#Restrict the number of possible options for the radio option in the usual manner...
SET_PROPERTY(CACHE ${PLATFORM_PREFIX}_RADIO PROPERTY STRINGS "cc1101;none")

#Add a 'global' definition for the specific MCU
GLOBAL_ADD_DEFINITIONS("-D${${PLATFORM_PREFIX}_MCU}")

#Set platform specific compile & linker options
GLOBAL_ADD_COMPILE_OPTIONS("-gdwarf-2" "-mcpu=cortex-m3" "-mthumb" "-fmessage-length=0" "-mno-sched-prolog" "-fno-builtin" "-ffunction-sections" "-fdata-sections")

#Add platform specific linker flags
#BEFORE flags are added right before the object list
#AFTER  flags are added at the end of the link command
INSERT_LINKER_FLAGS(BEFORE "--specs=nano.specs" "-Wl,-gc-sections")
INSERT_LINKER_FLAGS(AFTER "-Wl,--start-group" "-lgcc" "-lc" "-lnosys" "-Wl,--end-group")

#Include the sources for the efm32gg chip
ADD_CHIP("efm32gg")

#Include the sources for the radio chip, if needed
IF(NOT ("${${PLATFORM_PREFIX}_RADIO}" STREQUAL "none"))
    ADD_CHIP(${PLATFORM_PREFIX}_RADIO)
ENDIF()

#Define the 'platform library'. Every platform must define a 'PLATFORM' object library
ADD_LIBRARY(PLATFORM OBJECT gecko_main.c)
