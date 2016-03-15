/*
 * si4460_registers.h
 *
 *  Created on: Dec 11, 2015
 *      Author: MaartenWeyn
 */

#ifndef FRAMEWORK_HAL_CHIPS_SI4460_EZRADIODRV_SI4460_REGISTERS_H_
#define FRAMEWORK_HAL_CHIPS_SI4460_EZRADIODRV_SI4460_REGISTERS_H_

// 868 LR Start 1: 863012500 - 2:  869387500
// 868 N/HR Start: 863100000

#define RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPFF_868_LR  \
  0x23 /* GROUP: Synth                                      */,\
  0x01 /* NUM_PROPS                                         */,\
  0x00 /* START_PROP                                        */,\
  0x2C /* SYNTH_PFDCP_CPFF,CP_FF_CUR_TEST[6],CP_FF_CUR[5:0] */\

#define RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPFF_868_NR RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPFF_868_LR
#define RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPFF_868_HR  \
  0x23 /* GROUP: Synth                                      */,\
  0x01 /* NUM_PROPS                                         */,\
  0x00 /* START_PROP                                        */,\
  0x39 /* SYNTH_PFDCP_CPFF,CP_FF_CUR_TEST[6],CP_FF_CUR[5:0] */\


#define RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPINT_868_LR  \
  0x23 /* GROUP: Synth                      */,\
  0x01 /* NUM_PROPS                         */,\
  0x01 /* START_PROP                        */,\
  0x0E /* SYNTH_PFDCP_CPINT,CP_INT_CUR[3:0] */\

#define RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPINT_868_NR RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPINT_868_LR
#define RADIO_CONFIG_SET_PROPERTY_SYNTH_PFDCP_CPINT_868_HR  \
  0x23 /* GROUP: Synth                      */,\
  0x01 /* NUM_PROPS                         */,\
  0x01 /* START_PROP                        */,\
  0x04 /* SYNTH_PFDCP_CPINT,CP_INT_CUR[3:0] */\


#define RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT3_868_LR  \
  0x23 /* GROUP: Synth                 */,\
  0x01 /* NUM_PROPS                    */,\
  0x03 /* START_PROP                   */,\
  0x04 /* SYNTH_LPFILT3,LPF_FF_R2[2:0] */\

#define RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT3_868_NR RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT3_868_LR
#define RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT3_868_HR  \
  0x23 /* GROUP: Synth                 */,\
  0x01 /* NUM_PROPS                    */,\
  0x03 /* START_PROP                   */,\
  0x05 /* SYNTH_LPFILT3,LPF_FF_R2[2:0] */\


#define RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT2_868_LR  \
  0x23 /* GROUP: Synth                 */,\
  0x01 /* NUM_PROPS                    */,\
  0x04 /* START_PROP                   */,\
  0x0C /* SYNTH_LPFILT2,LPF_FF_C2[4:0] */\

#define RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT2_868_NR RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT2_868_LR
#define RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT2_868_HR  \
  0x23 /* GROUP: Synth                 */,\
  0x01 /* NUM_PROPS                    */,\
  0x04 /* START_PROP                   */,\
  0x04 /* SYNTH_LPFILT2,LPF_FF_C2[4:0] */\


#define RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT1_868_LR  \
  0x23 /* GROUP: Synth                                                    */,\
  0x01 /* NUM_PROPS                                                       */,\
  0x05 /* START_PROP                                                      */,\
  0x73 /* SYNTH_LPFILT1,LPF_FF_C1[6:4],LPF_FF_C1_CODE[3:2],LPF_FF_C3[1:0] */\

#define RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT1_868_NR RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT1_868_LR
#define RADIO_CONFIG_SET_PROPERTY_SYNTH_LPFILT1_868_HR  \
  0x23 /* GROUP: Synth                                                    */,\
  0x01 /* NUM_PROPS                                                       */,\
  0x05 /* START_PROP                                                      */,\
  0x01 /* SYNTH_LPFILT1,LPF_FF_C1[6:4],LPF_FF_C1_CODE[3:2],LPF_FF_C3[1:0] */\

#define RADIO_CONFIG_SET_PROPERTY_PA_TC_868_LR  \
  0x11 /* CMD: Set property              */,\
  0x22 /* GROUP: Pa                      */,\
  0x01 /* NUM_PROPS                      */,\
  0x03 /* START_PROP                     */,\
  0x1D /* PA_TC,FSK_MOD_DLY[7:5],TC[4:0] */\

#define RADIO_CONFIG_SET_PROPERTY_PA_TC_868_NR RADIO_CONFIG_SET_PROPERTY_PA_TC_868_LR
#define RADIO_CONFIG_SET_PROPERTY_PA_TC_868_HR  \
  0x22 /* GROUP: Pa                      */,\
  0x01 /* NUM_PROPS                      */,\
  0x03 /* START_PROP                     */,\
  0x3D /* PA_TC,FSK_MOD_DLY[7:5],TC[4:0] */\



#define RADIO_CONFIG_SET_PROPERTY_MODEM_RAW_EYE_868_LR  \
  0x20 /* GROUP: Modem                          */,\
  0x02 /* NUM_PROPS                             */,\
  0x46 /* START_PROP                            */,\
  0x00 /* MODEM_RAW_EYE,RAWEYE[2:0],RAWEYE[7:0] */,\
  0x3D /* DATA1                                 */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_RAW_EYE_868_NR  \
  0x20 /* GROUP: Modem                          */,\
  0x02 /* NUM_PROPS                             */,\
  0x46 /* START_PROP                            */,\
  0x01 /* MODEM_RAW_EYE,RAWEYE[2:0],RAWEYE[7:0] */,\
  0x3F /* DATA1                                 */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_RAW_EYE_868_HR  \
  0x20 /* GROUP: Modem                          */,\
  0x02 /* NUM_PROPS                             */,\
  0x46 /* START_PROP                            */,\
  0x00 /* MODEM_RAW_EYE,RAWEYE[2:0],RAWEYE[7:0] */,\
  0x85 /* DATA1                                 */\


#define RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_0_868_LR  \
  0x21 /* GROUP: Modem chflt                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                */,\
  0x0C /* NUM_PROPS                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         */,\
  0x00 /* START_PROP                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        */,\
  0xCC /* MODEM_CHFLT_RX1_CHFLT_COE,RX1_CHFLT_COE13[1:0],RX1_CHFLT_COE13[7:0],RX1_CHFLT_COE12[3:2],RX1_CHFLT_COE12[7:0],RX1_CHFLT_COE11[5:4],RX1_CHFLT_COE11[7:0],RX1_CHFLT_COE10[7:6],RX1_CHFLT_COE10[7:0],RX1_CHFLT_COE9[1:0],RX1_CHFLT_COE9[7:0],RX1_CHFLT_COE8[3:2],RX1_CHFLT_COE8[7:0],RX1_CHFLT_COE7[5:4],RX1_CHFLT_COE7[7:0],RX1_CHFLT_COE6[7:6],RX1_CHFLT_COE6[7:0],RX1_CHFLT_COE5[1:0],RX1_CHFLT_COE5[7:0],RX1_CHFLT_COE4[3:2],RX1_CHFLT_COE4[7:0],RX1_CHFLT_COE3[5:4],RX1_CHFLT_COE3[7:0],RX1_CHFLT_COE2[7:6],RX1_CHFLT_COE2[7:0],RX1_CHFLT_COE1[1:0],RX1_CHFLT_COE1[7:0],RX1_CHFLT_COE0[3:2],RX1_CHFLT_COE0[7:0] */,\
  0xA1 /* DATA1                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x30 /* DATA2                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xA0 /* DATA3                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x21 /* DATA4                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xD1 /* DATA5                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xB9 /* DATA6                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xC9 /* DATA7                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xEA /* DATA8                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x05 /* DATA9                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x12 /* DATA10                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            */,\
  0x11 /* DATA11                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_0_868_NR RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_0_868_LR
#define RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_0_868_HR  \
  0x21 /* GROUP: Modem chflt                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                */,\
  0x0C /* NUM_PROPS                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         */,\
  0x00 /* START_PROP                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        */,\
  0x7E /* MODEM_CHFLT_RX1_CHFLT_COE,RX1_CHFLT_COE13[1:0],RX1_CHFLT_COE13[7:0],RX1_CHFLT_COE12[3:2],RX1_CHFLT_COE12[7:0],RX1_CHFLT_COE11[5:4],RX1_CHFLT_COE11[7:0],RX1_CHFLT_COE10[7:6],RX1_CHFLT_COE10[7:0],RX1_CHFLT_COE9[1:0],RX1_CHFLT_COE9[7:0],RX1_CHFLT_COE8[3:2],RX1_CHFLT_COE8[7:0],RX1_CHFLT_COE7[5:4],RX1_CHFLT_COE7[7:0],RX1_CHFLT_COE6[7:6],RX1_CHFLT_COE6[7:0],RX1_CHFLT_COE5[1:0],RX1_CHFLT_COE5[7:0],RX1_CHFLT_COE4[3:2],RX1_CHFLT_COE4[7:0],RX1_CHFLT_COE3[5:4],RX1_CHFLT_COE3[7:0],RX1_CHFLT_COE2[7:6],RX1_CHFLT_COE2[7:0],RX1_CHFLT_COE1[1:0],RX1_CHFLT_COE1[7:0],RX1_CHFLT_COE0[3:2],RX1_CHFLT_COE0[7:0] */,\
  0x64 /* DATA1                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x1B /* DATA2                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xBA /* DATA3                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x58 /* DATA4                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x0B /* DATA5                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xDD /* DATA6                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xCE /* DATA7                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xD6 /* DATA8                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xE6 /* DATA9                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xF6 /* DATA10                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            */,\
  0x00 /* DATA11                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            */\


#define RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_1_868_LR  \
  0x21 /* GROUP: Modem chflt */,\
  0x06 /* NUM_PROPS          */,\
  0x0C /* START_PROP         */,\
  0x0A /* DATA0              */,\
  0x04 /* DATA1              */,\
  0x15 /* DATA2              */,\
  0xFC /* DATA3              */,\
  0x03 /* DATA4              */,\
  0x00 /* DATA5              */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_1_868_NR RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_1_868_LR
#define RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE_1_868_HR  \
  0x21 /* GROUP: Modem chflt */,\
  0x06 /* NUM_PROPS          */,\
  0x0C /* START_PROP         */,\
  0x03 /* DATA0              */,\
  0x03 /* DATA1              */,\
  0x15 /* DATA2              */,\
  0xF0 /* DATA3              */,\
  0x3F /* DATA4              */,\
  0x00 /* DATA5              */\


#define RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_0_868_LR  \
  0x21 /* GROUP: Modem chflt                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                */,\
  0x0C /* NUM_PROPS                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         */,\
  0x12 /* START_PROP                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        */,\
  0xCC /* MODEM_CHFLT_RX2_CHFLT_COE,RX2_CHFLT_COE13[1:0],RX2_CHFLT_COE13[7:0],RX2_CHFLT_COE12[3:2],RX2_CHFLT_COE12[7:0],RX2_CHFLT_COE11[5:4],RX2_CHFLT_COE11[7:0],RX2_CHFLT_COE10[7:6],RX2_CHFLT_COE10[7:0],RX2_CHFLT_COE9[1:0],RX2_CHFLT_COE9[7:0],RX2_CHFLT_COE8[3:2],RX2_CHFLT_COE8[7:0],RX2_CHFLT_COE7[5:4],RX2_CHFLT_COE7[7:0],RX2_CHFLT_COE6[7:6],RX2_CHFLT_COE6[7:0],RX2_CHFLT_COE5[1:0],RX2_CHFLT_COE5[7:0],RX2_CHFLT_COE4[3:2],RX2_CHFLT_COE4[7:0],RX2_CHFLT_COE3[5:4],RX2_CHFLT_COE3[7:0],RX2_CHFLT_COE2[7:6],RX2_CHFLT_COE2[7:0],RX2_CHFLT_COE1[1:0],RX2_CHFLT_COE1[7:0],RX2_CHFLT_COE0[3:2],RX2_CHFLT_COE0[7:0] */,\
  0xA1 /* DATA1                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x30 /* DATA2                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xA0 /* DATA3                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x21 /* DATA4                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xD1 /* DATA5                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xB9 /* DATA6                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xC9 /* DATA7                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xEA /* DATA8                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x05 /* DATA9                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x12 /* DATA10                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            */,\
  0x11 /* DATA11                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_0_868_NR RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_0_868_LR
#define RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_0_868_HR  \
  0x21 /* GROUP: Modem chflt                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                */,\
  0x0C /* NUM_PROPS                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         */,\
  0x12 /* START_PROP                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        */,\
  0x7E /* MODEM_CHFLT_RX2_CHFLT_COE,RX2_CHFLT_COE13[1:0],RX2_CHFLT_COE13[7:0],RX2_CHFLT_COE12[3:2],RX2_CHFLT_COE12[7:0],RX2_CHFLT_COE11[5:4],RX2_CHFLT_COE11[7:0],RX2_CHFLT_COE10[7:6],RX2_CHFLT_COE10[7:0],RX2_CHFLT_COE9[1:0],RX2_CHFLT_COE9[7:0],RX2_CHFLT_COE8[3:2],RX2_CHFLT_COE8[7:0],RX2_CHFLT_COE7[5:4],RX2_CHFLT_COE7[7:0],RX2_CHFLT_COE6[7:6],RX2_CHFLT_COE6[7:0],RX2_CHFLT_COE5[1:0],RX2_CHFLT_COE5[7:0],RX2_CHFLT_COE4[3:2],RX2_CHFLT_COE4[7:0],RX2_CHFLT_COE3[5:4],RX2_CHFLT_COE3[7:0],RX2_CHFLT_COE2[7:6],RX2_CHFLT_COE2[7:0],RX2_CHFLT_COE1[1:0],RX2_CHFLT_COE1[7:0],RX2_CHFLT_COE0[3:2],RX2_CHFLT_COE0[7:0] */,\
  0x64 /* DATA1                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x1B /* DATA2                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xBA /* DATA3                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x58 /* DATA4                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0x0B /* DATA5                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xDD /* DATA6                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xCE /* DATA7                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xD6 /* DATA8                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xE6 /* DATA9                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             */,\
  0xF6 /* DATA10                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            */,\
  0x00 /* DATA11                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            */\


#define RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_1_868_LR  \
  0x21 /* GROUP: Modem chflt */,\
  0x06 /* NUM_PROPS          */,\
  0x1E /* START_PROP         */,\
  0x0A /* DATA0              */,\
  0x04 /* DATA1              */,\
  0x15 /* DATA2              */,\
  0xFC /* DATA3              */,\
  0x03 /* DATA4              */,\
  0x00 /* DATA5              */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_1_868_NR RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_1_868_LR
#define RADIO_CONFIG_SET_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE_1_868_HR  \
  0x21 /* GROUP: Modem chflt */,\
  0x06 /* NUM_PROPS          */,\
  0x1E /* START_PROP         */,\
  0x03 /* DATA0              */,\
  0x03 /* DATA1              */,\
  0x15 /* DATA2              */,\
  0xF0 /* DATA3              */,\
  0x3F /* DATA4              */,\
  0x00 /* DATA5              */\



#define RADIO_CONFIG_SET_PROPERTY_MODEM_DSA_QUAL_868_LR  \
  0x20 /* GROUP: Modem                                */,\
  0x01 /* NUM_PROPS                                   */,\
  0x5D /* START_PROP                                  */,\
  0x05 /* MODEM_DSA_QUAL,EYE_QUAL_SEL[7],ARRQUAL[6:0] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_DSA_QUAL_868_NR  \
  0x20 /* GROUP: Modem                                */,\
  0x01 /* NUM_PROPS                                   */,\
  0x5D /* START_PROP                                  */,\
  0x09 /* MODEM_DSA_QUAL,EYE_QUAL_SEL[7],ARRQUAL[6:0] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_DSA_QUAL_868_HR  \
  0x20 /* GROUP: Modem                                */,\
  0x01 /* NUM_PROPS                                   */,\
  0x5D /* START_PROP                                  */,\
  0x04 /* MODEM_DSA_QUAL,EYE_QUAL_SEL[7],ARRQUAL[6:0] */\

/*******************************************************************
				FREQUENCY
********************************************************************/

#define RADIO_CONFIG_SET_PROPERTY_MODEM_CLKGEN_BAND_868  \
0x20 /* GROUP: Modem                                            */,\
0x01 /* NUM_PROPS                                               */,\
0x51 /* START_PROP                                              */,\
0x08/* MODEM_CLKGEN_BAND,FORCE_SY_RECAL[4],SY_SEL[3],BAND[2:0] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_CLKGEN_BAND_434  \
0x20 /* GROUP: Modem                                            */,\
0x01 /* NUM_PROPS                                               */,\
0x51 /* START_PROP                                              */,\
0x0A /* MODEM_CLKGEN_BAND,FORCE_SY_RECAL[4],SY_SEL[3],BAND[2:0] */\

/*******************************************************************
					DATA RATE
********************************************************************/

#define RADIO_CONFIG_SET_PROPERTY_MODEM_DATA_RATE_LR  \
  0x20 /* GROUP: Modem                                                 */,\
  0x03 /* NUM_PROPS                                                    */,\
  0x03 /* START_PROP                                                   */,\
  0x05 /* MODEM_DATA_RATE,DATA_RATE[7:0],DATA_RATE[7:0],DATA_RATE[7:0] */,\
  0xDC /* DATA1                                                        */,\
  0x00 /* DATA2                                                        */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_DATA_RATE_NR  \
  0x20 /* GROUP: Modem                                                 */,\
  0x03 /* NUM_PROPS                                                    */,\
  0x03 /* START_PROP                                                   */,\
  0x10 /* MODEM_DATA_RATE,DATA_RATE[7:0],DATA_RATE[7:0],DATA_RATE[7:0] */,\
  0xF4 /* DATA1                                                        */,\
  0x3C /* DATA2                                                        */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_DATA_RATE_HR  \
  0x20 /* GROUP: Modem                                                 */,\
  0x03 /* NUM_PROPS                                                    */,\
  0x03 /* START_PROP                                                   */,\
  0x32 /* MODEM_DATA_RATE,DATA_RATE[7:0],DATA_RATE[7:0],DATA_RATE[7:0] */,\
  0xDC /* DATA1                                                        */,\
  0xDC /* DATA2                                                        */\


#define RADIO_CONFIG_SET_PROPERTY_MODEM_TX_NCO_MODE_LR  \
  0x20 /* GROUP: Modem                                                                 */,\
  0x04 /* NUM_PROPS                                                                    */,\
  0x06 /* START_PROP                                                                   */,\
  0x05 /* MODEM_TX_NCO_MODE,TXOSR[3:2],NCOMOD[1:0],NCOMOD[7:0],NCOMOD[7:0],NCOMOD[7:0] */,\
  0x8C /* DATA1                                                                        */,\
  0xBA /* DATA2                                                                        */,\
  0x80 /* DATA3                                                                        */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_TX_NCO_MODE_NR  \
  0x20 /* GROUP: Modem                                                                 */,\
  0x04 /* NUM_PROPS                                                                    */,\
  0x06 /* START_PROP                                                                   */,\
  0x09 /* MODEM_TX_NCO_MODE,TXOSR[3:2],NCOMOD[1:0],NCOMOD[7:0],NCOMOD[7:0],NCOMOD[7:0] */,\
  0x8C /* DATA1                                                                        */,\
  0xBA /* DATA2                                                                        */,\
  0x80 /* DATA3                                                                        */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_TX_NCO_MODE_HR RADIO_CONFIG_SET_PROPERTY_MODEM_TX_NCO_MODE_NR

#define RADIO_CONFIG_SET_PROPERTY_MODEM_DECIMATION_CFG1_LR \
  0x20 /* GROUP: Modem                                           */,\
  0x01 /* NUM_PROPS                                              */,\
  0x1E /* START_PROP                                             */,\
  0x20 /* MODEM_DECIMATION_CFG1,NDEC2[7:6],NDEC1[5:4],NDEC0[3:1] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_DECIMATION_CFG1_NR \
  0x20 /* GROUP: Modem                                           */,\
  0x01 /* NUM_PROPS                                              */,\
  0x1E /* START_PROP                                             */,\
  0x10 /* MODEM_DECIMATION_CFG1,NDEC2[7:6],NDEC1[5:4],NDEC0[3:1] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_DECIMATION_CFG1_HR  \
  0x20 /* GROUP: Modem                                           */,\
  0x01 /* NUM_PROPS                                              */,\
  0x1E /* START_PROP                                             */,\
  0x00 /* MODEM_DECIMATION_CFG1,NDEC2[7:6],NDEC1[5:4],NDEC0[3:1] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_OSR_LR  \
  0x20 /* GROUP: Modem                        */,\
  0x02 /* NUM_PROPS                           */,\
  0x22 /* START_PROP                          */,\
  0x01 /* MODEM_BCR_OSR,RXOSR[3:0],RXOSR[7:0] */,\
  0x53 /* DATA1                               */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_OSR_NR  \
  0x20 /* GROUP: Modem                        */,\
  0x02 /* NUM_PROPS                           */,\
  0x22 /* START_PROP                          */,\
  0x00 /* MODEM_BCR_OSR,RXOSR[3:0],RXOSR[7:0] */,\
  0x75 /* DATA1                               */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_OSR_HR  \
  0x11 /* CMD: Set property                   */,\
  0x20 /* GROUP: Modem                        */,\
  0x02 /* NUM_PROPS                           */,\
  0x22 /* START_PROP                          */,\
  0x00 /* MODEM_BCR_OSR,RXOSR[3:0],RXOSR[7:0] */,\
  0x4E /* DATA1                               */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_NCO_OFFSET_LR  \
  0x20 /* GROUP: Modem                                          */,\
  0x03 /* NUM_PROPS                                             */,\
  0x24 /* START_PROP                                            */,\
  0x01 /* MODEM_BCR_NCO_OFFSET,NCOFF[5:0],NCOFF[7:0],NCOFF[7:0] */,\
  0x83 /* DATA1                                                 */,\
  0x2B /* DATA2                                                 */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_NCO_OFFSET_NR  \
  0x20 /* GROUP: Modem                                          */,\
  0x03 /* NUM_PROPS                                             */,\
  0x24 /* START_PROP                                            */,\
  0x04 /* MODEM_BCR_NCO_OFFSET,NCOFF[5:0],NCOFF[7:0],NCOFF[7:0] */,\
  0x60 /* DATA1                                                 */,\
  0x43 /* DATA2                                                 */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_NCO_OFFSET_HR  \
  0x20 /* GROUP: Modem                                          */,\
  0x03 /* NUM_PROPS                                             */,\
  0x24 /* START_PROP                                            */,\
  0x06 /* MODEM_BCR_NCO_OFFSET,NCOFF[5:0],NCOFF[7:0],NCOFF[7:0] */,\
  0x90 /* DATA1                                                 */,\
  0x6A /* DATA2                                                 */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_GAIN_LR  \
  0x20 /* GROUP: Modem                           */,\
  0x02 /* NUM_PROPS                              */,\
  0x27 /* START_PROP                             */,\
  0x01 /* MODEM_BCR_GAIN,CRGAIN[2:0],CRGAIN[7:0] */,\
  0x83 /* DATA1                                  */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_GAIN_NR  \
  0x20 /* GROUP: Modem                           */,\
  0x02 /* NUM_PROPS                              */,\
  0x27 /* START_PROP                             */,\
  0x02 /* MODEM_BCR_GAIN,CRGAIN[2:0],CRGAIN[7:0] */,\
  0x6E /* DATA1                                  */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_BCR_GAIN_HR  \
  0x20 /* GROUP: Modem                           */,\
  0x02 /* NUM_PROPS                              */,\
  0x27 /* START_PROP                             */,\
  0x07 /* MODEM_BCR_GAIN,CRGAIN[2:0],CRGAIN[7:0] */,\
  0xFF /* DATA1                                  */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_WAIT_LR  \
  0x20 /* GROUP: Modem                           */,\
  0x01 /* NUM_PROPS                              */,\
  0x2D /* START_PROP                             */,\
  0x12 /* MODEM_AFC_WAIT,SHWAIT[7:4],LGWAIT[3:0] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_WAIT_NR RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_WAIT_LR
#define RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_WAIT_HR  \
  0x20 /* GROUP: Modem                           */,\
  0x01 /* NUM_PROPS                              */,\
  0x2D /* START_PROP                             */,\
  0x23 /* MODEM_AFC_WAIT,SHWAIT[7:4],LGWAIT[3:0] */\



#define RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_LIMITER_LR  \
  0x20 /* GROUP: Modem                              */,\
  0x02 /* NUM_PROPS                                 */,\
  0x30 /* START_PROP                                */,\
  0x07 /* MODEM_AFC_LIMITER,AFCLIM[6:0],AFCLIM[7:0] */,\
  0x5A /* DATA1                                     */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_LIMITER_NR  \
  0x20 /* GROUP: Modem                              */,\
  0x02 /* NUM_PROPS                                 */,\
  0x30 /* START_PROP                                */,\
  0x02 /* MODEM_AFC_LIMITER,AFCLIM[6:0],AFCLIM[7:0] */,\
  0x80 /* DATA1                                     */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_LIMITER_HR  \
  0x20 /* GROUP: Modem                              */,\
  0x02 /* NUM_PROPS                                 */,\
  0x30 /* START_PROP                                */,\
  0x00 /* MODEM_AFC_LIMITER,AFCLIM[6:0],AFCLIM[7:0] */,\
  0x9E /* DATA1                                     */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_RFPD_DECAY_LR  \
  0x20 /* GROUP: Modem                         */,\
  0x01 /* NUM_PROPS                            */,\
  0x39 /* START_PROP                           */,\
  0x4A /* MODEM_AGC_RFPD_DECAY,RFPD_DECAY[7:0] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_RFPD_DECAY_NR  \
  0x20 /* GROUP: Modem                         */,\
  0x01 /* NUM_PROPS                            */,\
  0x39 /* START_PROP                           */,\
  0x1A /* MODEM_AGC_RFPD_DECAY,RFPD_DECAY[7:0] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_RFPD_DECAY_HR  \
  0x20 /* GROUP: Modem                         */,\
  0x01 /* NUM_PROPS                            */,\
  0x39 /* START_PROP                           */,\
  0x11 /* MODEM_AGC_RFPD_DECAY,RFPD_DECAY[7:0] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_IFPD_DECAY_LR  \
  0x20 /* GROUP: Modem                         */,\
  0x01 /* NUM_PROPS                            */,\
  0x3A /* START_PROP                           */,\
  0x4A /* MODEM_AGC_IFPD_DECAY,IFPD_DECAY[7:0] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_IFPD_DECAY_NR  \
  0x20 /* GROUP: Modem                         */,\
  0x01 /* NUM_PROPS                            */,\
  0x3A /* START_PROP                           */,\
  0x1A /* MODEM_AGC_IFPD_DECAY,IFPD_DECAY[7:0] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AGC_IFPD_DECAY_HR  \
  0x20 /* GROUP: Modem                         */,\
  0x01 /* NUM_PROPS                            */,\
  0x3A /* START_PROP                           */,\
  0x11 /* MODEM_AGC_IFPD_DECAY,IFPD_DECAY[7:0] */\


#define RADIO_CONFIG_SET_PROPERTY_MODEM_SPIKE_DET_LR  \
  0x20 /* GROUP: Modem                                            */,\
  0x01 /* NUM_PROPS                                               */,\
  0x54 /* START_PROP                                              */,\
  0x03 /* MODEM_SPIKE_DET,SPIKE_DETECT_EN[7],SPIKE_THRESHOLD[6:0] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_SPIKE_DET_NR  \
  0x20 /* GROUP: Modem                                            */,\
  0x01 /* NUM_PROPS                                               */,\
  0x54 /* START_PROP                                              */,\
  0x04 /* MODEM_SPIKE_DET,SPIKE_DETECT_EN[7],SPIKE_THRESHOLD[6:0] */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_SPIKE_DET_HR  \
  0x20 /* GROUP: Modem                                            */,\
  0x01 /* NUM_PROPS                                               */,\
  0x54 /* START_PROP                                              */,\
  0x03 /* MODEM_SPIKE_DET,SPIKE_DETECT_EN[7],SPIKE_THRESHOLD[6:0] */\

#define RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_CHANNEL_STEP_SIZE_LR  \
  0x40 /* GROUP: Freq control                                                          */,\
  0x02 /* NUM_PROPS                                                                    */,\
  0x04 /* START_PROP                                                                   */,\
  0x27 /* FREQ_CONTROL_CHANNEL_STEP_SIZE,CHANNEL_STEP_SIZE[7:0],CHANNEL_STEP_SIZE[7:0] */,\
  0x62 /* DATA1                                                                        */\


/*******************************************************************
  				FREQUENCY DEPENDENT	DATA RATE
********************************************************************/


#define RADIO_CONFIG_SET_PROPERTY_MODEM_FREQ_DEV_868_LR  \
  0x20 /* GROUP: Modem                                        */,\
  0x03 /* NUM_PROPS                                           */,\
  0x0A /* START_PROP                                          */,\
  0x00 /* MODEM_FREQ_DEV,FREQDEV[0],FREQDEV[7:0],FREQDEV[7:0] */,\
  0x00 /* DATA1                                               */,\
  0xC2 /* DATA2                                               */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_FREQ_DEV_868_NR  \
  0x20 /* GROUP: Modem                                        */,\
  0x03 /* NUM_PROPS                                           */,\
  0x0A /* START_PROP                                          */,\
  0x00 /* MODEM_FREQ_DEV,FREQDEV[0],FREQDEV[7:0],FREQDEV[7:0] */,\
  0x07 /* DATA1                                               */,\
  0xE0 /* DATA2                                               */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_FREQ_DEV_868_HR  \
  0x20 /* GROUP: Modem                                        */,\
  0x03 /* NUM_PROPS                                           */,\
  0x0A /* START_PROP                                          */,\
  0x00 /* MODEM_FREQ_DEV,FREQDEV[0],FREQDEV[7:0],FREQDEV[7:0] */,\
  0x06 /* DATA1                                               */,\
  0x90 /* DATA2                                               */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_FREQ_DEV_433_LR  \
  0x20 /* GROUP: Modem                                        */,\
  0x03 /* NUM_PROPS                                           */,\
  0x0A /* START_PROP                                          */,\
  0x00 /* MODEM_FREQ_DEV,FREQDEV[0],FREQDEV[7:0],FREQDEV[7:0] */,\
  0x01 /* DATA1                                               */,\
  0x22 /* DATA2                                               */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_FREQ_DEV_433_NR  \
  0x20 /* GROUP: Modem                                        */,\
  0x03 /* NUM_PROPS                                           */,\
  0x0A /* START_PROP                                          */,\
  0x00 /* MODEM_FREQ_DEV,FREQDEV[0],FREQDEV[7:0],FREQDEV[7:0] */,\
  0x0B /* DATA1                                               */,\
  0xD0 /* DATA2                                               */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_FREQ_DEV_433_HR  \
  0x20 /* GROUP: Modem                                        */,\
  0x03 /* NUM_PROPS                                           */,\
  0x0A /* START_PROP                                          */,\
  0x00 /* MODEM_FREQ_DEV,FREQDEV[0],FREQDEV[7:0],FREQDEV[7:0] */,\
  0x09 /* DATA1                                               */,\
  0xD8 /* DATA2												  */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_GAIN_868_LR  \
  0x20 /* GROUP: Modem                                                               */,\
  0x02 /* NUM_PROPS                                                                  */,\
  0x2E /* START_PROP                                                                 */,\
  0x80 /* MODEM_AFC_GAIN,ENAFC[7],AFCBD[6],AFC_GAIN_DIV[5],AFCGAIN[4:0],AFCGAIN[7:0] */,\
  0x30 /* DATA1                                                                      */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_GAIN_868_NR  \
  0x20 /* GROUP: Modem                                                               */,\
  0x02 /* NUM_PROPS                                                                  */,\
  0x2E /* START_PROP                                                                 */,\
  0x81 /* MODEM_AFC_GAIN,ENAFC[7],AFCBD[6],AFC_GAIN_DIV[5],AFCGAIN[4:0],AFCGAIN[7:0] */,\
  0x18 /* DATA1                                                                      */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_GAIN_868_HR  \
  0x20 /* GROUP: Modem                                                               */,\
  0x02 /* NUM_PROPS                                                                  */,\
  0x2E /* START_PROP                                                                 */,\
  0x86 /* MODEM_AFC_GAIN,ENAFC[7],AFCBD[6],AFC_GAIN_DIV[5],AFCGAIN[4:0],AFCGAIN[7:0] */,\
  0x90 /* DATA1                                                                      */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_GAIN_433_LR  \
  0x20 /* GROUP: Modem                                                               */,\
  0x02 /* NUM_PROPS                                                                  */,\
  0x2E /* START_PROP                                                                 */,\
  0x82 /* MODEM_AFC_GAIN,ENAFC[7],AFCBD[6],AFC_GAIN_DIV[5],AFCGAIN[4:0],AFCGAIN[7:0] */,\
  0x30 /* DATA1                                                                      */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_GAIN_433_NR  \
  0x20 /* GROUP: Modem                                                               */,\
  0x02 /* NUM_PROPS                                                                  */,\
  0x2E /* START_PROP                                                                 */,\
  0x81 /* MODEM_AFC_GAIN,ENAFC[7],AFCBD[6],AFC_GAIN_DIV[5],AFCGAIN[4:0],AFCGAIN[7:0] */,\
  0x18 /* DATA1                                                                      */\

#define RADIO_CONFIG_SET_PROPERTY_MODEM_AFC_GAIN_433_HR  \
  0x20 /* GROUP: Modem                                                               */,\
  0x02 /* NUM_PROPS                                                                  */,\
  0x2E /* START_PROP                                                                 */,\
  0x86 /* MODEM_AFC_GAIN,ENAFC[7],AFCBD[6],AFC_GAIN_DIV[5],AFCGAIN[4:0],AFCGAIN[7:0] */,\
  0x90 /* DATA1                                                                      */\


#define RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_868_LR_01  \
  0x40 /* GROUP: Freq control                             */,\
  0x03 /* NUM_PROPS                                       */,\
  0x01 /* START_PROP                                      */,\
  0x0B /* FREQ_CONTROL_FRAC,FRAC[3:0],FRAC[7:0],FRAC[7:0] */,\
  0x15 /* DATA1                                           */,\
  0xA9 /* DATA2                                           */\

#define RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_868_LR_02  \
  0x11 /* CMD: Set property                               */,\
  0x40 /* GROUP: Freq control                             */,\
  0x03 /* NUM_PROPS                                       */,\
  0x01 /* START_PROP                                      */,\
  0x0F /* FREQ_CONTROL_FRAC,FRAC[3:0],FRAC[7:0],FRAC[7:0] */,\
  0x01 /* DATA1                                           */,\
  0xF8 /* DATA2                                           */\

#define RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_868_NR  \
  0x40 /* GROUP: Freq control                             */,\
  0x03 /* NUM_PROPS                                       */,\
  0x01 /* START_PROP                                      */,\
  0x0B /* FREQ_CONTROL_FRAC,FRAC[3:0],FRAC[7:0],FRAC[7:0] */,\
  0x23 /* DATA1                                           */,\
  0x72 /* DATA2                                           */\

#define RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_868_HR RADIO_CONFIG_SET_PROPERTY_FREQ_CONTROL_FRAC_868_NR


/*******************************************************************
  				SYNC WORD
********************************************************************/

//Although the Sync Word byte(s) are transmitted/received in descending order (i.e., Byte 3 first, followed by Byte 2, etc.), each byte is transmitted/received in little-endian fashion (i.e., least significant bit first).
#define RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS0_0  \
  0x11 /* GROUP: Sync                                       */,\
  0x04 /* NUM_PROPS                                         */,\
  0x01 /* START_PROP                                        */,\
  0x67 /* SYNC_BITS,BITS[7:0],BITS[7:0],BITS[7:0],BITS[7:0] */,\
  0x0B /* DATA1                                             */,\
  0x00 /* DATA2                                             */,\
  0x00 /* DATA3                                             */\


#define RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS0_1  \
  0x11 /* GROUP: Sync                                       */,\
  0x04 /* NUM_PROPS                                         */,\
  0x01 /* START_PROP                                        */,\
  0xD0 /* SYNC_BITS,BITS[7:0],BITS[7:0],BITS[7:0],BITS[7:0] */,\
  0xE6 /* DATA1                                             */,\
  0x00 /* DATA2                                             */,\
  0x00 /* DATA3                                             */\

#define RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS1_0  \
  0x11 /* GROUP: Sync                                       */,\
  0x04 /* NUM_PROPS                                         */,\
  0x01 /* START_PROP                                        */,\
  0x2F /* SYNC_BITS,BITS[7:0],BITS[7:0],BITS[7:0],BITS[7:0] */,\
  0x19 /* DATA1                                             */,\
  0x00 /* DATA2                                             */,\
  0x00 /* DATA3                                             */\


#define RADIO_CONFIG_SET_PROPERTY_SYNC_BITS_CS1_1  \
  0x11 /* GROUP: Sync                                       */,\
  0x04 /* NUM_PROPS                                         */,\
  0x01 /* START_PROP                                        */,\
  0x98 /* SYNC_BITS,BITS[7:0],BITS[7:0],BITS[7:0],BITS[7:0] */,\
  0xF4 /* DATA1                                             */,\
  0x00 /* DATA2                                             */,\
  0x00 /* DATA3                                             */\


#define RADIO_CONFIG_SET_PROPERTY_MODEM_RSSI_CONTROL \
  0x20 /* GROUP: Modem                                                        */,\
  0x01 /* NUM_PROPS                                                           */,\
  0x4C /* START_PROP                                                          */,\
  0x02 /* MODEM_RSSI_CONTROL,CHECK_THRESH_AT_LATCH[5],AVERAGE[4:3],LATCH[2:0] */\


#define RADIO_CONFIG_SET_PROPERTY_MODEM_RSSI_CONTROL2 \
  0x20 /* GROUP: Modem                                                                          */,\
  0x01 /* NUM_PROPS                                                                             */,\
  0x4D /* START_PROP                                                                            */,\
  0x00 /* MODEM_RSSI_CONTROL2,RSSIJMP_DWN[5],RSSIJMP_UP[4],ENRSSIJMP[3],JMPDLYLEN[2],ENJMPRX[1] */\



#define  RADIO_CONFIG_SET_PROPERTY_PTI_ENABLE \
  0xF0 /* GROUP: PTI                                                                          	*/,\
  0x04 /* NUM_PROPS                                                                             */,\
  0x00 /* START_PROP                                                                            */,\
  0x80 /* PTI_EN                                                                            	*/,\
  0x13 /* BAUD 0x13            -> 500 kbaud                    									*/,\
  0x88 /* BAUD 0x88                                           									*/,\
  0x60 /* PTI_LOG_EN: RX_EN, TX_EN                             									*/\

#endif /* FRAMEWORK_HAL_CHIPS_SI4460_EZRADIODRV_SI4460_REGISTERS_H_ */
