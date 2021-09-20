---
title: Architecture
permalink: /docs/architecture/
---

Sub-IoT is an open source software stack created to support different chip architectures (ARM Cortex-M3, Cortus APS...), and compiler toolchains.
Sub-IoT is based on a simple and flexible framework comprising a lightweight cooperative scheduler.
Sub-IoT is highly configurable by design. Configuration options are selected by setting various parameters through cmake (see [building section]({{ site.baseurl }}{% link _docs/building.md %})).

The following diagram shows the major components of the Sub-IoT platform:
![Sub-IoT software stack]({{ site.baseurl }}/img/architecture.png)

## Hardware Abstraction Layer (HAL)
Positioned at the bottom of the Sub-IoT software stack, the hardware abstraction layer (HAL) provides standard interfaces that expose device hardware capabilities to the higher-level layers.
The HAL consists of multiple library modules, each of which implements an interface for a specific type of CPU or radio.

## Framework
This component is exposing the feature-set of the OS through dedicated APIs. The Sub-IoT framework includes the following key services:

- A low power cooperative scheduler: this service is used for creating, scheduling, and maintaining tasks. Each task is assigned a priority from 0 (high priority) to 7 (low priority). Tasks cannot preempt each other, therefore, tasks must run to completion. Tasks can only be preempted by hardware interrupts.
- A ciphering library that provides AES encryption/decryption functions (AES CTR, AES CCM, AES CBC-MAC).
- A console and a shell interface.
- FEC, PN9 encoder.
- A timer manager to provide more advanced capabilities than the low level HAL timer interface.
- ...

## Modules
The DASH7 Alliance protocol is implemented as a module. Different modules can coexist and run within the same system.
For better clarity, the D7A module maintains a clear separation between the ISO layers.

## Application
The Sub-IoT code comes with a set of sample applications. Only one application is running along with the system. The application can be selected in the build system through cmake.
