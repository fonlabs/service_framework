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

#include <LampAboutData.h>
#include <LampService.h>
#include <LampState.h>
#include <LampValues.h>
#include <OEMProvisioning.h>

#include <aj_msg.h>
#include <aj_debug.h>
#include <aj_creds.h>
#include <aj_nvram.h>
#include <aj_config.h>

#include <OEMCode.h>

#ifdef ONBOARDING_SERVICE
    #include <alljoyn/onboarding/OnboardingManager.h>
#endif

#include <alljoyn/services_common/PropertyStore.h>
#include <alljoyn/services_common/ServicesCommon.h>

/**
 * Per-module definition of the current module for debug logging.  Must be defined
 * prior to first inclusion of aj_debug.h
 */
#define AJ_MODULE LAMP_ABOUT_DATA

/**
 * Turn on per-module debug printing by setting this variable to non-zero value
 * (usually in debugger).
 */
#ifndef NDEBUG
uint8_t dbgLAMP_ABOUT_DATA = 1;
#endif



const char* LAMP_GetID(void)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    return AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEVICE_ID);
}

const char* LAMP_GetName(void)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    return AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEVICE_NAME);
}

void LAMP_SetName(const char* name)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    AJSVC_PropertyStore_SetValue(AJSVC_PROPERTY_STORE_DEVICE_NAME, name);
    AJSVC_PropertyStore_SaveAll();
}

static AJ_Status FactoryReset(void)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    AJSVC_PropertyStore_ResetAll();
    OEM_FactoryReset();

    // this will clear onboarding data, state, and credentials
    AJ_NVRAM_Clear();

    // Force disconnect of AJ and services and reconnection of WiFi on restart of app
    return AJ_ERR_RESTART_APP;
}

static AJ_Status Restart(void)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    OEM_Restart();
    return AJ_ERR_RESTART_APP;
}

static AJ_Status SetPasscode(const char* routerRealm, const uint8_t* newPasscode, uint8_t newPasscodeLen)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    AJ_Status status = AJ_OK;

    char newStringPasscode[PASSWORD_VALUE_LENGTH + 1];
    status = AJ_RawToHex(newPasscode, newPasscodeLen, newStringPasscode, sizeof(newStringPasscode), FALSE);
    if (status != AJ_OK) {
        return status;
    }

    // TODO: where does this get used?
    if (AJSVC_PropertyStore_SetValue(AJSVC_PROPERTY_STORE_REALM_NAME, routerRealm) &&
        AJSVC_PropertyStore_SetValue(AJSVC_PROPERTY_STORE_PASSCODE, newStringPasscode)) {

        status = AJSVC_PropertyStore_SaveAll();
        if (status != AJ_OK) {
            return status;
        }

        AJ_ClearCredentials();
        //Force disconnect of AJ and services to refresh current sessions
        status = AJ_ERR_READ;
    } else {
        status = AJSVC_PropertyStore_LoadAll();
        if (status != AJ_OK) {
            return status;
        }
    }

    return status;
}

static uint8_t IsValueValid(const char* key, const char* value)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    return TRUE;
}

#define INITIAL_PASSCODE "000000"

uint32_t LAMP_PasswordCallback(uint8_t* buffer, uint32_t bufLen)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    strcpy((char*) buffer, INITIAL_PASSCODE);
    return (uint32_t) strlen((char*) INITIAL_PASSCODE);
}

void LAMP_SetupAboutConfigData(void)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    AJCFG_Start(&FactoryReset, &Restart, &SetPasscode, &IsValueValid);

    // first read in the data from the propertystore
    PropertyStore_Init();
}
