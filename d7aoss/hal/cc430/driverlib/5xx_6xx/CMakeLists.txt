SET(CMAKE_INCLUDE_CURRENT_DIR ON)

ADD_LIBRARY( DRV
    gpio.c
    pmm.c
    rtc.c
    timera.c
    timerb.c
    timer.c
    timerd.c
    tlv.c
    uart_baudrate.c
    uart.c
    wdt.c
    ucs.c
    )

