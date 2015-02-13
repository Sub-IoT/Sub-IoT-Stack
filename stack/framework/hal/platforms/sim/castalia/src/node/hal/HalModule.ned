package node.hal;


simple HalModule
{
 parameters:
	bool collectTraceInfo = default (false);
	bool traceToDebug	  = default (false);
	double interTaskDelay = default (0.000100); // 100µs
	double taskExecutionTime  = 	default (0);		//No delay yet until we figure out a good model for this
	
	
 gates:
 	output toCommunicationModule;
	output toSensorDeviceManager;
	input fromCommunicationModule;
	input fromSensorDeviceManager;
	input fromResourceManager;
}

