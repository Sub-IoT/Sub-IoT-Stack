/*
 *  This program does a continuous TX on a specific channel
 *  Authors:
 *      glenn.ergeerts@artesis.be
 */


#include <string.h>
#include <stdint.h>

#include <d7aoss.h>

#include <phy/phy.h>

#include <hal/system.h>
#include <hal/leds.h>

#include "../../d7aoss/phy/cc1101/cc1101_constants.h"
#include "../../d7aoss/phy/cc1101/cc1101_phy.h"

uint8_t buffer[256];

void main()
{
	system_init(buffer, sizeof(buffer), buffer, sizeof(buffer));
	phy_init();

	int p = cc1101_interface_read_single_reg(PARTNUM);
	p = cc1101_interface_read_single_reg(VERSION);

    cc1101_interface_strobe(RF_STX);

	while(1);
}
