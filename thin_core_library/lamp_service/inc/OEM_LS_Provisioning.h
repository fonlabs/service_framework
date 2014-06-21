#ifndef _OEM_LS_PROVISIONING_H_
#define _OEM_LS_PROVISIONING_H_
/**
 * @file OEM_LS_Provisioning.h
 * @defgroup property_store The OEM-defined properties used by About/Config Service
 * @{
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

/**
 * Initialize the property store
 *
 * @param  None
 * @return Status of the operation
 */
AJ_Status PropertyStore_Init(void);

/**
 * Persist device ID
 *
 * @param  None
 * @return None
 */
void SavePersistentDeviceId(void);

/**
 * The max length of the default language name
 */
#define LANG_VALUE_LENGTH 7

/**
 * The max length of the realm name
 */
#define KEY_VALUE_LENGTH 10

/**
 * The max length of the machine ID
 */
#define MACHINE_ID_LENGTH (UUID_LENGTH * 2)

/**
 * The max length of the device name
 */
#define DEVICE_NAME_VALUE_LENGTH LSF_MAX_NAME_LENGTH

/**
 * The max length of the password
 */
#define PASSWORD_VALUE_LENGTH (AJ_ADHOC_LEN * 2)

#endif
