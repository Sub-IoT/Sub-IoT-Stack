
package node.application.oss7Test;

// The sensor node module. Connects to the wireless channel in order to communicate 
// with other nodes. Connects to psysical processes so it can sample them.

simple Oss7Test like node.application.iApplication {
 parameters:
 	string applicationID = default ("OSS7Test");
	bool collectTraceInfo = default (false);
	int packetHeaderOverhead = default (8);
	int priority = default (1);
	int constantDataPayload = default (8);
        bool isBlinker = default (true);
        double phyDelayForValidRssi = default (0);
        double phyDelayForSleep2Rx = default (0);

 gates:
 	output toCommunicationModule;
	output toSensorDeviceManager;
	input fromCommunicationModule;
	input fromSensorDeviceManager;
	input fromResourceManager;
}

