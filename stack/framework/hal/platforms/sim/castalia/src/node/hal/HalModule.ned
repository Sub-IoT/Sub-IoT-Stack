package node.hal;


simple HalModule
{
 parameters:
	bool collectTraceInfo = default (true);

 gates:
 	output toCommunicationModule;
	output toSensorDeviceManager;
	input fromCommunicationModule;
	input fromSensorDeviceManager;
	input fromResourceManager;
}

