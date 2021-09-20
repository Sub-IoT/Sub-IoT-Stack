---
title: Running the examples
permalink: /docs/running-examples/
---

In this section we are going to show how to run the standard examples to get an end-to-end system running.
We will show different communication schemes, as introduced in the [previous section]({{ site.baseurl }}{% link _docs/D7AP-intro.md %}).

For the rest of this section we assume you have 2 supported [boards]({{ site.baseurl }}{% link _docs/hardware.md %}) ready,
and you are able to [build]({{ site.baseurl }}{% link _docs/building.md %}) the `gateway`, `sensor_push`, `sensor_action` and `sensor_pull` sample applications.
Also make sure to check the platform notes specific for your platform (linked from [here]({{ site.baseurl }}{% link _docs/hardware.md %})), for more information on how to attach and configure your platform.

# Push communication

This example shows the use case of a sensor pushing data to a gateway which is always listening.
Make sure to flash one board with the `sensor_push` firmware and another one with the `gateway` firmware.
The `sensor_push` example broadcasts sensor values every 10 seconds. The `gateway` will receive the packets,
and transmit the Application Layer Protocol (ALP) payload (see [D7AP intro]({{ site.baseurl }}{% link _docs/D7AP-intro.md %}) for more info) over the serial console (see the platform notes for your specific platform to find out how to access the serial console) to your PC.

This payload is a binary format. We will be using [pyd7a](https://github.com/Sub-IoT/pyd7a) for parsing this, so make sure to get and install this as described in the README.md .
After installation, you can use the `unsolicited_response_logger.py` script to connect with your gateway using a serial port and print the received data:

	$ PYTHONPATH=. python -u examples/unsolicited_response_logger.py -d /dev/ttyACM1
	connected to /dev/ttyACM1, node UID b570000091418 running D7AP v1.1, application "gatewa" with git sha1 73f0c73
	Command with tag 136 (executing)
	        actions:
	                action: ReturnFileData: file-id=64, size=1, offset=0, length=8, data=[86, 246, 250, 0, 0, 0, 0, 0]
	        interface status: interface-id=215, status=unicast=False, nls=False, retry=False, missed=False, fifo_token=10, rx_level=40, seq_nr=0, target_rx_level=80, addressee=ac=1, id_type=IdType.UID, id=0x3237303400630011L, response_to=exp=3 mant17, link_budget=50, channel_header=coding=ChannelCoding.PN9, class=ChannelClass.NORMAL_RATE, band=ChannelBand.BAND_868

	Command with tag 112 (executing)
	        actions:
	                action: ReturnFileData: file-id=64, size=1, offset=0, length=8, data=[87, 30, 251, 0, 0, 0, 0, 0]
	        interface status: interface-id=215, status=unicast=False, nls=False, retry=False, missed=False, fifo_token=118, rx_level=40, seq_nr=0, target_rx_level=80, addressee=ac=1, id_type=IdType.UID, id=0x3237303400630011L, response_to=exp=3 mant17, link_budget=50, channel_header=coding=ChannelCoding.PN9, class=ChannelClass.NORMAL_RATE, band=ChannelBand.BAND_868

The raw sensor data is contained in the `data` field. The other fields contain for instance the sensor node UID, reception level and link budget, channel information etc. For now, we are not going to dive into this in more detail, refer to the D7A specification for more info.

# Pull communication

In this example the sensor does not push sensor data to gateway(s) continuously, but instead writes the sensor value to a local file,
which can then be fetched on request. The sensor will sniff the channel every second for background ad hoc synchronization frames, to be able to receive requests from other nodes. The gateway will synchronize all nodes in the network using ad hoc synchronization frames, after which it will send the query in a foreground frame. For this example we need one node running the `gateway` application and one or more node(s) running `sensor_pull`. For executing the query we will be using the `query_nodes.py` example which is provided by pyd7a.
Running this script by providing the serial device of the gateway will show something like this:

	$ PYTHONPATH=. python2 -u examples/query_nodes.py -d /dev/ttyACM1
	connected to /dev/ttyACM1, node UID 433731340037002b running D7AP v1.1, application "gatewa" with git sha1 a8fe586
	Executing query...
	Command with tag 52 (executing)
	        actions:
	                action: ReturnFileData: file-id=64, size=1, offset=0, length=8, data=[14, 200, 0, 0, 0, 0, 0, 0]
	        interface status: interface-id=215, status=unicast=False, nls=False, retry=False, missed=False, fifo_token=239, rx_level=26, seq_nr=0, target_rx_level=80, addressee=ac=17, id_type=IdType.UID, id=0x41303039002f002aL, response_to=exp=0 mant0, link_budget=36, channel_header=coding=ChannelCoding.PN9, class=ChannelClass.NORMAL_RATE, band=ChannelBand.BAND_868, channel_index=0

	Command with tag 52 (executing)
	        actions:
	                action: ReturnFileData: file-id=64, size=1, offset=0, length=8, data=[14, 24, 1, 0, 0, 0, 0, 0]
	        interface status: interface-id=215, status=unicast=False, nls=False, retry=False, missed=False, fifo_token=239, rx_level=23, seq_nr=0, target_rx_level=80, addressee=ac=17, id_type=IdType.UID, id=0x41303039002c003dL, response_to=exp=0 mant0, link_budget=33, channel_header=coding=ChannelCoding.PN9, class=ChannelClass.NORMAL_RATE, band=ChannelBand.BAND_868, channel_index=0

	Command with tag 52 (completed, without error)

The script connects to the modem and then executes a query which requests the sensor data file from all nodes. In this case we can see we get multiple response commands on executing our query; 2 nodes are answering before the query completes. The requested data (in this case the first 8 bytes of file-id 64) is displayed (in the data=[] field), together with metadata supplied by the DASH7 interface like the UID or link budget.

# What's next

The examples described above show the push and pull communication schemes. In the future this will be extended to show how to use dormant sessions etc. We will also provide more info on how to configure the DASH7 modem itself, covering aspects like the frequency band, the channel, QoS settings, channel scanning, frequency agility. Furthermore, expect more documentation on how to integrate a DASH7 stack into your own application.
