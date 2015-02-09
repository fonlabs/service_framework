#ifndef _LAMP_ABOUT_DATA_H_
#define _LAMP_ABOUT_DATA_H_
/**
 * @file LampAboutData.h
 * @defgroup lamp_about About/Config PropertyStore used by the Lamp Service
 * @{
 */
/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
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

#include <aj_status.h>
#include <aj_msg.h>
#include <aj_debug.h>

/**
 * Set up the About and Config Service data structures
 *
 * @param   None
 * @return  None
 */
void LAMP_SetupAboutConfigData(void);

/**
 * Callback to set the AllJoyn password
 *
 * @param buffer    The password buffer
 * @param bufLen    The buffer length
 * @return          The length of the password written to buffer
 */
uint32_t LAMP_PasswordCallback(uint8_t* buffer, uint32_t bufLen);


/**
 * Get the Lamp ID.  If the ID does not yet exist,
 * generate and persist it before returning it.
 *
 * @param   None
 * @return  The Lamp ID
 */
const char* LAMP_GetID(void);

/**
 * Get the Lamp Name
 *
 * @param   None
 * @return  The Lamp name
 */
const char* LAMP_GetName(void);

/**
 * Set the Lamp Name
 *
 * @param name  The Lamp name
 * @return      None
 */
void LAMP_SetName(const char* name);

/**
 * @}
 */
#endif
