#ifndef _LAMP_MANAGER_H_
#define _LAMP_MANAGER_H_
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

#include <LSFTypes.h>
#include <Manager.h>
#include <ControllerClientDefs.h>

#include <alljoyn/InterfaceDescription.h>


#include <LSFResponseCodes.h>

namespace lsf {

class ControllerClient;

/**
 * Abstract base class implemented by User Application Developers.
 * The callbacks defined in this class allow the User Application
 * to be informed when Lamps specific AllJoyn method
 * replies or signals are received from the Lighting Controller
 * Service
 */
class LampManagerCallback {
  public:

    /**
     * Destructor
     */
    virtual ~LampManagerCallback() { }

    /**
     * Indicates that a reply has been received for the GetAllLampIDs method call
     *
     * @param responseCode  The response code
     * @param lampIDs       The Lamp IDs
     */
    virtual void GetAllLampIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& lampIDs) { }

    /**
     * Indicates that a reply has been received for the GetLampName method call
     *
     * @param responseCode The response code
     * @param lampID       The Lamp ID
     * @param language
     * @param lampName     The Lamp Name
     */
    virtual void GetLampNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFString& language, const LSFString& lampName) { }

    /**
     * Indicates that a reply has been received for the GetLampName method call
     *
     * @param responseCode The response code
     * @param lampID       The Lamp ID
     * @param language
     * @param manufacturer
     */
    virtual void GetLampManufacturerReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFString& language, const LSFString& manufacturer) { }

    /**
     * Indicates that a reply has been received for the SetLampName method call
     *
     * @param responseCode The response code
     * @param lampID       The Lamp ID
     * @param language
     */
    virtual void SetLampNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFString& language) { }

    /**
     *  Indicates that the signal LampsNameChanged has been received
     *
     *  @param lampIDs   The Lamp IDs
     */
    virtual void LampsNameChangedCB(const LSFStringList& lampIDs) { }

    /**
     *  Indicates that the signal LampsFound has been received
     *
     *  @param lampIDs   The Lamp IDs
     */
    virtual void LampsFoundCB(const LSFStringList& lampIDs) { }

    /**
     *  Indicates that the signal LampsLost has been received
     *
     *  @param lampIDs   The Lamp IDs
     */
    virtual void LampsLostCB(const LSFStringList& lampIDs) { }

    /**
     * Indicates that a reply has been received for the GetLampDetails method call
     *
     * @param responseCode The response code
     * @param lampID       The Lamp ID
     * @param lampDetails  The Lamp Details
     */
    virtual void GetLampDetailsReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampDetails& lampDetails) { }

    /**
     * Indicates that a reply has been received for the GetLampParameters method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param lampParameters  The Lamp Parameters
     */
    virtual void GetLampParametersReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampParameters& lampParameters) { }

    /**
     * Indicates that a reply has been received for the GetLampParametersField method call
     *
     * @param responseCode        The response code
     * @param lampID              The Lamp ID
     * @param energyUsageMilliwatts
     */
    virtual void GetLampParametersEnergyUsageMilliwattsFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& energyUsageMilliwatts) { }

    /**
     * Indicates that a reply has been received for the GetLampParametersField method call
     *
     * @param responseCode        The response code
     * @param lampID              The Lamp ID
     * @param brightnessLumens
     */
    virtual void GetLampParametersLumensFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& brightnessLumens) { }

    /**
     * Indicates that a reply has been received for the GetLampState method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param lampState       Lamp State
     */
    virtual void GetLampStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampState& lampState) { }

    /**
     * Indicates that a reply has been received for the GetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param onOff
     */
    virtual void GetLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const bool& onOff) { }

    /**
     * Indicates that a reply has been received for the GetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param hue
     */
    virtual void GetLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& hue) { }

    /**
     * Indicates that a reply has been received for the GetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param saturation
     */
    virtual void GetLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& saturation) { }

    /**
     * Indicates that a reply has been received for the GetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param brightness
     */
    virtual void GetLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& brightness) { }

    /**
     * Indicates that a reply has been received for the GetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param colorTemp
     */
    virtual void GetLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& colorTemp) { }

    /**
     * Indicates that a reply has been received for the ResetLampState method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void ResetLampStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     *  Indicates that the signal LampsStateChanged has been received
     *
     *  @param lampIDs   The Lamp IDs
     */
    virtual void LampsStateChangedCB(const LSFStringList& lampIDs) { }

    /**
     * Indicates that a reply has been received for the TransitionLampState method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampState method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void PulseLampWithStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampPreset method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void PulseLampWithPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the GetLampFaults method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param faultCodes      List of Lamp Fault Codes
     */
    virtual void GetLampFaultsReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampFaultCodeList& faultCodes) { }

    /**
     * Indicates that a reply has been received for the GetLampFaults method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param lampServiceVersion
     */
    virtual void GetLampServiceVersionReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& lampServiceVersion) { }

    /**
     * Indicates that a reply has been received for the ClearLampFault method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param faultCode
     */
    virtual void ClearLampFaultReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampFaultCode& faultCode) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void ResetLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void ResetLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void ResetLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void ResetLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void ResetLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateToPreset method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateToPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) { }
    /**
     * Get Lamp Supported Languages Reply callback
     */
    virtual void GetLampSupportedLanguagesReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFStringList& supportedLanguages) { };
};

/**
 * LampManager is used by the application to make the controller do useful things related to Lamps
 */
class LampManager : public Manager {

    friend class ControllerClient;

  public:
    /**
     * LampManager constructor
     */
    LampManager(ControllerClient& controller, LampManagerCallback& callback);

    /**
     * Get the IDs of all visible Lamps
     * Response in  LampManagerCallback::GetAllLampIDsReplyCB
     */
    ControllerClientStatus GetAllLampIDs(void);
    /**
     * Get Lamp Manufacturer
     */
    ControllerClientStatus GetLampManufacturer(const LSFString& lampID, const LSFString& language = LSFString("en"));

    /**
     * Get the name of the specified Lamp
     * Response in LampManagerCallback::GetLampNameReplyCB
     */
    ControllerClientStatus GetLampName(const LSFString& lampID, const LSFString& language = LSFString("en"));

    /**
     * Set an Lamp's Name
     * Response in LampManagerCallback::SetLampNameReplyCB
     *
     * @param lampID    The Lamp ID
     * @param lampName  The Lamp Name
     * @param language
     */
    ControllerClientStatus SetLampName(const LSFString& lampID, const LSFString& lampName, const LSFString& language = LSFString("en"));

    /**
     * Get the details of a Lamp
     * Return in LampManagerCallback::GetLampDetailsReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus GetLampDetails(const LSFString& lampID);

    /**
     * Get the parameters of a given Lamp
     * Response in LampManagerCallback::GetLampParametersReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus GetLampParameters(const LSFString& lampID);

    /**
     * Get a given parameter field from the Lamp
     * Response in LampManagerCallback::GetLampParametersFieldReplyCB
     */
    ControllerClientStatus GetLampParametersEnergyUsageMilliwattsField(const LSFString& lampID) {
        return GetLampParametersField(lampID, LSFString("Energy_Usage_Milliwatts"));
    }

    /**
     * Get a given parameter field from the Lamp
     * Response in LampManagerCallback::GetLampParametersFieldReplyCB
     */
    ControllerClientStatus GetLampParametersLumensField(const LSFString& lampID) {
        return GetLampParametersField(lampID, LSFString("Brightness_Lumens"));
    }


    /**
     * Get the Lamp's full state
     * Response in LampManagerCallback::GetLampStateReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus GetLampState(const LSFString& lampID);

    /**
     * Get the Lamp's state param
     * Response in LampManagerCallback::GetLampStateFieldReplyCB
     */
    ControllerClientStatus GetLampStateOnOffField(const LSFString& lampID) {
        return GetLampStateField(lampID, LSFString("OnOff"));
    }

    /**
     * Get the Lamp's state param
     * Response in LampManagerCallback::GetLampStateFieldReplyCB
     */
    ControllerClientStatus GetLampStateHueField(const LSFString& lampID) {
        return GetLampStateField(lampID, LSFString("Hue"));
    }

    /**
     * Get the Lamp's state param
     * Response in LampManagerCallback::GetLampStateFieldReplyCB
     */
    ControllerClientStatus GetLampStateSaturationField(const LSFString& lampID) {
        return GetLampStateField(lampID, LSFString("Saturation"));
    }

    /**
     * Get the Lamp's state param
     * Response in LampManagerCallback::GetLampStateFieldReplyCB
     */
    ControllerClientStatus GetLampStateBrightnessField(const LSFString& lampID) {
        return GetLampStateField(lampID, LSFString("Brightness"));
    }

    /**
     * Get the Lamp's state param
     * Response in LampManagerCallback::GetLampStateFieldReplyCB
     */
    ControllerClientStatus GetLampStateColorTempField(const LSFString& lampID) {
        return GetLampStateField(lampID, LSFString("ColorTemp"));
    }

    /**
     * Reset the Lamp's state to the default
     * Response in LampManagerCallback::ResetLampStateReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus ResetLampState(const LSFString& lampID);

    /**
     * Reset the Lamp's state param
     * Response in LampManagerCallback::ResetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus ResetLampStateOnOffField(const LSFString& lampID) {
        return ResetLampStateField(lampID, LSFString("OnOff"));
    }

    /**
     * Reset the Lamp's state param
     * Response in LampManagerCallback::ResetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus ResetLampStateHueField(const LSFString& lampID) {
        return ResetLampStateField(lampID, LSFString("Hue"));
    }

    /**
     * Reset the Lamp's state param
     * Response in LampManagerCallback::ResetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus ResetLampStateSaturationField(const LSFString& lampID) {
        return ResetLampStateField(lampID, LSFString("Saturation"));
    }

    /**
     * Reset the Lamp's state param
     * Response in LampManagerCallback::ResetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus ResetLampStateBrightnessField(const LSFString& lampID) {
        return ResetLampStateField(lampID, LSFString("Brightness"));
    }

    /**
     * Reset the Lamp's state param
     * Response in LampManagerCallback::ResetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus ResetLampStateColorTempField(const LSFString& lampID) {
        return ResetLampStateField(lampID, LSFString("ColorTemp"));
    }

    /**
     * Transition the Lamp to a given state
     * Response in LampManagerCallback::TransitionLampStateReplyCB
     */
    ControllerClientStatus TransitionLampState(const LSFString& lampID, const LampState& lampState, const uint32_t& transitionPeriod = 0);

    /**
     * Transition the Lamp to a given state
     * Response in LampManagerCallback::TransitionLampStateReplyCB
     */
    ControllerClientStatus PulseLampWithState(const LSFString& lampID, const LampState& toLampState, const uint32_t& period, const uint32_t& duration, const uint32_t& numPulses, const LampState& fromLampState = LampState());

    /**
     * Transition the Lamp to a given state
     * Response in LampManagerCallback::TransitionLSFStringReplyCB
     */
    ControllerClientStatus PulseLampWithPreset(const LSFString& lampID, const LSFString& toPresetID, const uint32_t& period, const uint32_t& duration, const uint32_t& numPulses, const LSFString& fromPresetID = CurrentStateIdentifier);

    /**
     * Set the Lamp's state param
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param onOff
     */
    ControllerClientStatus TransitionLampStateOnOffField(const LSFString& lampID, const bool& onOff) {
        LSFString name("OnOff");
        return TransitionLampStateBooleanField(lampID, name, onOff);
    }

    /**
     * Set the Lamp's state param
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     */
    ControllerClientStatus TransitionLampStateHueField(const LSFString& lampID, const uint32_t& hue, const uint32_t& transitionPeriod = 0) {
        LSFString name("Hue");
        return TransitionLampStateIntegerField(lampID, name, hue, transitionPeriod);
    }

    /**
     * Set the Lamp's state param
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     */
    ControllerClientStatus TransitionLampStateSaturationField(const LSFString& lampID, const uint32_t& saturation, const uint32_t& transitionPeriod = 0) {
        LSFString name("Saturation");
        return TransitionLampStateIntegerField(lampID, name, saturation, transitionPeriod);
    }

    /**
     * Set the Lamp's state param
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     */
    ControllerClientStatus TransitionLampStateBrightnessField(const LSFString& lampID, const uint32_t& brightness, const uint32_t& transitionPeriod = 0) {
        LSFString name("Brightness");
        return TransitionLampStateIntegerField(lampID, name, brightness, transitionPeriod);
    }

    /**
     * Set the Lamp's state param
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     */
    ControllerClientStatus TransitionLampStateColorTempField(const LSFString& lampID, const uint32_t& colorTemp, const uint32_t& transitionPeriod = 0) {
        LSFString name("ColorTemp");
        return TransitionLampStateIntegerField(lampID, name, colorTemp, transitionPeriod);
    }

    /**
     * Transition the Lamp to a given preset
     * Response in LampManagerCallback::TransitionLampStateToPresetReplyCB
     *
     * @param lampID    The id of the Lamp
     * @param presetID The id of the preset
     * @param transitionPeriod
     */
    ControllerClientStatus TransitionLampStateToPreset(const LSFString& lampID, const LSFString& presetID, const uint32_t& transitionPeriod = 0);

    /**
     * Get a list of the Lamp's fault codes
     * Response in LampManagerCallback::GetLampFaultReplyCB
     *
     * @param lampID    The id of the Lamp
     */
    ControllerClientStatus GetLampFaults(const LSFString& lampID);

    /**
     * Get a list of the Lamp's fault codes
     * Response in LampManagerCallback::GetLampFaultReplyCB
     *
     * @param lampID    The id of the Lamp
     */
    ControllerClientStatus GetLampServiceVersion(const LSFString& lampID);

    /**
     * Reset the Lamp's faults
     * Response in LampManagerCallback::ClearLampFaultReplyCB
     *
     * @param lampID    The id of the Lamp
     * @param faultCode Lamp fault code
     */
    ControllerClientStatus ClearLampFault(const LSFString& lampID, const LampFaultCode& faultCode);
    /**
     * Get Lamp Supported Languages
     */
    ControllerClientStatus GetLampSupportedLanguages(const LSFString& lampID);

    /**
     * Get the Lamp Group Info and Name
     */
    ControllerClientStatus GetLampDataSet(const LSFString& lampID, const LSFString& language = LSFString("en"));

  private:

    ControllerClientStatus GetLampStateField(const LSFString& lampID, const LSFString& stateFieldName);
    ControllerClientStatus ResetLampStateField(const LSFString& lampID, const LSFString& stateFieldName);
    ControllerClientStatus TransitionLampStateIntegerField(const LSFString& lampID, const LSFString& stateFieldName, const uint32_t& value, const uint32_t& transitionPeriod = 0);
    ControllerClientStatus TransitionLampStateBooleanField(const LSFString& lampID, const LSFString& stateFieldName, const bool& value);
    ControllerClientStatus GetLampParametersField(const LSFString& lampID, const LSFString& stateFieldName);

    void LampsNameChanged(LSFStringList& idList) {
        callback.LampsNameChangedCB(idList);
    }

    void LampsStateChanged(LSFStringList& idList) {
        callback.LampsStateChangedCB(idList);
    }

    void LampsFound(LSFStringList& idList) {
        callback.LampsFoundCB(idList);
    }

    void LampsLost(LSFStringList& idList) {
        callback.LampsLostCB(idList);
    }

    // method reply handlers
    void GetAllLampIDsReply(LSFResponseCode& responseCode, LSFStringList& idList) {
        callback.GetAllLampIDsReplyCB(responseCode, idList);
    }

    void GetLampManufacturerReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language, LSFString& manufacturer) {
        callback.GetLampManufacturerReplyCB(responseCode, lsfId, language, manufacturer);
    }
    void GetLampNameReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language, LSFString& name) {
        callback.GetLampNameReplyCB(responseCode, lsfId, language, name);
    }
    void GetLampSupportedLanguagesReply(ajn::Message& message);

    void SetLampNameReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language) {
        callback.SetLampNameReplyCB(responseCode, lsfId, language);
    }

    void GetLampStateReply(ajn::Message& message);
    void GetLampStateFieldReply(ajn::Message& message);

    void ResetLampStateReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.ResetLampStateReplyCB(responseCode, lsfId);
    }

    void ResetLampStateFieldReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName);

    void TransitionLampStateReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.TransitionLampStateReplyCB(responseCode, lsfId);
    }

    void PulseLampWithStateReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.PulseLampWithStateReplyCB(responseCode, lsfId);
    }

    void PulseLampWithPresetReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.PulseLampWithPresetReplyCB(responseCode, lsfId);
    }

    void TransitionLampStateFieldReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName);

    void TransitionLampStateToPresetReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.TransitionLampStateToPresetReplyCB(responseCode, lsfId);
    }

    void GetLampFaultsReply(ajn::Message& message);
    void GetLampServiceVersionReply(ajn::Message& message);
    void ClearLampFaultReply(ajn::Message& message);
    void GetLampDetailsReply(ajn::Message& message);
    void GetLampParametersReply(ajn::Message& message);
    void GetLampParametersFieldReply(ajn::Message& message);
    LampManagerCallback&    callback;
};

}

#endif
