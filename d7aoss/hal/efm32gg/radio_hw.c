
#include <stdbool.h>

#include "em_gpio.h"
#include "em_int.h"

#include "../../phy/cc1101/cc1101_core.h" // TODO refactor, don't depend on this here
#include "../../phy/cc1101/cc1101_phy.h" // TODO refactor, don't depend on this here
#include "radio_hw.h"

InterruptHandlerDescriptor interrupt_table[6] = {
		{.gdoSetting = 0x00, .edge = GDOEdgeRising, .handler = rx_data_isr},
		{.gdoSetting = 0x02, .edge = GDOEdgeFalling, .handler = tx_data_isr},
		{.gdoSetting = 0x05, .edge = GDOEdgeRising, .handler = end_of_packet_isr},
		{.gdoSetting = 0x06, .edge = GDOEdgeFalling, .handler = end_of_packet_isr},
		{.gdoSetting = 0x04, .edge = GDOEdgeRising, .handler = rx_fifo_overflow_isr},
		{.handler = 0},
};

void CC1101_ISR(GDOLine gdo)
{
	uint8_t gdo_setting = ReadSingleReg(gdo);
	uint8_t index = 0;
	InterruptHandlerDescriptor descriptor;
	do {
		descriptor = interrupt_table[index];
		if ((gdo_setting & 0x7f) == (descriptor.gdoSetting | descriptor.edge)) {
			descriptor.handler();
			break;
		}
		index++;
	}
	while (descriptor.handler != 0);
}

void radioDisableGDO0Interrupt()
{
    GPIO_IntDisable( 1 << RADIO_PIN_GDO0 );
}

void radioEnableGDO0Interrupt()
{
    GPIO_IntEnable( 1 << RADIO_PIN_GDO0 );
}

void radioDisableGDO2Interrupt()
{
    GPIO_IntDisable( 1 << RADIO_PIN_GDO2 );
}

void radioEnableGDO2Interrupt()
{
    GPIO_IntEnable( 1 << RADIO_PIN_GDO2 );
}

void radioClearInterruptPendingLines()
{
    GPIO_IntClear( (1 << RADIO_PIN_GDO0) | (1 << RADIO_PIN_GDO2) );
}

void radioConfigureInterrupt(void)
{    
    GPIO_PinModeSet( RADIO_PORT_GDO0, RADIO_PIN_GDO0, gpioModeInputPullFilter, 1 ); // GDO0 Input, PullUp, Filter
    GPIO_PinModeSet( RADIO_PORT_GDO2, RADIO_PIN_GDO2, gpioModeInputPullFilter, 1 ); // GDO2 Input, PullUp, Filter
    GPIO_IntConfig( RADIO_PORT_GDO0, RADIO_PIN_GDO0, true, true, false ); // GDO0 Interrupt on rising/falling edges. Disabled by default.
    GPIO_IntConfig( RADIO_PORT_GDO2, RADIO_PIN_GDO2, true, true, false ); // GDO2 Interrupt on rising/falling edges. Disabled by default.
    radioClearInterruptPendingLines();
    INT_Enable();
}

void radio_isr(void)
{
     if ( GPIO_IntGet() & RADIO_PIN_GDO0 )
    {
        GPIO_IntClear( 1 << RADIO_PIN_GDO0 );
        CC1101_ISR(GDOLine0);
    }
    
    if ( GPIO_IntGet() & RADIO_PIN_GDO2 )
    {
        GPIO_IntClear( 1 << RADIO_PIN_GDO2 );
        CC1101_ISR(GDOLine2);
    }
}

void GPIO_EVEN_IRQHandler(void)
{
    radio_isr();
}

void GPIO_ODD_IRQHandler(void)
{
    radio_isr();
}
