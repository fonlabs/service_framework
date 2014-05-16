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

#include <aj_msg.h>
#include <aj_debug.h>
#include <aj_creds.h>
#include <aj_nvram.h>
#include <aj_config.h>

#include <LampValues.h>
#include <OEMProvisioning.h>
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
#define AJ_MODULE OEM_PROVISIONING

#ifndef NDEBUG
uint8_t dbgOEM_PROVISIONING = 1;
#endif

static const char DEFAULT_LANGUAGE[] = "en";
static const char* DEFAULT_LANGUAGES[] = { DEFAULT_LANGUAGE };
static const char SUPPORTED_LANG2[] = "de-AT";
static const char* SUPPORTED_LANGUAGES[] = { DEFAULT_LANGUAGE, SUPPORTED_LANG2 };
const char** propertyStoreDefaultLanguages = SUPPORTED_LANGUAGES;
const uint8_t AJSVC_PROPERTY_STORE_NUMBER_OF_LANGUAGES = sizeof(SUPPORTED_LANGUAGES) / sizeof(char*);


/**
 * property structure
 */
typedef struct _PropertyStoreEntry {
    const char* keyName; // The property key name as shown in About and Config documentation

    // msb=public/private; bit number 3 - initialise once; bit number 2 - multi-language value; bit number 1 - announce; bit number 0 - read/write
    uint8_t mode0Write : 1;
    uint8_t mode1Announce : 1;
    uint8_t mode2MultiLng : 1;
    uint8_t mode3Init : 1;
    uint8_t mode4 : 1; // always send with Config, even if nonwritable
    uint8_t mode5 : 1;
    uint8_t mode6 : 1;
    uint8_t mode7Public : 1;
} PropertyStoreEntry;

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

// Default device name goes here, in all languages
static const char DEFAULT_DEVICE_NAME_LANG1[] = "Device Name";
static const char DEFAULT_DEVICE_NAME_LANG2[] = "Device Name (DE)";
static const char* DEFAULT_NAMES[] = { DEFAULT_DEVICE_NAME_LANG1, DEFAULT_DEVICE_NAME_LANG2 };

// plain-text description of what this device is.
static const char DEFAULT_DESCRIPTION_LANG1[] = "AC IOE device";
static const char DEFAULT_DESCRIPTION_LANG2[] = "Mein erstes IOE Geraet";
static const char* DEFAULT_DESCRIPTIONS[] = { DEFAULT_DESCRIPTION_LANG1, DEFAULT_DESCRIPTION_LANG2 };

// manufacturer name goes here in all supported languages
static const char DEFAULT_MANUFACTURER_LANG1[] = "Company A(EN)";
static const char DEFAULT_MANUFACTURER_LANG2[] = "Firma A(DE-AT)";
static const char* DEFAULT_MANUFACTURERS[] = { DEFAULT_MANUFACTURER_LANG1, DEFAULT_MANUFACTURER_LANG2 };

static const char* DEFAULT_DEVICE_MODELS[] = { "0.0.1" };
static const char* DEFAULT_DATE_OF_MANUFACTURES[] = { "2014-02-01" };


extern const char HardwareVersion[];
extern const char FirmwareVersion[];

static const char* DEFAULT_SOFTWARE_VERSIONS[] = { FirmwareVersion };
static const char* DEFAULT_HARDWARE_VERSIONS[] = { HardwareVersion };

static const char DEFAULT_SUPPORT_URL_LANG1[] = "www.company_a.com";
static const char DEFAULT_SUPPORT_URL_LANG2[] = "www.company_a.com/de-AT";
static const char* DEFAULT_SUPPORT_URLS[] = { DEFAULT_SUPPORT_URL_LANG1, DEFAULT_SUPPORT_URL_LANG2 };




const char** propertyStoreDefaultValues[AJSVC_PROPERTY_STORE_NUMBER_OF_KEYS] =
{
// "Default Values per language",                    "Key Name"
    NULL,                                           /*DeviceId*/
    NULL,                                           /*AppId*/
    DEFAULT_NAMES,                                  /*DeviceName*/
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
    DEFAULT_HARDWARE_VERSIONS,                      /*HardwareVersion*/
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

/**
 * properties container for runtime values
 */
typedef struct _PropertyStoreRuntimeEntry {
    char** value;  // An array of size 1 or AJSVC_PROPERTY_STORE_NUMBER_OF_LANGUAGES mutable buffers depending on whether the property is multilingual
    uint8_t size; // The size of the value buffer(s)
} PropertyStoreConfigEntry;

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



#define AJ_PROPERTIES_NV_ID_BEGIN (AJ_NVRAM_ID_CREDS_MAX + 1)
#define AJ_PROPERTIES_NV_ID_END   (AJ_PROPERTIES_NV_ID_BEGIN + (int)AJSVC_PROPERTY_STORE_NUMBER_OF_RUNTIME_KEYS * (int)AJSVC_PROPERTY_STORE_NUMBER_OF_LANGUAGES - 1)



const PropertyStoreEntry propertyStoreProperties[AJSVC_PROPERTY_STORE_NUMBER_OF_KEYS] =
{
//  { "Key Name            ", W, A, M, I .. . . ., P },
    { "DeviceId",             0, 1, 0, 1, 0, 0, 0, 1 },
    { "AppId",                0, 1, 0, 1, 0, 0, 0, 1 },
    { "DeviceName",           1, 1, 1, 1, 0, 0, 0, 1 },
    { "DefaultLanguage",      1, 1, 0, 0, 0, 0, 0, 1 },
    { "Passcode",             1, 0, 0, 0, 0, 0, 0, 0 },
    { "RealmName",            1, 0, 0, 0, 0, 0, 0, 0 },
// Add other runtime keys above this line
    { "AppName",              0, 1, 0, 0, 0, 0, 0, 1 },
    { "Description",          0, 0, 1, 0, 0, 0, 0, 1 },
    { "Manufacturer",         0, 1, 1, 0, 1, 0, 0, 1 },
    { "ModelNumber",          0, 1, 0, 0, 0, 0, 0, 1 },
    { "DateOfManufacture",    0, 0, 0, 0, 0, 0, 0, 1 },
    { "SoftwareVersion",      0, 0, 0, 0, 0, 0, 0, 1 },
    { "AJSoftwareVersion",    0, 0, 0, 0, 0, 0, 0, 1 },
    { "MaxLength",            0, 1, 0, 0, 0, 0, 0, 1 },
// Add other mandatory about keys above this line
    { "HardwareVersion",      0, 0, 0, 0, 0, 0, 0, 1 },
    { "SupportUrl",           0, 0, 1, 0, 0, 0, 0, 1 },
// Add other optional about keys above this line
};

static const char* defaultLanguagesKeyName = { "SupportedLanguages" };

uint8_t AJSVC_PropertyStore_GetMaxValueLength(AJSVC_PropertyStoreFieldIndices fieldIndex)
{
    switch (fieldIndex) {
    case AJSVC_PROPERTY_STORE_DEVICE_NAME:
        return DEVICE_NAME_VALUE_LENGTH;

    case AJSVC_PROPERTY_STORE_DEFAULT_LANGUAGE:
        return LANG_VALUE_LENGTH;

    case AJSVC_PROPERTY_STORE_PASSCODE:
        return PASSWORD_VALUE_LENGTH;

    default:
        return KEY_VALUE_LENGTH;
    }
}

const char* AJSVC_PropertyStore_GetFieldName(AJSVC_PropertyStoreFieldIndices fieldIndex)
{
    if ((int8_t)fieldIndex <= (int8_t)AJSVC_PROPERTY_STORE_ERROR_FIELD_INDEX || (int8_t)fieldIndex >= (int8_t)AJSVC_PROPERTY_STORE_NUMBER_OF_KEYS) {
        return "N/A";
    }
    return propertyStoreProperties[fieldIndex].keyName;
}

AJSVC_PropertyStoreFieldIndices AJSVC_PropertyStore_GetFieldIndex(const char* fieldName)
{
    AJSVC_PropertyStoreFieldIndices fieldIndex = 0;
    for (; fieldIndex < AJSVC_PROPERTY_STORE_NUMBER_OF_KEYS; fieldIndex++) {
        if (!strcmp(propertyStoreProperties[fieldIndex].keyName, fieldName)) {
            return fieldIndex;
        }
    }
    return AJSVC_PROPERTY_STORE_ERROR_FIELD_INDEX;
}

static int8_t GetLanguageIndexForProperty(int8_t langIndex, AJSVC_PropertyStoreFieldIndices fieldIndex)
{
    if (propertyStoreProperties[fieldIndex].mode2MultiLng) {
        return langIndex;
    }
    return AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX;
}

const char* AJSVC_PropertyStore_GetValueForLang(AJSVC_PropertyStoreFieldIndices fieldIndex, int8_t langIndex)
{
    if ((int8_t)fieldIndex <= (int8_t)AJSVC_PROPERTY_STORE_ERROR_FIELD_INDEX || (int8_t)fieldIndex >= (int8_t)AJSVC_PROPERTY_STORE_NUMBER_OF_KEYS) {
        return NULL;
    }
    langIndex = GetLanguageIndexForProperty(langIndex, fieldIndex);
    if (langIndex <= AJSVC_PROPERTY_STORE_ERROR_LANGUAGE_INDEX || langIndex >= AJSVC_PROPERTY_STORE_NUMBER_OF_LANGUAGES) {
        return NULL;
    }
    if (fieldIndex < AJSVC_PROPERTY_STORE_NUMBER_OF_RUNTIME_KEYS &&
        (propertyStoreProperties[fieldIndex].mode0Write || propertyStoreProperties[fieldIndex].mode3Init) &&
        propertyStoreRuntimeValues[fieldIndex].value != NULL &&
        (propertyStoreRuntimeValues[fieldIndex].value[langIndex]) != NULL &&
        (propertyStoreRuntimeValues[fieldIndex].value[langIndex])[0] != '\0') {
        AJ_InfoPrintf(("Has key [%s] runtime Value [%s]\n", propertyStoreProperties[fieldIndex].keyName, propertyStoreRuntimeValues[fieldIndex].value[langIndex]));
        return propertyStoreRuntimeValues[fieldIndex].value[langIndex];
    } else if (propertyStoreDefaultValues[fieldIndex] != NULL &&
               (propertyStoreDefaultValues[fieldIndex])[langIndex] != NULL) {
        AJ_InfoPrintf(("Has key [%s] default Value [%s]\n", propertyStoreProperties[fieldIndex].keyName, (propertyStoreDefaultValues[fieldIndex])[langIndex]));
        return (propertyStoreDefaultValues[fieldIndex])[langIndex];
    }

    return NULL;
}

const char* AJSVC_PropertyStore_GetValue(AJSVC_PropertyStoreFieldIndices fieldIndex)
{
    return AJSVC_PropertyStore_GetValueForLang(fieldIndex, AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX);
}

const char* AJSVC_PropertyStore_GetLanguageName(int8_t langIndex)
{
    if (langIndex <= AJSVC_PROPERTY_STORE_ERROR_LANGUAGE_INDEX ||
        langIndex >= AJSVC_PROPERTY_STORE_NUMBER_OF_LANGUAGES) {
        return "N/A";
    }
    return propertyStoreDefaultLanguages[langIndex];
}

int8_t AJSVC_PropertyStore_GetLanguageIndex(const char* const language)
{
    uint8_t langIndex;
    const char* search = language;
    if (search != NULL) {
        if (search[0] == '\0') { // Check for empty language, if yes then search for current default language index
            search = AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEFAULT_LANGUAGE);
            if (search == NULL) {
                return AJSVC_PROPERTY_STORE_ERROR_LANGUAGE_INDEX;
            }
        }
        langIndex = AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX;
        for (; langIndex < AJSVC_PROPERTY_STORE_NUMBER_OF_LANGUAGES; langIndex++) {
            if (!strcmp(search, propertyStoreDefaultLanguages[langIndex])) {
                return (int8_t)langIndex;
            }
        }
    }
    return AJSVC_PROPERTY_STORE_ERROR_LANGUAGE_INDEX;
}

uint8_t AJSVC_PropertyStore_SetValueForLang(AJSVC_PropertyStoreFieldIndices fieldIndex, int8_t langIndex, const char* value)
{
    size_t var_size;
    if ((int8_t)fieldIndex <= (int8_t)AJSVC_PROPERTY_STORE_ERROR_FIELD_INDEX ||
        (int8_t)fieldIndex >= (int8_t)AJSVC_PROPERTY_STORE_NUMBER_OF_RUNTIME_KEYS) {
        return FALSE;
    }
    langIndex = GetLanguageIndexForProperty(langIndex, fieldIndex);
    if (langIndex <= AJSVC_PROPERTY_STORE_ERROR_LANGUAGE_INDEX || langIndex >= AJSVC_PROPERTY_STORE_NUMBER_OF_LANGUAGES) {
        return FALSE;
    }
    AJ_InfoPrintf(("Set key [%s] defaultValue [%s]\n", propertyStoreProperties[fieldIndex].keyName, value));
    var_size = propertyStoreRuntimeValues[fieldIndex].size;
    strncpy(propertyStoreRuntimeValues[fieldIndex].value[langIndex], value, var_size - 1);
    (propertyStoreRuntimeValues[fieldIndex].value[langIndex])[var_size - 1] = '\0';

    return TRUE;
}

uint8_t AJSVC_PropertyStore_SetValue(AJSVC_PropertyStoreFieldIndices fieldIndex, const char* value)
{
    return AJSVC_PropertyStore_SetValueForLang(fieldIndex, AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX, value);
}

int8_t AJSVC_PropertyStore_GetCurrentDefaultLanguageIndex()
{
    const char* currentDefaultLanguage = AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEFAULT_LANGUAGE);
    int8_t currentDefaultLanguageIndex = AJSVC_PropertyStore_GetLanguageIndex(currentDefaultLanguage);
    if (currentDefaultLanguageIndex == AJSVC_PROPERTY_STORE_ERROR_LANGUAGE_INDEX) {
        currentDefaultLanguageIndex = AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX;
        AJ_WarnPrintf(("Failed to find default language %s defaulting to %s", (currentDefaultLanguage != NULL ? currentDefaultLanguage : "NULL"), propertyStoreDefaultLanguages[AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX]));
    }
    return currentDefaultLanguageIndex;
}

static void ClearPropertiesInRAM()
{
    uint8_t langIndex;
    char* buf;
    AJSVC_PropertyStoreFieldIndices fieldIndex = 0;
    for (; fieldIndex < AJSVC_PROPERTY_STORE_NUMBER_OF_RUNTIME_KEYS; fieldIndex++) {
        if (propertyStoreRuntimeValues[fieldIndex].value) {
            langIndex = AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX;
            for (; langIndex < AJSVC_PROPERTY_STORE_NUMBER_OF_LANGUAGES; langIndex++) {
                if (propertyStoreProperties[fieldIndex].mode2MultiLng || langIndex == AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX) {
                    buf = propertyStoreRuntimeValues[fieldIndex].value[langIndex];
                    if (buf) {
                        memset(buf, 0, propertyStoreRuntimeValues[fieldIndex].size);
                    }
                }
            }
        }
    }
}

static void InitMandatoryPropertiesInRAM()
{
    char* machineIdValue = propertyStoreRuntimeValues[AJSVC_PROPERTY_STORE_APP_ID].value[AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX];
    const char* currentAppIdValue = AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_APP_ID);
    const char* currentDeviceIdValue = AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEVICE_ID);

    if (currentAppIdValue == NULL || currentAppIdValue[0] == '\0') {
        AJ_GUID theAJ_GUID;
        AJ_Status status = AJ_GetLocalGUID(&theAJ_GUID);
        if (status == AJ_OK) {
            AJ_GUID_ToString(&theAJ_GUID, machineIdValue, propertyStoreRuntimeValues[AJSVC_PROPERTY_STORE_APP_ID].size);
        }
    }

    if (currentDeviceIdValue == NULL || currentDeviceIdValue[0] == '\0') {
        AJSVC_PropertyStore_SetValue(AJSVC_PROPERTY_STORE_DEVICE_ID, machineIdValue);
    }
}



/*
 * This function is registered with About and handles property store read requests
 */
static AJ_Status AboutPropGetter(AJ_Message* msg, const char* language)
{
    AJ_Status status = AJ_ERR_INVALID;
    int8_t langIndex;
    AJSVC_PropertyStoreCategoryFilter filter;

    memset(&filter, 0, sizeof(AJSVC_PropertyStoreCategoryFilter));

    if (msg->msgId == AJ_SIGNAL_ABOUT_ANNOUNCE) {
        filter.bit2Announce = TRUE;
        langIndex = AJSVC_PropertyStore_GetLanguageIndex(language);
        status = AJ_OK;
    } else if (msg->msgId == AJ_REPLY_ID(AJ_METHOD_ABOUT_GET_ABOUT_DATA)) {
        filter.bit0About = TRUE;
        langIndex = AJSVC_PropertyStore_GetLanguageIndex(language);
        status = (langIndex == AJSVC_PROPERTY_STORE_ERROR_LANGUAGE_INDEX) ? AJ_ERR_UNKNOWN : AJ_OK;
    }
    if (status == AJ_OK) {
        status = AJSVC_PropertyStore_ReadAll(msg, filter, langIndex);
    }
    return status;
}

AJ_Status PropertyStore_Init()
{
    AJ_Status status = AJ_OK;
    status = AJSVC_PropertyStore_LoadAll();
    InitMandatoryPropertiesInRAM();
    /*
     * About needs to get values from the property store
     */
    AJ_AboutRegisterPropStoreGetter(AboutPropGetter);
    return status;
}

static AJ_Status PropertyStore_ReadConfig(uint16_t index, void* ptr, uint16_t size)
{
    AJ_Status status = AJ_OK;
    uint16_t sizeRead = 0;

    AJ_NV_DATASET* nvramHandle = AJ_NVRAM_Open(index, "r", 0);
    if (nvramHandle != NULL) {
        sizeRead = AJ_NVRAM_Read(ptr, size, nvramHandle);
        status = AJ_NVRAM_Close(nvramHandle);
        if (sizeRead != sizeRead) {
            status = AJ_ERR_WRITE;
        }
    }

    return status;
}

static AJ_Status PropertyStore_WriteConfig(uint16_t index, void* ptr, uint16_t size, char* mode)
{
    AJ_Status status = AJ_OK;
    uint16_t sizeWritten = 0;

    AJ_NV_DATASET* nvramHandle = AJ_NVRAM_Open(index, mode, size);
    if (nvramHandle != NULL) {
        sizeWritten = AJ_NVRAM_Write(ptr, size, nvramHandle);
        status = AJ_NVRAM_Close(nvramHandle);
        if (sizeWritten != size) {
            status = AJ_ERR_WRITE;
        }
    }

    return status;
}

AJ_Status AJSVC_PropertyStore_LoadAll()
{
    AJ_Status status = AJ_OK;
    void* buf = NULL;
    uint16_t size = 0;
    uint16_t entry;

    int8_t langIndex = AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX;
    for (; langIndex < AJSVC_PROPERTY_STORE_NUMBER_OF_LANGUAGES; langIndex++) {
        AJSVC_PropertyStoreFieldIndices fieldIndex = 0;
        for (; fieldIndex < AJSVC_PROPERTY_STORE_NUMBER_OF_RUNTIME_KEYS; fieldIndex++) {
            if (propertyStoreRuntimeValues[fieldIndex].value == NULL ||
                !propertyStoreProperties[fieldIndex].mode0Write ||
                (langIndex != AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX && !propertyStoreProperties[fieldIndex].mode2MultiLng)) {
                continue;
            }
            buf = propertyStoreRuntimeValues[fieldIndex].value[langIndex];
            if (buf) {
                size = propertyStoreRuntimeValues[fieldIndex].size;
                entry = (int)fieldIndex + (int)langIndex * (int)AJSVC_PROPERTY_STORE_NUMBER_OF_RUNTIME_KEYS;
                status = PropertyStore_ReadConfig(AJ_PROPERTIES_NV_ID_BEGIN + entry, buf, size);
                AJ_InfoPrintf(("nvram read fieldIndex=%d [%s] langIndex=%d [%s] entry=%d val=%s size=%u status=%s\n", (int)fieldIndex, propertyStoreProperties[fieldIndex].keyName, (int)langIndex, propertyStoreDefaultLanguages[langIndex], (int)entry, propertyStoreRuntimeValues[fieldIndex].value[langIndex], (int)size, AJ_StatusText(status)));
            }
        }
    }

    return status;
}

AJ_Status AJSVC_PropertyStore_SaveAll()
{
    AJ_Status status = AJ_OK;
    void* buf = NULL;
    uint16_t size = 0;
    uint16_t entry;

    int8_t langIndex = AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX;
    for (; langIndex < AJSVC_PROPERTY_STORE_NUMBER_OF_LANGUAGES; langIndex++) {
        AJSVC_PropertyStoreFieldIndices fieldIndex = 0;
        for (; fieldIndex < AJSVC_PROPERTY_STORE_NUMBER_OF_RUNTIME_KEYS; fieldIndex++) {
            if (propertyStoreRuntimeValues[fieldIndex].value == NULL ||
                !propertyStoreProperties[fieldIndex].mode0Write ||
                (langIndex != AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX && !propertyStoreProperties[fieldIndex].mode2MultiLng)) {
                continue;
            }
            buf = propertyStoreRuntimeValues[fieldIndex].value[langIndex];
            if (buf) {
                size = propertyStoreRuntimeValues[fieldIndex].size;
                entry = (int)fieldIndex + (int)langIndex * (int)AJSVC_PROPERTY_STORE_NUMBER_OF_RUNTIME_KEYS;
                status = PropertyStore_WriteConfig(AJ_PROPERTIES_NV_ID_BEGIN + entry, buf, size, "w");
                AJ_InfoPrintf(("nvram write fieldIndex=%d [%s] langIndex=%d [%s] entry=%d val=%s size=%u status=%s\n", (int)fieldIndex, propertyStoreProperties[fieldIndex].keyName, (int)langIndex, propertyStoreDefaultLanguages[langIndex], (int)entry, propertyStoreRuntimeValues[fieldIndex].value[langIndex], (int)size, AJ_StatusText(status)));
            }
        }
    }
    AJ_AboutSetShouldAnnounce(); // Set flag for sending an updated Announcement
    return status;
}

static uint8_t UpdateFieldInRAM(AJSVC_PropertyStoreFieldIndices fieldIndex, int8_t langIndex, const char* fieldValue)
{
    uint8_t ret = FALSE;

    if (propertyStoreProperties[fieldIndex].mode0Write && propertyStoreProperties[fieldIndex].mode7Public) {
        ret = AJSVC_PropertyStore_SetValueForLang(fieldIndex, langIndex, fieldValue);
    } else {
        AJ_ErrPrintf(("UpdateFieldInRAM ERROR - field %s has read only attribute or is private\n", propertyStoreProperties[fieldIndex].keyName));
    }

    return ret;
}

static uint8_t DeleteFieldFromRAM(AJSVC_PropertyStoreFieldIndices fieldIndex, int8_t langIndex)
{
    return UpdateFieldInRAM(fieldIndex, langIndex, "");
}

AJ_Status AJSVC_PropertyStore_ReadAll(AJ_Message* msg, AJSVC_PropertyStoreCategoryFilter filter, int8_t langIndex)
{
    AJ_Status status = AJ_OK;
    AJ_Arg array;
    AJ_Arg array2;
    AJ_Arg dict;
    const char* value;
    AJ_Arg arg;
    uint8_t rawValue[16];
    uint8_t index;
    const char* ajVersion;
    AJSVC_PropertyStoreFieldIndices fieldIndex = 0;

    AJ_InfoPrintf(("PropertyStore_ReadAll()\n"));

    status = AJ_MarshalContainer(msg, &array, AJ_ARG_ARRAY);
    if (status != AJ_OK) {
        return status;
    }

    for (; fieldIndex < AJSVC_PROPERTY_STORE_NUMBER_OF_KEYS; fieldIndex++) {
        if (propertyStoreProperties[fieldIndex].mode7Public &&
            (filter.bit0About
             || propertyStoreProperties[fieldIndex].mode4
             || (filter.bit1Config && propertyStoreProperties[fieldIndex].mode0Write)
             || (filter.bit2Announce && propertyStoreProperties[fieldIndex].mode1Announce))) {
            value = AJSVC_PropertyStore_GetValueForLang(fieldIndex, langIndex);

            if (value == NULL && (int8_t)fieldIndex >= (int8_t)AJSVC_PROPERTY_STORE_NUMBER_OF_MANDATORY_KEYS) {     // Non existing values are skipped!
                AJ_WarnPrintf(("PropertyStore_ReadAll - Failed to get value for field=(name=%s, index=%d) and language=(name=%s, index=%d), skipping.\n", AJSVC_PropertyStore_GetFieldName(fieldIndex), (int)fieldIndex, AJSVC_PropertyStore_GetLanguageName(langIndex), (int)langIndex));
            } else {
                if (fieldIndex == AJSVC_PROPERTY_STORE_APP_ID) {
                    if (value == NULL) {
                        AJ_ErrPrintf(("PropertyStore_ReadAll - Failed to get value for mandatory field=(name=%s, index=%d) and language=(name=%s, index=%d), aborting.\n", AJSVC_PropertyStore_GetFieldName(fieldIndex), (int)fieldIndex, AJSVC_PropertyStore_GetLanguageName(langIndex), (int)langIndex));
                        return AJ_ERR_NULL;
                    }

                    status = AJ_MarshalContainer(msg, &dict, AJ_ARG_DICT_ENTRY);
                    if (status != AJ_OK) {
                        return status;
                    }
                    status = AJ_MarshalArgs(msg, "s", propertyStoreProperties[fieldIndex].keyName);
                    if (status != AJ_OK) {
                        return status;
                    }

                    status = AJ_MarshalVariant(msg, "ay");
                    if (status != AJ_OK) {
                        return status;
                    }
                    status = AJ_HexToRaw(value, 0, rawValue, sizeof(rawValue));
                    if (status != AJ_OK) {
                        return status;
                    }
                    status = AJ_MarshalArg(msg, AJ_InitArg(&arg, AJ_ARG_BYTE, AJ_ARRAY_FLAG, rawValue, sizeof(rawValue)));
                    if (status != AJ_OK) {
                        return status;
                    }

                    status = AJ_MarshalCloseContainer(msg, &dict);
                    if (status != AJ_OK) {
                        return status;
                    }
                } else if (fieldIndex == AJSVC_PROPERTY_STORE_MAX_LENGTH) {
                    status = AJ_MarshalContainer(msg, &dict, AJ_ARG_DICT_ENTRY);
                    if (status != AJ_OK) {
                        return status;
                    }
                    status = AJ_MarshalArgs(msg, "s", propertyStoreProperties[fieldIndex].keyName);
                    if (status != AJ_OK) {
                        return status;
                    }

                    status = AJ_MarshalVariant(msg, "q");
                    if (status != AJ_OK) {
                        return status;
                    }
                    status = AJ_MarshalArgs(msg, "q", DEVICE_NAME_VALUE_LENGTH);
                    if (status != AJ_OK) {
                        return status;
                    }

                    status = AJ_MarshalCloseContainer(msg, &dict);
                    if (status != AJ_OK) {
                        return status;
                    }
                    AJ_InfoPrintf(("Has key [%s] runtime Value [%d]\n", propertyStoreProperties[AJSVC_PROPERTY_STORE_MAX_LENGTH].keyName, DEVICE_NAME_VALUE_LENGTH));
                } else if (fieldIndex == AJSVC_PROPERTY_STORE_AJ_SOFTWARE_VERSION) {
                    ajVersion = AJ_GetVersion();
                    if (ajVersion == NULL) {
                        AJ_ErrPrintf(("PropertyStore_ReadAll - Failed to get value for mandatory field=(name=%s, index=%d) and language=(name=%s, index=%d), aborting.\n", AJSVC_PropertyStore_GetFieldName(fieldIndex), (int)fieldIndex, AJSVC_PropertyStore_GetLanguageName(langIndex), (int)langIndex));
                        return AJ_ERR_NULL;
                    }

                    status = AJ_MarshalContainer(msg, &dict, AJ_ARG_DICT_ENTRY);
                    if (status != AJ_OK) {
                        return status;
                    }
                    status = AJ_MarshalArgs(msg, "s", propertyStoreProperties[fieldIndex].keyName);
                    if (status != AJ_OK) {
                        return status;
                    }

                    status = AJ_MarshalVariant(msg, "s");
                    if (status != AJ_OK) {
                        return status;
                    }
                    status = AJ_MarshalArgs(msg, "s", ajVersion);
                    if (status != AJ_OK) {
                        return status;
                    }

                    status = AJ_MarshalCloseContainer(msg, &dict);
                    if (status != AJ_OK) {
                        return status;
                    }
                    AJ_InfoPrintf(("Has key [%s] runtime Value [%s]\n", propertyStoreProperties[AJSVC_PROPERTY_STORE_AJ_SOFTWARE_VERSION].keyName, ajVersion));
                } else {
                    if (value == NULL) {
                        AJ_ErrPrintf(("PropertyStore_ReadAll - Failed to get value for mandatory field=(name=%s, index=%d) and language=(name=%s, index=%d), aborting.\n", AJSVC_PropertyStore_GetFieldName(fieldIndex), (int)fieldIndex, AJSVC_PropertyStore_GetLanguageName(langIndex), (int)langIndex));
                        return AJ_ERR_NULL;
                    }

                    status = AJ_MarshalContainer(msg, &dict, AJ_ARG_DICT_ENTRY);
                    if (status != AJ_OK) {
                        return status;
                    }
                    status = AJ_MarshalArgs(msg, "s", propertyStoreProperties[fieldIndex].keyName);
                    if (status != AJ_OK) {
                        return status;
                    }

                    status = AJ_MarshalVariant(msg, "s");
                    if (status != AJ_OK) {
                        return status;
                    }
                    status = AJ_MarshalArgs(msg, "s", value);
                    if (status != AJ_OK) {
                        return status;
                    }

                    status = AJ_MarshalCloseContainer(msg, &dict);
                    if (status != AJ_OK) {
                        return status;
                    }
                }
            }
        }
    }

    if (filter.bit0About) {
        // Add supported languages
        status = AJ_MarshalContainer(msg, &dict, AJ_ARG_DICT_ENTRY);
        if (status != AJ_OK) {
            return status;
        }
        status = AJ_MarshalArgs(msg, "s", defaultLanguagesKeyName);
        if (status != AJ_OK) {
            return status;
        }
        status = AJ_MarshalVariant(msg, "as");
        if (status != AJ_OK) {
            return status;
        }
        status = AJ_MarshalContainer(msg, &array2, AJ_ARG_ARRAY);
        if (status != AJ_OK) {
            return status;
        }

        index = AJSVC_PROPERTY_STORE_NO_LANGUAGE_INDEX;
        for (; index < AJSVC_PROPERTY_STORE_NUMBER_OF_LANGUAGES; index++) {
            status = AJ_MarshalArgs(msg, "s", propertyStoreDefaultLanguages[index]);
            if (status != AJ_OK) {
                return status;
            }
        }

        status = AJ_MarshalCloseContainer(msg, &array2);
        if (status != AJ_OK) {
            return status;
        }
        status = AJ_MarshalCloseContainer(msg, &dict);
        if (status != AJ_OK) {
            return status;
        }
    }
    status = AJ_MarshalCloseContainer(msg, &array);
    if (status != AJ_OK) {
        return status;
    }

    return status;
}

AJ_Status AJSVC_PropertyStore_Update(const char* key, int8_t langIndex, const char* value)
{
    AJSVC_PropertyStoreFieldIndices fieldIndex = AJSVC_PropertyStore_GetFieldIndex(key);
    if (fieldIndex == AJSVC_PROPERTY_STORE_ERROR_FIELD_INDEX || fieldIndex >= AJSVC_PROPERTY_STORE_NUMBER_OF_RUNTIME_KEYS) {
        return AJ_ERR_INVALID;
    }
    if (!UpdateFieldInRAM(fieldIndex, langIndex, value)) {
        return AJ_ERR_FAILURE;
    }
    return AJ_OK;
}

AJ_Status AJSVC_PropertyStore_Reset(const char* key, int8_t langIndex)
{
    AJSVC_PropertyStoreFieldIndices fieldIndex = AJSVC_PropertyStore_GetFieldIndex(key);
    if (fieldIndex == AJSVC_PROPERTY_STORE_ERROR_FIELD_INDEX || fieldIndex >= AJSVC_PROPERTY_STORE_NUMBER_OF_RUNTIME_KEYS) {
        return AJ_ERR_INVALID;
    }
    if (!DeleteFieldFromRAM(fieldIndex, langIndex)) {
        return AJ_ERR_FAILURE;
    }
    InitMandatoryPropertiesInRAM();
    return AJ_OK;
}

AJ_Status AJSVC_PropertyStore_ResetAll()
{
    ClearPropertiesInRAM();
    InitMandatoryPropertiesInRAM();
    return AJSVC_PropertyStore_SaveAll();
}
