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

IF(D7AOSS_HAL_DRIVER STREQUAL "SIM")
        SET(HAL_SRCS
                        sim/sim_button.c
                        sim/sim_crc.c
                        sim/sim_leds.c
                        sim/sim_rtc.c
                        sim/sim_system.c
                        sim/sim_uart.c
                        sim/sim_timer.c
        )

ENDIF()

ADD_LIBRARY(HAL ${HAL_API} ${HAL_SRCS})

IF(D7AOSS_HAL_DRIVER STREQUAL "CC430")
    TARGET_LINK_LIBRARIES(HAL DRV)
ELSEIF(D7AOSS_HAL_DRIVER STREQUAL "STUB")
    TARGET_LINK_LIBRARIES(HAL rt)
ELSE()
    TARGET_LINK_LIBRARIES(HAL)
ENDIF()

