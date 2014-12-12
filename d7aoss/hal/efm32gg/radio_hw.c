
#include <stdbool.h>

#include "em_gpio.h"
#include "em_int.h"

#include "../../phy/cc1101/cc1101_core.h" // TODO refactor, don't depend on this here
#include "../../phy/cc1101/cc1101_phy.h" // TODO refactor, don't depend on this here
#include "radio_hw.h"


// turn on/off the debug prints
#if 0
#define DPRINT(...) log_print_string(__VA_ARGS__)
#else
#define DPRINT(...)  
#endif


InterruptHandlerDescriptor interrupt_table[6] = {
    {.gdoSetting = GDO_SETTING_RXFilled,        .edge = GDO_EDGE_RXFilled,      .handler = rx_data_isr          },
    {.gdoSetting = GDO_SETTING_TXBelowThresh,   .edge = GDO_EDGE_TXBelowThresh, .handler = tx_data_isr          },
    {.gdoSetting = GDO_SETTING_TXUnderflow,     .edge = GDO_EDGE_TXUnderflow,   .handler = end_of_packet_isr    },
    {.gdoSetting = GDO_SETTING_EndOfPacket,     .edge = GDO_EDGE_EndOfPacket,   .handler = end_of_packet_isr    },
    {.gdoSetting = GDO_SETTING_RXOverflow,      .edge = GDO_EDGE_RXOverflow,    .handler = rx_fifo_overflow_isr },
    {.handler = 0},
};

void CC1101_ISR(GDOLine gdo, GDOEdge edge)
{
    DPRINT("%s ISR %s", (gdo == GDOLine0)? "GDO0\0": "GDO2\0", (edge == GDOEdgeRising)? "EDGE_RISING\0": "EDGE_FALLING\0" );

    uint8_t gdo_setting = ReadSingleReg(gdo) | edge;
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
    if ( GPIO_IntGet() & (1 << RADIO_PIN_GDO0) )
    {
        // get pin level to know on which edge was the interrupt
        uint8_t Edge = (GPIO_PinInGet( RADIO_PORT_GDO0, 1 << RADIO_PIN_GDO0 ))? GDOEdgeRising : GDOEdgeFalling;

        // clear the interrupt flag
        GPIO_IntClear( 1 << RADIO_PIN_GDO0 );

        // call the isr
        CC1101_ISR(GDOLine0, Edge);
    }
    
    if ( GPIO_IntGet() & (1 << RADIO_PIN_GDO2) )
    {
        // get pin level to know on which edge was the interrupt
        uint8_t Edge = (GPIO_PinInGet( RADIO_PORT_GDO2, 1 << RADIO_PIN_GDO2 ))? GDOEdgeRising : GDOEdgeFalling;

        // clear the interrupt flag
        GPIO_IntClear( 1 << RADIO_PIN_GDO2 );

        // call the isr
        CC1101_ISR(GDOLine2, Edge);
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
