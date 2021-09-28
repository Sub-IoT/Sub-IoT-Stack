#[[
Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.

This file is part of Sub-IoT.
See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    #note: they're two different params. This is on purpose
    EXPORT_GLOBAL_COMPILE_DEFINITIONS("-DDEBUG_EFM=1" "-DDEBUG=1")
ENDIF()

SET(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/device/linker/ezr32lg.ld" CACHE FILEPATH "")

SET(HAL_UART_USE_DMA_TX "TRUE" CACHE BOOL "Enable/Disable the use of DMA for UART TX" FORCE)
SET(HAL_SUPPORT_HW_AES "TRUE" CACHE BOOL "AES is supported by an hardware accelerated module")

IF(${PLATFORM_BUILD_BOOTLOADABLE_VERSION})
    SET(LINKER_SCRIPT_BOOTLOADABLE "${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/device/linker/ezr32lg_bootloader.ld" CACHE FILEPATH "")
ENDIF()

SET(LINKER_FLAGS "-Xlinker  -Map=.map" CACHE STRING "")

ENABLE_LANGUAGE(ASM)
#SET_PROPERTY(SOURCE ezr32lg/CMSIS/device/src/startup_gcc_ezr32lg.s PROPERTY LANGUAGE ASM)
INCLUDE_DIRECTORIES(CMSIS/include
		    CMSIS/device/include
		    emlib/inc
		    glib
		    #kits/common/drivers
		    #kits/common/bsp
		    usb/inc
            inc_platform
		    emdrv/spidrv/inc
		    emdrv/gpiointerrupt/inc/
		    emdrv/common/inc
		    emdrv/dmadrv/inc
		    kits/common/drivers
		    )

##Export the 'inc' directory globally		    
EXPORT_GLOBAL_INCLUDE_DIRECTORIES(	inc
		    						CMSIS/device/include
		    						CMSIS/include
									emlib/inc		
		    						emdrv/spidrv/inc
		    						emdrv/gpiointerrupt/inc/
		   							emdrv/common/inc
		    						emdrv/dmadrv/inc
									kits/common/drivers)

#Export the em libs to the platform directory ONLY
#Please note we include 'inc' here since for complicated cmake reasons directories exported 
#with 'GLOBAL_INCLUDE_DIRECTORIES'from chip directories are not exported to the platform directory
EXPORT_PLATFORM_INCLUDE_DIRECTORIES(CMSIS/include
		    CMSIS/device/include
		    emlib/inc
		    glib
		    #kits/common/drivers
		    #kits/common/bsp
		    inc_platform
		    inc
		    usb/inc
		    emdrv/spidrv/inc
		    emdrv/gpiointerrupt/inc/
		    emdrv/common/inc
		    emdrv/dmadrv/inc
			kits/common/drivers
		    )

LIST(APPEND CHIP_SRCS
  CMSIS/device/src/system_ezr32lg.c
  CMSIS/device/src/startup_ezr32lg.c
  emlib/src/em_assert.c
  emlib/src/em_adc.c
  emlib/src/em_aes.c
  emlib/src/em_system.c
  emlib/src/em_cmu.c
  emlib/src/em_emu.c
  emlib/src/em_gpio.c
  emlib/src/em_usart.c
  emlib/src/em_rtc.c
  emlib/src/em_dma.c
  emlib/src/em_int.c
  emlib/src/em_lcd.c
  emlib/src/em_timer.c
  emlib/src/em_i2c.c
  emlib/src/em_wdog.c
  #emlib/inc/em_wdog.h
  emlib/src/em_prs.c
  kits/common/drivers/dmactrl.c
#         kits/common/drivers/display.c
#         kits/common/drivers/textdisplay.c
        #kits/common/drivers/retargettextdisplay.c
#         kits/common/drivers/displayls013b7dh03.c
#         kits/common/drivers/displaypalemlib.c
#         kits/common/drivers/udelay.c
        #kits/common/drivers/cdc.c
#         kits/common/bsp/bsp_bcc.c
#         kits/common/bsp/bsp_stk.c
#         kits/common/bsp/bsp_trace.c
  ezr32lg_adc.c 
  ezr32lg_aes.c
  ezr32lg_mcu.c
  ezr32lg_uart.c
  ezr32lg_spi.c
  ezr32lg_atomic.c
  ezr32lg_timer.c
  ezr32lg_system.c                    
  ezr32lg_gpio.c                    
  ezr32lg_i2c.c 
  ezr32lg_watchdog.c
  emdrv/gpiointerrupt/src/gpiointerrupt.c
  emdrv/spidrv/src/spidrv.c       
  emdrv/dmadrv/src/dmadrv.c
  emdrv/dmadrv/inc/dmadrv.h
)

IF(${PLATFORM_USE_USB_CDC})
  LIST(APPEND CHIP_SRCS
  	usb/src/em_usbd.c
  	usb/src/em_usbdch9.c
  	usb/src/em_usbdep.c
  	usb/src/em_usbdint.c
  	#usb/src/em_usbh.c
  	usb/src/em_usbhal.c
  	#usb/src/em_usbhep.c
  	#usb/src/em_usbhint.c
  	usb/src/em_usbtimer.c
  )
ENDIF()

ADD_LIBRARY(${CHIP_LIBRARY_NAME} OBJECT ${CHIP_SRCS})
