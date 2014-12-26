SET(CMAKE_INCLUDE_CURRENT_DIR ON)

IF(D7AOSS_PHY_RADIO_DRIVER STREQUAL "cc430")
	SET(RADIOSRCS 
			cc430/cc430_interrupts.c
			cc430/cc430_phy.c
			cc430/cc430_registers.h
			cc430/cc430_settings.c
			cc430/rf1a.c
	)
ENDIF()

IF(D7AOSS_PHY_RADIO_DRIVER STREQUAL "cc1101")
    INCLUDE_DIRECTORIES(cc1101/)
    INCLUDE_DIRECTORIES(../framework/)
    INCLUDE_DIRECTORIES(../hal/)
        SET(RADIOSRCS
                        cc1101/cc1101_core.c
                        cc1101/cc1101_interrupts.c
                        cc1101/cc1101_phy.c
                        cc1101/cc1101_settings.c
        )
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
    fec.c
    ${RADIOSRCS}
)
