CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (D7AOSS C)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

#######################################
# Configuration
#######################################
SET(D7AOSS_PHY_RADIO_DRIVER "cc430" CACHE STRING "Radio chip driver used by PHY")
SET_PROPERTY(CACHE D7AOSS_PHY_RADIO_DRIVER PROPERTY STRINGS cc430 cc1101 stub sim)

SET(D7AOSS_HAL_DRIVER "cc430" CACHE STRING "HAL implementation")
SET_PROPERTY(CACHE D7AOSS_HAL_DRIVER PROPERTY STRINGS cc430 stm32l msp430 efm32gg stub sim)
IF(D7AOSS_HAL_DRIVER STREQUAL "efm32gg")
	SET(CMAKE_C_FLAGS
  		"${CMAKE_C_FLAGS} -gdwarf-2 -mcpu=cortex-m3 -mthumb -fmessage-length=0 -mno-sched-prolog -fno-builtin -ffunction-sections -fdata-sections"
	)
	SET(CMAKE_EXE_LINKER_FLAGS "--specs=nano.specs -Xlinker --gc-sections -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group" CACHE STRING "" FORCE)
	ADD_DEFINITIONS(-DEFM32GG990F1024 -DDEBUG_EFM=1 -DDEBUG=1) # TODO make configurable
ELSEIF(D7AOSS_HAL_DRIVER STREQUAL "stm32l")
	SET(CMAKE_C_FLAGS
  		"${CMAKE_C_FLAGS} -fno-common -Wstrict-prototypes -ffunction-sections -fdata-sections -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd -msoft-float" CACHE STRING "" FORCE
	)
	SET(CMAKE_EXE_LINKER_FLAGS "--specs=nano.specs -Wl,-gc-sections" CACHE STRING "" FORCE)
	ADD_DEFINITIONS("-DHSE_VALUE=8000000" "-DSTM32L1XX_MD")
ENDIF()	

SET(D7AOSS_HAL_STM32L_BOARD "stm32l152c-discovery" CACHE STRING "STM32L HAL board config")
SET_PROPERTY(CACHE D7AOSS_HAL_STM32L_BOARD PROPERTY STRINGS stm32l152c-discovery otheruse)

OPTION(D7AOSS_BUILD_TESTS BOOL "Build tests")

OPTION(D7AOSS_BUILD_EXAMPLES BOOL "Build examples")

#######################################
# Modules
#######################################
ADD_SUBDIRECTORY(d7aoss)

IF(D7AOSS_BUILD_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
ENDIF()

IF(D7AOSS_BUILD_TESTS)
    ADD_SUBDIRECTORY(tests)

ENDIF()

# TODO temp for testing
IF(D7AOSS_HAL_DRIVER STREQUAL "efm32gg")
	ADD_SUBDIRECTORY("test applications/efm32_hal")
ENDIF()
