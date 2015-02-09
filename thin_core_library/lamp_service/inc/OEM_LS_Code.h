#ifndef _OEM_LS_CODE_H_
#define _OEM_LS_CODE_H_
/**
 * @file OEM_LS_Code.h
 * @defgroup oem_ls_code The OEM APIs used by the Lamp Service
 * @{
 */
/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
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
 * OEM-specific initialization.
 *  Anything OEM-specific required at startup should go here.
 *  The Lamp Service will read the state and property store from NVRAM after executing this method.
 *  As such, there is no need for the OEM to do so here.
 *  It is suggested that the OEM initialize any OEM-specific data and structures here.
 *  This is a good place to initialize any areas of NVRAM that will be used by the OEM.
 *
 * @param  None
 * @return None
 */
void OEM_LS_Initialize(void);

/**
 * Return a string that describes the current active faults
 *
 * @param  None
 * @return String describing the current active faults or NULL if there are no active faults
 */
const char* OEM_LS_GetFaultsText(void);

/**
 * OEM-defined default state
 * This function is called when the lamp boots from the factory state.
 * It should set the initial LampState values.  If this lamp already has a LampState stored in
 * NVRAM, this function will not be called.
 * No need to explicitly save the state in NVRAM.
 * The Lamp Service will do that after making this call and any time the state is changed.
 *
 * @param  state The lamp state to set
 * @return None
 */
void OEM_LS_SetFactoryState(LampState* state);

/**
 * Prepare to restart the Lamp.
 * The main loop will call this function before restarting.
 * This is for OEM-specific pre-restart work.
 * DO NOT PERFORM THE RESTART HERE; the Lamp Service will do that in the main loop.
 *
 * @param  None
 * @return None
 */
void OEM_LS_Restart(void);

/**
 * Reset the Lamp to factory settings
 * The main loop will call this function before doing a factory reset.
 * The Lamp Service will erase data managed by the framework (e.g., state, name, language) then it will restart the device.
 * The Lamp Service will call this function so that any OEM-managed data in NVRAM can be erased
 * as part of the FactoryReset.
 *
 * @param  None
 * @return None
 */
void OEM_LS_DoFactoryReset(void);

/**
 * Serialize the Lamp's current real-time parameters.
 * The Lamp Service will serialize parameters defined by the framework here.
 * The OEM should add anything else specific to their lamps.
 *
 * @param   msg   The msg to serialize data into
 * @return  Status of the operation
 */
AJ_Status OEM_LS_PopulateParameters(AJ_Message* msg);

/**
 * Return the Lamp's current energy usage in milliwatts
 *
 * @param   None
 * @return  The Lamp's current energy usage in milliwatts
 */
uint32_t OEM_LS_GetEnergyUsageMilliwatts(void);

/**
 * Get the Lamp's current Brightness in Lumens
 *
 * @param   None
 * @return  The Lamp's current Brightness in Lumens
 */
uint32_t OEM_LS_GetBrightnessLumens(void);

#ifdef ONBOARDING_SERVICE

/**
 * The default settings for the Onboarding service.
 * The OEM may modify this if they want to change timeout settings or
 * use security when onboarding.
 */
extern AJOBS_Settings OEM_LS_OnboardingSettings;

#endif

/**
 * Change the lamp's on/off state in response
 * to the setting of Lamp State Interface onOff property.
 * The current state should be retrieved from the NVRAM,
 * updated and the new state should be saved back in the NVRAM.
 *
 * @param  onoff On or off
 * @return Status of the operation
 */
LampResponseCode OEM_LS_SetOnOff(uint8_t onoff);

/**
 * Change the lamp's hue in response
 * to the setting of Lamp State Interface hue property.
 * The current state should be retrieved from the NVRAM,
 * updated and the new state should be saved back in the NVRAM.
 *
 * @param  hue   The hue
 * @return Status of the operation
 */
LampResponseCode OEM_LS_SetHue(uint32_t hue);

/**
 * Change the lamp's brightness in response
 * to the setting of Lamp State Interface brightness property.
 * The current state should be retrieved from the NVRAM,
 * updated and the new state should be saved back in the NVRAM.
 *
 * @param  brightness  The brightness
 * @return Status of the operation
 */
LampResponseCode OEM_LS_SetBrightness(uint32_t brightness);

/**
 * Change the lamp's saturation in response
 * to the setting of Lamp State Interface saturation property.
 * The current state should be retrieved from the NVRAM,
 * updated and the new state should be saved back in the NVRAM.
 *
 * @param  saturation The saturation
 * @return Status of the operation
 */
LampResponseCode OEM_LS_SetSaturation(uint32_t saturation);

/**
 * Change the lamp's color temperature in response
 * to the setting of Lamp State Interface colorTemp property.
 * The current state should be retrieved from the NVRAM,
 * updated and the new state should be saved back in the NVRAM.
 *
 * @param   colorTemp     The color temperature
 * @return  Status of the operation
 */
LampResponseCode OEM_LS_SetColorTemp(uint32_t colorTemp);

/**
 * This function needs to implemented by the OEM to support the Transition Effect
 *
 * @param  newState          New state of the Lamp to transition to
 * @param  timestamp         Timestamp (in ms) of when to start the transition.
 * @param  transitionPeriod  The time period (in ms) to transition over
 * @return Status of the operation
 */
LampResponseCode OEM_LS_TransitionState(LampState* newState, uint64_t timestamp, uint32_t transitionPeriod);

/**
 * This function needs to implemented by the OEM to support the Transition Effect for one or more fields of
 * the Lamp State
 *
 * @param  newStateContainer Container holding the values of a one/more Lamp State fields
 * @param  timestamp         Timestamp (in ms) of when to start the transition.
 * @param  transitionPeriod  The time period (in ms) to transition over
 * @return Status of the operation
 */
LampResponseCode OEM_LS_TransitionStateFields(LampStateContainer* newStateContainer, uint64_t timestamp, uint32_t transitionPeriod);

/**
 * This function needs to implemented by the OEM to support the Pulse Effect
 *
 * @param  fromState        Specifies the LampState that the Lamp needs to be in when starting a pulse
 * @param  toState          Specifies the LampState that the Lamp needs to be in when ending a pulse
 * @param  period           Period of the pulse (in ms). Period refers to the time duration between the start of two pulses
 * @param  duration         The duration of a single pulse (in ms). This must be less than the period
 * @param  numPulses        Number of pulses
 * @param  timestamp        Time stamp (in ms) of when to start applying the pulse effect from
 *
 * @return Status of the operation
 *
 */
LampResponseCode OEM_LS_ApplyPulseEffect(LampState* fromState, LampState* toState, uint32_t period, uint32_t duration, uint32_t numPulses, uint64_t timestamp);

/**
 * This function needs to implemented by the OEM to support the Pulse Effect on state fields
 *
 * @param  fromStateContainer Specifies the LampState field values that the Lamp needs to be in when starting a pulse
 * @param  toStateContainer   Specifies the LampState field values that the Lamp needs to be in when ending a pulse
 * @param  period             Period of the pulse (in ms). Period refers to the time duration between the start of two pulses
 * @param  duration           The duration of a single pulse (in ms). This must be less than the period
 * @param  numPulses          Number of pulses
 * @param  timestamp          Time stamp (in ms) of when to start applying the pulse effect from
 *
 * @return Status of the operation
 *
 * NOTE:
 * The Apply Pulse Effect accepts two parameters - the From State and the To State.
 * If the user wants the Lamp to pulse from the Lamp's current state to another
 * state, the From State is specified as a NULL dictionary.  When the fromStateContainer has
 * no valid fields, the OEM code should use the current state of the lamp as the from state
 * just before starting the Pulse effect
 */
LampResponseCode OEM_LS_ApplyPulseEffectOnStateFields(LampStateContainer* fromStateContainer, LampStateContainer* toStateContainer, uint32_t period, uint32_t duration, uint32_t numPulses, uint64_t timestamp);

/**
 * Serialize all active fault codes into a message.
 * Fault codes are unsigned 32-bit values that are
 * defined by the OEM.
 *
 * @param msg   The message to serialize into
 * @return      Status of the operation
 */
LampResponseCode OEM_LS_PopulateFaults(AJ_Message* msg);

/**
 * Clear the lamp fault with the given code. Fault codes are unsigned
 * 32-bit values that are defined by the OEM.
 * This will be invoked when the ClearLampFault interface method is
 * invoked. The response code is defined by the OEM.
 *
 * @param fault The fault code to clear
 * @return      Status of the operation
 */
LampResponseCode OEM_LS_ClearFault(LampFaultCode fault);

/**
 * Serialize the Lamp's details.
 * The Lamp Service's definition of this function populates all framework-defined
 * details fields into the message.  The OEM should add any additional
 * fields that they define.
 *
 * @param msg   The msg to serialize data into
 * @return      Status of the operation
 */
LampResponseCode OEM_LS_PopulateDetails(AJ_Message* msg);

/**
 * This struct holds all fields of the Lamp's Details.
 * The OEM may add to this but any new fields should be serialized in the
 * function OEM_LS_PopulateDetails.  The fields below are all required by
 * the framework.
 */
typedef struct {
    LampMake lampMake;                     /**< The make of the lamp */
    LampModel lampModel;                   /**< The model of the lamp */
    DeviceType deviceType;                 /**< The type of device */
    LampType lampType;                     /**< The type of lamp */
    BaseType baseType;                     /**< The make of the lamp base */
    uint32_t deviceLampBeamAngle;          /**< The beam angle */
    uint8_t deviceDimmable;                /**< Is the Lamp dimmable */
    uint8_t deviceColor;                   /**< Does the lamp support color */
    uint8_t variableColorTemp;             /**< variable color temperature? */
    uint8_t deviceHasEffects;              /**< Are effects available? */
    uint32_t deviceMinVoltage;             /**< Minimum voltage */
    uint32_t deviceMaxVoltage;             /**< Maximum voltage */
    uint32_t deviceWattage;                /**< Lamp Wattage */
    uint32_t deviceIncandescentEquivalent; /**< Incandescent equivalent */
    uint32_t deviceMaxLumens;              /**< Maximum light output in Lumens */
    uint32_t deviceMinTemperature;         /**< Minimum supported color temperature */
    uint32_t deviceMaxTemperature;         /**< Maximum supported color temperature */
    uint32_t deviceColorRenderingIndex;    /**< Lamp's Color Rendering Index */
} LampDetailsStruct;


/**
 * A global struct to hold this Lamp's details.
 * If the OEM adds Details fields, they must be initialized in this struct,
 * which is defined in OEM_LS_Code.c
 */
extern const LampDetailsStruct LampDetails;

/**
 * The about icon MIME type
 */
extern const char* aboutIconMimetype;

/**
 * The about icon raw data in the format specified by aboutIconMimetype.
 * Note that when showing an icon, the About client may choose to use either
 * this raw image OR fetch the image pointed to by aboutIconUrl.
 * The OEM may modify the icon by changing this variable in OEM_LS_Code.c
 */
extern const uint8_t aboutIconContent[];

/**
 * The about icon size.  Should be set to sizeof(aboutIconContent)
 */
extern const size_t aboutIconSize;

/**
 * The about icon URL
 * Note that when showing an icon, the About client may choose to use either
 * fetch it from this URL or use the raw data in aboutIconContent.
 * The OEM should set its value in OEM_LS_Code.c
 */
extern const char* aboutIconUrl;

/**
 * The routing node prefix to discover.
 *
 * NOTE: If this not set to org.alljoyn.BusNode, AllJoyn-ON would not be
 * able to on-board this device
 */
extern const char* routingNodePrefix;

/**
 * @}
 */
#endif
