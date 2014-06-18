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

IF(D7AOSS_HAL_DRIVER STREQUAL "cc430")
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

IF(D7AOSS_HAL_DRIVER STREQUAL "stm32l")
    INCLUDE_DIRECTORIES(stm32l/)
    INCLUDE_DIRECTORIES(stm32l/driverlib/STM32L1xx_StdPeriph_Driver/inc)
    INCLUDE_DIRECTORIES(stm32l/driverlib/CMSIS/Device/ST/STM32L1xx/Include)
    INCLUDE_DIRECTORIES(stm32l/driverlib/CMSIS/Include)
    INCLUDE_DIRECTORIES(stm32l/driverlib/STM32_USB-FS-Device_Driver/inc)
    INCLUDE_DIRECTORIES(stm32l/usbserial)
    ADD_SUBDIRECTORY(stm32l/driverlib)
    SET(HAL_SRCS
                        stm32l/interrupts.c
                        stm32l/rotating_buffer.h
                        stm32l/rotating_buffer.c
                        stm32l/stm32l_crc.c
                        stm32l/stm32l_system.c
                        stm32l/usbserial/hw_config.c
                        stm32l/usbserial/usb_prop.c
                        stm32l/usbserial/usb_desc.c
                        stm32l/usbserial/usb_pwr.c
                        stm32l/usbserial/usb_istr.c
                        stm32l/usbserial/usb_endp.c
                        stm32l/driverlib/STM32_USB-FS-Device_Driver/src/usb_init.c
                        stm32l/driverlib/STM32_USB-FS-Device_Driver/src/usb_mem.c
                        stm32l/driverlib/STM32_USB-FS-Device_Driver/src/usb_core.c
                        stm32l/driverlib/STM32_USB-FS-Device_Driver/src/usb_regs.c
                        stm32l/driverlib/STM32_USB-FS-Device_Driver/src/usb_sil.c
                        stm32l/driverlib/STM32_USB-FS-Device_Driver/src/usb_int.c
                        #etc
                        stm32l/radio_spi_hw.c
                        stm32l/radio_spi_settings.h
                        stm32l/stm32l_led.c
                        stm32l/stm32l_timer.c
                        stm32l/systick.c
                        stm32l/stm32l_button.c
                        stm32l/stm32l_rtc.c
                        stm32l/stm32l_virtual_uart.c
                        stm32l/system/debug_hardfault.c
                        stm32l/system/startup.c
                        stm32l/system/syscall.c
                        stm32l/system/STM32L1xx_MD_Handlers.h
                        stm32l/board_config/${D7AOSS_HAL_STM32L_BOARD}/system_stm32l1xx.c
        )
ENDIF()

IF(D7AOSS_HAL_DRIVER STREQUAL "msp430")
        SET(HAL_SRCS
                        msp430/msp430_button.c
                        msp430/msp430_addresses.h
                        msp430/msp430_crc.c
                        msp430/msp430_leds.c
                        msp430/msp430_rtc.c
                        msp430/msp430_system.c
                        msp430/msp430_uart.c
                        msp430/msp430_timer.c
                        msp430/msp430_spi.c
        )

        ADD_SUBDIRECTORY(msp430/driverlib)
ENDIF()

IF(D7AOSS_HAL_DRIVER STREQUAL "stub")
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

IF(D7AOSS_HAL_DRIVER STREQUAL "sim")
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
SET_TARGET_PROPERTIES(HAL PROPERTIES LINKER_LANGUAGE C)

IF(D7AOSS_HAL_DRIVER STREQUAL "cc430")
    TARGET_LINK_LIBRARIES(HAL DRV)
ELSEIF(D7AOSS_HAL_DRIVER STREQUAL "stm32l")
    TARGET_LINK_LIBRARIES(HAL DRV)
ELSEIF(D7AOSS_HAL_DRIVER STREQUAL "msp430")
    TARGET_LINK_LIBRARIES(HAL DRV)
ELSEIF(D7AOSS_HAL_DRIVER STREQUAL "stub")
    TARGET_LINK_LIBRARIES(HAL rt)
ELSE()
    TARGET_LINK_LIBRARIES(HAL)
ENDIF()

