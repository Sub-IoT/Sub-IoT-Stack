SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(HAL_API
    button.h
    crc.h
    leds.h
    rtc.h
    system.h
    uart.h
    timer.h
)

IF(D7AOSS_HAL_DRIVER STREQUAL "CC430")
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

IF(D7AOSS_HAL_DRIVER STREQUAL "MSP430")
        SET(HAL_SRCS
                        msp430/msp430_button.c
                        msp430/msp430_addresses.h
                        msp430/msp430_crc.c
                        msp430/msp430_leds.c
                        msp430/msp430_rtc.c
                        msp430/msp430_system.c
                        msp430/msp430_uart.c
                        msp430/msp430_timer.c
        )

        ADD_SUBDIRECTORY(msp430/driverlib)
ENDIF()

IF(D7AOSS_HAL_DRIVER STREQUAL "STUB")
        SET(HAL_SRCS
                        stub/stub_button.c
                        stub/stub_crc.c
                        stub/stub_leds.c
                        stub/stub_rtc.c
                        stub/stub_system.c
                        stub/stub_uart.c
                        stub/stub_timer.c
        )

ENDIF()

ADD_LIBRARY(HAL ${HAL_API} ${HAL_SRCS})

IF(D7AOSS_HAL_DRIVER STREQUAL "CC430")
    TARGET_LINK_LIBRARIES(HAL DRV)
ELSEIF(D7AOSS_HAL_DRIVER STREQUAL "MSP430")
    TARGET_LINK_LIBRARIES(HAL DRV)
ELSEIF(D7AOSS_HAL_DRIVER STREQUAL "STUB")
    TARGET_LINK_LIBRARIES(HAL rt)
ELSE()
    TARGET_LINK_LIBRARIES(HAL)
ENDIF()

