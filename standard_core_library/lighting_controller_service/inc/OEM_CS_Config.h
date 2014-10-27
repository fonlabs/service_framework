#ifndef _OEM_CS_CONFIG_H_
#define _OEM_CS_CONFIG_H_
/**
 * \ingroup ControllerService
 */
/**
 * @file
 * This file provides definitions for OEM configurations
 */
/******************************************************************************
 * Copyright (c) 2014, AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/
#include <LSFTypes.h>

namespace lsf {

/**
 * Maximum number of supported LSF entities i.e. Lamp Groups, Scenes,
 * Master Scenes, etc
 */
#define OEM_CS_MAX_SUPPORTED_NUM_LSF_ENTITY 100

/**
 * Maximum number of supported Lamps
 */
#define OEM_CS_MAX_SUPPORTED_LAMPS 100

/**
 * Maximum number of outstanding requests
 */
#define OEM_CS_MAX_LAMP_CLIENTS_METHOD_QUEUE_SIZE 200

/**
 * Timeout for Lamp Method Calls
 */
#define OEM_CS_LAMP_METHOD_CALL_TIMEOUT 25000

/**
 * Timeout used in the check to see if the Controller Service is still connected
 * to the routing node
 */
#define OEM_CS_TIMEOUT_MS_CONNECTED_TO_ROUTING_NODE 5000

/**
 * Returns the factory set value of the default lamp state. The
 * PresetManager will use this value to initialize the default
 * lamp state if it does not find a persisted value for the same
 *
 * @param  defaultState Container to pass back the Default Lamp State
 * @return None
 */
void OEM_CS_GetFactorySetDefaultLampState(LampState& defaultState);

/**
 * Returns the time sync time stamp used to synchronize state
 * changes in amonst multiple Lamp Services
 *
 * @param  timeStamp Container to pass back the time stamp
 * @return None
 */
void OEM_CS_GetSyncTimeStamp(uint64_t& timeStamp);

/**
 * LSFPropertyStore pre-declaration
 */
class LSFPropertyStore;

/**
 * If no Factory Configuration ini file is found,
 * this function will be called to populate
 * the default values.
 *
 * @param  propStore The property store
 * @return None
 */
void OEM_CS_PopulateDefaultProperties(LSFPropertyStore& propStore);

/**
 * Pure virtual base class implemented by Controller Service and the reference for which is passed in
 * to the OEM firmware through the OEM_CS_FirmwareStart() function so that the firmware may call back
 * the controller service when the device connect to / disconnects from the network
 */
class OEM_CS_NetworkCallback {
  public:
    /** Destructor */
    virtual ~OEM_CS_NetworkCallback() { }

    /**
     * This OEM firmware should invoke this function whenever the device running
     * the Controller Service connects to a Network
     * @return none
     */
    virtual void Connected(void) = 0;

    /**
     * This OEM firmware should invoke this function whenever the device running
     * the Controller Service disconnects from a Network
     * @return none
     */
    virtual void Disconnected(void) = 0;
};

/**
 * Controller Service will invoke this function at system startup.
 * OEMs should add code here to initialize and start the firmware and
 * return true/false accordingly.
 * The firmware should also save off the reference to the OEM_CS_NetworkCallback object.
 * The firmware should invoke the Connected() function defined in this callback
 * whenever the device connects to a network and it should invoke the Disconnected()
 * function defined in this callback whenever the device disconnects from a network
 * @param  Reference to a OEM_CS_NetworkCallback object
 * @return true/false indicating the status of the operation
 */
bool OEM_CS_FirmwareStart(OEM_CS_NetworkCallback& networkCallback);

/**
 * The Controller Service will invoke this function
 * when the system is being shut down.
 * OEMs should add code here to stop the firmware and perform any cleanup
 * operations and return true/false accordingly
 * @return true/false indicating the status of the operation
 */
bool OEM_CS_FirmwareStop(void);

/**
 * This function returns the MAC address of the device running the Controller
 * Service as a 48-bit unsigned integer
 * @return MAC address of the device running the Controller
 *         Service as a 48-bit unsigned integer
 */
uint64_t OEM_CS_GetMACAddress(void);

/**
 * This function returns true if the device running the Controller Service is
 * connected to a network
 * @return true/false
 */
bool OEM_CS_IsNetworkConnected(void);

/**
 * Possible values for Power - one of the static
 * parameters used in the rank computation
 */
typedef enum _OEM_CS_RankParam_Power {
    BATTERY_POWERED_NOT_CHARGABLE = 0,
    BATTERY_POWERED_CHARGABLE,
    ALWAYS_AC_CONNECTED,
    OEM_CS_RANKPARAM_POWER_LAST_VALUE
} OEM_CS_RankParam_Power;

/**
 * This function returns value of the Power - one of the static
 * parameters used in the rank computation
 * @return One of the values defined by the enum OEM_CS_RankParam_Power
 */
OEM_CS_RankParam_Power OEM_CS_GetRankParam_Power(void);

/**
 * Possible values for Power - one of the static
 * parameters used in the rank computation
 */
typedef enum _OEM_CS_RankParam_Mobility {
    MOSTLY_MOBILE = 0,
    MOSTLY_PROXIMAL,
    ALWAYS_STATIONARY,
    OEM_CS_RANKPARAM_MOBILITY_LAST_VALUE
} OEM_CS_RankParam_Mobility;

/**
 * This function returns value of the Mobility - one of the static
 * parameters used in the rank computation
 * @return One of the values defined by the enum OEM_CS_RankParam_Mobility
 */
OEM_CS_RankParam_Mobility OEM_CS_GetRankParam_Mobility(void);

/**
 * Possible values for Availability - one of the static
 * parameters used in the rank computation
 */
typedef enum _OEM_CS_RankParam_Availability {
    ALWAYS_AVAILABLE,
    OEM_CS_RANKPARAM_AVAILABILITY_LAST_VALUE
} OEM_CS_RankParam_Availability;

/**
 * This function returns value of the Availability - one of the static
 * parameters used in the rank computation
 * @return One of the values defined by the enum OEM_CS_RankParam_Availability
 */
OEM_CS_RankParam_Availability OEM_CS_GetRankParam_Availability(void);

} //lsf

#endif
