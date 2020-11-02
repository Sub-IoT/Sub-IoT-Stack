/* * OSS-7 - An opensource implementation of the DASH7 Alliance Protocol for ultra
 * lowpower wireless sensor communication
 *
 * Copyright 2015 University of Antwerp
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

/*! \file ubx.h
 *
 *  \author Liam Wickins <liamw9534@gmail.com>
 *
 */

#ifndef __NEO_M8N_UBX_H_
#define __NEO_M8N_UBX_H_

#include <stdint.h>

#pragma pack (1)

#define UBX_PACKET_SYNC_CHAR1  0xB5
#define UBX_PACKET_SYNC_CHAR2  0x62

#define UBX_HEADER_AND_CRC_LENGTH  8
#define UBX_HEADER_LENGTH          6
#define UBX_CRC_LENGTH             2


#define UBX_SET_PACKET_HEADER(p, cls, id, length) \
    p.syncChars[0] = UBX_PACKET_SYNC_CHAR1; \
    p.syncChars[1] = UBX_PACKET_SYNC_CHAR2; \
    p.msgClass = cls; \
    p.msgId = id; \
    p.msgLength = length

#define UBX_IS_MSG(p, cls, id) (cls == p.msgClass && id == p.msgId)


#define UBX_PACKET_SIZE(p) UBX_HEADER_AND_CRC_LENGTH + p.msgLength

typedef enum
{
    UBX_MSG_CLASS_NAV = 0x01,
    UBX_MSG_CLASS_RXM = 0x02,
    UBX_MSG_CLASS_INF = 0x04,
    UBX_MSG_CLASS_ACK = 0x05,
    UBX_MSG_CLASS_CFG = 0x06,
    UBX_MSG_CLASS_MON = 0x0A,
    UBX_MSG_CLASS_AID = 0x0B,
    UBX_MSG_CLASS_TIM = 0x0D,
    UBX_MSG_CLASS_LOG = 0x21
} UBX_MessageClass_t;

typedef enum
{
    UBX_MSG_ID_RXM_PMREQ     = 0x41,
} UBX_MessageID_RXM_t;

typedef enum
{
    UBX_MSG_ID_ACK_ACK       = 0x01,
    UBX_MSG_ID_ACK_NACK      = 0x00
} UBX_MessageID_ACK_t;

typedef enum
{
    UBX_MSG_ID_NAV_AOPSTATUS = 0x60,
    UBX_MSG_ID_NAV_CLOCK     = 0x22,
    UBX_MSG_ID_NAV_DGPS      = 0x31,
    UBX_MSG_ID_NAV_DOP       = 0x04,
    UBX_MSG_ID_NAV_POSECEF   = 0x01,
    UBX_MSG_ID_NAV_POSLLH    = 0x02,
    UBX_MSG_ID_NAV_PVT       = 0x07,
    UBX_MSG_ID_NAV_SBAS      = 0x32,
    UBX_MSG_ID_NAV_SOL       = 0x06,
    UBX_MSG_ID_NAV_STATUS    = 0x03,
    UBX_MSG_ID_NAV_SVINFO    = 0x30,
    UBX_MSG_ID_NAV_TIMEGPS   = 0x20,
    UBX_MSG_ID_NAV_TIMEUTC   = 0x21,
    UBX_MSG_ID_NAV_VELECEF   = 0x11,
    UBX_MSG_ID_NAV_VELNED    = 0x12
} UBX_MessageID_NAV_t;

/* This message can be concatenated multiple times into a single message */
typedef struct
{
    uint8_t  clsID;
    uint8_t  msgID;
} UBX_ACK_t;

/****************************** RXM *************************************/

#define UBX_RXM_PMREQ_WAKEUP_SOURCE_UART 0x08U

typedef struct
{
	uint8_t  version;
	uint8_t  reserved1[3];
	uint32_t duration;
	uint32_t flags;
	uint32_t wakeupSources;
} UBX_RXM_PMREQ_t;

/****************************** NAV *************************************/

#define UBX_NAV_STATUS_FLAGS_GPSFIXOK  0x01U
#define UBX_NAV_STATUS_FLAGS_DIFFSOLN  0x02U
#define UBX_NAV_STATUS_FLAGS_WKNSET    0x04U
#define UBX_NAV_STATUS_FLAGS_TOWSET    0x08U

#define UBX_NAV_STATUS_FIXSTAT_DGPSISTAT   0x01U
#define UBX_NAV_STATUS_FIXSTAT_MAPMATCHING 0xC0U

enum {
    UBX_NAV_STATUS_FIXSTAT_MAPMATCHING_NONE,
    UBX_NAV_STATUS_FIXSTAT_MAPMATCHING_VALID_NOT_USED,
    UBX_NAV_STATUS_FIXSTAT_MAPMATCHING_VALID_AND_USED,
    UBX_NAV_STATUS_FIXSTAT_MAPMATCHING_VALID_AND_USED_DR,
};

#define UBX_NAV_STATUS_FLAGS2_PSMSTATE      0x03U

enum {
    UBX_NAV_STATUS_FLAGS2_PSMSTATE_ACQUISITION,
    UBX_NAV_STATUS_FLAGS2_PSMSTATE_TRACKING,
    UBX_NAV_STATUS_FLAGS2_PSMSTATE_OPTIMIZED,
    UBX_NAV_STATUS_FLAGS2_PSMSTATE_INACTIVE,
};

#define UBX_NAV_STATUS_FLAGS2_SPOOFDETSTATE 0x18U

enum {
    UBX_NAV_STATUS_FLAGS2_SPOOFDETSTATE_UNKNOWN_OFF,
    UBX_NAV_STATUS_FLAGS2_SPOOFDETSTATE_INDICATED,
    UBX_NAV_STATUS_FLAGS2_SPOOFDETSTATE_MULTI_INDICATED,
};

enum {
    UBX_NAV_STATUS_GPSFIX_NOFIX,
    UBX_NAV_STATUS_GPSFIX_DEAD_RECKONING,
    UBX_NAV_STATUS_GPSFIX_2DFIX,
    UBX_NAV_STATUS_GPSFIX_3DFIX,
    UBX_NAV_STATUS_GPSFIX_GPS_DEAD_RECKONING,
    UBX_NAV_STATUS_GPSFIX_TIME_ONLY
};

typedef struct
{
    uint32_t iTOW;
    uint8_t  gpsFix;
    uint8_t  flags;
    uint8_t  fixStat;
    uint8_t  flags2;
    uint32_t ttff;
    uint32_t msss;
} UBX_NAV_STATUS_t;

typedef struct
{
    uint32_t iTOW;
    int32_t  lon;
    int32_t  lat;
    int32_t  height;
    int32_t  hMSL;
    uint32_t hAcc;
    uint32_t vAcc;
} UBX_NAV_POSLLH_t;

#pragma anon_unions

typedef struct
{
    uint8_t  syncChars[2];
    uint8_t  msgClass;
    uint8_t  msgId;
    uint16_t msgLength;            /* Excludes header and CRC bytes */
    union
    {
        uint8_t             payloadAndCrc[64];  /* CRC is appended to payload */
        UBX_ACK_t			UBX_ACK;
        UBX_NAV_STATUS_t    UBX_NAV_STATUS;
        UBX_NAV_POSLLH_t    UBX_NAV_POSLLH;
        UBX_RXM_PMREQ_t		UBX_RXM_PMREQ;
    };
} UBX_Packet_t;

#pragma pack ()

#endif // __NEO_M8N_UBX_H_
