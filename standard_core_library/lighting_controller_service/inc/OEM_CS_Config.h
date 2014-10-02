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
#define OEM_CS_LAMP_METHOD_CALL_TIMEOUT 250

/**
 * Ping frequency
 */
#define OEM_CS_PING_FREQUENCY_IN_MS 5000

/**
 * Ping back-off
 */
#define OEM_CS_PING_BACKOFF_IN_MS 2000

/**
 * Link timeout in seconds
 */
#define OEM_CS_LINK_TIMEOUT 5

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
 * This function returns the rank of this Controller Service
 *
 * @param None
 * @return Rank of this Controller Service
 */
uint64_t OEM_CS_GetRank(void);

/**
 * This function returns true if this Controller Service is the leader
 *
 * @param None
 * @return Boolean indicating if this Controller Service if the leader
 */
bool OEM_CS_IsLeader(void);

} //lsf

#endif
