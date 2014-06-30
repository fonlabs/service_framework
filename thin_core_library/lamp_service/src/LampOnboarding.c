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

#ifdef ONBOARDING_SERVICE

/**
 * Per-module definition of the current module for debug logging.  Must be defined
 * prior to first inclusion of aj_debug.h
 */
#define AJ_MODULE LAMP_ONBOARDING

#include <alljoyn.h>
#include <aj_nvram.h>
#include <LampOnboarding.h>
#include <LampAboutData.h>
#include <OEM_LS_Code.h>

#include <alljoyn/onboarding/OnboardingService.h>
#include <alljoyn/onboarding/OnboardingManager.h>
#include <alljoyn/services_common/PropertyStore.h>
#include <alljoyn/services_common/ServicesCommon.h>
#include <alljoyn/services_common/ServicesHandlers.h>

/**
 * Turn on per-module debug printing by setting this variable to non-zero value
 * (usually in debugger).
 */
#ifndef NDEBUG
uint8_t dbgLAMP_ONBOARDING = 1;
#endif

static const char* GenerateSoftAPSSID(char* obSoftAPssid)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    const char* deviceId;
    size_t deviceIdLen;
    char serialId[AJOBS_DEVICE_SERIAL_ID_LEN + 1] = { 0 };
    size_t serialIdLen;
    char product[AJOBS_DEVICE_PRODUCT_NAME_LEN + 1] = { 0 };
    size_t productLen;

    const char* deviceProductName = AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEVICE_NAME);

    if ((obSoftAPssid[0] == '\0') && (deviceProductName != NULL)) {
        deviceId = AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEVICE_ID);
        if (deviceId != NULL) {
            deviceIdLen = strlen(deviceId);
            productLen = min(strlen(deviceProductName), AJOBS_DEVICE_PRODUCT_NAME_LEN + AJOBS_DEVICE_MANUFACTURE_NAME_LEN);

            serialIdLen = min(deviceIdLen, AJOBS_DEVICE_SERIAL_ID_LEN);
            memcpy(product, deviceProductName, productLen);
            product[productLen] = '\0';

            // can't have spaces in SSID
            {
                size_t i = 0;
                for (; i < serialIdLen; ++i) {
                    if (product[i] == ' ') {
                        product[i] = '_';
                    }
                }
            }

            memcpy(serialId, deviceId + (deviceIdLen - serialIdLen), serialIdLen);
            serialId[serialIdLen] = '\0';
            snprintf(obSoftAPssid, AJOBS_SSID_MAX_LENGTH + 1, "AJ_%s_%s", product, serialId);
        }
    }

    AJ_AlwaysPrintf(("%s: SoftAP: %s\n", __FUNCTION__, obSoftAPssid));
    return obSoftAPssid;
}

#define AJ_OBS_OBINFO_NV_ID (AJ_NVRAM_ID_FOR_APPS - 2)

AJ_Status LAMP_InitOnboarding(void)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    AJ_Status status = AJ_OK;

    AJOBS_RegisterObjectList();

    GenerateSoftAPSSID(OEM_LS_OnboardingSettings.AJOBS_SoftAPSSID);
    status = AJOBS_Start(&OEM_LS_OnboardingSettings);
    return status;
}

#endif
