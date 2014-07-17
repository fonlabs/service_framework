#ifndef _OEM_CS_CONFIG_H_
#define _OEM_CS_CONFIG_H_
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

/*
 * Maximum number of supported LSF entities i.e. Lamp Groups, Scenes,
 * Master Scenes, etc
 */
#define MAX_SUPPORTED_NUM_LSF_ENTITY 100

/*
 * Maximum number of supported Lamps
 */
#define MAX_SUPPORTED_LAMPS 100

/*
 * Maximum number of outstanding requests
 */
#define MAX_LAMP_CLIENTS_METHOD_QUEUE_SIZE 200

/*
 * Timeout for Pings
 */
#define PING_TIMEOUT_IN_MS 300

/*
 * Returns the factory set value of the default lamp state. The
 * PresetManager will use this value to initialize the default
 * lamp state if it does not find a persisted value for the same
 *
 * @param  defaultState Container to pass back the Default Lamp State
 * @return None
 */
void GetFactorySetDefaultLampState(LampState& defaultState);

/*
 * Returns the time sync time stamp used to synchronize state
 * changes in amonst multiple Lamp Services
 *
 * @param  timeStamp Container to pass back the time stamp
 * @return None
 */
void GetSyncTimeStamp(uint64_t& timeStamp);

uint32_t GetLinkTimeoutSeconds();

/**
 * LSFPropertyStore predeclaration
 */
class LSFPropertyStore;

/**
 * If no Factory Configuration ini file is found,
 * this function will be called to populate
 * the default values.
 *
 * @param propStore The property store
 */
void PopulateDefaultProperties(LSFPropertyStore& propStore);

} //lsf

#endif
