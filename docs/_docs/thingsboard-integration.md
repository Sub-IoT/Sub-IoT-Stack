---
title: Integrating gateways with ThingsBoard platform
permalink: /docs/thingsboard-integration/
---

## Note

This integration is deprecated. This is mainly left here as an example but should probably be updated for use.

## Introduction

This is not strictly related to OSS-7 in terms of stack development, but it can be useful if you want to deploy a number of OSS-7 gateways and build an application on top of this. [ThingsBoard](https://thingsboard.io/) is an open source IoT platform for managing devices, data storage and visualization.
The solution exists of 3 parts: gateway(s), the ThingsBoard platform, and (optionally) an application.
It does not require customization of the ThingsBoard platform code or extra plug-ins, using the live version or a stock installation is sufficient. Please refer to ThingsBoard's [getting started documentation](https://thingsboard.io/docs/getting-started-guides/helloworld/).

## Gateway setup

The [oss7-thingsboard-gateway](https://github.com/MOSAIC-LoPoW/oss7-thingsboard-gateway) repository contains a python gateway implementation which interfaces with the DASH7 modem module using ALP commands on the one hand and interfaces with the ThingsBoard platform using it's MQTT API on the other hand. The gateway is responsible for communicating with the platform, it handles reconnection, telemetry upload, RPC etc. The D7 gateway script transparently passes all received ALP commands to ThingsBoard for further processing. On boot it will read all system files of the gateway's modem, making sure they are stored in the platform as well. Finally, it will listen for RPC commands received from the platform, for example to execute an ALP command (more on this later).

After following the [README](https://github.com/MOSAIC-LoPoW/oss7-thingsboard-gateway) your gateway device and the modem device in your gateway should be visible in the platform. When opening a device the last state is visible in the attributes. As can be seen in the screenshot below this contains the D7AP system files (for GW nodes), among other attributes. The nodes received by the gateways are automatically added to the list of devices, and file updates coming from these nodes are stored as well. The platform contains a 'digital twin' for each device with the last known state.

![devices]({{ site.baseurl }}/img/tb-devices.png)

Using this data it becomes possible to build simple dashboards in ThingsBoard without writing custom code, as shown below:

![dashboard]({{ site.baseurl }}/img/tb-dashboard.png)

By default, the user files are not parsed at the gateway side but transmitted as raw bytes, since the content of these files is not specified in the standard (as opposed to the system files, which are parsed at the gateway). This also means that these values cannot be visualized in the platform as is. There are multiple options available:
- extend gateway.py to parse the files (according to your application) and transmit parsed values to the platform
- write a ThingsBoard plugin which parses the data at the platform side
- use an application on top of ThingsBoard which takes the raw data, parses this, and augments the device's data in ThingsBoard with the parsed data using it's API.

We will describe the first and the last option in the next sections respectively.

## Extend the gateway to parse your user files

This is most straightforward way to ensure your user files are parsed and stored as readable attribute on your device. The gateway is extended with a plug-in (written in python) which is responsible for parsing the file data. An example is provided [here](https://github.com/MOSAIC-LoPoW/oss7-thingsboard-gateway/tree/master/plugin-example), basically it boils down to implementing a function `parse_file_data(file_offset, file_data)` which returns the name of the attribute and value. The path to the plug-in can be specified by supplying the `-p <path>` parameter.

## Application integration (optional)

While this option also allows to parse user files (like the option described above) it takes more time to set up. The advantage is that it might be easier to maintain since this parsing logic is only contained in the backed instead of in each gateway.

### Forward to MQTT

As described above the raw ALP commands are stored in ThingsBoard. IN this section we will be configuring ThingsBoard to publish these ALP commands to an (external) MQTT broker. A separate application can then subscribe to this topic, parse the messages and act upon this.
To configure this behavior we first need to add an MQTT plug-in in the plug-in view. The settings of this plug-in need to be modified for your environment, for example pointing to the host on which your broker runs. Next, we need to configure a rule which will forward the correct messages to the MQTT plug-in. This can be done by adding a rule in the rule view.

![devices]({{ site.baseurl }}/img/tb-mqtt-rule.png)

The rule consists of a filter, which makes sure only the ALP commands are forwarded.

![devices]({{ site.baseurl }}/img/tb-mqtt-rule-filter.png)

Finally, we need to specify the action to execute. Because we are using an MQTT plug-in we need to define the topic to publish to. We also need to define the format of the payload, which in this case is JSON containing the device ID and the ALP command.

![devices]({{ site.baseurl }}/img/tb-mqtt-rule-action.png)

After this configuration and activating the plug-in and rule all incoming data should be forwarded to your MQTT broker.

### Parse in application

Now that the ALP commands are forwarded to MQTT we can use a separate application to parse them according to our business logic or system knowledge. An example of this is the `backend-example.py` script in the [oss7-thingsboard-backend-example](https://github.com/MOSAIC-LoPoW/oss7-thingsboard-backend-example) repository. This python script subscribes to the topic and uses [pyd7a](https://github.com/MOSAIC-LoPoW/pyd7a) to parse the file data. Next, it uses the ThingsBoard API to update the device attribute with the (parsed) sensor value, so this data comes available in the platform.

### Execute commands from the application

The solution also allows to execute commands on a gateway node directly from the application layer. A command here is D7AP ALP command, which can be used for example to adapt the active channel class of the node in the gateway, or to query the nodes in the network using ad-hoc synchronization. This is demonstrated in the the `gateway-command-example.py` script in the [oss7-thingsboard-backend-example](https://github.com/MOSAIC-LoPoW/oss7-thingsboard-backend-example) repository. This script uses the ThingsBoard API to execute a RPC call which is forwarded to the gateway device by ThingsBoard. The gateway will then execute the ALP command.
