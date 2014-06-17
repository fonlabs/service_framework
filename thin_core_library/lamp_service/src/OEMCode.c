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

LampResponseCode OEM_SetLampOnOff(uint8_t onoff)
{
    LampState state;
    AJ_InfoPrintf(("%s: %u\n", __FUNCTION__, onoff));
    LAMP_GetState(&state);
    state.onOff = onoff;
    LAMP_SetState(&state);
    return LAMP_OK;
}

LampResponseCode OEM_SetLampHue(uint32_t hue)
{
    LampState state;
    AJ_InfoPrintf(("%s: %u\n", __FUNCTION__, hue));
    LAMP_GetState(&state);
    state.hue = hue;
    LAMP_SetState(&state);
    return LAMP_OK;
}

LampResponseCode OEM_SetLampBrightness(uint32_t brightness)
{
    LampState state;
    AJ_InfoPrintf(("%s: %u\n", __FUNCTION__, brightness));
    LAMP_GetState(&state);
    state.brightness = brightness;
    LAMP_SetState(&state);
    return LAMP_OK;
}

LampResponseCode OEM_SetLampSaturation(uint32_t saturation)
{
    LampState state;
    AJ_InfoPrintf(("%s: %u\n", __FUNCTION__, saturation));
    LAMP_GetState(&state);
    state.saturation = saturation;
    LAMP_SetState(&state);
    return LAMP_OK;
}

LampResponseCode OEM_SetLampColorTemp(uint32_t colorTemp)
{
    LampState state;
    AJ_InfoPrintf(("%s: %u\n", __FUNCTION__, colorTemp));
    LAMP_GetState(&state);
    state.colorTemp = colorTemp;
    LAMP_SetState(&state);
    return LAMP_OK;
}

LampResponseCode OEM_ApplyPulseEffect(LampState* fromState, LampState* toState, uint32_t period, uint32_t duration, uint32_t numPulses, uint64_t timestamp)
{
    AJ_InfoPrintf(("%s: fromState(Hue=%u,Saturation=%u,colorTemp=%u,Brightness=%u,OnOff=%u), toState(Hue=%u,Saturation=%u,colorTemp=%u,Brightness=%u,OnOff=%u), period=%u, ratio=%u, numPulses=%u, start=%u/%u\n", __FUNCTION__,
                   fromState->hue, fromState->saturation, fromState->colorTemp, fromState->brightness, fromState->onOff,
                   toState->hue, toState->saturation, toState->colorTemp, toState->brightness, toState->onOff,
                   period, duration, numPulses, (uint32_t) (timestamp), (uint32_t) (timestamp >> 32)));
    return LAMP_OK;
}

LampResponseCode OEM_TransitionLampState(LampState* newState, uint64_t timestamp, uint32_t transitionPeriod)
{
    AJ_InfoPrintf(("%s: (Hue=%u,Saturation=%u,colorTemp=%u,Brightness=%u,OnOff=%u), transitionPeriod=%u\n", __FUNCTION__,
                   newState->hue, newState->saturation, newState->colorTemp, newState->brightness, newState->onOff, transitionPeriod));

    LAMP_SetState(newState);
    return LAMP_OK;
}

void OEM_Initialize(void)
{
    // TODO: vendor-specific initialization goes here
}

const char* OEM_GetFaultsText(void)
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

uint32_t OEM_GetEnergyUsageMilliwatts(void)
{
    uint32_t energyUsageMilliwatts = 15;
    AJ_InfoPrintf(("%s: energy usage %u mW\n", __FUNCTION__, energyUsageMilliwatts));
    return energyUsageMilliwatts;
}

uint32_t OEM_GetBrightnessLumens(void)
{
    uint32_t brightnessLumens = 100;
    AJ_InfoPrintf(("%s: brightness %u lumens\n", __FUNCTION__, brightnessLumens));
    return brightnessLumens;
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
    AJ_InfoPrintf(("\n%s\n", __FUNCTION__));

    AJ_InfoPrintf(("Energy_Usage_Milliwatts: %u\n", OEM_GetEnergyUsageMilliwatts()));
    AJ_MarshalArgs(msg, "{sv}", "Energy_Usage_Milliwatts", "u", OEM_GetEnergyUsageMilliwatts());
    AJ_InfoPrintf(("Brightness_Lumens: %u\n", OEM_GetBrightnessLumens()));
    AJ_MarshalArgs(msg, "{sv}", "Brightness_Lumens", "u", OEM_GetBrightnessLumens());
    return AJ_OK;
}

const LampDetailsStruct LampDetails = {
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


    AJ_InfoPrintf(("LampVersion: %u\n", LAMP_GetServiceVersion()));
    AJ_MarshalArgs(msg, "{sv}", "LampVersion", "u", LAMP_GetServiceVersion());

    AJ_InfoPrintf(("Make: %u\n", LampDetails.lampMake));
    AJ_MarshalArgs(msg, "{sv}", "Make", "u", LampDetails.lampMake);

    AJ_InfoPrintf(("Model: %u\n", LampDetails.lampModel));
    AJ_MarshalArgs(msg, "{sv}", "Model", "u", LampDetails.lampModel);

    AJ_InfoPrintf(("Type: %u\n", LampDetails.deviceType));
    AJ_MarshalArgs(msg, "{sv}", "Type", "u", LampDetails.deviceType);

    AJ_InfoPrintf(("LampType: %u\n", LampDetails.lampType));
    AJ_MarshalArgs(msg, "{sv}", "LampType", "u", LampDetails.lampType);

    AJ_InfoPrintf(("LampBaseType: %u\n", LampDetails.baseType));
    AJ_MarshalArgs(msg, "{sv}", "LampBaseType", "u", LampDetails.baseType);


    AJ_InfoPrintf(("LampBeamAngle: %u\n", LampDetails.deviceLampBeamAngle));
    AJ_MarshalArgs(msg, "{sv}", "LampBeamAngle", "u", LampDetails.deviceLampBeamAngle);

    AJ_InfoPrintf(("Dimmable: %s\n", (LampDetails.deviceDimmable ? "TRUE" : "FALSE")));
    AJ_MarshalArgs(msg, "{sv}", "Dimmable", "b", (LampDetails.deviceDimmable ? TRUE : FALSE));
    AJ_InfoPrintf(("Color: %s\n", (LampDetails.deviceColor ? "TRUE" : "FALSE")));
    AJ_MarshalArgs(msg, "{sv}", "Color", "b", (LampDetails.deviceColor ? TRUE : FALSE));
    AJ_InfoPrintf(("VariableColorTemp: %s\n", (LampDetails.variableColorTemp ? "TRUE" : "FALSE")));
    AJ_MarshalArgs(msg, "{sv}", "VariableColorTemp", "b", (LampDetails.variableColorTemp ? TRUE : FALSE));
    AJ_InfoPrintf(("HasEffects: %s\n", (LampDetails.deviceHasEffects ? "TRUE" : "FALSE")));
    AJ_MarshalArgs(msg, "{sv}", "HasEffects", "b", (LampDetails.deviceHasEffects ? TRUE : FALSE));

    AJ_InfoPrintf(("MinVoltage: %u\n", LampDetails.deviceMinVoltage));
    AJ_MarshalArgs(msg, "{sv}", "MinVoltage", "u", LampDetails.deviceMinVoltage);

    AJ_InfoPrintf(("MaxVoltage: %u\n", LampDetails.deviceMaxVoltage));
    AJ_MarshalArgs(msg, "{sv}", "MaxVoltage", "u", LampDetails.deviceMaxVoltage);

    AJ_InfoPrintf(("Wattage: %u\n", LampDetails.deviceWattage));
    AJ_MarshalArgs(msg, "{sv}", "Wattage", "u", LampDetails.deviceWattage);

    AJ_InfoPrintf(("IncandescentEquivalent: %u\n", LampDetails.deviceIncandescentEquivalent));
    AJ_MarshalArgs(msg, "{sv}", "IncandescentEquivalent", "u", LampDetails.deviceIncandescentEquivalent);

    AJ_InfoPrintf(("MaxLumens: %u\n", LampDetails.deviceMaxLumens));
    AJ_MarshalArgs(msg, "{sv}", "MaxLumens", "u", LampDetails.deviceMaxLumens);

    AJ_InfoPrintf(("MinTemperature: %u\n", LampDetails.deviceMinTemperature));
    AJ_MarshalArgs(msg, "{sv}", "MinTemperature", "u", LampDetails.deviceMinTemperature);

    AJ_InfoPrintf(("MaxTemperature: %u\n", LampDetails.deviceMaxTemperature));
    AJ_MarshalArgs(msg, "{sv}", "MaxTemperature", "u", LampDetails.deviceMaxTemperature);

    AJ_InfoPrintf(("ColorRenderingIndex: %u\n", LampDetails.deviceColorRenderingIndex));
    AJ_MarshalArgs(msg, "{sv}", "ColorRenderingIndex", "u", LampDetails.deviceColorRenderingIndex);

    AJ_InfoPrintf(("LampID: %s\n", AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEVICE_ID)));
    AJ_MarshalArgs(msg, "{sv}", "LampID", "s", AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEVICE_ID));
    return LAMP_OK;
}

const char* aboutIconMimetype = "image/png";

const uint8_t aboutIconContent[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44
    , 0x52, 0x00, 0x00, 0x00, 0x63, 0x00, 0x00, 0x00, 0x9C, 0x08, 0x06, 0x00, 0x00, 0x00, 0x6B
    , 0xFF, 0x15, 0x59, 0x00, 0x00, 0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6F, 0x66, 0x74
    , 0x77, 0x61, 0x72, 0x65, 0x00, 0x41, 0x64, 0x6F, 0x62, 0x65, 0x20, 0x49, 0x6D, 0x61, 0x67
    , 0x65, 0x52, 0x65, 0x61, 0x64, 0x79, 0x71, 0xC9, 0x65, 0x3C, 0x00, 0x00, 0x03, 0x66, 0x69
    , 0x54, 0x58, 0x74, 0x58, 0x4D, 0x4C, 0x3A, 0x63, 0x6F, 0x6D, 0x2E, 0x61, 0x64, 0x6F, 0x62
    , 0x65, 0x2E, 0x78, 0x6D, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x3F, 0x78, 0x70, 0x61
    , 0x63, 0x6B, 0x65, 0x74, 0x20, 0x62, 0x65, 0x67, 0x69, 0x6E, 0x3D, 0x22, 0xEF, 0xBB, 0xBF
    , 0x22, 0x20, 0x69, 0x64, 0x3D, 0x22, 0x57, 0x35, 0x4D, 0x30, 0x4D, 0x70, 0x43, 0x65, 0x68
    , 0x69, 0x48, 0x7A, 0x72, 0x65, 0x53, 0x7A, 0x4E, 0x54, 0x63, 0x7A, 0x6B, 0x63, 0x39, 0x64
    , 0x22, 0x3F, 0x3E, 0x20, 0x3C, 0x78, 0x3A, 0x78, 0x6D, 0x70, 0x6D, 0x65, 0x74, 0x61, 0x20
    , 0x78, 0x6D, 0x6C, 0x6E, 0x73, 0x3A, 0x78, 0x3D, 0x22, 0x61, 0x64, 0x6F, 0x62, 0x65, 0x3A
    , 0x6E, 0x73, 0x3A, 0x6D, 0x65, 0x74, 0x61, 0x2F, 0x22, 0x20, 0x78, 0x3A, 0x78, 0x6D, 0x70
    , 0x74, 0x6B, 0x3D, 0x22, 0x41, 0x64, 0x6F, 0x62, 0x65, 0x20, 0x58, 0x4D, 0x50, 0x20, 0x43
    , 0x6F, 0x72, 0x65, 0x20, 0x35, 0x2E, 0x30, 0x2D, 0x63, 0x30, 0x36, 0x30, 0x20, 0x36, 0x31
    , 0x2E, 0x31, 0x33, 0x34, 0x37, 0x37, 0x37, 0x2C, 0x20, 0x32, 0x30, 0x31, 0x30, 0x2F, 0x30
    , 0x32, 0x2F, 0x31, 0x32, 0x2D, 0x31, 0x37, 0x3A, 0x33, 0x32, 0x3A, 0x30, 0x30, 0x20, 0x20
    , 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x22, 0x3E, 0x20, 0x3C, 0x72, 0x64, 0x66, 0x3A, 0x52
    , 0x44, 0x46, 0x20, 0x78, 0x6D, 0x6C, 0x6E, 0x73, 0x3A, 0x72, 0x64, 0x66, 0x3D, 0x22, 0x68
    , 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x77, 0x77, 0x77, 0x2E, 0x77, 0x33, 0x2E, 0x6F, 0x72
    , 0x67, 0x2F, 0x31, 0x39, 0x39, 0x39, 0x2F, 0x30, 0x32, 0x2F, 0x32, 0x32, 0x2D, 0x72, 0x64
    , 0x66, 0x2D, 0x73, 0x79, 0x6E, 0x74, 0x61, 0x78, 0x2D, 0x6E, 0x73, 0x23, 0x22, 0x3E, 0x20
    , 0x3C, 0x72, 0x64, 0x66, 0x3A, 0x44, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6F
    , 0x6E, 0x20, 0x72, 0x64, 0x66, 0x3A, 0x61, 0x62, 0x6F, 0x75, 0x74, 0x3D, 0x22, 0x22, 0x20
    , 0x78, 0x6D, 0x6C, 0x6E, 0x73, 0x3A, 0x78, 0x6D, 0x70, 0x4D, 0x4D, 0x3D, 0x22, 0x68, 0x74
    , 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x6E, 0x73, 0x2E, 0x61, 0x64, 0x6F, 0x62, 0x65, 0x2E, 0x63
    , 0x6F, 0x6D, 0x2F, 0x78, 0x61, 0x70, 0x2F, 0x31, 0x2E, 0x30, 0x2F, 0x6D, 0x6D, 0x2F, 0x22
    , 0x20, 0x78, 0x6D, 0x6C, 0x6E, 0x73, 0x3A, 0x73, 0x74, 0x52, 0x65, 0x66, 0x3D, 0x22, 0x68
    , 0x74, 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x6E, 0x73, 0x2E, 0x61, 0x64, 0x6F, 0x62, 0x65, 0x2E
    , 0x63, 0x6F, 0x6D, 0x2F, 0x78, 0x61, 0x70, 0x2F, 0x31, 0x2E, 0x30, 0x2F, 0x73, 0x54, 0x79
    , 0x70, 0x65, 0x2F, 0x52, 0x65, 0x73, 0x6F, 0x75, 0x72, 0x63, 0x65, 0x52, 0x65, 0x66, 0x23
    , 0x22, 0x20, 0x78, 0x6D, 0x6C, 0x6E, 0x73, 0x3A, 0x78, 0x6D, 0x70, 0x3D, 0x22, 0x68, 0x74
    , 0x74, 0x70, 0x3A, 0x2F, 0x2F, 0x6E, 0x73, 0x2E, 0x61, 0x64, 0x6F, 0x62, 0x65, 0x2E, 0x63
    , 0x6F, 0x6D, 0x2F, 0x78, 0x61, 0x70, 0x2F, 0x31, 0x2E, 0x30, 0x2F, 0x22, 0x20, 0x78, 0x6D
    , 0x70, 0x4D, 0x4D, 0x3A, 0x4F, 0x72, 0x69, 0x67, 0x69, 0x6E, 0x61, 0x6C, 0x44, 0x6F, 0x63
    , 0x75, 0x6D, 0x65, 0x6E, 0x74, 0x49, 0x44, 0x3D, 0x22, 0x78, 0x6D, 0x70, 0x2E, 0x64, 0x69
    , 0x64, 0x3A, 0x46, 0x37, 0x37, 0x46, 0x31, 0x31, 0x37, 0x34, 0x30, 0x37, 0x32, 0x30, 0x36
    , 0x38, 0x31, 0x31, 0x38, 0x46, 0x44, 0x39, 0x42, 0x36, 0x37, 0x35, 0x36, 0x38, 0x36, 0x34
    , 0x35, 0x34, 0x36, 0x41, 0x22, 0x20, 0x78, 0x6D, 0x70, 0x4D, 0x4D, 0x3A, 0x44, 0x6F, 0x63
    , 0x75, 0x6D, 0x65, 0x6E, 0x74, 0x49, 0x44, 0x3D, 0x22, 0x78, 0x6D, 0x70, 0x2E, 0x64, 0x69
    , 0x64, 0x3A, 0x31, 0x32, 0x45, 0x43, 0x37, 0x36, 0x36, 0x35, 0x33, 0x38, 0x41, 0x38, 0x31
    , 0x31, 0x45, 0x33, 0x42, 0x31, 0x37, 0x46, 0x38, 0x44, 0x36, 0x39, 0x32, 0x31, 0x42, 0x33
    , 0x45, 0x33, 0x45, 0x39, 0x22, 0x20, 0x78, 0x6D, 0x70, 0x4D, 0x4D, 0x3A, 0x49, 0x6E, 0x73
    , 0x74, 0x61, 0x6E, 0x63, 0x65, 0x49, 0x44, 0x3D, 0x22, 0x78, 0x6D, 0x70, 0x2E, 0x69, 0x69
    , 0x64, 0x3A, 0x31, 0x32, 0x45, 0x43, 0x37, 0x36, 0x36, 0x34, 0x33, 0x38, 0x41, 0x38, 0x31
    , 0x31, 0x45, 0x33, 0x42, 0x31, 0x37, 0x46, 0x38, 0x44, 0x36, 0x39, 0x32, 0x31, 0x42, 0x33
    , 0x45, 0x33, 0x45, 0x39, 0x22, 0x20, 0x78, 0x6D, 0x70, 0x3A, 0x43, 0x72, 0x65, 0x61, 0x74
    , 0x6F, 0x72, 0x54, 0x6F, 0x6F, 0x6C, 0x3D, 0x22, 0x41, 0x64, 0x6F, 0x62, 0x65, 0x20, 0x50
    , 0x68, 0x6F, 0x74, 0x6F, 0x73, 0x68, 0x6F, 0x70, 0x20, 0x43, 0x53, 0x35, 0x20, 0x4D, 0x61
    , 0x63, 0x69, 0x6E, 0x74, 0x6F, 0x73, 0x68, 0x22, 0x3E, 0x20, 0x3C, 0x78, 0x6D, 0x70, 0x4D
    , 0x4D, 0x3A, 0x44, 0x65, 0x72, 0x69, 0x76, 0x65, 0x64, 0x46, 0x72, 0x6F, 0x6D, 0x20, 0x73
    , 0x74, 0x52, 0x65, 0x66, 0x3A, 0x69, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x63, 0x65, 0x49, 0x44
    , 0x3D, 0x22, 0x78, 0x6D, 0x70, 0x2E, 0x69, 0x69, 0x64, 0x3A, 0x39, 0x42, 0x33, 0x35, 0x42
    , 0x42, 0x41, 0x38, 0x35, 0x39, 0x32, 0x30, 0x36, 0x38, 0x31, 0x31, 0x39, 0x32, 0x42, 0x30
    , 0x38, 0x46, 0x42, 0x35, 0x41, 0x41, 0x35, 0x34, 0x30, 0x43, 0x34, 0x34, 0x22, 0x20, 0x73
    , 0x74, 0x52, 0x65, 0x66, 0x3A, 0x64, 0x6F, 0x63, 0x75, 0x6D, 0x65, 0x6E, 0x74, 0x49, 0x44
    , 0x3D, 0x22, 0x78, 0x6D, 0x70, 0x2E, 0x64, 0x69, 0x64, 0x3A, 0x46, 0x37, 0x37, 0x46, 0x31
    , 0x31, 0x37, 0x34, 0x30, 0x37, 0x32, 0x30, 0x36, 0x38, 0x31, 0x31, 0x38, 0x46, 0x44, 0x39
    , 0x42, 0x36, 0x37, 0x35, 0x36, 0x38, 0x36, 0x34, 0x35, 0x34, 0x36, 0x41, 0x22, 0x2F, 0x3E
    , 0x20, 0x3C, 0x2F, 0x72, 0x64, 0x66, 0x3A, 0x44, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74
    , 0x69, 0x6F, 0x6E, 0x3E, 0x20, 0x3C, 0x2F, 0x72, 0x64, 0x66, 0x3A, 0x52, 0x44, 0x46, 0x3E
    , 0x20, 0x3C, 0x2F, 0x78, 0x3A, 0x78, 0x6D, 0x70, 0x6D, 0x65, 0x74, 0x61, 0x3E, 0x20, 0x3C
    , 0x3F, 0x78, 0x70, 0x61, 0x63, 0x6B, 0x65, 0x74, 0x20, 0x65, 0x6E, 0x64, 0x3D, 0x22, 0x72
    , 0x22, 0x3F, 0x3E, 0x7D, 0x14, 0xA7, 0xED, 0x00, 0x00, 0x0B, 0x26, 0x49, 0x44, 0x41, 0x54
    , 0x78, 0xDA, 0xEC, 0x5D, 0x01, 0x68, 0x5E, 0xD5, 0x15, 0x3E, 0x6D, 0x24, 0x20, 0x74, 0x28
    , 0x1D, 0x19, 0x8A, 0x25, 0x12, 0x69, 0xA9, 0x38, 0x0A, 0x4A, 0xC4, 0xD1, 0xE1, 0xE8, 0x08
    , 0x4C, 0x26, 0x8E, 0x8E, 0x49, 0x47, 0x47, 0xA4, 0x23, 0xE0, 0xC8, 0x68, 0x69, 0x89, 0x74
    , 0xB4, 0x58, 0x56, 0x28, 0x14, 0x43, 0x07, 0x45, 0x31, 0xB4, 0x28, 0x93, 0x8D, 0x86, 0x05
    , 0xC3, 0xA4, 0x62, 0x99, 0x2C, 0x2C, 0x28, 0x0B, 0x1D, 0x8E, 0x06, 0xC5, 0xC2, 0x48, 0xB0
    , 0x18, 0x1A, 0x16, 0x1A, 0x16, 0x16, 0x2C, 0x0B, 0x0B, 0x86, 0x15, 0xC2, 0x7E, 0xBA, 0xFB
    , 0xED, 0xBF, 0xCF, 0xDE, 0xBE, 0xBC, 0xFB, 0xFF, 0xFF, 0xFB, 0xFF, 0x7B, 0xCF, 0xB9, 0xEF
    , 0xFD, 0xF7, 0x83, 0x43, 0xAA, 0xEF, 0xE7, 0xBD, 0xFB, 0xEE, 0xF7, 0xCE, 0x3D, 0xF7, 0x9C
    , 0x7B, 0xEE, 0xB9, 0x9B, 0x6E, 0xDF, 0xBE, 0x4D, 0x05, 0xC4, 0x7D, 0x4A, 0xB6, 0x5B, 0xAE
    , 0xAD, 0x2B, 0x99, 0x2D, 0xE2, 0x4B, 0x6D, 0x0A, 0x94, 0x8C, 0x7B, 0x95, 0xEC, 0x56, 0xB2
    , 0x4B, 0xC9, 0x63, 0x4A, 0x1E, 0x57, 0xF2, 0x80, 0x92, 0xEE, 0x9C, 0xF7, 0x49, 0x88, 0xB9
    , 0xA6, 0x64, 0x41, 0xC9, 0xBC, 0x92, 0x69, 0xFD, 0x37, 0x92, 0x61, 0x41, 0x87, 0x92, 0x3D
    , 0x4A, 0x9E, 0xD6, 0x7F, 0xF7, 0xE8, 0xFF, 0xE7, 0x0B, 0xCB, 0x4A, 0xFE, 0xA2, 0x64, 0x52
    , 0xC9, 0x94, 0x92, 0xC5, 0x48, 0x06, 0x51, 0xAF, 0x92, 0x01, 0x25, 0xFD, 0x4A, 0xB6, 0x0A
    , 0xB6, 0x03, 0xDA, 0x32, 0xAE, 0x65, 0xA5, 0x9D, 0xC8, 0xD8, 0xA2, 0x64, 0x50, 0xC9, 0x8B
    , 0x7A, 0x08, 0x0A, 0x09, 0x18, 0xD6, 0xDE, 0x57, 0x72, 0x5E, 0x6B, 0x4E, 0x69, 0xC9, 0x80
    , 0xD1, 0x3D, 0xAC, 0xE4, 0xA8, 0xB0, 0x16, 0xE4, 0xD1, 0x96, 0xD3, 0x7A, 0x28, 0x2B, 0x0D
    , 0x19, 0xD0, 0x84, 0x63, 0x4A, 0x86, 0x34, 0x21, 0x45, 0xC3, 0x55, 0x25, 0x27, 0x94, 0x7C
    , 0x58, 0x74, 0x32, 0xF6, 0x2A, 0x39, 0xD7, 0xC4, 0x2C, 0x28, 0x44, 0xBC, 0xA3, 0xE4, 0x25
    , 0x6D, 0xFC, 0x0B, 0x45, 0x46, 0xB7, 0x26, 0x61, 0x2F, 0x95, 0x0B, 0xAB, 0x4A, 0x4E, 0x6A
    , 0x9B, 0x52, 0x08, 0x32, 0xF6, 0x29, 0xB9, 0xA0, 0x87, 0xA7, 0xB2, 0x02, 0xD3, 0xE1, 0x17
    , 0x5C, 0x6B, 0x89, 0x4B, 0x32, 0x3A, 0x95, 0xBC, 0xAA, 0x8D, 0x74, 0x3B, 0x60, 0x59, 0x13
    , 0x32, 0x15, 0x1A, 0x19, 0x18, 0x96, 0x2E, 0x2A, 0x79, 0x8A, 0xDA, 0x0B, 0x15, 0x25, 0xA7
    , 0x94, 0x0C, 0x87, 0x42, 0xC6, 0x4E, 0x25, 0x1F, 0x94, 0xC4, 0x48, 0x37, 0x8B, 0xB7, 0x94
    , 0x1C, 0xD2, 0xE4, 0x88, 0x91, 0x01, 0x4D, 0xF8, 0xA3, 0x92, 0x2E, 0x8A, 0xC0, 0x6C, 0xEB
    , 0xA7, 0xDA, 0x71, 0x64, 0x27, 0xA3, 0x4F, 0xC9, 0x1F, 0x4A, 0x6E, 0xA8, 0xF3, 0x02, 0xBE
    , 0xC8, 0x0F, 0x95, 0xDC, 0xE2, 0x24, 0x03, 0x1A, 0x71, 0x99, 0xAA, 0xD1, 0xD5, 0x88, 0xBB
    , 0x81, 0x70, 0xCA, 0xF3, 0xCD, 0x0C, 0x59, 0xCD, 0x90, 0x81, 0x75, 0x84, 0x2B, 0x71, 0x68
    , 0xAA, 0x89, 0x37, 0xB5, 0x0D, 0xF1, 0x4A, 0x46, 0x97, 0x26, 0x62, 0x7B, 0xEC, 0xEF, 0xBA
    , 0x18, 0xD6, 0x0E, 0xA2, 0x17, 0x32, 0x3A, 0xF4, 0xD0, 0xF4, 0x74, 0xEC, 0xE7, 0x86, 0xF1
    , 0x13, 0x6D, 0xD8, 0x9D, 0x93, 0xF1, 0x8A, 0x92, 0x5F, 0xC6, 0xFE, 0xCD, 0x05, 0x84, 0x4F
    , 0x9E, 0xA0, 0xEA, 0x2A, 0xA3, 0x33, 0x32, 0xFA, 0xB4, 0x2F, 0xD1, 0x11, 0xFB, 0x37, 0x37
    , 0x3E, 0x51, 0xF2, 0x9D, 0x46, 0xA6, 0xBC, 0x8D, 0x90, 0x01, 0x3B, 0x31, 0x43, 0xD5, 0x35
    , 0xE8, 0x88, 0xE6, 0x70, 0x56, 0xC9, 0x71, 0x17, 0x64, 0xBC, 0x4D, 0xD5, 0x65, 0xD1, 0x88
    , 0xE6, 0x81, 0x69, 0xEE, 0x93, 0x4A, 0xFE, 0xD6, 0x0A, 0x19, 0x30, 0xD6, 0x1F, 0xC5, 0xBE
    , 0x74, 0x36, 0x5C, 0x7D, 0xBB, 0x96, 0xFF, 0x51, 0x8B, 0x0C, 0xD8, 0x87, 0x4F, 0xA9, 0x9A
    , 0x26, 0x13, 0xE1, 0x06, 0x87, 0xB4, 0x0F, 0x92, 0x9B, 0x8C, 0x83, 0x4A, 0xDE, 0x88, 0xFD
    , 0xE7, 0x14, 0xC8, 0x3C, 0xD9, 0x41, 0x96, 0x0C, 0x14, 0x1B, 0x19, 0x58, 0x9B, 0xB8, 0x11
    , 0x8D, 0xB6, 0x17, 0x20, 0xC9, 0xE1, 0x54, 0x1E, 0x32, 0x90, 0x4A, 0xF3, 0xEB, 0xD8, 0x6F
    , 0x5E, 0xB0, 0xA6, 0xE4, 0xE1, 0x2C, 0xED, 0xC8, 0x22, 0x03, 0xB6, 0xE2, 0x73, 0x8A, 0x21
    , 0x0F, 0x9F, 0x40, 0x98, 0x64, 0xB8, 0x11, 0x32, 0xFA, 0xF5, 0x74, 0x36, 0xC2, 0x1F, 0x96
    , 0xB5, 0x76, 0xAC, 0xD7, 0x23, 0xE3, 0x32, 0x55, 0x73, 0x5D, 0x23, 0xFC, 0xE2, 0xC7, 0x4A
    , 0xDE, 0xAD, 0x45, 0x46, 0xB7, 0x36, 0xDC, 0x11, 0xFE, 0x71, 0x89, 0xAA, 0xEB, 0x1E, 0x56
    , 0x32, 0x5E, 0x56, 0x72, 0x26, 0xF6, 0x13, 0x0B, 0xB0, 0x1A, 0xB8, 0xCD, 0x34, 0xE4, 0x69
    , 0x32, 0x3E, 0xA3, 0xF0, 0x92, 0x91, 0xCB, 0x8C, 0x9F, 0x53, 0x35, 0x99, 0x61, 0x03, 0x19
    , 0xD8, 0x98, 0x32, 0x13, 0xFB, 0x87, 0x15, 0x58, 0x33, 0x7F, 0x26, 0x8B, 0x0C, 0x24, 0x9F
    , 0x9D, 0x8B, 0xFD, 0xC3, 0x3E, 0x54, 0x7D, 0x5D, 0xFF, 0xA5, 0xCD, 0xC6, 0x85, 0xBE, 0xD8
    , 0x37, 0xEC, 0x40, 0x42, 0x47, 0x6F, 0xF2, 0x1F, 0x9B, 0x0D, 0x47, 0x2F, 0x4E, 0x67, 0x65
    , 0xD0, 0x97, 0x1E, 0xA6, 0xA2, 0xBD, 0x90, 0x03, 0x76, 0x48, 0x7D, 0xD7, 0x24, 0xE3, 0x80
    , 0x92, 0xDF, 0xC5, 0x7E, 0x11, 0x01, 0x62, 0x55, 0x5F, 0x33, 0x87, 0xA9, 0x38, 0x9D, 0x95
    , 0x03, 0x32, 0x32, 0x7B, 0x22, 0x19, 0xE1, 0xE0, 0x31, 0x93, 0x8C, 0x18, 0xA1, 0x95, 0x45
    , 0x4F, 0x24, 0x23, 0x1C, 0x74, 0x27, 0x64, 0x60, 0x1B, 0x70, 0x67, 0xEC, 0x0F, 0x51, 0x74
    , 0x99, 0x64, 0x44, 0xC8, 0x62, 0x6B, 0x42, 0x46, 0xDC, 0x5F, 0x11, 0x08, 0x19, 0xF7, 0x08
    , 0x91, 0x81, 0x45, 0xF9, 0xA5, 0xC0, 0x3B, 0x08, 0x95, 0x1C, 0x76, 0x32, 0x3D, 0xAB, 0x23
    , 0x21, 0x43, 0x02, 0x48, 0xE4, 0x7A, 0x2B, 0x60, 0x22, 0x90, 0x15, 0xC3, 0x99, 0xA6, 0xB4
    , 0xC5, 0x9C, 0x4D, 0x71, 0x63, 0x90, 0xC2, 0x4E, 0xA2, 0x3E, 0xCC, 0xDC, 0xBE, 0xB5, 0x84
    , 0x8C, 0x55, 0x81, 0x97, 0x7D, 0x48, 0xC9, 0xF7, 0x03, 0x25, 0xA2, 0x53, 0x7F, 0x2C, 0xDC
    , 0x23, 0xC5, 0xFF, 0x87, 0xA9, 0x8A, 0xD0, 0x4B, 0xA3, 0xC4, 0xD1, 0x44, 0xEA, 0x6B, 0xDC
    , 0x25, 0xD0, 0x0E, 0x54, 0x6A, 0x33, 0xCB, 0x4F, 0xEC, 0x27, 0xFE, 0x2D, 0x72, 0x6B, 0x09
    , 0x19, 0x37, 0x85, 0xC8, 0xD8, 0xAB, 0x35, 0x64, 0x29, 0x35, 0x7C, 0x71, 0x23, 0xBD, 0xF7
    , 0x6E, 0x48, 0xA0, 0x0D, 0x37, 0x93, 0x61, 0x0A, 0xFF, 0x58, 0x17, 0x68, 0x40, 0x87, 0xD6
    , 0x8E, 0x04, 0xE3, 0x02, 0xED, 0xC0, 0xF3, 0xCC, 0x6D, 0x5E, 0xA8, 0x8B, 0xD8, 0x2B, 0x49
    , 0x06, 0xB0, 0x2C, 0xA4, 0x1D, 0x03, 0x86, 0xA1, 0x5C, 0x49, 0x0D, 0x5B, 0x1C, 0x98, 0xA0
    , 0xBB, 0xD3, 0x2C, 0x87, 0x84, 0xFA, 0x61, 0xC9, 0x24, 0x63, 0x4E, 0xA8, 0x11, 0x3D, 0x29
    , 0x43, 0x3E, 0xCE, 0xFC, 0xFC, 0xF1, 0xD4, 0x74, 0xF6, 0x47, 0x42, 0xFD, 0x30, 0x1F, 0x02
    , 0x19, 0x94, 0x1A, 0xAA, 0xD2, 0x5F, 0xAA, 0x4F, 0xAC, 0x64, 0x4C, 0x20, 0xA4, 0x62, 0x74
    , 0xD7, 0x4C, 0x32, 0xAE, 0x09, 0x92, 0xB1, 0x97, 0xEE, 0x6C, 0x3D, 0x40, 0x96, 0xC4, 0xBB
    , 0x4C, 0xCF, 0xBD, 0x44, 0x77, 0xCA, 0x4A, 0x74, 0xA6, 0x3E, 0x0A, 0x4E, 0xA0, 0x0D, 0x0B
    , 0x26, 0x19, 0x9F, 0x08, 0x92, 0x91, 0x36, 0xE4, 0xA3, 0x4C, 0xCF, 0x1D, 0x4B, 0x4D, 0x67
    , 0xA5, 0xF6, 0xA2, 0x5C, 0x4D, 0xDC, 0x8B, 0x64, 0x0D, 0x1C, 0x1D, 0xF2, 0x25, 0xC9, 0xD5
    , 0x02, 0xC1, 0x97, 0xB1, 0xC3, 0xF0, 0x79, 0xFE, 0xAE, 0xED, 0x89, 0x2F, 0xA0, 0xA8, 0xF0
    , 0x23, 0xC6, 0xF3, 0x3E, 0x26, 0xB9, 0x5A, 0x59, 0xAF, 0x29, 0xF9, 0x85, 0xA9, 0x19, 0x15
    , 0xCD, 0x90, 0x14, 0xD0, 0xF1, 0x7D, 0x96, 0xAF, 0xD6, 0x97, 0x56, 0x54, 0x8C, 0xE9, 0xAC
    , 0x64, 0xD1, 0xB2, 0xAF, 0x2A, 0xB9, 0x6D, 0x4E, 0x19, 0x4F, 0x49, 0x0C, 0x32, 0x92, 0x31
    , 0x9E, 0x32, 0xDC, 0x52, 0xA8, 0x98, 0x64, 0x84, 0x94, 0x6B, 0x8B, 0x86, 0x6D, 0x33, 0x7C
    , 0x9E, 0x2B, 0xFA, 0xAB, 0xF5, 0x31, 0x46, 0x3F, 0x69, 0x4C, 0x67, 0x6F, 0x08, 0xCE, 0xA2
    , 0xFE, 0x4A, 0xD5, 0xEA, 0x09, 0x1B, 0x34, 0x63, 0x96, 0x1A, 0xAC, 0x71, 0xE1, 0xD1, 0x90
    , 0x0F, 0x30, 0xF8, 0x1C, 0xE6, 0x7D, 0x0F, 0x92, 0xEC, 0x92, 0xF3, 0x5D, 0x45, 0x5E, 0xD2
    , 0x5B, 0x02, 0xA4, 0x8B, 0xB5, 0xCC, 0x6B, 0x43, 0x0E, 0x60, 0xF5, 0xEB, 0x9F, 0x8E, 0x3B
    , 0xCB, 0xD4, 0x3E, 0xE9, 0x1D, 0xBD, 0xE9, 0x91, 0x60, 0xC3, 0x7A, 0xC6, 0x18, 0xC9, 0x02
    , 0x59, 0x2A, 0x7B, 0x3C, 0x86, 0x47, 0x3E, 0x34, 0x5E, 0x7E, 0x1F, 0xC9, 0x6E, 0xAD, 0x9E
    , 0xA2, 0x54, 0x18, 0x2A, 0x4D, 0x06, 0x3C, 0xF1, 0x49, 0x61, 0x42, 0x06, 0x3C, 0x7E, 0x1C
    , 0xA3, 0x16, 0xCF, 0x5F, 0x02, 0x1B, 0x2A, 0x25, 0x64, 0x6D, 0xB0, 0x44, 0xAC, 0xE8, 0x4F
    , 0x82, 0x8D, 0x44, 0x6C, 0xFF, 0x41, 0xFD, 0x17, 0x7E, 0xCF, 0x3F, 0xC8, 0x4D, 0x06, 0xCB
    , 0xAA, 0xBE, 0xEF, 0x2D, 0xAD, 0x81, 0xD7, 0x05, 0xDF, 0x11, 0x81, 0xC1, 0x87, 0x29, 0xB5
    , 0x96, 0x94, 0xB5, 0xEC, 0x3A, 0x29, 0x1C, 0x1E, 0xD9, 0xA2, 0x87, 0x10, 0xD7, 0xE1, 0x91
    , 0x77, 0x8C, 0xF0, 0x87, 0xB4, 0x56, 0x9C, 0xA7, 0x8C, 0x45, 0x3D, 0x5B, 0x85, 0x04, 0x34
    , 0xF6, 0x37, 0x82, 0x8D, 0xFD, 0x2A, 0x4D, 0x5E, 0x4F, 0x6F, 0xAF, 0x38, 0xB8, 0x27, 0x2A
    , 0xDA, 0x4C, 0x07, 0x60, 0xB8, 0x57, 0xB4, 0x56, 0xAC, 0x35, 0x4A, 0x46, 0xA7, 0x0E, 0x49
    , 0x3C, 0x24, 0x48, 0xC8, 0x0E, 0xBA, 0x73, 0x50, 0xD5, 0x75, 0x6A, 0x2D, 0x05, 0xD5, 0x9C
    , 0xA5, 0x41, 0xEB, 0x2E, 0x0A, 0xBE, 0x97, 0xB5, 0x76, 0x88, 0x2D, 0x3B, 0x04, 0x2B, 0x60
    , 0xC7, 0x85, 0x55, 0xF9, 0x80, 0x43, 0x9F, 0x63, 0xD4, 0xE2, 0xE9, 0x4B, 0x68, 0xC5, 0x6B
    , 0xB6, 0x8B, 0xF5, 0x8A, 0x7F, 0xA1, 0xF0, 0x97, 0x54, 0xB5, 0x4E, 0x33, 0x98, 0xD7, 0x8A
    , 0xC1, 0xAD, 0xE8, 0xFB, 0xE0, 0x7E, 0xDD, 0x5A, 0xE3, 0xA5, 0xD2, 0x84, 0xF0, 0x81, 0x9F
    , 0xB5, 0x5D, 0xAC, 0x97, 0x37, 0x75, 0x84, 0xE4, 0xB2, 0x47, 0xD0, 0x71, 0x7D, 0xC6, 0x30
    , 0x33, 0xDD, 0xC2, 0x7C, 0x7E, 0xD1, 0xD0, 0x36, 0x29, 0x22, 0x96, 0xA9, 0x46, 0xE1, 0xAF
    , 0x46, 0xC8, 0x40, 0x4D, 0x3D, 0xC9, 0xCC, 0x3F, 0x17, 0x3E, 0xC7, 0x98, 0x65, 0xE8, 0xE3
    , 0xC6, 0x70, 0x96, 0xD1, 0xCE, 0x33, 0x4C, 0x25, 0x61, 0x89, 0xEB, 0x24, 0x93, 0xAD, 0x7E
    , 0x4B, 0xFB, 0x06, 0xAB, 0x4D, 0x86, 0x47, 0x4C, 0xDF, 0x02, 0x61, 0xF2, 0x8F, 0x05, 0x87
    , 0xDC, 0x1D, 0x54, 0x27, 0xFB, 0xA5, 0x91, 0xF4, 0x4E, 0x18, 0x9D, 0x97, 0x84, 0x5E, 0x02
    , 0x4E, 0xDF, 0xFE, 0x16, 0xC2, 0x23, 0xA3, 0x86, 0x6F, 0x21, 0xA9, 0x15, 0xA7, 0xC9, 0x51
    , 0x5D, 0x5B, 0x69, 0x63, 0x6E, 0xFA, 0x1C, 0xC8, 0xDE, 0x78, 0xAF, 0x89, 0xE9, 0x71, 0xA7
    , 0xD6, 0x2A, 0x09, 0xED, 0xC6, 0xF3, 0x1F, 0x6D, 0xC4, 0xF6, 0xE6, 0x21, 0x03, 0xE9, 0xF1
    , 0x33, 0xC4, 0x1F, 0x72, 0xAE, 0x68, 0x27, 0x69, 0x29, 0x67, 0xA7, 0x22, 0x92, 0xF0, 0x6C
    , 0x93, 0x24, 0xBA, 0x44, 0xC3, 0xF5, 0xD0, 0xF3, 0x64, 0xA1, 0xCF, 0xD5, 0x9A, 0x23, 0x7B
    , 0x44, 0x87, 0x11, 0x1E, 0x59, 0xCF, 0x11, 0x1E, 0x31, 0x67, 0x2E, 0xFB, 0x84, 0x88, 0x98
    , 0xCD, 0x13, 0xCE, 0xC9, 0x7B, 0x64, 0x03, 0xBE, 0xCC, 0xCF, 0x88, 0x7F, 0x43, 0xE6, 0xB4
    , 0x0E, 0x67, 0x34, 0x1A, 0x1E, 0x31, 0x7D, 0x14, 0x90, 0xF9, 0x85, 0xD0, 0x10, 0x85, 0x53
    , 0x66, 0xDE, 0x6F, 0xF8, 0xD7, 0x20, 0x23, 0xA7, 0x3C, 0x77, 0x5B, 0x06, 0x5D, 0x46, 0x1B
    , 0xAE, 0xD7, 0xF9, 0xED, 0x51, 0xE3, 0xB7, 0x7B, 0x84, 0xDA, 0xFB, 0x69, 0xDE, 0xBE, 0x6D
    , 0x66, 0xB3, 0xCC, 0x04, 0xC9, 0x24, 0x2F, 0x98, 0x85, 0x66, 0x46, 0xEA, 0x4C, 0x67, 0x4D
    , 0xDF, 0x68, 0xBF, 0xA0, 0x5F, 0x91, 0x0F, 0x4D, 0x68, 0x06, 0x64, 0xBB, 0x92, 0xFF, 0x32
    , 0x7F, 0x69, 0xAF, 0xA7, 0xDA, 0x70, 0xD8, 0xF2, 0xBB, 0xFE, 0xD4, 0xEF, 0x6E, 0x08, 0x68
    , 0xC5, 0x4C, 0x33, 0xFD, 0xDA, 0x2C, 0x19, 0x90, 0x37, 0x98, 0x5F, 0xF0, 0xBD, 0x8C, 0x36
    , 0xEC, 0x53, 0xF2, 0xB9, 0xBE, 0xFE, 0x1F, 0x25, 0x43, 0xA9, 0xEB, 0xBB, 0x84, 0x86, 0xA8
    , 0xFE, 0x66, 0xFA, 0xB4, 0x95, 0xA3, 0xE1, 0xB8, 0x3D, 0x73, 0x84, 0x66, 0x9E, 0xB0, 0x5C
    , 0x43, 0x12, 0xDC, 0xCD, 0x8C, 0x70, 0x83, 0x44, 0x3D, 0x77, 0xCC, 0x3A, 0xBF, 0xD9, 0x4C
    , 0x4C, 0xAF, 0x95, 0x0D, 0x96, 0xF0, 0x88, 0x4F, 0x32, 0xBE, 0xE4, 0xCE, 0x1A, 0x41, 0xBE
    , 0x05, 0x4B, 0xDC, 0x67, 0xB7, 0x80, 0xAD, 0xF8, 0x55, 0xB3, 0xC1, 0xD5, 0x56, 0x4F, 0xB0
    , 0xE4, 0x3E, 0xD6, 0xE1, 0x51, 0xCA, 0xB7, 0x7D, 0x81, 0x3B, 0x6A, 0xD0, 0x50, 0x0C, 0xCA
    , 0x87, 0x66, 0x24, 0xDE, 0x31, 0xA7, 0x76, 0xE4, 0xCD, 0x89, 0xBD, 0x8F, 0x59, 0x2B, 0xCE
    , 0x92, 0xD0, 0x71, 0xA2, 0x26, 0xAE, 0x30, 0x0D, 0x09, 0x23, 0xA9, 0xA0, 0x25, 0xEC, 0xD5
    , 0x73, 0xC6, 0xD7, 0x8F, 0x38, 0xD0, 0x79, 0x23, 0x38, 0xE8, 0x3B, 0x9B, 0x3D, 0x3D, 0xA5
    , 0xDE, 0x46, 0x75, 0xC2, 0xE4, 0xB5, 0xE0, 0xAA, 0x42, 0x02, 0xD6, 0x74, 0x3F, 0x60, 0x78
    , 0xE1, 0xDD, 0x29, 0xBF, 0x03, 0x87, 0xFC, 0x6E, 0xC9, 0xD0, 0x86, 0x93, 0x46, 0xC4, 0x80
    , 0x0B, 0xA3, 0xAD, 0x10, 0xE1, 0x52, 0x33, 0xB8, 0xC6, 0x67, 0x0C, 0x01, 0xF7, 0xEB, 0x2F
    , 0x1F, 0xB6, 0xAA, 0xD7, 0xE2, 0x94, 0xFE, 0x40, 0xFF, 0xFB, 0x4B, 0xE2, 0xA9, 0x8D, 0x52
    , 0xD1, 0xF6, 0x6C, 0xBE, 0x95, 0x9B, 0xB8, 0x2C, 0x57, 0x31, 0xCC, 0xF0, 0xD2, 0xF8, 0xD2
    , 0x93, 0x8D, 0xFB, 0xB6, 0x8D, 0x3D, 0xF3, 0xC6, 0x75, 0xAE, 0x22, 0x35, 0x13, 0xAD, 0x12
    , 0xE1, 0x9A, 0x8C, 0x49, 0xE2, 0xD9, 0xA8, 0xB9, 0xDB, 0xE8, 0x80, 0x2C, 0x24, 0x81, 0x39
    , 0xCE, 0x34, 0xA3, 0x37, 0x5D, 0xDC, 0xC4, 0x75, 0x21, 0x97, 0xF3, 0x8C, 0x64, 0x8C, 0x64
    , 0x8C, 0xD1, 0x70, 0x0C, 0xA7, 0x98, 0xC9, 0xB8, 0x46, 0x8E, 0xF2, 0x93, 0x5D, 0x93, 0x31
    , 0x4A, 0xFE, 0x0B, 0xC3, 0x24, 0x76, 0x69, 0x89, 0x36, 0x26, 0x83, 0x1D, 0x49, 0x39, 0x89
    , 0x1C, 0x70, 0xF7, 0x01, 0xB6, 0x10, 0x9B, 0xB2, 0xC9, 0x20, 0x43, 0xEC, 0xA7, 0xC7, 0x78
    , 0xDE, 0xDB, 0x96, 0x40, 0xE2, 0x39, 0xA6, 0x30, 0x79, 0xA7, 0xAB, 0xBE, 0xF3, 0x41, 0x06
    , 0xE4, 0x23, 0xCF, 0x9D, 0x70, 0xC0, 0x78, 0x16, 0x3A, 0xE3, 0x4C, 0x46, 0xA7, 0xCC, 0x78
    , 0x6E, 0x03, 0xA2, 0xD6, 0x8F, 0xBB, 0xEC, 0x37, 0x97, 0x53, 0x5B, 0x13, 0xBD, 0x7A, 0xEA
    , 0xE9, 0x0B, 0xBF, 0x55, 0xF2, 0xB3, 0x1A, 0xD7, 0xE1, 0x0C, 0x7E, 0x41, 0x7E, 0x13, 0xD6
    , 0x60, 0xB4, 0x0F, 0xB9, 0xBC, 0xA1, 0x2F, 0x32, 0x00, 0x24, 0x00, 0xF8, 0xAA, 0xC5, 0x31
    , 0xA7, 0xE7, 0xF5, 0x36, 0x0C, 0x28, 0xB9, 0xE0, 0x91, 0x88, 0x9A, 0x27, 0x51, 0x86, 0x48
    , 0x86, 0x6F, 0xED, 0xF8, 0x06, 0xD9, 0x6B, 0x65, 0xF9, 0xDE, 0x64, 0x7F, 0xC8, 0xD5, 0x74
    , 0x96, 0x8B, 0x0C, 0xE0, 0x32, 0xF9, 0x3B, 0x97, 0x63, 0xC3, 0xD1, 0x6A, 0x1A, 0x58, 0x66
    , 0xFD, 0xBD, 0xC7, 0x77, 0x5A, 0xD0, 0x5A, 0xE9, 0xBC, 0x36, 0x96, 0xEF, 0x82, 0x91, 0x23
    , 0x1E, 0xEF, 0x3D, 0x40, 0x1B, 0x17, 0xB6, 0xFA, 0x3C, 0x0F, 0x4F, 0xC0, 0x59, 0xF2, 0x54
    , 0xA4, 0xCC, 0xB7, 0x66, 0xF8, 0x4E, 0x93, 0x59, 0xD3, 0xDA, 0xB1, 0xA8, 0x9D, 0xC1, 0xEF
    , 0x79, 0x26, 0x02, 0x99, 0xE4, 0x8F, 0xD0, 0x9D, 0xA8, 0x70, 0xA1, 0xC8, 0x00, 0xB0, 0xEC
    , 0x79, 0x90, 0xCA, 0x01, 0xAC, 0xE2, 0x9D, 0xF0, 0x75, 0x73, 0x0E, 0x32, 0x30, 0x74, 0xFC
    , 0xB9, 0x24, 0x64, 0xE4, 0x5D, 0x69, 0x0C, 0x8E, 0x0C, 0x44, 0x4F, 0xFF, 0x45, 0x72, 0xE5
    , 0x93, 0x5C, 0x01, 0x35, 0xB9, 0xBE, 0xE5, 0xF3, 0x01, 0x1C, 0x15, 0x9F, 0x31, 0xBE, 0x5E
    , 0x2D, 0x81, 0x56, 0x78, 0x4F, 0xDC, 0xE3, 0x2A, 0xBF, 0x3D, 0x5D, 0x02, 0x32, 0xA6, 0x7C
    , 0x3F, 0x80, 0x63, 0x98, 0x02, 0x8A, 0x7E, 0xDA, 0x19, 0xB4, 0xFB, 0x7E, 0xF2, 0x5C, 0x77
    , 0x97, 0x4B, 0x33, 0xE6, 0x0B, 0xAE, 0x15, 0x73, 0xC4, 0x50, 0x00, 0x99, 0x8B, 0x8C, 0x9B
    , 0x25, 0x20, 0x83, 0xCA, 0x42, 0xC6, 0x6A, 0xC1, 0xC9, 0x58, 0x29, 0x13, 0x19, 0xEB, 0x05
    , 0x27, 0x63, 0xAD, 0x4C, 0x64, 0xD4, 0x03, 0xF6, 0xBC, 0x21, 0x0A, 0xBB, 0x49, 0x50, 0x9E
    , 0x25, 0x7B, 0x4D, 0xF8, 0x4A, 0xBB, 0x90, 0x81, 0x69, 0xEF, 0x0B, 0x01, 0xD8, 0x95, 0x49
    , 0xDD, 0x0E, 0x31, 0x84, 0x60, 0x33, 0xC6, 0x48, 0xAE, 0x24, 0x46, 0x96, 0x2F, 0xB1, 0x68
    , 0x99, 0xDA, 0xB6, 0x85, 0x66, 0x84, 0x74, 0x34, 0x1D, 0xA2, 0xCC, 0xF7, 0x4A, 0xD9, 0x3C
    , 0x4E, 0x32, 0x6C, 0x2F, 0x34, 0x18, 0x10, 0x21, 0x70, 0x4E, 0xBB, 0xA4, 0x1E, 0xCE, 0x79
    , 0x34, 0x1C, 0x54, 0x3D, 0x2B, 0x11, 0x19, 0xDB, 0x98, 0x91, 0xC5, 0x7E, 0x82, 0xE4, 0x0E
    , 0x55, 0x01, 0xB0, 0x5E, 0x7F, 0x4C, 0x72, 0x36, 0xC8, 0x15, 0x0E, 0x01, 0xFE, 0x4D, 0xFC
    , 0xFB, 0x25, 0x5C, 0xC1, 0xCB, 0x9A, 0xB7, 0xE4, 0x30, 0x55, 0xA1, 0x88, 0x60, 0xC8, 0x58
    , 0x8B, 0xDD, 0x5D, 0x0C, 0xA7, 0x2F, 0x74, 0xDC, 0x8A, 0x64, 0x84, 0x03, 0x16, 0x03, 0x7E
    , 0x4F, 0x60, 0x2F, 0x3D, 0x4B, 0x32, 0x71, 0x2C, 0x6C, 0x1F, 0x78, 0x40, 0xFA, 0xE5, 0x39
    , 0xC9, 0xA8, 0xE5, 0x85, 0x23, 0xAD, 0xFE, 0x38, 0xD7, 0x70, 0x60, 0x01, 0x32, 0x10, 0x91
    , 0xFC, 0xD6, 0x23, 0xD5, 0x80, 0x10, 0x86, 0x29, 0xC4, 0x84, 0x8E, 0x08, 0x13, 0x01, 0x20
    , 0xE1, 0xE0, 0x79, 0xC9, 0x61, 0x2A, 0x04, 0x32, 0x26, 0x28, 0x1C, 0x60, 0xE7, 0x53, 0x5B
    , 0xC7, 0xA6, 0x42, 0x72, 0x04, 0x3B, 0x24, 0xDB, 0x13, 0x82, 0x9F, 0x31, 0x10, 0x10, 0x21
    , 0xA2, 0x6D, 0xE1, 0x34, 0xE0, 0x36, 0x0F, 0x3C, 0x89, 0x4D, 0x21, 0x75, 0x52, 0xEA, 0xA8
    , 0x08, 0x04, 0x2A, 0x51, 0x69, 0xE1, 0xA8, 0xE4, 0x97, 0xC0, 0x19, 0x9B, 0x92, 0xAC, 0xAB
    , 0xDE, 0x2A, 0x9E, 0xA1, 0xEA, 0x11, 0x41, 0xD1, 0xE9, 0x8B, 0x1E, 0x38, 0x9F, 0xCD, 0x88
    , 0x10, 0x20, 0x23, 0x46, 0x6D, 0xE3, 0x30, 0x55, 0x1C, 0x84, 0x14, 0x9B, 0x42, 0xA2, 0x98
    , 0xD4, 0x09, 0x9A, 0x66, 0x81, 0x18, 0x31, 0xAD, 0xE6, 0x5E, 0x76, 0xB5, 0xE1, 0xB4, 0x16
    , 0xC9, 0xA1, 0x0C, 0xE5, 0x2D, 0x2E, 0x5A, 0x48, 0x29, 0x5D, 0x12, 0x9B, 0x2D, 0xBE, 0x83
    , 0x70, 0xC8, 0xA9, 0x00, 0x6C, 0x0A, 0xF2, 0x69, 0xDB, 0x22, 0x6F, 0xAA, 0x16, 0x42, 0x8A
    , 0x4D, 0x21, 0x84, 0xBF, 0xD8, 0xCE, 0x64, 0x6C, 0x0D, 0x88, 0x8C, 0xB6, 0x89, 0x4D, 0xD9
    , 0x6C, 0xC6, 0x10, 0x05, 0xB0, 0xB0, 0xA3, 0x71, 0xCC, 0x42, 0x46, 0xE9, 0x0C, 0xB8, 0xCD
    , 0x66, 0x20, 0x69, 0x0C, 0x87, 0xA4, 0x20, 0x36, 0x85, 0x10, 0xB6, 0xC4, 0xF6, 0x01, 0xC4
    , 0xC7, 0x70, 0xA6, 0xED, 0x80, 0xA4, 0xC3, 0x1A, 0xCA, 0xD4, 0x16, 0x84, 0xBC, 0x1A, 0x9D
    , 0xBE, 0x88, 0xB6, 0x24, 0x63, 0x2E, 0x76, 0x77, 0x6D, 0x70, 0x86, 0xD0, 0x81, 0x73, 0x4A
    , 0x0E, 0x17, 0xAC, 0x8F, 0xBC, 0xD4, 0x96, 0x0A, 0x81, 0x0C, 0xE0, 0x65, 0x25, 0x67, 0x0A
    , 0x42, 0x04, 0x26, 0x14, 0x48, 0x52, 0x58, 0x28, 0xA3, 0x66, 0x24, 0x40, 0xC6, 0xF7, 0x05
    , 0x0A, 0x37, 0x11, 0x1A, 0xD3, 0xF0, 0x61, 0x6A, 0xE1, 0xF8, 0x85, 0x22, 0x91, 0x01, 0x20
    , 0x71, 0xEC, 0x75, 0x92, 0x3B, 0xB6, 0x8D, 0x2C, 0xFE, 0xC4, 0x28, 0x55, 0xE3, 0x64, 0xEC
    , 0x9E, 0xB8, 0x24, 0x19, 0x09, 0x50, 0x3E, 0xEF, 0x15, 0x3D, 0xCF, 0x97, 0x02, 0xFC, 0x08
    , 0x6C, 0x67, 0x1B, 0x91, 0x9C, 0x68, 0x84, 0x40, 0x46, 0x02, 0x64, 0xF4, 0xBD, 0xA8, 0x35
    , 0x85, 0x2B, 0x44, 0x32, 0xAD, 0x49, 0x18, 0xA3, 0x00, 0x56, 0x22, 0x43, 0x22, 0x23, 0x01
    , 0xE2, 0x43, 0xC8, 0xD4, 0xE8, 0xA7, 0x6A, 0x75, 0xB5, 0x6E, 0x87, 0xF7, 0xC6, 0x8E, 0x5A
    , 0x9C, 0x15, 0x7B, 0x89, 0xAA, 0x09, 0x06, 0x41, 0x55, 0x6E, 0x08, 0x91, 0x8C, 0x2C, 0xEF
    , 0xBC, 0x57, 0x6B, 0x0E, 0xF2, 0x60, 0x77, 0xA5, 0x86, 0xB8, 0xB4, 0xE1, 0x35, 0xD3, 0x7D
    , 0x66, 0x53, 0xB2, 0x1C, 0xF2, 0x8B, 0x16, 0x81, 0x8C, 0xB6, 0xC1, 0xFF, 0x04, 0x18, 0x00
    , 0x61, 0xBF, 0xD8, 0xEB, 0x52, 0x39, 0x31, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E
    , 0x44, 0xAE, 0x42, 0x60, 0x82
};

const size_t aboutIconSize = sizeof(aboutIconContent);

const char* aboutIconUrl = "";

const char* routingNodePrefix = "org.alljoyn.BusNode";
