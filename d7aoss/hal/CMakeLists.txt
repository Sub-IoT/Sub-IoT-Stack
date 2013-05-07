SET(CMAKE_INCLUDE_CURRENT_DIR ON)

IF(HalDriver STREQUAL "CC430")
        SET(HAL_SRCS
                        cc430/cc430_button.c
                        cc430/cc430_addresses.h
                        cc430/cc430_crc.c
                        cc430/cc430_leds.c
                        cc430/cc430_rtc.c
                        cc430/cc430_system.c
                        cc430/cc430_uart.c
                        cc430/cc430_timer.c
        )

        ADD_SUBDIRECTORY(cc430/driverlib)
ENDIF()

ADD_LIBRARY(HAL ${HAL_SRCS})
TARGET_LINK_LIBRARIES(HAL D7AOSS)

