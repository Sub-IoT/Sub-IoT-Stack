//********************************************************************************
//*  Copyright: National ICT Australia,  2007 - 2010                             *
//*  Developed at the ATP lab, Networked Systems research theme                  *
//*  Author(s): Athanassios Boulis, Dimosthenis Pediaditakis                     *
//*  This file is distributed under the terms in the attached LICENSE file.      *
//*  If you do not find this file, copies can be found by writing to:            *
//*                                                                              *
//*      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia                 *
//*      Attention:  License Inquiry.                                            *
//*                                                                              *
//*******************************************************************************/

package node.sensorManager;

// The sensor node module. Connects to the wireless channel in order to communicate 
// with other nodes. Connects to psysical processes so it can sample them. 
// Default values represent temperature sensor

simple SensorManager {
 parameters:
	bool collectTraceInfo = default (false);
	int numSensingDevices = default (1);		// how many sensing devices the node has 
												// (has to be <= SensorNetwork.numPhysicalProcesses)

	string pwrConsumptionPerDevice = default ("0.02");

	string sensorTypes = default ("Temperature");	// Humidity OR Temperature OR Light

	string corrPhyProcess = default ("0");		//holds the indexes of the corresponding phy processes for
												//each sensor (usually it should be : "0 1 2 3")

	string maxSampleRates = default ("1");		//number of samples per second for each sensor

	string devicesBias = default ("0.1");		//If the output signal is not zero when the measured property is zero

	string devicesDrift = default ("");			//If the output signal slowly changes independent of the 
												//measured property
												
	string devicesNoise = default ("0.1");		//random deviation of the signal that varies in time

	string devicesHysterisis = default ("");	//the sensor not instantly follows the change of the property 
												//being measured and therefore involves the history of the 
												//measured property

	string devicesSensitivity = default ("0");	//holds the minimum value which can be sensed by each sensing device.

	string devicesResolution = default ("0.001");	//holds the sensing resolution for each device 
													//(the returned value will always be a multiple of 
													//number, given here)
													
	string devicesSaturation = default ("1000");	//holds the saturation value for each sensing device

 gates:
	output toApplicationModule;
	output toNodeContainerModule[];
	input fromNodeContainerModule[];
	input fromApplicationModule;
	input fromResourceManager;
}

