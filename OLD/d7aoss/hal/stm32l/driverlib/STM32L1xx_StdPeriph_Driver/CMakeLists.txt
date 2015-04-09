ADD_DEFINITIONS(-DUSE_STDPERIPH_DRIVER)
INCLUDE_DIRECTORIES(inc)

SET(HEADERS
    inc/stm32l1xx_crc.h
    inc/stm32l1xx_rcc.h
    inc/stm32l1xx_gpio.h
    inc/stm32l1xx_spi.h
    inc/stm32l1xx_syscfg.h
    inc/stm32l1xx_exti.h
    inc/stm32l1xx_rtc.h
    inc/stm32l1xx_pwr.h
    inc/misc.h
    inc/stm32l1xx_conf.h
)

ADD_LIBRARY( DRV OBJECT ${HEADERS}
    src/stm32l1xx_crc.c
    src/stm32l1xx_rcc.c
    src/stm32l1xx_gpio.c
    src/stm32l1xx_spi.c
    src/stm32l1xx_syscfg.c
    src/stm32l1xx_exti.c
    src/stm32l1xx_rtc.c
    src/stm32l1xx_pwr.c
    src/misc.c
)

