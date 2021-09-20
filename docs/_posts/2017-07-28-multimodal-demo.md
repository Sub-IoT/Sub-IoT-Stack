---
layout: post
title:  "Multi-modal low-power communication & localization"
date:   2017-07-27 09:00:00
author: Glenn
---
This demo done for [ITF2017](http://www.itf2017.be/) shows the combination of LPWAN and DASH7 for IoT communication and localization using the same hardware, together with passive RFID for activation and configuration.
By seamlessly switching from LPWAN (LoRa in this case) to DASH7 when moving in range of a locally deployed DASH7 network we can save a lot on energy consumption.
The energy cost per message when using DASH7 is about 100x smaller than transmitting the same message over LoRa (depending on the settings of course). Besides saving on energy consumption we can also improve our system, by for instance transmitting more often while indoor to offload more sensor data or improve localization accuracy or enabling firmware upgrades.


<div class="embed-responsive embed-responsive-16by9">
  <iframe class="embed-responsive-item" src="https://www.youtube.com/embed/nSqkZRmuEiI?color=white&theme=light"></iframe>
</div>
<br>

This multi-modal approach clearly has advantages, especially since the hardware BoM cost is not impacted by using both DASH7 and LoRaWAN or Sigfox,
since all 3 technologies are operating in the same frequency band. We are using the Semtech sx1276 in this case (which has LoRa modulation on board),
but also supports the GFSK modulation used by DASH7. The sx1276 is compatible with Sigfox as well, when implementing the Sigfox 'software' modulation on the MCU.
With the recent news of the [Murata ABZ module supporting the Sigfox stack](http://www.murata.com/en-eu/about/newsroom/news/product/frontend/2017/0718?mkt_tok=eyJpIjoiTkRFNFlqRTRNMkZpT1dGayIsInQiOiJDbEk4eG5oQVNkYURSZ0JKYnpCWDd5aGZ5aXE3dG1renZ6THBNQ1BOOWpNNEk1ckFXRmhtV1wvMjhOQk9jS1NcLzh6QlpJTnFaQnc3eVVOeW9IVzJJTFdxUFcyUEVUbXViZThrRUprR2oyVnMwR2RXOG51RDZtaCttSmFhQ3EyWUcyIn0%3D) it should be much easier to add Sigfox capabilities to your product, without having to engage in a certification process. The Murata module uses the sx1276 chip as well (together with an STM32L0) and thus supports LoRa and DASH7 as well. In fact, we are actively porting Sub-IoT to the Murata module. More news on this soon.

The hardware demo-ed here adds an passive RFID chip and RF chip, which of course does impact the BoM. However, we don't need a separate antenna for the UHF, we are reusing the same antenna used for DASH7 and LoRa. The RFID chip is not strictly needed for handover between LPWAN and DASH7,
since this can be achieved by polling for a DASH7 network as well. However, using passive RFID means we can save on the energy consumption needed for the polling. Additionally, the RFID chip enables us to provision configuration to the tag when entering a network. We can, for example, configure the DASH7 access profile to use when entering a specific site.

In this demo we are using DASH7-over-LoRa, meaning we run the normal DASH7 stack on top of a LoRa PHY.
Although this PHY mode is currently not part of the specification it is interesting to experiment with this, for certain use cases.
Using the LoRa PHY has no impact on the upper layer of the DASH7 stack, it is fully transparent. The biggest advantage is of course the increased range,
but this comes with increased power consumption as well. For this demo we mainly selected DASH7-over-LoRa instead of using LoRaWAN for practical reasons:
we didn't want to be dependent on a public LoRaWAN network being available (which turned out to be a good decision since there was no coverage at our booth!)
and neither did we want to deploy our own private LoRaWAN network. So using DASH7-over-LoRa was just the easier option for this demo. In a real product a public LoRaWAN network or Sigfox will have to be used for off-site coverage of course.
