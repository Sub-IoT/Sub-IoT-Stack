/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2017 University of Antwerp
 * Copyright (c) 2017 STMicroelectronics International N.V. (see below)
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

/*
 * This code is based on code provided by STMicroelectronics, with the following license and disclaimer:
 *
 * @file    lora.c
 * @author  MCD Application Team
 * @version V1.1.2
 * @date    08-September-2017
 * @brief   lora API to drive the lora state Machine
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
 * All rights reserved.</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted, provided that the following conditions are met:
 *
 * 1. Redistribution of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of STMicroelectronics nor the names of other
 *    contributors to this software may be used to endorse or promote products
 *    derived from this software without specific written permission.
 * 4. This software, including modifications and/or derivative works of this
 *    software, must execute solely and exclusively on microcontroller or
 *    microprocessor devices manufactured by or for STMicroelectronics.
 * 5. Redistribution and use of this software other than as permitted under
 *    this license is void and will automatically terminate your rights under
 *    this license.
 *
 * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
 * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
 * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */


#include "lorawan_stack.h"
#include "hw.h"
#include "LoRaMac.h"
#include "LoRaMacTest.h"
#include "debug.h"
#include "scheduler.h"

// TODO configurable
#define LORAWAN_ADR_ENABLED                         1 // TODO configurable?
#define LORAWAN_PUBLIC_NETWORK_ENABLED              1 // TODO configurable?
#define LORAWAN_CLASS                               CLASS_A // TODO configurable?
#define JOINREQ_NBTRIALS                            3
#define LORAWAN_APP_DATA_BUFF_SIZE                  64 // TODO = max?
#define LORAWAN_DATARATE                            0 // TODO configurable? assuming ADR for now
#define OVER_THE_AIR_ACTIVATION                     1

typedef enum
{
  STATE_NOT_JOINED,
  STATE_JOINED,
  STATE_JOIN_FAILED
} state_t;

static state_t state = STATE_NOT_JOINED;
static LoRaMacPrimitives_t loraMacPrimitives;
static LoRaMacCallback_t loraMacCallbacks;
static LoRaMacStatus_t loraMacStatus;
static uint8_t devEui[8] = { 0 };
static uint8_t appEui[8] = { 0 };
static uint8_t appKey[16] = { 0 };
static bool next_tx = true;
static bool use_confirmed_tx = false;

static uint8_t payload_data_buffer[LORAWAN_APP_DATA_BUFF_SIZE];
static lora_AppData_t app_data = { payload_data_buffer, 0 ,0 };

static lora_rx_callback_t rx_callback = NULL;
static join_completed_callback_t join_completed_callback = NULL;

static void run_fsm()
{
  switch(state)
  {
    case STATE_NOT_JOINED:
    {
#if( OVER_THE_AIR_ACTIVATION != 0 )
      MlmeReq_t mlmeReq;

      mlmeReq.Type = MLME_JOIN;
      mlmeReq.Req.Join.DevEui = devEui;
      mlmeReq.Req.Join.AppEui = appEui;
      mlmeReq.Req.Join.AppKey = appKey;
      mlmeReq.Req.Join.NbTrials = JOINREQ_NBTRIALS;

      //if(next_tx == true)
      {
          LoRaMacMlmeRequest(&mlmeReq);
      }

      //sched_post_task_prio(&run_fsm, MIN_PRIORITY);
#else
      MibRequestConfirm_t mibReq;
      mibReq.Type = MIB_NET_ID;
      mibReq.Param.NetID = LORAWAN_NETWORK_ID;
      LoRaMacMibSetRequestConfirm( &mibReq );

      mibReq.Type = MIB_DEV_ADDR;
      mibReq.Param.DevAddr = DevAddr;
      LoRaMacMibSetRequestConfirm( &mibReq );

      mibReq.Type = MIB_NWK_SKEY;
      mibReq.Param.NwkSKey = NwkSKey;
      LoRaMacMibSetRequestConfirm( &mibReq );

      mibReq.Type = MIB_APP_SKEY;
      mibReq.Param.AppSKey = AppSKey;
      LoRaMacMibSetRequestConfirm( &mibReq );

      mibReq.Type = MIB_NETWORK_JOINED;
      mibReq.Param.IsNetworkJoined = true;
      LoRaMacMibSetRequestConfirm( &mibReq );
      state = STATE_JOINED;
#endif
      break;
    }
    case STATE_JOINED:
    {
      log_print_string("JOINED");
      sched_cancel_task(&run_fsm);
      if(join_completed_callback)
        join_completed_callback(true);
      break;
    }
    case STATE_JOIN_FAILED:
    {
      log_print_string("JOIN FAILED");
      if(join_completed_callback)
        join_completed_callback(false);
      break;
    }
    default:
    {
      assert(false);
      break;
    }
  }

}

static void mcps_confirm(McpsConfirm_t *McpsConfirm)
{
  // TODO
}

static void mcps_indication(McpsIndication_t *mcpsIndication)
{
  if(mcpsIndication->Status != LORAMAC_EVENT_INFO_STATUS_OK)
  {
    log_print_string("mcps_indication status: %i", mcpsIndication->Status);
    return;
  }

  if( mcpsIndication->RxData == true )
  {
    log_print_string("received %i bytes for port %i", mcpsIndication->BufferSize, mcpsIndication->Port);
    app_data.Port = mcpsIndication->Port;
    app_data.BuffSize = mcpsIndication->BufferSize;
    memcpy1(app_data.Buff, mcpsIndication->Buffer, app_data.BuffSize);
    rx_callback(&app_data);
  }
}

static void mlme_confirm(MlmeConfirm_t *mlmeConfirm)
{
  switch(mlmeConfirm->MlmeRequest)
  {
    case MLME_JOIN:
    {
      if(mlmeConfirm->Status == LORAMAC_EVENT_INFO_STATUS_OK)
      {
        log_print_string("join succeeded");
        state = STATE_JOINED;
      }
      else
      {
        log_print_string("join failed");
        state = STATE_JOIN_FAILED;
        sched_post_task(&run_fsm);
      }
      break;
    }
    default:
      log_print_string("mlme_confirm called for not implemented mlme request %i", mlmeConfirm->MlmeRequest);
      break;
  }

  next_tx = true; // TODO needed?
}

static bool is_joined()
{
  MibRequestConfirm_t mibReq;
  mibReq.Type = MIB_NETWORK_JOINED;
  LoRaMacMibGetRequestConfirm(&mibReq);
  return (mibReq.Param.IsNetworkJoined == true);
}


// TODO assuming OTAA for now
void lorawan_stack_init(uint8_t dEUI[8], uint8_t aEUI[8], uint8_t aKey[16], join_completed_callback_t join_completed_cb, lora_rx_callback_t rx_cb) {
  memcpy(devEui, dEUI, 8);
  memcpy(appEui, aEUI, 8);
  memcpy(appKey, aKey, 16);

  HW_Init(); // TODO refactor

  rx_callback = rx_cb;
  join_completed_callback = join_completed_cb;

  state = STATE_NOT_JOINED;

  log_print_string("Init using OTAA");
  log_print_string("DevEui:");
  log_print_data(devEui, 8);
  log_print_string("AppEui:");
  log_print_data(appEui, 8);
  log_print_string("AppKey:");
  log_print_data(appKey, 16);

  sched_register_task(&run_fsm);
  sched_post_task(&run_fsm);

  loraMacPrimitives.MacMcpsConfirm = &mcps_confirm;
  loraMacPrimitives.MacMcpsIndication = &mcps_indication;
  loraMacPrimitives.MacMlmeConfirm = &mlme_confirm;

  // Initialization for the region EU868
  loraMacStatus = LoRaMacInitialization(&loraMacPrimitives, &loraMacCallbacks, LORAMAC_REGION_EU868);
  if(loraMacStatus == LORAMAC_STATUS_OK) {
    log_print_string("init OK");
  } else {
    log_print_string("init failed");
  }

  MibRequestConfirm_t mibReq;
  mibReq.Type = MIB_ADR;
  mibReq.Param.AdrEnable = LORAWAN_ADR_ENABLED;
  LoRaMacMibSetRequestConfirm( &mibReq );

  mibReq.Type = MIB_PUBLIC_NETWORK;
  mibReq.Param.EnablePublicNetwork = LORAWAN_PUBLIC_NETWORK_ENABLED;
  LoRaMacMibSetRequestConfirm( &mibReq );

  mibReq.Type = MIB_DEVICE_CLASS;
  mibReq.Param.Class = LORAWAN_CLASS;
  LoRaMacMibSetRequestConfirm( &mibReq );

#if defined( REGION_EU868 )
  LoRaMacTestSetDutyCycleOn(true);

#if( USE_SEMTECH_DEFAULT_CHANNEL_LINEUP == 1 )
  LoRaMacChannelAdd( 3, ( ChannelParams_t )LC4 );
  LoRaMacChannelAdd( 4, ( ChannelParams_t )LC5 );
  LoRaMacChannelAdd( 5, ( ChannelParams_t )LC6 );
  LoRaMacChannelAdd( 6, ( ChannelParams_t )LC7 );
  LoRaMacChannelAdd( 7, ( ChannelParams_t )LC8 );
  LoRaMacChannelAdd( 8, ( ChannelParams_t )LC9 );
  LoRaMacChannelAdd( 9, ( ChannelParams_t )LC10 );

  mibReq.Type = MIB_RX2_DEFAULT_CHANNEL;
  mibReq.Param.Rx2DefaultChannel = ( Rx2ChannelParams_t ){ 869525000, DR_3 };
  LoRaMacMibSetRequestConfirm( &mibReq );

  mibReq.Type = MIB_RX2_CHANNEL;
  mibReq.Param.Rx2Channel = ( Rx2ChannelParams_t ){ 869525000, DR_3 };
  LoRaMacMibSetRequestConfirm( &mibReq );
#endif

#endif
}


lorawan_stack_error_t lorawan_stack_send(uint8_t* payload, uint8_t length, uint8_t app_port, bool request_ack) {
  if(!is_joined()) {
    log_print_string("TX not possible, not joined");
    return LORAWAN_STACK_ERROR_NOT_JOINED;
  }

  if(length > LORAWAN_APP_DATA_BUFF_SIZE)
      length = LORAWAN_APP_DATA_BUFF_SIZE;

  memcpy1(app_data.Buff, payload, length);
  app_data.BuffSize = length;
  app_data.Port = app_port;

  McpsReq_t mcpsReq;
  LoRaMacTxInfo_t txInfo;
  if(LoRaMacQueryTxPossible(app_data.BuffSize, &txInfo) != LORAMAC_STATUS_OK )
  {
    // Send empty frame in order to flush MAC commands
    log_print_string("TX not possible, max payloadsize %i, trying to transmit %i", txInfo.MaxPossiblePayload, txInfo.CurrentPayloadSize);
    mcpsReq.Type = MCPS_UNCONFIRMED;
    mcpsReq.Req.Unconfirmed.fBuffer = NULL;
    mcpsReq.Req.Unconfirmed.fBufferSize = 0;
    mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DATARATE;
    LoRaMacMcpsRequest(&mcpsReq);
    //state = STATE_SLEEP;
    return LORAWAN_STACK_ERROR_TX_NOT_POSSIBLE;
  }

  if(!request_ack)
  {
    mcpsReq.Type = MCPS_UNCONFIRMED;
    mcpsReq.Req.Unconfirmed.fPort = app_data.Port;
    mcpsReq.Req.Unconfirmed.fBuffer = app_data.Buff;
    mcpsReq.Req.Unconfirmed.fBufferSize = app_data.BuffSize;
    mcpsReq.Req.Unconfirmed.Datarate = LORAWAN_DATARATE;
  }
  else
  {
    mcpsReq.Type = MCPS_CONFIRMED;
    mcpsReq.Req.Confirmed.fPort = app_data.Port;
    mcpsReq.Req.Confirmed.fBuffer = app_data.Buff;
    mcpsReq.Req.Confirmed.fBufferSize = app_data.BuffSize;
    mcpsReq.Req.Confirmed.NbTrials = 8;
    mcpsReq.Req.Confirmed.Datarate = LORAWAN_DATARATE;
  }

  LoRaMacStatus_t status = LoRaMacMcpsRequest(&mcpsReq);
  if(status != LORAMAC_STATUS_OK) {
    //  state = STATE_SLEEP;
    log_print_string("failed sending data (status %i)", status);
    return LORAWAN_STACK_ERROR_UNKNOWN;
  }

  return LORAWAN_STACK_ERROR_OK;
}
