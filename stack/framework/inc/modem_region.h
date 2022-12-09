#ifndef MODEM_REGION_H
#define MODEM_REGION_H

typedef enum
{
    MODEM_REGION_AS923_1_DUTY_CYCLE, // //LoRaWAN: duty cycle enabled, dwell time disabled (Japan, Malaysia, Singapore)
    MODEM_REGION_AU915,
    MODEM_REGION_CN470,
    MODEM_REGION_CN779,
    MODEM_REGION_EU433,
    MODEM_REGION_EU868,
    MODEM_REGION_KR920,
    MODEM_REGION_IN865,
    MODEM_REGION_US915,
    MODEM_REGION_RU864,
    MODEM_REGION_AS923_1_DUTY_CYCLE_DWELL_TIME, // //LoRaWAN: duty cycle enabled, dwell time enabled (Hong Kong, Thailand)
    MODEM_REGION_AS923_1_NO_RESTRICTIONS, // //LoRaWAN: duty cycle disabled, dwell time disabled (Australia - note: customer should make choice of this or AU915)
    MODEM_REGION_AS923_2, //LoRaWAN: duty cycle enabled, dwell time disabled (Indonesia, Vietnam)
    MODEM_REGION_AS923_3, //LoRaWAN: duty cycle enabled, dwell time disabled (Algeria, Comoros, Cuba, Iran, Jordan, Kuwait, Philippines, Qatar, Saudi Arabia, Somalia, Syria, UAE)
    MODEM_REGION_AS923_4, //LoRaWAN: duty cycle enabled, dwell time disabled (Israel)
} modem_region_t;

#endif //MODEM_REGION_H

