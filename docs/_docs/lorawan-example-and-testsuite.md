---
title: LoRaWAN example and LoRaWAN CI tests
permalink: /docs/lorawan-example-and-testsuite/
---

# LoRaWAN Example and LoRaWAN tests in the Testsuite

This section explains how to run the LoRaWAN example application, and the parameters required to register a device running the application on a LoRaWAN Network Server. For a general overview of how to build the stack and run the examples, it is recommended to read the [building]({{ site.baseurl }}{% link _docs/building.md %}) and [running examples]({{ site.baseurl }}{% link _docs/running-examples.md %}) sections first.

By default Sub-IoT assumes use of DASH7, and the LoRaWAN stack must be specifically included in the build process to enable it. The sensor_push_lorawan application provides an example application using LoRaWAN communication, where a device sends a LoRaWAN frame containing a two byte payload (+ 13 bytes of LoRaWAN header) every minute:

    $ cmake ../stack/ -DAPP_GATEWAY=y -DAPP_SENSOR_PUSH_LORAWAN=y -DMODULE_LORAWAN=y
    $ make flash-sensor_push_lorawan

If the device has not yet joined the LoRaWAN network, passing a payload to the stack will automatically trigger the LoRaWAN join process. In Sub-IoT, OTAA (Over The Air Activation) is the only supported LoRaWAN join process; ABP (Authentication By Personalisation) is not supported.

In order for the device to successfully join the network it must be registered on the LoRaWAN Network Server (LNS) (whether the Things Stack, Chirpstack, etc.). The exact instructions for registering a device will differ depending on the LNS used, but in general the information required by the LNS about the device is:

- **LoRaWAN version**: The LoRaWAN version currently supported in the stack is MAC V1.0.3. 

- **Regional Parameters**: The regional parameters version that needs to be selected for the device is PHY V1.0.3 REV A.

- **Frequency Plan**: The frequency plan for the region the device will be deployed in should be selected (EU868, US915 etc). The build of Sub-IoT then needs to match this. Sub-IoT builds are for EU868 by default, but US915 and AS923 are also supported and can be configured through the CMake option MODULE_LORAWAN_REGION e.g. if you are in the US you can configure this by adding -DMODULE_LORAWAN_REGION=US915 to the CMake command provided above.

- **DevEui**: The DevEui is the globally-unique Extended Unique Identifier (EUI-64) assigned to the device.

- **AppEui/JoinEui**: The AppEui is the unique EUI-64 assigned to the Appication Server. In later versions this parameters has been renamed to JoinEui.

- **AppKey**: The AppKey is the encryption key for the application-layer messages.

The final three parameters are device-specific, and need to be set the same on both the device and LNS for the LNS to be able to identify the device and decode encrypted packets. On Sub-IoT, the DevEUI uses the UidFile, so if you haven't written to it, it will use the hardware UID of your processor. The other keys are set in the filesystem. The default values can be found in /stack/fs/dtap_fs_data.c. To change the filesystem, you can follow the [tutorial]({{ site.baseurl }}{% link _docs/filesystem.md %}) from the documentation. Alternatively, to see the values of the EUIs and AppKey, you can also make a build with logging turned on for the LoRaWAN module (add -DFRAMEWORK_LOG_ENABLED=y -DMODULE_LORAWAN_LOG_ENABLED=y to CMake). The first time the device attempts a join, it will print out the DevEui, AppEui and AppKey (e.g. DevEui: xx xx xx xx xx xx xx x)

# Running LoRaWAN tests in the Testsuite

The Sub-IoT-testsuite repository provides a set of tools and tests for automated integration testing of Sub-IoT. The set of tests covering DASH7 can be ran using 2 devices running the provided "modem" application. However, the LoRaWAN tests require the use of the [Semtech Conformance Test Bench](https://lora-developers.semtech.com/build/tools/conformance-test-bench/introduction-and-objectives/) (CTB), which consists of a Raspberry Pi, a Semtech Picocell Gateway module, and particular software provided in the earlier link. The version of the CTB that is supported in the integration tests is v1.0.0. The CTB must also be connected to a functioning LNS.

With the CTB, the LoRaWAN integration tests can be automated using the gitlab-ci.yml provided in the Sub-IoT-Stack repository. The manual steps required to achieve this are:

1. Define the required variables for the LoRaWAN tests (used in gitlab-ci.yml) in the pipeline settings of your CI. These are:
    - **LORAWAN_GW_IP_ADDR**: the IP address of the CTB.
    - **LORAWAN_DUT_DEV_EUI**: the DevEui of the device under test .
    - **LORAWAN_DUT_APP_KEY**: the AppKey of the device under test.
    - **LORAWAN_GRAB_TEST_RESULTS**: a flag which allows the CTB to trigger a final job on the runner which pulls in the test results (set as "false").
    - **LORAWAN_CI_TRIGGER_FINAL_JOB**: an access token used to trigger the final job.
    - **LORAWAN_CI_ACCESS_TOKEN**: an access token used to pull information about previous jobs.
2. Copy the check_if_finished.py and reset_gw_server.py scripts from the lorawan-test-scripts folder of the testsuite to the CTB. Set the GATEWAY_ADDR variable in check_if_finished.py to be the local IP address of the CTB. Set the LORAWAN_CI_TRIGGER_FINAL_JOB to be the same as defined in the CI. Set CI_PIPELINE_TRIGGER to be the API call which triggers your CI pipeline e.g. for GitLab this is https://gitlab.com/api/v4/projects/YOUR_PROJECT_ID/trigger/pipeline' Set reset_gw_server.py to run on boot and set check_if_finished.py to be run as a cronjob every five minutes.
3. The LoRaWAN tests are by default configured to run only on scheduled pipelines. Configure your pipeline schedule.


