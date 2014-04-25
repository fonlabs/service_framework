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

#include <alljoyn/onboarding/OnboardingService.h>
#include <alljoyn/onboarding/OnboardingManager.h>
#include <PropertyStoreOEMProvisioning.h>
#include <alljoyn/services_common/PropertyStore.h>
#include <alljoyn/services_common/ServicesCommon.h>
#include <alljoyn/services_common/ServicesHandlers.h>

static const char* GenerateSoftAPSSID(char* obSoftAPssid)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    const char* deviceId;
    size_t deviceIdLen;
    char manufacture[AJOBS_DEVICE_MANUFACTURE_NAME_LEN + 1] = { 0 };
    size_t manufacureLen;
    char product[AJOBS_DEVICE_PRODUCT_NAME_LEN + 1] = { 0 };
    size_t productLen;
    char serialId[AJOBS_DEVICE_SERIAL_ID_LEN + 1] = { 0 };
    size_t serialIdLen;

    if (obSoftAPssid[0] == '\0') {
        deviceId = AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEVICE_ID);
        deviceIdLen = strlen(deviceId);
        manufacureLen = min(strlen(deviceManufactureName), AJOBS_DEVICE_MANUFACTURE_NAME_LEN);
        productLen = min(strlen(deviceProductName), AJOBS_DEVICE_PRODUCT_NAME_LEN);
        serialIdLen = min(deviceIdLen, AJOBS_DEVICE_SERIAL_ID_LEN);
        memcpy(manufacture, deviceManufactureName, manufacureLen);
        manufacture[manufacureLen] = '\0';
        memcpy(product, deviceProductName, productLen);
        product[productLen] = '\0';
        memcpy(serialId, deviceId + (deviceIdLen - serialIdLen), serialIdLen);
        serialId[serialIdLen] = '\0';
        snprintf(obSoftAPssid, AJOBS_SSID_MAX_LENGTH + 1, "AJ_%s_%s_%s", manufacture, product, serialId);
    }

    AJ_InfoPrintf(("%s: SoftAP: %s\n", __FUNCTION__, obSoftAPssid));
    return obSoftAPssid;
}

#define AJ_OBS_OBINFO_NV_ID (AJ_PROPERTIES_NV_ID_END + 1)

AJ_Status OnboardingReadInfo(AJOBS_Info* info)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    AJ_Status status = AJ_OK;
    size_t size = sizeof(AJOBS_Info);
    AJ_NV_DATASET* nvramHandle;

    if (NULL == info) {
        return AJ_ERR_NULL;
    }
    memset(info, 0, size);

    nvramHandle = AJ_NVRAM_Open(AJ_OBS_OBINFO_NV_ID, "r", 0);
    if (nvramHandle != NULL) {
        size_t sizeRead = AJ_NVRAM_Read(info, size, nvramHandle);
        status = AJ_NVRAM_Close(nvramHandle);
        if (sizeRead != sizeRead) {
            status = AJ_ERR_READ;
        } else {
            AJ_AlwaysPrintf(("Read Info values: state=%d, ssid=%s authType=%d pc=%s\n", info->state, info->ssid, info->authType, info->pc));
        }
    } else {
        status = AJ_ERR_INVALID;
    }

    return status;
}

AJ_Status OnboardingWriteInfo(AJOBS_Info* info)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    AJ_Status status = AJ_OK;
    size_t size = sizeof(AJOBS_Info);
    AJ_NV_DATASET* nvramHandle;

    if (NULL == info) {
        return AJ_ERR_NULL;
    }

    AJ_AlwaysPrintf(("Going to write Info values: state=%d, ssid=%s authType=%d pc=%s\n", info->state, info->ssid, info->authType, info->pc));

    nvramHandle = AJ_NVRAM_Open(AJ_OBS_OBINFO_NV_ID, "w", size);
    if (nvramHandle != NULL) {
        size_t sizeWritten = AJ_NVRAM_Write(info, size, nvramHandle);
        status = AJ_NVRAM_Close(nvramHandle);
        if (sizeWritten != size) {
            status = AJ_ERR_WRITE;
        }
    } else {
        status = AJ_ERR_WRITE;
    }

    return status;
}

static AJOBS_Settings obSettings = AJOBS_DEFAULT_SETTINGS;

AJ_Status LAMP_InitOnboarding(void)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    AJ_Status status = AJ_OK;
    GenerateSoftAPSSID(obSettings.AJOBS_SoftAPSSID);
    status = AJOBS_Start(&obSettings, &OnboardingReadInfo, &OnboardingWriteInfo);
    return status;
}

#endif
