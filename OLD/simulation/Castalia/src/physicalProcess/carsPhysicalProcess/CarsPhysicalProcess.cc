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

#include "CarsPhysicalProcess.h"

#define K_PARAM 0.1
#define A_PARAM 1

Define_Module(CarsPhysicalProcess);

void CarsPhysicalProcess::initialize()
{
	readIniFileParameters();

	road_length =
	    sqrt((point1_x_coord - point2_x_coord) * (point1_x_coord - point2_x_coord) +
			 (point1_y_coord - point2_y_coord) * (point1_y_coord - point2_y_coord));

	int i, j;
	sources_snapshots = new sourceSnapshot *[max_num_cars];
	for (i = 0; i < max_num_cars; i++) {
		sources_snapshots[i] = new sourceSnapshot[2];
		for (j = 0; j < 2; j++) {
			sources_snapshots[i][j].time = -1;
		}
	}

	double arrival = genk_dblrand(0) * car_interarrival + car_interarrival / 2;
	trace() << "First car arrival at " << arrival;
	scheduleAt(arrival,	new cMessage("New car arrival message", TIMER_SERVICE));

	declareOutput("Cars generated on the road");
}

void CarsPhysicalProcess::handleMessage(cMessage * msg)
{
	switch (msg->getKind()) {
		case PHYSICAL_PROCESS_SAMPLING: {
			PhysicalProcessMessage *phyMsg = check_and_cast < PhysicalProcessMessage * >(msg);
			// int nodeIndex = phyMsg->getSrcID();
			// int sensorIndex = phyMsg->getSensorIndex();

			// get the sensed value based on node location
			phyMsg->setValue(calculateScenarioReturnValue(
				phyMsg->getXCoor(), phyMsg->getYCoor(), phyMsg->getSendingTime()));
			// Send reply back to the node who made the request
			send(phyMsg, "toNode", phyMsg->getSrcID());
			return;
		}

		case TIMER_SERVICE: {
			int pos = -1;
			for (int i = 0; pos == -1 && i < max_num_cars; i++) {
				if (sources_snapshots[i][1].time < simTime())
					pos = i;
			}

			if (pos != -1) {
				trace() << "New car arrives on the bridge, index " << pos;
				if (genk_dblrand(0) > 0.5) {
					sources_snapshots[pos][0].x = point1_x_coord;
					sources_snapshots[pos][0].y = point1_y_coord;
					sources_snapshots[pos][1].x = point2_x_coord;
					sources_snapshots[pos][1].y = point2_y_coord;
				} else {
					sources_snapshots[pos][0].x = point2_x_coord;
					sources_snapshots[pos][0].y = point2_y_coord;
					sources_snapshots[pos][1].x = point1_x_coord;
					sources_snapshots[pos][1].y = point1_y_coord;
				}
				sources_snapshots[pos][0].time = simTime();
				sources_snapshots[pos][1].time = simTime() + road_length / car_speed;
				collectOutput("Cars generated on the road");
			}

			double arrival = genk_dblrand(0) * car_interarrival + car_interarrival / 2;
			scheduleAt(simTime() + arrival,	msg);
			return;
		}

		default: {
			opp_error(":\n Physical Process received message other than PHYSICAL_PROCESS_SAMPLING");
		}
	}
}

void CarsPhysicalProcess::finishSpecific()
{
	int i;
	for (i = 0; i < max_num_cars; i++) {
		delete[]sources_snapshots[i];
	}
	delete[]sources_snapshots;
}

void CarsPhysicalProcess::readIniFileParameters(void)
{
	max_num_cars = par("max_num_cars");
	car_speed = par("car_speed");
	car_value = par("car_value");
	car_interarrival = par("car_interarrival");
	point1_x_coord = par("point1_x_coord");
	point1_y_coord = par("point1_y_coord");
	point2_x_coord = par("point2_x_coord");
	point2_y_coord = par("point2_y_coord");
	description = par("description");
}

double CarsPhysicalProcess::calculateScenarioReturnValue(const double &x_coo,
							 const double &y_coo, const simtime_t &stime)
{
	double retVal = 0.0f;
	int i;
	double linear_coeff, distance, x, y;

	for (i = 0; i < max_num_cars; i++) {
		if (sources_snapshots[i][1].time >= stime) {
			linear_coeff = (stime - sources_snapshots[i][0].time) /
			    (sources_snapshots[i][1].time - sources_snapshots[i][0].time);
			x = sources_snapshots[i][0].x + linear_coeff * 
				(sources_snapshots[i][1].x - sources_snapshots[i][0].x);
			y = sources_snapshots[i][0].y + linear_coeff * 
				(sources_snapshots[i][1].y - sources_snapshots[i][0].y);
			distance = sqrt((x_coo - x) * (x_coo - x) +
				 (y_coo - y) * (y_coo - y));
			retVal += pow(K_PARAM * distance + 1, -A_PARAM) * car_value;
		}
	}
	return retVal;
}

