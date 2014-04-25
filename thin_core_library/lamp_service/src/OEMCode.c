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
#include <alljoyn/services_common/PropertyStore.h>

#include <aj_nvram.h>
#include <aj_crypto.h>


/**
 * Per-module definition of the current module for debug logging.  Must be defined
 * prior to first inclusion of aj_debug.h
 */
#define AJ_MODULE OEM_CODE

uint32_t OEM_GetFirmwareVersion(void)
{
    return 1;
}

uint32_t OEM_GetHardwareVersion(void)
{
    return 1;
}

LampResponseCode OEM_TransitionLampState(LampState* newState, uint32_t timestamp)
{
    printf("%s: (Hue=%u,Saturation=%u,colorTemp=%u,Brightness=%u,OnOff=%u)\n", __FUNCTION__,
           newState->hue, newState->saturation, newState->colorTemp, newState->brightness, newState->onOff);

    LAMP_SetState(newState);
    return LAMP_OK;
}



void OEM_Initialize(void)
{
    // TODO: vendor-specific initialization goes here
}

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


uint32_t OEM_GetPowerDraw()
{
    return 15;
}


uint32_t OEM_GetOutput()
{
    return 100;
}

uint32_t OEM_GetRemainingLife()
{
    return 10;
}


LampResponseCode OEM_GetLampFaults(AJ_Message* msg)
{
    printf("\n%s\n", __FUNCTION__);
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
    printf("\n%s: code=%d\n", __FUNCTION__, fault);
    // TOOD: clear the fault code
    return LAMP_OK;
}


AJ_Status OEM_GetLampParameters(AJ_Message* msg)
{
    AJ_MarshalArgs(msg, "{sv}", "power_draw", "u", OEM_GetPowerDraw());
    AJ_MarshalArgs(msg, "{sv}", "output", "u", OEM_GetOutput());
    return AJ_OK;
}


// TODO: modify this file with information specific to the manufacturer

// these first two must not be static as they are used by the Config object
const char* deviceProductName = "deviceProductName";
const char* deviceManufactureName = "deviceManufactureName";



LampDetails_t LampDetails = {
    .lampMake = MAKE_LIFX,
    .lampModel = MODEL_LED,
    .deviceType = TYPE_LAMP,
    .lampType = LAMPTYPE_PAR30,
    .baseType = BASETYPE_E17,
    .deviceLampBeamAngle = 100,
    .deviceDimmable = TRUE,
    .deviceColor = TRUE,
    .variableColorTemperature = TRUE,
    .deviceHasEffects = TRUE,
    .deviceVoltage = 120,
    .deviceWattage = 9,
    .deviceWattageEquivalent = 75,
    .deviceMaxOutput = 900,
    .deviceMinTemperature = 2700,
    .deviceMaxTemperature = 5000,
    .deviceColorRenderingIndex = 93,
    .deviceLifespan = 20
};


LampResponseCode LAMP_MarshalDetails(AJ_Message* msg)
{
    AJ_InfoPrintf(("%s", __FUNCTION__));

    AJ_MarshalArgs(msg, "{sv}", "LampVersion", "u", LAMP_GetServiceVersion());
    AJ_MarshalArgs(msg, "{sv}", "HardwareVersion", "u", OEM_GetHardwareVersion());
    AJ_MarshalArgs(msg, "{sv}", "FirmwareVersion", "u", OEM_GetFirmwareVersion());

    AJ_MarshalArgs(msg, "{sv}", "Manufacturer", "s", deviceManufactureName);


    AJ_MarshalArgs(msg, "{sv}", "Make", "u", LampDetails.lampMake);
    AJ_MarshalArgs(msg, "{sv}", "Model", "u", LampDetails.lampModel);
    AJ_MarshalArgs(msg, "{sv}", "Type", "u", LampDetails.deviceType);
    AJ_MarshalArgs(msg, "{sv}", "LampType", "u", LampDetails.lampType);
    AJ_MarshalArgs(msg, "{sv}", "LampBaseType", "u", LampDetails.baseType);


    AJ_MarshalArgs(msg, "{sv}", "LampBeamAngle", "u", LampDetails.deviceLampBeamAngle);

    AJ_MarshalArgs(msg, "{sv}", "Dimmable", "b", LampDetails.deviceDimmable);
    AJ_MarshalArgs(msg, "{sv}", "Color", "b", LampDetails.deviceColor);
    AJ_MarshalArgs(msg, "{sv}", "VariableColorTemperature", "b", LampDetails.variableColorTemperature);
    AJ_MarshalArgs(msg, "{sv}", "HasEffects", "b", LampDetails.deviceHasEffects);

    AJ_MarshalArgs(msg, "{sv}", "Voltage", "u", LampDetails.deviceVoltage);
    AJ_MarshalArgs(msg, "{sv}", "Wattage", "u", LampDetails.deviceWattage);
    AJ_MarshalArgs(msg, "{sv}", "WattageEquivalent", "u", LampDetails.deviceWattageEquivalent);
    AJ_MarshalArgs(msg, "{sv}", "MaxOutput", "u", LampDetails.deviceMaxOutput);
    AJ_MarshalArgs(msg, "{sv}", "MinTemperature", "u", LampDetails.deviceMinTemperature);
    AJ_MarshalArgs(msg, "{sv}", "MaxTemperature", "u", LampDetails.deviceMaxTemperature);

    AJ_MarshalArgs(msg, "{sv}", "ColorRenderingIndex", "u", LampDetails.deviceColorRenderingIndex);
    AJ_MarshalArgs(msg, "{sv}", "Lifespan", "u", LampDetails.deviceLifespan);

    AJ_MarshalArgs(msg, "{sv}", "NodeID", "s", AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEVICE_ID));
    return LAMP_OK;
}
