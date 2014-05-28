/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Athanassios Boulis, Dimosthenis Pediaditakis                 *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *  
 ****************************************************************************/

#include "CustomizablePhysicalProcess.h"

Define_Module(CustomizablePhysicalProcess);

void CustomizablePhysicalProcess::initialize()
{
	readIniFileParameters();
	if (inputType == SCENARIO_BASED) {
		readScenariosFromIniFile();
		initHelpStructures();	// Allocate and Initialize sourcesEvolution
		time = -1;
	}
}

void CustomizablePhysicalProcess::handleMessage(cMessage * msg)
{
	if (msg->getKind() != PHYSICAL_PROCESS_SAMPLING)
		opp_error("Physical Process received message other than PHYSICAL_PROCESS_SAMPLING");

	PhysicalProcessMessage *receivedMsg = check_and_cast < PhysicalProcessMessage * >(msg);
	int nodeIndex = receivedMsg->getSrcID();
	// int sensorIndex = receivedMsg->getSensorIndex();
	double returnValue;

	switch (inputType) {
		case DIRECT_NODE_VALUES:{
			returnValue = valuesTable[nodeIndex];
			break;
		}

		case SCENARIO_BASED:{
			returnValue = calculateScenarioReturnValue(
				receivedMsg->getXCoor(),
				receivedMsg->getYCoor(),
				receivedMsg->getSendingTime());
			break;
		}

		case TRACE_FILE:{
			/* 
			 * TODO: add this functionality in the future
			 */
			break;
		}
	}

	// Send reply back to the node who made the request
	receivedMsg->setValue(returnValue);
	send(receivedMsg, "toNode", nodeIndex);
}

void CustomizablePhysicalProcess::finishSpecific()
{
	if (inputType == SCENARIO_BASED) {
		// deallocate sourcesEvolution
		for (int i = 0; i < numSources; i++) {
			delete[]sources_snapshots[i];
		}
		delete[]sources_snapshots;
		delete[]curr_source_state;
		delete[]source_index;
	} else {
		delete[]valuesTable;
	}
}

void CustomizablePhysicalProcess::readIniFileParameters(void)
{
	inputType = par("inputType");
	numNodes = getParentModule()->par("numNodes");

	switch (inputType) {
		case 0: {
			inputType = DIRECT_NODE_VALUES;
			break;
		}

		case 1: {
			inputType = SCENARIO_BASED;
			break;
		}

		case 2: {
			inputType = TRACE_FILE;
			break;
		}

		default: {
			opp_error("\nError! Illegal value of parameter \"inputType\" of CustomizablePhysicalProcess module.\n");
		}
	}

	if (inputType == DIRECT_NODE_VALUES) {
		cStringTokenizer valuesTokenizer(par("directNodeValueAssignment"), " ");
		int totalTokens = 0;
		int isFirstToken = 1;

		while (valuesTokenizer.hasMoreTokens()) {
			totalTokens++;
			string token(valuesTokenizer.nextToken());

			if (isFirstToken) {
				string::size_type posA = token.find("(", 0);
				string::size_type posB = token.find(")", 0);

				if ((posA != 0) || (posB == string::npos))
					opp_error("\nError!(A) Illegal parameter format \"directNodeValueAssignment\" of CustomizablePhysicalProcess module.\n");

				token = token.substr(1, token.size() - 1);

				if (token.size() < 1)
					opp_error("\nError!(B) Illegal parameter format \"directNodeValueAssignment\" of CustomizablePhysicalProcess module.\n");

				defaultValue = atof(token.c_str());
				valuesTable = new double[numNodes];
				for (int i = 0; i < numNodes; i++)
					valuesTable[i] = defaultValue;
				isFirstToken = 0;
			} else {
				string::size_type tokenSize = token.length();
				string::size_type posA = token.find(":");

				if (tokenSize >= 3) {
					if ((posA == string::npos) || (posA == 0) || (posA == tokenSize - 1))
						opp_error("\nError! Illegal parameter format \"directNodeValueAssignment\" of CustomizablePhysicalProcess module.\n");

					string tokA;
					string tokB;
					tokA = token.substr(0, posA);
					tokB = token.substr(posA + 1);

					int nodeId = atoi(tokA.c_str());
					double nodeVal = atof(tokB.c_str());

					if (nodeId < 0 || nodeId > numNodes - 1)
						opp_error("\nError! Illegal parameter format \"directNodeValueAssignment\" of CustomizablePhysicalProcess module.\n");

					valuesTable[nodeId] = nodeVal;
				}
			}
		}
	} else if (inputType == SCENARIO_BASED) {
		k = par("multiplicative_k");
		a = par("attenuation_exp_a");
		sigma = par("sigma");
		max_num_snapshots = par("max_num_snapshots");
		description = par("description");
		numSources = par("numSources");
		if (numSources > 5)
			opp_error("Physical Process parameter \"numSources\" has been initialized with invalid value \"%d\"",
numSources);

		/* 
		 * ALLOCATE memory space for the sourceSnapshot 2D array that holds the list of snapshots for every source
		 */

		int i, j;
		sources_snapshots = new sourceSnapshot *[numSources];
		for (i = 0; i < numSources; i++) {
			sources_snapshots[i] = new sourceSnapshot[max_num_snapshots];
			for (j = 0; j < max_num_snapshots; j++)
				sources_snapshots[i][j].time = -1;
		}
	} else {
		/*
		 * TODO: add this functionality in the future for the reading of the TraceFile.
		 */
	}
}

void CustomizablePhysicalProcess::readScenariosFromIniFile(void)
{
	int i, j;
	int totalValidSourcesScenarios = 0, validated;
	char buffer[100];

	for (i = 0; i < numSources; i++) {	//for every source_X string parameter that contains the scenarios for physical process X
		sprintf(buffer, "source_%d", i);
		cStringTokenizer snapshotTokenizer(par(buffer), ";");
		validated = 0;
		j = 0;		//the snapshot index
		while (snapshotTokenizer.hasMoreTokens()) {	//for every snapshot inside the scenario string DO...
			if (!validated) {
				validated = 1;
				totalValidSourcesScenarios++;	//non empty scenario
			}

			cStringTokenizer valuesTokenizer(snapshotTokenizer.nextToken());	//get the next snapshot and split it with " " as delimiter

			if (valuesTokenizer.hasMoreTokens()) {	//get each one of the snapshot parameters
				sources_snapshots[i][j].time = (simtime_t) atof(valuesTokenizer.nextToken());
				if (!valuesTokenizer.hasMoreTokens())
					opp_error("\nPhysical Process parameter error. Malformed  parameter : source_%d\n", i);

				sources_snapshots[i][j].x = atof(valuesTokenizer.nextToken());
				if (!valuesTokenizer.hasMoreTokens())
					opp_error("\nPhysical Process parameter error. Malformed  parameter : source_%d\n", i);

				sources_snapshots[i][j].y = atof(valuesTokenizer.nextToken());
				if (!valuesTokenizer.hasMoreTokens())
					opp_error("\nPhysical Process parameter error. Malformed  parameter : source_%d\n", i);

				sources_snapshots[i][j].value = atof(valuesTokenizer.nextToken());
				if (valuesTokenizer.hasMoreTokens())
					opp_error("\nPhysical Process parameter error. Malformed  parameter : source_%d\n", i);
			}

			j++;
		}
		if (j > max_num_snapshots)
			opp_error("\nPhysical Process intialization ERROR! You tried to pass more snapshots than the parameter \"max_num_snapshots\" specifies.");
	}
}

void CustomizablePhysicalProcess::initHelpStructures(void)
{
	int i;
	simtime_t starting_time;
	curr_source_state = new sourceSnapshot[numSources];
	source_index = new int[numSources];
	for (i = 0; i < numSources; i++)
		source_index[i] = -1;
}

double CustomizablePhysicalProcess::calculateScenarioReturnValue(const double &x_coo, 
			const double &y_coo, const simtime_t & stime)
{
	int i;
	double retVal, linear_coeff, distance;

	if (stime - time >= SIMTIME_STEP) {
		time = stime;

		// Update the source_index info [that is the current snapshot for each source]
		for (i = 0; i < numSources; i++) {
			if (source_index[i] >= -1) {
				if (time >= sources_snapshots[i][source_index[i] + 1].time)
					source_index[i]++;

				// check if the end is reached
				if (sources_snapshots[i][source_index[i] + 1].time == -1)
					source_index[i] = -2;
			}
		}

		// Update the current state of all sources ==> (x -- y -- value)  
		// with respect to time and the current snapshot od each source
		for (i = 0; i < numSources; i++) {
			if (source_index[i] >= 0) {
				linear_coeff =
				    (time - sources_snapshots[i][source_index[i]].time) /
				    (sources_snapshots[i][source_index[i] + 1].time -
				     sources_snapshots[i][source_index[i]].time);

				curr_source_state[i].x =
				    sources_snapshots[i][source_index[i]].x + linear_coeff *
				    (sources_snapshots[i][source_index[i] + 1].x -
				     sources_snapshots[i][source_index[i]].x);

				curr_source_state[i].y =
				    sources_snapshots[i][source_index[i]].y + linear_coeff *
				    (sources_snapshots[i][source_index[i] + 1].y -
				     sources_snapshots[i][source_index[i]].y);

				curr_source_state[i].value =
				    sources_snapshots[i][source_index[i]].value + linear_coeff *
				    (sources_snapshots[i][source_index[i] + 1].value -
				     sources_snapshots[i][source_index[i]].value);
			}
		}
	}

	/* Now that we know the current state of your process calculate its effect on all the nodes */
	// add all active sources
	retVal = 0.0f;
	for (i = 0; i < numSources; i++) {
		if (source_index[i] >= 0) {
			distance =
			    sqrt((x_coo - curr_source_state[i].x) * (x_coo - curr_source_state[i].x) +
				 	(y_coo - curr_source_state[i].y) * (y_coo - curr_source_state[i].y));
			retVal += pow(k * distance + 1, -a) * curr_source_state[i].value;
		}
	}

	return retVal;
}
