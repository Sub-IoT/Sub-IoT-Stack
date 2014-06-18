CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (D7AOSS C)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

#######################################
# Configuration
#######################################
SET(D7AOSS_PHY_RADIO_DRIVER "cc430" CACHE STRING "Radio chip driver used by PHY")
SET_PROPERTY(CACHE D7AOSS_PHY_RADIO_DRIVER PROPERTY STRINGS cc430 cc1101 stub sim)

SET(D7AOSS_HAL_DRIVER "cc430" CACHE STRING "HAL implementation")
SET_PROPERTY(CACHE D7AOSS_HAL_DRIVER PROPERTY STRINGS cc430 stm32l msp430 stub sim)

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


