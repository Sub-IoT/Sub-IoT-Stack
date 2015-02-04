IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    #note: they're two different params. This is on purpose
    ADD_GLOBAL_DEFINITIONS("-DDEBUG_EFM=1" "-DDEBUG=1")
#    GLOBAL_ADD_DEFINITIONS("-DDEBUG_EFM=1")
#    GLOBAL_ADD_DEFINITIONS("-DDEBUG=1")
ENDIF()


#Add the linker script to use to the linker flags
INSERT_LINKER_FLAGS(BEFORE "-T${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/device/linker/efm32gg.ld")

ENABLE_LANGUAGE(ASM)
SET_PROPERTY(SOURCE efm32gg/CMSIS/device/src/startup_gcc_efm32gg.s PROPERTY LANGUAGE ASM)
INCLUDE_DIRECTORIES(CMSIS/include
		    CMSIS/device/include
		    emlib/inc
		    kits/common/drivers
		    )
#An object library with name '${CHIP_LIBRARY_NAME}' MUST be generated
ADD_LIBRARY (${CHIP_LIBRARY_NAME} OBJECT
                    CMSIS/device/src/system_efm32gg.c
                    CMSIS/device/src/startup_gcc_efm32gg.s
                    emlib/src/em_assert.c
                    emlib/src/em_system.c
                    emlib/src/em_cmu.c
                    emlib/src/em_emu.c
                    emlib/src/em_gpio.c
                    emlib/src/em_usart.c
                    emlib/src/em_rtc.c
                    emlib/src/em_dma.c
                    emlib/src/em_int.c
                    kits/common/drivers/dmactrl.c
                    efm32gg_system.c
                    efm32gg_uart.c
                    efm32gg_leds.c
                    efm32gg_int.c
                    )

#    SET(HAL_SRCS
#                        efm32gg_leds.c
#                        efm32gg_system.c
#                        efm32gg_uart.c
#                        efm32gg_timer.c
#                        efm32gg_spi.c
#                        radio_hw.c
#                        crc_software.c
#        )
