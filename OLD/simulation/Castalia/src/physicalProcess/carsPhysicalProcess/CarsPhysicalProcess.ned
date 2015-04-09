//********************************************************************************
//*  Copyright: National ICT Australia,  2007 - 2010                             *
//*  Developed at the ATP lab, Networked Systems research theme                  *
//*  Author(s): Yuriy Tselishchev                                                *
//*  This file is distributed under the terms in the attached LICENSE file.      *
//*  If you do not find this file, copies can be found by writing to:            *
//*                                                                              *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                 *
//*      Attention:  License Inquiry.                                            *
//*                                                                              *
//*******************************************************************************/

package physicalProcess.carsPhysicalProcess;

// The physical process module simulates a physical process that could be measured/sampled
// by a sensing device on the nodes. Different sensing devices (e.g temperature, pressure,
// light, acceleration) would be represented by distinct PhysicalProcess modules. A node
// simply ask the process for a sample which is returned as soon as it is calculated.

simple CarsPhysicalProcess like physicalProcess.iPhysicalProcess {
 parameters:
	bool collectTraceInfo = default (false);

	int max_num_cars = default (5);			// how many cars can be present at once
	double car_speed = default (16.0);		// in meters per second
	double car_value = default (30.0);		// abstract value produced by moving car, this will be passed to sensors
	double car_interarrival = default (20);	// average interval between car's arrival, in seconds           

	double point1_x_coord;
	double point2_x_coord;
	double point1_y_coord;
	double point2_y_coord;

	string description = default ("Moving cars");

 gates:
	output toNode[];
	input fromNode[];
}
