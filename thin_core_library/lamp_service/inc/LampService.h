#ifndef _LAMP_SERVICE_H_
#define _LAMP_SERVICE_H_
/**
 * @file LampService.h
 * @defgroup lamp_service The AllJoyn Lamp Service APIs
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

#include <alljoyn.h>

/**
 * Run the lamp service with no callback.
 * This is useful if you want to run the lamp service from its own thread
 *
 * @param None
 * @return None
 */
void LAMP_RunService(void);

/**
 * Typedef for the LSF callback
 *
 * @param  None
 * @return None
 */
typedef void (*LampServiceCallback)(void);

/**
 * Get the LSF version
 *
 * @param   None
 * @return  The version
 */
uint32_t LAMP_GetServiceVersion(void);

/**
 * Run the Lamp Service with a callback that occurs after
 * we time out waiting for a message.  Use this if you want to run
 * OEM-specific code on the same thread as the AllJoyn event loop
 *
 * @param timeout   The timeout (in ms)
 * @param callback  The callback function
 * @return          None
 */
void LAMP_RunServiceWithCallback(uint32_t timeout, LampServiceCallback callback);

/**
 * Schedule the signal org.allseen.LSF.LampService.LampStateChanged to be sent ASAP.
 * We call this every time the Lamp State is changed and written to NVRAM.
 *
 * @param  None
 * @return None
 */
void LAMP_SendStateChangedSignal(void);

/**
 * Call this method to indicate that some fault has occurred
 * in the Lamp
 *
 * @param  None
 * @return None
 */
void LAMP_SetFaults(void);

/**
 * This function must be called when all faults are cleared
 * in order to clear the Faults notification.
 *
 * @param  None
 * @return None
 */
void LAMP_ClearFaults(void);

/**
 * Restarts the lamp service at the next opportunity when running on its own
 * thread, restarting the Wifi connection if already established.
 *
 * @param  None
 * @return None
 */
void LAMP_Restart(void);

/**
 * Performs a factory reset at the next opportunity, clearing all internal state
 * and restarting the service.
 *
 * @param  None
 * @return None
 */
void LAMP_FactoryReset(void);

/**
 * @}
 */
#endif
