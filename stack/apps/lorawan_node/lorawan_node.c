/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2017 University of Antwerp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "log.h"

#include "lorawan_stack.h"

#define LORAWAN_APP_PORT                            2
#define LORAWAN_CONFIRMED_MSG                       ENABLE
#define APP_TX_DUTYCYCLE                            10000 // in ms
#define LORAWAN_ADR_ON                              1
#define JOINREQ_NBTRIALS                            3

static void LoraTxData( lora_AppData_t *AppData, FunctionalState* IsTxConfirmed)
{
  // TODO
  /* USER CODE BEGIN 3 */
  uint16_t pressure = 0;
  int16_t temperature = 0;
  uint16_t humidity = 0;
  uint8_t batteryLevel;


  temperature = 0;     /* in °C * 100 */
  pressure    = 0;  /* in hPa / 10 */
  humidity    = 0;        /* in %*10     */

  uint32_t i = 0;

  batteryLevel = 0;                     /* 1 (very low) to 254 (fully charged) */

  AppData->Port = LORAWAN_APP_PORT;

  *IsTxConfirmed =  LORAWAN_CONFIRMED_MSG;


  AppData->Buff[i++] = 0;
  AppData->Buff[i++] = ( pressure >> 8 ) & 0xFF;
  AppData->Buff[i++] = pressure & 0xFF;
  AppData->Buff[i++] = ( temperature >> 8 ) & 0xFF;
  AppData->Buff[i++] = temperature & 0xFF;
  AppData->Buff[i++] = ( humidity >> 8 ) & 0xFF;
  AppData->Buff[i++] = humidity & 0xFF;
  AppData->Buff[i++] = batteryLevel;

  AppData->BuffSize = i;

  /* USER CODE END 3 */
}

static LoRaMainCallback_t lora_main_callbacks = {
  NULL, //HW_GetBatteryLevel,
  HW_GetUniqueId,
  HW_GetRandomSeed,
  LoraTxData, // TODO
  NULL //LoraRxData
};

static LoRaParam_t lora_init_params = {
  TX_ON_TIMER,
  APP_TX_DUTYCYCLE,
  CLASS_A,
  LORAWAN_ADR_ON,
  DR_0,
  LORAWAN_PUBLIC_NETWORK,
  JOINREQ_NBTRIALS
};

void bootstrap()
{
    log_print_string("Device booted\n");

    HW_Init(); // TODO refactor

    lora_Init(&lora_main_callbacks, &lora_init_params);


    while(1)
    {
        lora_fsm();

//        DISABLE_IRQ();
//        /* if an interrupt has occurred after DISABLE_IRQ, it is kept pending
//         * and cortex will not enter low power anyway  */
//        if ( lora_getDeviceState( ) == DEVICE_STATE_SLEEP )
//        {
//        #ifndef LOW_POWER_DISABLE
//          LowPower_Handler( );
//        #endif
//        }
//        ENABLE_IRQ();
    }
}
