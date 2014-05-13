#ifndef _OEM_CODE_H_
#define _OEM_CODE_H_
/**
 * @file OEMCode.h
 * @defgroup oem_code OEM-specific code
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

#include <LampService.h>
#include <LampState.h>
#include <LampResponseCodes.h>
#include <LampValues.h>

#ifdef ONBOARDING_SERVICE
#include <alljoyn/onboarding/OnboardingManager.h>
#endif

/**
 * Get the Firmware version
 *
 * @param  None
 * @return  The firmware version
 */
uint32_t OEM_GetFirmwareVersion(void);

/**
 * Get the hardware version
 *
 * @param  None
 * @return  The hardware version
 */
uint32_t OEM_GetHardwareVersion(void);

/**
 * OEM-specific initialization
 *
 * @param  None
 * @return None
 */
void OEM_Initialize(void);

/**
 * Restart the Lamp
 *
 * @param  None
 * @return None
 */
void OEM_Restart(void);

/**
 * Reset the Lamp to factory settings
 *
 * @param  None
 * @return None
 */
void OEM_FactoryReset(void);

/**
 * Serialize the Lamp's current real-time parameters
 *
 * @param msg   The msg to serialize data into
 * @return      AJ_OK if no errors occured
 */
AJ_Status OEM_GetLampParameters(AJ_Message* msg);

/**
 * Return the current power draw, in milliamps
 *
 * @return  The power draw
 */
uint32_t OEM_GetEnergyUsageMilliwatts();

/**
 * Get the current light output
 *
 * @return the current light output
 */
uint32_t OEM_GetBrightnessLumens();

/**
 * Get the remaining life
 *
 * @return  The remaining life span
 */
uint32_t OEM_GetRemainingLife();

#ifdef ONBOARDING_SERVICE

/**
 * The default settings for the onboarding service
 */
extern const AJOBS_Settings OEM_OnboardingSettings;

#endif

/**
 * Change the lamp state
 *
 * @param newState  New state of the Lamp
 * @param timestamp Timestamp of when to transition.
 * @param transition_period The time to transition over
 * @return          LAMP_OK if no errors occured
 */
LampResponseCode OEM_TransitionLampState(LampState* newState, uint64_t timestamp, uint32_t transition_period);

/*
 * This function needs to implemented by the OEM to support the Pulse Effect
 * @param  lampState             Specifies the LampState(onOff, hue, saturation, colorTemp) that the Lamp needs to be in when transitioning the brightness as a part of pulse effect
 * @param  period                Period of the pulse (in ms)
 * @param  duration              Ratio of the pulse (% of period during which the brightness should be increased from 0% to 100%)
 * @param  numPulses             Number of pulses
 * @param  startTimeStamp   Start time stamp of the pulse effect
 *
 * @return Status of the operation
 */
LampResponseCode OEM_ApplyPulseEffect(LampState* fromState, LampState* toState, uint32_t period, uint32_t duration, uint32_t numPulses, uint64_t startTimeStamp);

/*
 * This function needs to implemented by the OEM to support the Strobe Effect
 * @param  lampState               Specifies the LampState(onOff, hue, saturation, colorTemp) that the Lamp needs to be in when transitioning the brightness as a part of strobe effect
 * @param  period                      Period of the strobe (in ms). OEM decides the ratio of the strobe.
 * @param  numStrobes           Number of strobes
 * @param  startTimeStamp   Start time stamp of the strobe effect
 *
 * @return Status of the operation
 */
LampResponseCode OEM_ApplyStrobeEffect(LampState* fromState, LampState* toState, uint32_t period, uint32_t numStrobes, uint64_t startTimeStamp);

/*
 * This function needs to implemented by the OEM to support the Cycle Effect
 * @param  lampStateA            Specifies the LampState(onOff, hue, saturation, brightness, colorTemp)
 * @param  lampStateB            Specifies the LampState(onOff, hue, saturation, brightness, colorTemp)
 * @param  period                      Period of the cycle (in ms)
 * @param  duration                  Duration of the cycle (duration (in ms) during which the Lamp will be in LampStateA and thereafter will be in LampStateB for the rest of the period)
 * @param  numCycles             Number of cycles
 * @param  startTimeStamp   Start time stamp of the cycle effect
 *
 * @return Status of the operation
 *
 * NOTE: When the Cycle Effect applies to a group of Lamps, the entity that calls this API on the Lamp should specify the input parameters appropriately. For Eg: When this effect needs to be applied for a group of 3 Lamps,
 * Parameters for Lamp 1 = (lampStateA, lampStateB, period=(3*(duration for which each Lamp needs to be in lampStateA)), duration =(duration for which each Lamp needs to be in lampStateA), numCycles, startTimeStamp=0)
 * Parameters for Lamp 2 = (lampStateA, lampStateB, period=(3*(duration for which each Lamp needs to be in lampStateA)), duration =(duration for which each Lamp needs to be in lampStateA), numCycles, startTimeStamp=(duration for which each Lamp needs to be in lampStateA))
 * Parameters for Lamp 3 = (lampStateA, lampStateB, period=(3*(duration for which each Lamp needs to be in lampStateA)), duration =(duration for which each Lamp needs to be in lampStateA), numCycles, startTimeStamp=(2*(duration for which each Lamp needs to be in lampStateA)))
 */
LampResponseCode OEM_ApplyCycleEffect(LampState* lampStateA, LampState* lampStateB, uint32_t period, uint32_t duration, uint32_t numCycles, uint64_t startTimeStamp);


/**
 * Serialize all active fault codes into a message.
 *
 * @param msg   The message to serialize into
 * @return      The LampResponseCode; LAMP_OK if no errors occured
 */
LampResponseCode OEM_GetLampFaults(AJ_Message* msg);

/**
 * Clear the lamp fault with the given code
 *
 * @param fault The fault code to clear
 * @return      LAMP_OK if the fault is valid and successfully cleared
 */
LampResponseCode OEM_ClearLampFault(LampFaultCode fault);

/**
 * Serialize the Lamp's details
 *
 * @param msg   The msg to serialize data into
 * @return      LAMP_OK if no errors occured
 */
LampResponseCode LAMP_MarshalDetails(AJ_Message* msg);

/**
 * This struct holds all fields of the Lamp's Details.
 */
typedef struct {
    const LampMake lampMake;        /**< The make of the lamp */
    const LampModel lampModel;      /**< The model of the lamp */

    const DeviceType deviceType;    /**< The type of device */
    const LampType lampType;        /**< The type of lamp */
    const BaseType baseType;        /**< The make of the lamp base */


    const uint32_t deviceLampBeamAngle; /**< The beam angle */
    uint8_t deviceDimmable;       /**< Dimmable? */
    uint8_t deviceColor;          /**< color? */
    uint8_t variableColorTemp;    /**< variable color temperature? */
    uint8_t deviceHasEffects;     /**< Are effects available? */

    const uint32_t deviceVoltage;       /**< voltage */
    const uint32_t deviceWattage;       /**< wattage */
    const uint32_t deviceWattageEquivalent; /**< Incandescent wattage equivalent */
    const uint32_t deviceMaxOutput;     /**< maximum output */
    const uint32_t deviceMinTemperature;    /**< lowest possible color temperature */
    const uint32_t deviceMaxTemperature;    /**< highest possible color temperature */
    uint32_t deviceColorRenderingIndex; /**< rendering index */
    const uint32_t deviceLifespan;      /**< estimated lifespan */
} LampDetails_t;


/**
 * A global struct to hold this Lamp's details.
 */
extern LampDetails_t LampDetails;

/**
 * @}
 */
#endif
