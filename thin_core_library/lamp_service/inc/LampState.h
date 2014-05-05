#ifndef _LAMP_STATE_H_
#define _LAMP_STATE_H_
/**
 * @file LampState.h
 * @defgroup lamp_state The Lamp State
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

#include <alljoyn.h>

#include <LampResponseCodes.h>

/**
 * Struct _LampState.  This is used to store the various state fields.
 */
typedef struct _LampState {
    uint32_t hue;           /**< The lamp hue */
    uint32_t saturation;    /**< The lamp color saturation */
    uint32_t colorTemp;     /**< The lamp color temperature */
    uint32_t brightness;    /**< The lamp brightness */
    uint8_t onOff;          /**< Is the lamp on or off */
} LampState;

/**
 * Unmarshal the LampState params from a message
 *
 * @param state[out] The state to read into
 * @param msg[in]    The message to read from
 * @return           LAMP_OK if all fields valid
 */
LampResponseCode LAMP_UnmarshalState(LampState* state, AJ_Message* msg);

/**
 * Serialize the Lamp's current state
 *
 * @param state The state to marshal
 * @param msg   The msg to serialize data into
 * @return      LAMP_OK if no errors occured
 */
LampResponseCode LAMP_MarshalState(LampState* state, AJ_Message* msg);

/**
 * Initialize the LampState.  Read it from NVRAM
 */
void LAMP_InitializeState();

/**
 * Get the current LampState
 *
 * @param[out] state    A pointer to a LampState object
 */
void LAMP_GetState(LampState* state);

/**
 * Set the new lamp state and save it in NVRAM.
 *
 * @param[in] state The new state
 */
void LAMP_SetState(const LampState* state);

/**
 * Clear the state
 */
void LAMP_ClearState(void);

/**
 * @}
 */
#endif
