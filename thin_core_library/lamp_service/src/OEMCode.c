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

#include <OEMCode.h>
#include <LampState.h>
#include <LampAboutData.h>
#include <OEMProvisioning.h>
#include <alljoyn/services_common/PropertyStore.h>
#include <alljoyn/services_common/ServicesCommon.h>

#include <aj_creds.h>
#include <aj_nvram.h>
#include <aj_crypto.h>

/**
 * Per-module definition of the current module for debug logging.  Must be defined
 * prior to first inclusion of aj_debug.h
 */
#define AJ_MODULE OEM_CODE

#ifndef NDEBUG
uint8_t dbgOEM_CODE = 1;
#endif

const char FirmwareVersion[] = "1.0";
const char HardwareVersion[] = "1.0";

LampResponseCode OEM_ApplyPulseEffect(LampState* fromState, LampState* toState, uint32_t period, uint32_t duration, uint32_t numPulses, uint64_t startTimeStamp)
{
    AJ_InfoPrintf(("%s: fromState(Hue=%u,Saturation=%u,colorTemp=%u,Brightness=%u,OnOff=%u), toState(Hue=%u,Saturation=%u,colorTemp=%u,Brightness=%u,OnOff=%u), period=%u, ratio=%u, numPulses=%u, start=%u/%u\n", __FUNCTION__,
                   fromState->hue, fromState->saturation, fromState->colorTemp, fromState->brightness, fromState->onOff,
                   toState->hue, toState->saturation, toState->colorTemp, toState->brightness, toState->onOff,
                   period, duration, numPulses, (uint32_t) (startTimeStamp), (uint32_t) (startTimeStamp >> 32)));
    return LAMP_OK;
}

LampResponseCode OEM_TransitionLampState(LampState* newState, uint64_t timestamp, uint32_t transition_period)
{
    AJ_InfoPrintf(("%s: (Hue=%u,Saturation=%u,colorTemp=%u,Brightness=%u,OnOff=%u)\n", __FUNCTION__,
                   newState->hue, newState->saturation, newState->colorTemp, newState->brightness, newState->onOff));

    LAMP_SetState(newState);
    return LAMP_OK;
}



void OEM_Initialize(void)
{
    // TODO: vendor-specific initialization goes here
}

const char* OEM_GetFaultsText()
{
    return "Some notification";
}

void OEM_InitialState(LampState* state)
{
    state->onOff = TRUE;
    state->colorTemp = 0;
    state->brightness = 0;
    state->hue = 0;
    state->saturation = 0;
}

#ifdef ONBOARDING_SERVICE

AJOBS_Settings OEM_OnboardingSettings = {
    .AJOBS_WAIT_FOR_SOFTAP_CONNECTION = 600000,
    .AJOBS_MAX_RETRIES = 2,
    .AJOBS_WAIT_BETWEEN_RETRIES = 180000,
    // do not mess with AJOBS_SoftAPSSID; it will be overwritten.
    // changing the format of the SSID will break compatibility with AJ-On
    .AJOBS_SoftAPSSID = { 0 },
    .AJOBS_SoftAPIsHidden = FALSE,
    .AJOBS_SoftAPPassphrase = NULL
};

#endif

void OEM_Restart(void)
{
    AJ_InfoPrintf(("%s\n", __FUNCTION__));
    // TODO: restart the device
}

void OEM_FactoryReset(void)
{
    AJ_InfoPrintf(("%s\n", __FUNCTION__));
    // TODO: anything related to the factory reset
}


uint32_t OEM_GetEnergyUsageMilliwatts()
{
    return 15;
}


uint32_t OEM_GetBrightnessLumens()
{
    return 100;
}

LampResponseCode OEM_GetLampFaults(AJ_Message* msg)
{
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));
    // this is an example of what this function might look like!
    LampFaultCode faults[2] = { 1, 4 };
    size_t i = 0;

    for (i = 0; i < 2; ++i) {
        AJ_MarshalArgs(msg, "u", (LampFaultCode) faults[i]);
    }

    return LAMP_OK;
}

LampResponseCode OEM_ClearLampFault(LampFaultCode fault)
{
    AJ_InfoPrintf(("\n%s: code=%d\n", __FUNCTION__, fault));
    // TOOD: clear the fault code
    return LAMP_OK;
}


AJ_Status OEM_GetLampParameters(AJ_Message* msg)
{
    AJ_MarshalArgs(msg, "{sv}", "Energy_Usage_Milliwatts", "u", OEM_GetEnergyUsageMilliwatts());
    AJ_MarshalArgs(msg, "{sv}", "Brightness_Lumens", "u", OEM_GetBrightnessLumens());
    return AJ_OK;
}


LampDetails_t LampDetails = {
    .lampMake = MAKE_LIFX,
    .lampModel = MODEL_LED,
    .deviceType = TYPE_LAMP,
    .lampType = LAMPTYPE_PAR30,
    .baseType = BASETYPE_E17,
    .deviceLampBeamAngle = 100,
    .deviceDimmable = TRUE,
    .deviceColor = TRUE,
    .variableColorTemp = TRUE,
    .deviceHasEffects = TRUE,
    .deviceMinVoltage = 120,
    .deviceMaxVoltage = 120,
    .deviceWattage = 9,
    .deviceIncandescentEquivalent = 75,
    .deviceMaxLumens = 900,
    .deviceMinTemperature = 2700,
    .deviceMaxTemperature = 5000,
    .deviceColorRenderingIndex = 0
};


LampResponseCode LAMP_MarshalDetails(AJ_Message* msg)
{
    AJ_InfoPrintf(("%s", __FUNCTION__));

    AJ_MarshalArgs(msg, "{sv}", "LampVersion", "u", LAMP_GetServiceVersion());

    AJ_MarshalArgs(msg, "{sv}", "Make", "u", LampDetails.lampMake);
    AJ_MarshalArgs(msg, "{sv}", "Model", "u", LampDetails.lampModel);
    AJ_MarshalArgs(msg, "{sv}", "Type", "u", LampDetails.deviceType);
    AJ_MarshalArgs(msg, "{sv}", "LampType", "u", LampDetails.lampType);
    AJ_MarshalArgs(msg, "{sv}", "LampBaseType", "u", LampDetails.baseType);


    AJ_MarshalArgs(msg, "{sv}", "LampBeamAngle", "u", LampDetails.deviceLampBeamAngle);

    AJ_MarshalArgs(msg, "{sv}", "Dimmable", "b", (LampDetails.deviceDimmable ? TRUE : FALSE));
    AJ_MarshalArgs(msg, "{sv}", "Color", "b", (LampDetails.deviceColor ? TRUE : FALSE));
    AJ_MarshalArgs(msg, "{sv}", "VariableColorTemp", "b", (LampDetails.variableColorTemp ? TRUE : FALSE));
    AJ_MarshalArgs(msg, "{sv}", "HasEffects", "b", (LampDetails.deviceHasEffects ? TRUE : FALSE));

    AJ_MarshalArgs(msg, "{sv}", "MinVoltage", "u", LampDetails.deviceMinVoltage);
    AJ_MarshalArgs(msg, "{sv}", "MaxVoltage", "u", LampDetails.deviceMaxVoltage);
    AJ_MarshalArgs(msg, "{sv}", "Wattage", "u", LampDetails.deviceWattage);
    AJ_MarshalArgs(msg, "{sv}", "IncandescentEquivalent", "u", LampDetails.deviceIncandescentEquivalent);
    AJ_MarshalArgs(msg, "{sv}", "MaxLumens", "u", LampDetails.deviceMaxLumens);
    AJ_MarshalArgs(msg, "{sv}", "MinTemperature", "u", LampDetails.deviceMinTemperature);
    AJ_MarshalArgs(msg, "{sv}", "MaxTemperature", "u", LampDetails.deviceMaxTemperature);

    AJ_MarshalArgs(msg, "{sv}", "ColorRenderingIndex", "u", LampDetails.deviceColorRenderingIndex);

    AJ_MarshalArgs(msg, "{sv}", "LampID", "s", AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEVICE_ID));
    return LAMP_OK;
}
