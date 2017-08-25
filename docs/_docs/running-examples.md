---
title: Running the examples
permalink: /docs/running-examples/
---

In this section we are going to show how to run the standard examples to get an end-to-end system running.
For now we will use the case of a sensor pushing data to a gateway which is always listening.

DASH7 supports more communication schemes (see [resources]({{ site.baseurl }}/resources/index.html)) on more background about DASH7) but we will limit ourselves to this simple case for now.

For the rest of this section we assume you have 2 supported [boards]({{ site.baseurl }}{% link _docs/hardware.md %}) ready,
and you are able to [build]({{ site.baseurl }}{% link _docs/building.md %}) the `gateway` and `sensor_push` sample applications.
Also make sure to check the platform notes specific for your platform (linked from [here]({{ site.baseurl }}{% link _docs/hardware.md %})), for more information on how to attach and configure your platform.

# Flashing

cmake will generate targets for flashing each application using JLink, by running `make flash-<appname>`.
If you are not using a JLink adapter you can of course flash the binary manually. For instance, if you want to
use the embedded ST-LINK on the B_L072Z_LRWAN1 you can copy over the generated bin file from `<build-dir>/apps/<app-name>/<app-name>.bin` to the mass storage device, as explained in the [B_L072Z_LRWAN1 platform notes]({{ site.baseurl }}{% link _docs/platform-lrwan1.md %}).
Otherwise, if you are using JLink just use the make target like this:

	$ make flash-sensor_push
	[100%] Built target sensor_push.elf
	Scanning dependencies of target flash-sensor_push
	SEGGER J-Link Commander V6.18a (Compiled Aug 11 2017 17:53:58)
	<snip>
	Downloading file [sensor_push.bin]...
	Comparing flash   [100%] Done.
	Erasing flash     [100%] Done.
	Programming flash [100%] Done.
	Verifying flash   [100%] Done.
	<snip>
	[100%] Built target flash-sensor_push

If all went well the application should be running on the target.

Make sure to flash one board with the `sensor_push` firmware and another one with the `gateway` firmware.

# Receiving the sensor data

The `sensor_push` example broadcasts sensor values every 10 seconds. The `gateway` will receive the packets,
and transmit the Application Layer Protocol (ALP) payload (see [resources]({{ site.baseurl }}/resources/index.html)) for more info) over the serial console (see the platform notes for your specific platform to find out how to access the serial console) to your PC.

This payload is a binary format. We will be using [pyd7a](https://github.com/MOSAIC-LoPoW/pyd7a) for parsing this, so make sure to get and install this as described in the README.md .
After installation you can use the `modem-example.py` script to connect with your gateway using a serial port and print the received data:

	$ PYTHONPATH=. python examples/modem_example.py -d /dev/ttyACM1
	connected to /dev/ttyACM1, node UID b570000091418 running D7AP v1.1, application "gatewa" with git sha1 73f0c73
	Command with tag 136 (executing)
	        actions:
	                action: ReturnFileData: file-id=64, size=1, offset=0, length=8, data=[86, 246, 250, 0, 0, 0, 0, 0]
	        interface status: interface-id=215, status=unicast=False, nls=False, retry=False, missed=False, fifo_token=10, rx_level=40, seq_nr=0, target_rx_level=80, addressee=ac=1, id_type=IdType.UID, id=0x3237303400630011L, response_to=exp=3 mant17, link_budget=50, channel_header=coding=ChannelCoding.PN9, class=ChannelClass.NORMAL_RATE, band=ChannelBand.BAND_868

	Command with tag 112 (executing)
	        actions:
	                action: ReturnFileData: file-id=64, size=1, offset=0, length=8, data=[87, 30, 251, 0, 0, 0, 0, 0]
	        interface status: interface-id=215, status=unicast=False, nls=False, retry=False, missed=False, fifo_token=118, rx_level=40, seq_nr=0, target_rx_level=80, addressee=ac=1, id_type=IdType.UID, id=0x3237303400630011L, response_to=exp=3 mant17, link_budget=50, channel_header=coding=ChannelCoding.PN9, class=ChannelClass.NORMAL_RATE, band=ChannelBand.BAND_868

The raw sensor data is contained in the `data` field. The other fields contain for instance the sensor node UID, reception level and link budget, channel information etc. For now we are not going to dive into this in more detail, refer to the D7A specification for more info.

pyd7a also contains a webgui to interface with a serial DASH7 modem. Using this GUI you can also view the incoming packets.

![modem-webgui log]({{site.baseurl}}/img/modem-webgui-log.png)

This can be started using:

	PYTHONPATH=. python modem-webgui/modem-webgui.py -d /dev/ttyACM1

Besides these tools pyd7a naturally allows to access the modem programmatically and thus integrate into your own system.

# What's next

The example described above is only the simplest case. In the future this will be extended to show how to use other communication schemes like querying using low-power wake up, using dormant sessions etc. We will also provide more info on how to configure the DASH7 modem itself, covering aspects like the frequency band, the channel, QoS settings, channel scanning, frequency agility. Furthermore, expect more documentation on how to integrate a DASH7 stack into your own application.
