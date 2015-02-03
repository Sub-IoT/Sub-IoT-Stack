SET(CMAKE_INCLUDE_CURRENT_DIR ON)

PLATFORM_PARAM(${PLATFORM_PREFIX}_MCU "EFM32GG990F1024" STRING "The MCU of the Gecko board")
PLATFORM_OPTION(${PLATFORM_PREFIX}_CC1101 "Enable support for the CC1101 radio chip" OFF)
PLATFORM_OPTION(${PLATFORM_PREFIX}_OVERRIDE_ATOMIC "Override the 'atomic' component" OFF)
PLATFORM_OPTION(${PLATFORM_PREFIX}_OVERRIDE_SCHEDULER "Override the 'scheduler' component" OFF)
PLATFORM_OPTION(${PLATFORM_PREFIX}_OVERRIDE_LOG "Override the 'log' component" OFF)

#TODO: some checking on the mcu values
GLOBAL_ADD_DEFINITIONS("-D${${PLATFORM_PREFIX}_MCU}")
# $<$<CONFIG:Debug>:-DDEBUG_EFM=1 -DDEBUG=1>) # TODO make configurable

#Disabled while we're just testing out the cmake system
#REQUIRE_TOOLCHAIN(gcc-arm-embedded)
ADD_CHIP("efm32gg")
IF(${PLATFORM_PREFIX}_ENABLE_CC1101)
    ADD_CHIP("cc1101")
ENDIF()
SET(PLATFORM_SOURCES gecko_main.c)

IF(${PLATFORM_PREFIX}_OVERRIDE_ATOMIC)
    OVERRIDE_COMPONENT(atomic overrides/atomic.c)
ENDIF()
IF(${PLATFORM_PREFIX}_OVERRIDE_SCHEDULER)
    OVERRIDE_COMPONENT(scheduler overrides/scheduler.c)
ENDIF()
IF(${PLATFORM_PREFIX}_OVERRIDE_LOG)
    OVERRIDE_COMPONENT(log overrides/log.c)
ENDIF()

#the platform MUST generate an 'Object' library with name 'PLATFORM'
ADD_LIBRARY(PLATFORM OBJECT ${PLATFORM_SOURCES})

