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

#include <aj_msg.h>
#include <aj_debug.h>
#include <aj_creds.h>
#include <aj_nvram.h>

#include <OEMCode.h>

#include <PropertyStoreOEMProvisioning.h>
#ifdef ONBOARDING_SERVICE
    #include <alljoyn/onboarding/OnboardingManager.h>
#endif

#include <alljoyn/services_common/PropertyStore.h>
#include <alljoyn/services_common/ServicesCommon.h>

#ifndef NDEBUG
#ifndef ER_DEBUG_PROPERTIES
#define ER_DEBUG_PROPERTIES 0
AJ_EXPORT uint8_t dbgPROPERTIES = ER_DEBUG_PROPERTIES;
#endif
#endif

/**
 * Per-module definition of the current module for debug logging.  Must be defined
 * prior to first inclusion of aj_debug.h
 */
#define AJ_MODULE LAMP_ABOUT_DATA

static const char DEFAULT_LANGUAGE[] = "en";
static const char* DEFAULT_LANGUAGES[] = { DEFAULT_LANGUAGE };
static const char SUPPORTED_LANG2[] = "de-AT";
static const char* SUPPORTED_LANGUAGES[] = { DEFAULT_LANGUAGE, SUPPORTED_LANG2 };
const char** propertyStoreDefaultLanguages = SUPPORTED_LANGUAGES;
const uint8_t AJSVC_PROPERTY_STORE_NUMBER_OF_LANGUAGES = sizeof(SUPPORTED_LANGUAGES) / sizeof(char*);

/**
 * property array of structure with defaults
 */
static const char* DEFAULT_PASSCODES[] = { "303030303030" }; // HEX encoded { '0', '0', '0', '0', '0', '0' }
#if     defined CONTROLPANEL_SERVICE
static const char* DEFAULT_APP_NAMES[] = { "Controlee" };
#elif   defined NOTIFICATION_SERVICE_PRODUCER
static const char* DEFAULT_APP_NAMES[] = { "Notifier" };
#elif   defined ONBOARDING_SERVICE
static const char* DEFAULT_APP_NAMES[] = { "Onboardee" };
#elif   defined CONFIG_SERVICE
static const char* DEFAULT_APP_NAMES[] = { "Configuree" };
#else
static const char* DEFAULT_APP_NAMES[] = { "Announcer" };
#endif
static const char DEFAULT_DESCRIPTION_LANG1[] = "AC IOE device";
static const char DEFAULT_DESCRIPTION_LANG2[] = "Mein erstes IOE Geraet";
static const char* DEFAULT_DESCRIPTIONS[] = { DEFAULT_DESCRIPTION_LANG1, DEFAULT_DESCRIPTION_LANG2 };
static const char DEFAULT_MANUFACTURER_LANG1[] = "Company A(EN)";
static const char DEFAULT_MANUFACTURER_LANG2[] = "Firma A(DE-AT)";
static const char* DEFAULT_MANUFACTURERS[] = { DEFAULT_MANUFACTURER_LANG1, DEFAULT_MANUFACTURER_LANG2 };
static const char* DEFAULT_DEVICE_MODELS[] = { "0.0.1" };
static const char* DEFAULT_DATE_OF_MANUFACTURES[] = { "2014-02-01" };
static const char* DEFAULT_SOFTWARE_VERSIONS[] = { "0.0.1" };
static const char* DEFAULT_H_ARDWARE_VERSIONS[] = { "0.0.1" };
static const char DEFAULT_SUPPORT_URL_LANG1[] = "www.company_a.com";
static const char DEFAULT_SUPPORT_URL_LANG2[] = "www.company_a.com/de-AT";
static const char* DEFAULT_SUPPORT_URLS[] = { DEFAULT_SUPPORT_URL_LANG1, DEFAULT_SUPPORT_URL_LANG2 };

const char** propertyStoreDefaultValues[AJSVC_PROPERTY_STORE_NUMBER_OF_KEYS] =
{
// "Default Values per language",                    "Key Name"
    NULL,                                           /*DeviceId*/
    NULL,                                           /*AppId*/
    NULL,                                           /*DeviceName*/
// Add other persisted keys above this line
    DEFAULT_LANGUAGES,                              /*DefaultLanguage*/
    DEFAULT_PASSCODES,                              /*Passcode*/
    NULL,                                           /*RealmName*/
// Add other configurable keys above this line
    DEFAULT_APP_NAMES,                              /*AppName*/
    DEFAULT_DESCRIPTIONS,                           /*Description*/
    DEFAULT_MANUFACTURERS,                          /*Manufacturer*/
    DEFAULT_DEVICE_MODELS,                          /*ModelNumber*/
    DEFAULT_DATE_OF_MANUFACTURES,                   /*DateOfManufacture*/
    DEFAULT_SOFTWARE_VERSIONS,                      /*SoftwareVersion*/
    NULL,                                           /*AJSoftwareVersion*/
    NULL,                                           /*MaxLength*/
// Add other mandatory about keys above this line
    DEFAULT_H_ARDWARE_VERSIONS,                      /*HardwareVersion*/
    DEFAULT_SUPPORT_URLS,                           /*SupportUrl*/
// Add other optional about keys above this line
};

/**
 * properties array of runtime values' buffers
 */
static char machineIdVar[MACHINE_ID_LENGTH + 1] = { 0 };
static char* machineIdVars[] = { machineIdVar };
static char deviceNameVar[DEVICE_NAME_VALUE_LENGTH + 1] = { 0 };
static char* deviceNameVars[] = { deviceNameVar };

static char defaultLanguageVar[LANG_VALUE_LENGTH + 1] = { 0 };
static char* defaultLanguageVars[] = { defaultLanguageVar };
static char passcodeVar[PASSWORD_VALUE_LENGTH + 1] = { 0 };
static char* passcodeVars[] = { passcodeVar };
static char realmNameVar[KEY_VALUE_LENGTH + 1] = { 0 };
static char* realmNameVars[] = { realmNameVar };

PropertyStoreConfigEntry propertyStoreRuntimeValues[AJSVC_PROPERTY_STORE_NUMBER_OF_RUNTIME_KEYS] =
{
//  {"Buffers for Values per language", "Buffer Size"},                  "Key Name"
    { machineIdVars,             MACHINE_ID_LENGTH + 1 },               /*DeviceId*/
    { machineIdVars,             MACHINE_ID_LENGTH + 1 },               /*AppId*/
    { deviceNameVars,            DEVICE_NAME_VALUE_LENGTH + 1 },        /*DeviceName*/
    { defaultLanguageVars,       LANG_VALUE_LENGTH + 1 },               /*DefaultLanguage*/
    { passcodeVars,              PASSWORD_VALUE_LENGTH + 1 },           /*Passcode*/
    { realmNameVars,             KEY_VALUE_LENGTH + 1 },                /*RealmName*/
};


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
