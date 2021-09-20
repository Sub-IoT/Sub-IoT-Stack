---
title: DASH7 Alliance Protocol intro
permalink: /docs/d7ap-intro/
---

On this page we give a brief introduction to the DASH7 Alliance Protocol.
The goal is to provide enough information to understand how the examples work and what the different
options are to get start with D7AP. For more in depth information see [resources]({{ site.baseurl }}/resources).

# Intro

DASH7 Alliance Protocol (D7AP) originates from ISO 18000-7, which specifies an active RFID standard.
D7AP extends this by adding a more general asynchronous MAC to enable more flexible communication than standard RFID requester + responder.
Important design considerations are:
- Bursty: Data transfer is abrupt and does not include content such as video, audio, or other isochronous forms of data.
- Light: For most applications, packet sizes are limited to 256 bytes. Transmission of multiple, consecutive packets may occur but is generally avoided if possible.
- Asynchronous: DASH7's main method of communication is by command-response, which by design requires no periodic network "hand-shaking" or synchronization between devices.
- Stealth: DASH7 devices does not need periodic beaconing to be able to respond in communication.
- Transitive: A DASH7 system of devices is inherently mobile or transitional. Unlike other wireless technologies DASH7 is upload-centric, not download-centric, thus devices do not need to be managed extensively by fixed infrastructure (i.e. base stations).

D7AP uses different sub-GHz bands (433, 868 and 915 MHz) for global availability. The sub-GHz frequencies enable a bigger range compared to 2.4 GHz solutions.
Because of this, D7AP allows to solve a lot of use cases using a star or tree (1 hop) topology, instead of the more power hungry and complex mesh topology.

D7AP specifies all layer of the OSI model, making it very easy to implement a use case since a lot of functionality is provided in the stack. As we will later discuss, you can make an application DASH7 enabled by adding a modem module and interfacing with this using common file operations.

# Filesystem

Central to D7AP is the notion of a filesystem with structured files. Every node has an obligatory set of files,
the system files, which are defined by the spec. The system files contain all the configuration and status of the stack itself.
The user files, which are not defined by the spec, can be used for example for your specific sensor data.
The application using the D7AP stack interacts with the filesystem of the local node and with the filesystem of remote nodes, transparently.
Files also have access rights associated to them, meaning that some files require authentication for reading or writing.
As already mentioned a lot of the behavior of the stack is stored in the system files. Since these files can be adapted remotely (see later), this means that it is possible to change the behavior of an already deployed network.

# Application Layer Protocol (ALP)

ALP is a simple binary protocol which allows you to interact with a filesystem and it's files. ALP provides operations for common file operations
like read, write, execute, create. Additionally, ALP also defines operations for queries (arithmetic or string comparison) and boolean logic, allowing you to query the filesystem and, for instance, only execute a read or write when a certain condition is met.
Another important concept of ALP is the interface. By default, ALP assumes you are using the local filesystem interface, but it can be any transport mechanism like serial, NFC, ... ALP commands can be forwarded to an interface, by providing the interface to use and the (interface specific) configuration.
Besides the local filesystem interface type the spec defines one other interface: the D7A Session Protocol (D7ASP) interface (see later).
By forwarding ALP commands over this interface they are executed against the network of D7A nodes (as specified by the addressee in the supplied D7ASP configuration, see later). This makes it possible to for example read sensor values of remote nodes or change their configuration, in the same way as you would do for a local file. The query operation enables to address nodes in a network in a smart way, depending on the content of their files, instead of addressing the nodes one by one. For instance, you could query the network for all nodes which have a temperature value > 25 degrees, and all nodes for which this query yields true will respond. Functionally, this behaves as a distributed database running on the nodes, which you can query.
ALP commands can be executed using an API when running on the same MCU, or using serial communication when using a separate modem MCU for the stack.

# Communication schemes

D7AP enables multiple communication schemes which we can divide in push or pull communication.
Push communication happens for instance when a sensor pushes data messages to a gateway. The sensor is sending (periodically or based on a trigger) a response to a read file command which in fact never happened. This is also called unsolicited responses and is the simplest scheme.
DASH7 makes it very simple to use this scheme by defining action files which are configured to be executed when a read or write operation on a file occurs.
For example, a sensor file can be configured that, when written to it, an action is triggered which results in pushing the sensor data.
From the application point of view only the sensor file is written. The stack will transparently execute the attached action file. A typical example is that the command in the action file reads the sensor file and transmits this data over a D7ASP interface with a certain configuration (addressee, QoS, ...).
As the action is contained in a file by itself it can be remotely updated, which means this behavior can be changed OTA without requiring adaptions to the application code itself.

One disadvantage of push communication is that the sensors are always sending data even at periods we might not be interested in this data. For example, you could imagine cases where we only want to see the data sporadically, or even interactively request it using a GUI. For these use cases actively polling for the data only when required, in a RFID like fashion, is more efficient. For this to work we need a way to wake up the nodes we want to query, since a DASH7 network is fully asynchronous, and we assume that nodes are battery powered and thus not able to be in receive mode continuously. To enable this functionality D7AP uses ad hoc synchronization. A requester will flood the channel with background frames containing basically a countdown timer until the 'real' request. The nodes are configured to regularly sniff for background frames. When they detect a background frame they go back to sleep until the countdown timer has elapsed, to be able to receive the query. This method ensures the whole network is synchronized only when needed. The low power wake-up is designed to be power efficient, and consumes a lot less than pushing data at higher rate. The logic analyzer screenshot below shows this behavior visually.
For a requester and 2 responders we show if they are in RX or TX state. Both responders enter RX state briefly at the scan interval, while the requester is continuously in RX in this case. At a certain moment the requester starts the ad hoc synchronization process, and thus switches to TX. During this period both responders scan for activity on the channel. This scan takes a little longer than before, since they both sense activity, and start waiting for a background synchronization frame. Once this is received, they know the time left before the requester will send the actual foreground frame, and go back to sleep during this period. Both responders are now effectively synchronized with the requester, and will switch to RX mode at the same time, to receive the foreground frame, at the end of the requester TX period. Once this has been received both responders will respond, which can be seen by the short TX spike (right after a short RX for Clear Channel Assessment).

![Ad hoc synchronization]({{site.baseurl}}/img/adhoc-sync.png)

An application can also use a combination of both communication schemes. A third option is defined as well: dormant sessions.
Dormant sessions are useful when you need to transmit data to a node which is not very urgent. Starting this as a dormant session with a timeout (of for example 6 hours) allows us to wait for a push message of the node we want to reach. When this happens the node is informed that we have a pending session for him and the dialog is extended. In this way we do not need to engage in ad hoc synchronization, resulting in an even more efficient downlink channel. However, if the node does not push a message before the timeout elapses the ad hoc synchronization will be started, so we can complete the command.

# D7ASP

Coming soon

# Access profiles

Coming soon

# Physical layer

Coming soon

# Examples

Please refer to the [running examples]({{ site.baseurl }}{% link _docs/running-examples.md %}) section to see this in action.
