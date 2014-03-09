INCLUDE_DIRECTORIES(inc)

SET(HEADERS
        inc/stm32l1xx_crc.h
)

ADD_LIBRARY( DRV ${HEADERS}
    src/stm32l1xx_crc
    )

