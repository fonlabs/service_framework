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

#include <LampState.h>
#include <LampService.h>
#include <aj_nvram.h>

/**
 * Per-module definition of the current module for debug logging.  Must be defined
 * prior to first inclusion of aj_debug.h
 */
#define AJ_MODULE LAMP_STATE

/*
 * The state object that represents the current lamp state.
 * This is mirrored in NVRAM and preserved across power cycles.
 * A signal will be sent when this changes if a session is active.
 */
static LampState TheLampState;

LampResponseCode LAMP_MarshalState(LampState* state, AJ_Message* msg)
{
    AJ_InfoPrintf(("%s\n", __FUNCTION__));
    AJ_Status status = AJ_MarshalArgs(msg, "{sv}", "hue", "u", state->hue);
    if (status != AJ_OK) {
        return LAMP_ERR_MESSAGE;
    }

    status = AJ_MarshalArgs(msg, "{sv}", "saturation", "u", state->saturation);
    if (status != AJ_OK) {
        return LAMP_ERR_MESSAGE;
    }

    status = AJ_MarshalArgs(msg, "{sv}", "colorTemperature", "u", state->colorTemp);
    if (status != AJ_OK) {
        return LAMP_ERR_MESSAGE;
    }

    status = AJ_MarshalArgs(msg, "{sv}", "brightness", "u", state->brightness);
    if (status != AJ_OK) {
        return LAMP_ERR_MESSAGE;
    }

    status = AJ_MarshalArgs(msg, "{sv}", "OnOff", "b", state->onOff);
    if (status != AJ_OK) {
        return LAMP_ERR_MESSAGE;
    }

    return LAMP_OK;
}


LampResponseCode LAMP_UnmarshalState(LampState* state, AJ_Message* msg)
{
    AJ_Arg array1, struct1;
    AJ_Status status = AJ_UnmarshalContainer(msg, &array1, AJ_ARG_ARRAY);
    LampResponseCode rc = LAMP_OK;

    do {
        char* field;
        char* sig;

        status = AJ_UnmarshalContainer(msg, &struct1, AJ_ARG_DICT_ENTRY);
        if (status != AJ_OK) {
            break;
        }

        status = AJ_UnmarshalArgs(msg, "s", &field);
        if (status != AJ_OK) {
            AJ_ErrPrintf(("AJ_UnmarshalArgs: %s\n", AJ_StatusText(status)));
            return LAMP_ERR_MESSAGE;
        }

        // Process the field!
        status = AJ_UnmarshalVariant(msg, (const char**) &sig);
        if (status != AJ_OK) {
            AJ_ErrPrintf(("AJ_UnmarshalVariant: %s\n", AJ_StatusText(status)));
            return LAMP_ERR_MESSAGE;
        }

        if (0 == strcmp(field, "OnOff")) {
            status = AJ_UnmarshalArgs(msg, sig, &state->onOff);
        } else if (0 == strcmp(field, "hue")) {
            status = AJ_UnmarshalArgs(msg, sig, &state->hue);
        } else if (0 == strcmp(field, "saturation")) {
            status = AJ_UnmarshalArgs(msg, sig, &state->saturation);
        } else if (0 == strcmp(field, "colorTemperature")) {
            status = AJ_UnmarshalArgs(msg, sig, &state->colorTemp);
        } else if (0 == strcmp(field, "brightness")) {
            status = AJ_UnmarshalArgs(msg, sig, &state->brightness);
        }

        status = AJ_UnmarshalCloseContainer(msg, &struct1);
        // if field invalid, throw the whole thing out and return the error
    } while (status == AJ_OK && rc == LAMP_OK);
    AJ_UnmarshalCloseContainer(msg, &array1);


    return rc;
}


#define LAMP_STATE_FD AJ_NVRAM_ID_FOR_APPS + 1


void LAMP_GetState(LampState* state)
{
    static uint8_t INIT = FALSE;
    if (INIT == FALSE) {
        AJ_NV_DATASET* id = AJ_NVRAM_Open(LAMP_STATE_FD, "r", 0);
        if (id != NULL) {
            AJ_NVRAM_Read(&TheLampState, sizeof(LampState), id);
            AJ_NVRAM_Close(id);
        }
        // else no LampState has been written yet
        INIT = TRUE;
    }

    memcpy(state, &TheLampState, sizeof(LampState));
}

void LAMP_SetState(const LampState* state)
{
    printf("\n%s\n", __FUNCTION__);
    int32_t diff = memcmp(state, &TheLampState, sizeof(LampState));

    if (diff) {
        printf("\n%s: Calling into NVRAM\n", __FUNCTION__);
        AJ_NV_DATASET* id = AJ_NVRAM_Open(LAMP_STATE_FD, "w", sizeof(LampState));
        memcpy(&TheLampState, state, sizeof(LampState));

        if (id != NULL) {
            AJ_NVRAM_Write(&TheLampState, sizeof(LampState), id);
            AJ_NVRAM_Close(id);
        }

        // this will cause the signal org.allseen.LSF.LampService.LampStateChanged
        // to be sent if there is a current session.
        LAMP_SendStateChangedSignal();
    }
}

void LAMP_ClearState(void)
{
    memset(&TheLampState, 0, sizeof(LampState));
    AJ_NVRAM_Delete(LAMP_STATE_FD);
}
