# OSS-7 Architecture

##Architecture overview##
OSS7 is an open source software stack created to support different chip architectures (ARM Cortex-M3, Cortus APS...), and compiler toolchains.
OSS7 is based on a simple and flexible framework comprising a lightweight cooperative scheduler. 
OSS7 is higly configurable by design. Configuration options are selected by setting verious parameters through [cmake](gettingstarted.md). 

The following diagram shows the major components of the OSS7 platform:
![OSS7 software stack](architecture.png)

###Hardware Abstraction Layer (HAL)###
Positioned at the bottom of the OSS7 software stack, the hardware abstraction layer (HAL) provides standard interfaces that expose device hardware capabilities to the higher-level layers.
The HAL consists of multiple library modules, each of which implements an interface for a specific type of CPU or radio.

###Framework###
This component is exposing the feature-set of the OS through dedicated APIs. The OSS7 framework includes the following key services:
  
- A low power cooperative scheduler: this service is used for creating, scheduling, and maintaining tasks. Each task is assigned a priority from 0 (high priority) to 7 (low priority). Tasks cannot preempt each other, therefore, tasks must run to completion. Tasks can only be preempted by hardware interrupts.
- A cyphering library that provides AES encryption/decryption functions (AES CTR, AES CCM, AES CBC-MAC).
- A console and a shell interface.
- FEC, PN9 encoder.
- A timer manager to provide more advanced capabilitie than the low level HAL timer interface.
- ...

###Modules###
The DASH7 Alliance protocol is implemented as a module. Different modules can coexist and run within the same system.
For better clarity, the D7A module maintains a clear separation between the ISO layers.

###Application###
The OSS7 code comes with a set a sample applications. Only one application is running along with the system. The application can be selected in the build system through cmake.
