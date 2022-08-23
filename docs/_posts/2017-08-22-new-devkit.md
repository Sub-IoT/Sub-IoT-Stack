---
layout: post
title:  "New recommended devkit"
date:   2017-08-22 11:00:00
author: Glenn
---

Until now, the Silicon Labs EZR32LG_WSTK6200 devkit was our recommend off the shelf kit to get started with Sub-IoT.
We had good experiences using this board, however it has the disadvantage of being too expensive. People wanting to give Sub-IoT
a try should be able to buy an off the shelf devkit for a reasonable price.

Recently we ported to a platform containing a Semtech sx1276 RF chip, to [demo DASH7 and LoRa multi-modal communication using one MCU and RF chip]({{ site.baseurl }}{% link _posts/2017-07-28-multimodal-demo.md %}). For yet another project we needed to support an STM32 MCU as well.
Porting to those 2 chips allows us to support another platform, the STMicroelectronics [B_L072Z_LRWAN1](http://www.st.com/content/st_com/en/products/evaluation-tools/product-evaluation-tools/mcu-eval-tools/stm32-mcu-eval-tools/stm32-mcu-discovery-kits/b-l072z-lrwan1.html).![B_L072Z_LRWAN1](https://i0.wp.com/blog.st.com/wp-content/uploads/RS7569_B_L072Z_side_antenna.jpg)
The B_L072Z_LRWAN1 is marketed by STMicroelectronics as an STM32L0 discovery kit for LoRa, but it can be used for DASH7 as well.
This devkit is a complete off the shelf solution, containing a programmer an STM32L0 MCU and sx1276 radio for a very reasonable price.
More precisely, it contains a [Murata CMWX1ZZABZ-091 module](http://wireless.murata.com/eng/products/rf-modules-1/lpwa/type-abz.html) which embeds a STM32L072CZ MCU and a Semtech sx1276 RF chip together on a stand-alone module.

This module is interesting because it allows to easily integrate a DASH7 modem on a custom design. By adding this module to your board you can access the DASH7 modem running on this module from your main application MCU, for instance.
For projects requiring multi-modal communication you can use DASH7, DASH7-over-LoRa, LoRaWAN or even SigFox ([since the Murata module has recently been certified for SigFox](https://www.murata.com/en-global/products/connectivitymodule/lpwa/overview/sigfox)).
