Welcome to AllJoyn Lighting Service Framework
==============================================

Building the Lighting Service Framework core modules on Ubuntu
==============================================================
1) Follow the instructions in https://allseenalliance.org/docs-and-downloads/documentation/configuring-build-environment-linux-platform to set up your ubuntu machine to build AllJoyn

2) You will need the following directory structure in your working directory.

core/

	service_framework/   (https://git.allseenalliance.org/cgit/lighting/service_framework.git)

	ajtcl/   (https://git.allseenalliance.org/cgit/core/ajtcl.git Commit ID:e797f8df27d81c8201a432af6b455452ab2b2008)

	alljoyn/ (https://git.allseenalliance.org/cgit/core/alljoyn.git Commit ID:96088edcd68faef2350500d773e72c7c8aa8f582)

base_tcl/ (https://git.allseenalliance.org/cgit/services/base_tcl.git Commit ID:a189555ea286cb6daf06600d97450082b70eb516)

base/ (https://git.allseenalliance.org/cgit/services/base.git Commit ID:87ab8e80f47c2f4630384155fa9e281582a06d69)

Checkout the appropriate Commit IDs on all of the above GIT projects except service_framework.git by navigating in to the appropriate project directory and running the following command

git checkout <commitID>

3) Open a command terminal and from under the core/service_framework/ directory, run the command "scons" to build the core modules

=====================================================================================================
Running the End-To-End Test on Ubuntu
=====================================================================================================

From under the directory core/service_framework/build/linux/x86/debug/dist/cpp/bin, run the alljoyn daemon using the following command:

	./alljoyn-daemon --config-file=./routerconfig.xml

	This will start the Routing Node

Now on another terminal, from under the directory core/service_framework/build/linux/thin_core_library/lamp_service/bin, run the lamp service using the following command:

	./lamp_service

In a third terminal, from under the directory core/service_framework/build/linux/standard_core_library/lighting_controller_service/bin, run the Lighting Controller Service:

	./lighting_controller_service

In a fourth terminal, from under the directory core/service_framework/build/linux/standard_core_library/lighting_controller_client/test, run the Lighting Controller Client Test App:

	./lighting_controller_client_sample

The Lighting Controller Client Test App should provide a drop down menu of commands that you can exercise to run the end to end test. You should also see the replies and the signals received back from the Controller Service 
on the terminal running the Lighting Controller Client Test App. 

===============================================================================
Enabling Logging in Lighting Controller Service and Lighting Controller Client
===============================================================================

In all the *.cc files under the src directories in the core/service_framework/standard_core_library, you will find a #define QCC_MODULE for that file.

	For Eg: in core/service_framework/standard_core_library/lighting_controller_service/src/ControllerService.cc it is #define QCC_MODULE "CONTROLLER_SERVICE"

To enable logging from the file ControllerService.cc, use the following command when running the Controller Service

	ER_DEBUG_CONTROLLER_SERVICE=7 ./lighting_controller_service

You can pass in multiple ER_DEBUG env variables when starting the Controller Service or Controller Client.

	For Eg: ER_DEBUG_CONTROLLER_SERVICE=7 ER_DEBUG_LAMP_CLIENTS=7 ./lighting_controller_service

Use ER_DEBUG_ALL=7 to enable all logging. But this would also enable all AllJoyn logging and result in lot of logs. If using ER_DEBUG_ALL dump the logs to a log file as below:

	ER_DEBUG_ALL=7 ./lighting_controller_service 2> log.txt