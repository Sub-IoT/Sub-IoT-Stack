SET(CMAKE_INCLUDE_CURRENT_DIR ON)
INCLUDE_DIRECTORIES(${D7AOSS_SOURCE_DIR}/hal/platforms)

ADD_LIBRARY(HAL
    button.c
    leds.c
    rtc.c
    system.c
    uart.c
)
ADD_SUBDIRECTORY(driverlib)

