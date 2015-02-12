cplusplus {{
#include <cstdint>
}}

packet RadioPacket {
	@customize(true);
	uint8_t buffer[];
	
	//for now only RSSI & LQI: may add other things later (such as time-of-arrival, ...)
	double RSSI;
	double LQI;
} 

cplusplus {{
//Include the 'extended' class definition
#include "RadioPacket.h"
}}
