SET(CMAKE_INCLUDE_CURRENT_DIR ON)

IF(PhyRadioDriver STREQUAL "CC430")
	SET(RADIOSRCS 
			cc430/cc430_interrupts.c
			cc430/cc430_phy.c
			cc430/cc430_registers.h
			cc430/cc430_settings.c
			cc430/rf1a.c
	)
ENDIF()

ADD_LIBRARY(PHY
    phy.c
    fec.c
    ${RADIOSRCS}
)
TARGET_LINK_LIBRARIES(PHY)

