/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Yuri Tselishchev                                             *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *  
 ****************************************************************************/

#ifndef _CARSPHYSICALPROCESS_H_
#define _CARSPHYSICALPROCESS_H_

#define SIMTIME_STEP 0.01

#include "CastaliaModule.h"
#include "PhysicalProcessMessage_m.h"

using namespace std;

typedef struct {
	simtime_t time;
	double x;
	double y;
} sourceSnapshot;

class CarsPhysicalProcess: public CastaliaModule {
 private:
	bool printDebugInfo;

	int max_num_cars;
	double car_speed;
	double car_value;
	double car_interarrival;
	double point1_x_coord;
	double point1_y_coord;
	double point2_x_coord;
	double point2_y_coord;

	double road_length;
	sourceSnapshot **sources_snapshots;	// N by M array, where N is numSources and, M is the 
										// maximum number of source snapshots. A source 
										// snapshot is a tuple (time, x, y, value)
	const char *description;

	double defaultValue;
	double *valuesTable;

 protected:
	virtual void initialize();
	virtual void handleMessage(cMessage * msg);
	virtual void finishSpecific();
	double calculateScenarioReturnValue(const double &x_coo,
					    const double &y_coo, const simtime_t & stime);
	void readIniFileParameters(void);
};

#endif
