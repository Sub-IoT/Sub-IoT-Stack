/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Yuriy Tselishchev                                            *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *  
 ****************************************************************************/

#ifndef CASTALIA_MESSAGES
#define CASTALIA_MESSAGES

#define BROADCAST_MAC_ADDRESS -1
#define BROADCAST_NETWORK_ADDRESS "-1"
#define SINK_NETWORK_ADDRESS "SINK"
#define PARENT_NETWORK_ADDRESS "PARENT"

enum CastaliaMessageTypes {
	NODE_STARTUP = 1,
	TIMER_SERVICE = 2,

	OUT_OF_ENERGY = 3,
	DESTROY_NODE = 4,

	SENSOR_READING_MESSAGE = 5,
	RESOURCE_MANAGER_DRAW_POWER = 6,
	PHYSICAL_PROCESS_SAMPLING = 7,

	WC_SIGNAL_START = 9,
	WC_SIGNAL_END = 10,
	WC_NODE_MOVEMENT = 11,

	RADIO_CONTROL_MESSAGE = 21,
	RADIO_CONTROL_COMMAND = 22,
	RADIO_ENTER_STATE = 23,
	RADIO_CONTINUE_TX = 24,

	MAC_LAYER_PACKET = 30,
	MAC_CONTROL_MESSAGE = 31,
	MAC_CONTROL_COMMAND = 32,

	NETWORK_LAYER_PACKET = 40,
	NETWORK_CONTROL_MESSAGE = 41,
	NETWORK_CONTROL_COMMAND = 42,

	APPLICATION_PACKET = 50
};

#endif
