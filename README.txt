Welcome to AllJoyn Lighting Service Framework
==============================================

Building the Lighting Service Framework core modules on Ubuntu
==============================================================
1) Follow the instructions in https://allseenalliance.org/docs-and-downloads/documentation/configuring-build-environment-linux-platform to set up your ubuntu machine to build AllJoyn

2) You will need the following directory structure in your working directory.

core/

	service_framework/   (https://git.allseenalliance.org/cgit/lighting/service_framework.git)

	ajtcl/   (https://git.allseenalliance.org/cgit/core/ajtcl.git Commit ID:6a5c6bd841bc65c87fb238ea6699ec2c3be10372)

	alljoyn/ (https://git.allseenalliance.org/cgit/core/alljoyn.git Commit ID:1914ce0387237b0c05dad597f54027ecd8fd2f57)

base_tcl/ (https://git.allseenalliance.org/cgit/services/base_tcl.git Commit ID:3127ef6fb7be7ea42e6bd639e84864225f840006)

base/ (https://git.allseenalliance.org/cgit/services/base.git Commit ID:56f3653e0a4e62d1f107c3a27019e7ee45dd2b52)

Checkout the appropriate Commit IDs on all of the above GIT projects except service_framework.git by navigating in to the appropriate project directory and running the following command

git checkout <commitID>

3) Open a command terminal and from under the core/service_framework/ directory, run the command "scons" to build the core modules

=====================================================================================================
Running the End-To-End Test on Ubuntu
=====================================================================================================

From under the directory core/service_framework/build/linux/x86/debug/dist/cpp/bin, run the alljoyn daemon using the following command:

	./alljoyn-daemon
	
	This will start a daemon that will support a max of 32 Lamp Services
	
	You could run the following command instead to specify a custom config file that will allow up to 100 Lamp Services
	
	./alljoyn-daemon --config-file=./routerconfig.xml

	This will start the Routing Node

Now on another terminal, from under the directory core/service_framework/build/linux/thin_core_library/lamp_service/bin, run the lamp service using the following command:

	./lamp_service
	
You could also use the shell script LaunchLampServices.sh that can be found in the core/service_framework/build/linux/thin_core_library/lamp_service/bin directory to launch from 1 up to a 100 Lamp Services on Ubuntu. 
Run the following command from under the core/service_framework/build/linux/thin_core_library/lamp_service/bin directory

./LaunchLampServices.sh

The script will then prompt you to enter the number of Lamp Services to launch. Enter the number and then press enter.

Please note that the script launches the lamp services as background processes and also creates a number of directories. In order to ensure that the 
script cleans those up properly on exit, use Ctrl+C to exit the script. The script will then prompt you to press enter. Once you do so, all the 
directories and background processes are cleaned up. If this process is not followed to exit the script, you would need to explicitly clean up all 
the lamp services and directories.

In a third terminal, from under the directory core/service_framework/build/linux/standard_core_library/lighting_controller_service/bin, run the Lighting Controller Service:

	./lighting_controller_service

In a fourth terminal, from under the directory core/service_framework/build/linux/standard_core_library/lighting_controller_client/test, run the Lighting Controller Client Test App:

	./lighting_controller_client_sample

The Lighting Controller Client Test App should provide a drop down menu of commands that you can exercise to run the end to end test. You should also see the replies and the signals received back from the Controller Service on the terminal running the Lighting Controller Client Test App.

NOTE: In order to verify ApplyScene and ApplyMasterScene,

        Bring up 20 lamp services using the LaunchLampServices.sh script
        Run the commands in the following order in the Lighting Controller Client Sample

(3):   GetAllLampIDs

(39):  CreateLampGroup (This will automatically create 8 sample Lamp Groups)

(63):  CreatePreset (Run Create Preset twice to create 2 presets as this is required by Create Scene)

(63):  CreatePreset

(70):  CreateScene (This will Create a sample Scene with the existent Lamp IDs and already created Lamp Groups and Presets)

(74):  ApplyScene

(78):  CreateMasterScene (This will Create a sample Master Scene with the already created Scene)

(82):  ApplyMasterScene

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