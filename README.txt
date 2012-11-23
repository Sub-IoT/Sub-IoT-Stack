
This project is build with CCS v5.3 and currently only supports the WizziMote.
(As discussed previously supporting multiple toolchains and platforms is something we will do later.)

Radio Abstraction Layer provides an API to implement by the radio driver.
The purpose is to be able to easily switch between different implementations for specific radio chips.
At the moment there is not much difference between PHY and RAL, but the goal is to put all reusable logic in the PHY layer, and only radio chip specific logic in the RAL implementation. 
At the moment there are 2 implementations, one for the CC430 radio and a stub one which only does logging.
To create a new driver should define a ral_interface struct which contains function pointers required by the API.
The implementation to be used should come from some kind of configure step later on, but is now defined using precompiler directives in d7aoss.h .

The HAL directory should contain the hardware abstraction layer API and its implementations. At the moment this contains code from TI's driverlib and some functions for button, leds, rtc, system and uart handling. Support for multiple hardware platforms is not implemented at the moment and should be discussed eventually.

The PHY directory contains the PHY API and implementation.
The RAL directory contains the RAL API and the implementation for CC430 and a stub implementation.
The d7aoss.h is supposed to be the high level API of the stack to be used by the applications. At the moment it only defines which RAL implementation to use.
There is also a basic logging functionality defined in log.h which allows logging to UART. (This should be expanded to allow specifying for example the layer and the log level.)

The phy_test example application listens on channel 10, button 1 sends one packet (fixed data). When receiving a packet the green led will toggle when the same packet data is received, and will toggle the red led if other data is received. Pressing button 3 will send a packet each second.

The tools/Dash7Logger directory contains a command line application (.NET) which outputs the logging received over UART in a formatted way.

To get started: in a CCS v5.3 workspace import d7aoss and examples/phy_test as existing CCS Eclipse projects and build them. 

Next steps according to us are:
- discuss and complete RAL + PHY API
- resolve TODOs (which are everywhere now)
- add documentation comments
- fix coding style everywhere
- add copyright/license header
- define MAC API (or should we use DLL instead of MAC?)
- implement MAC

