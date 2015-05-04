SET(CMAKE_INCLUDE_CURRENT_DIR ON)

IF(D7AOSS_PHY_RADIO_DRIVER STREQUAL "cc1101" OR D7AOSS_PHY_RADIO_DRIVER STREQUAL "cc430")
    INCLUDE_DIRECTORIES(cc1101/)
    INCLUDE_DIRECTORIES(../framework/)
    INCLUDE_DIRECTORIES(../hal/)
    SET(RADIOSRCS
        cc1101/cc1101_interface.c
        cc1101/cc1101_phy.c
        cc1101/cc1101_settings.c
    )

    IF(D7AOSS_PHY_RADIO_DRIVER STREQUAL "cc1101")
        LIST(APPEND RADIOSRCS cc1101/cc1101_interface_spi.c)
    ELSE()
        LIST(APPEND RADIOSRCS cc1101/cc1101_interface_cc430.c)
    ENDIF()
ENDIF()

IF(D7AOSS_PHY_RADIO_DRIVER STREQUAL "stub")
    SET(RADIOSRCS
        stub/stub_phy.c 
    )
ENDIF()

IF(D7AOSS_PHY_RADIO_DRIVER STREQUAL "sim")
    SET(RADIOSRCS
        sim/sim_phy.c
    )
ENDIF()

ADD_LIBRARY(PHY OBJECT
    phy.c
    # fec.c TODO not used for now
    ${RADIOSRCS}
)
