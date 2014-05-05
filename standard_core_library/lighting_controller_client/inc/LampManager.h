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
    virtual void GetAllLampIDsReplyCB(const LSFResponseCode& responseCode, const LSF_ID_List& lampIDs) { }

    /**
     * Indicates that a reply has been received for the GetLampName method call
     *
     * @param responseCode The response code
     * @param lampID       The Lamp ID
     * @param lampName     The Lamp Name
     */
    virtual void GetLampNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const LSF_Name& lampName) { }

    /**
     * Indicates that a reply has been received for the SetLampName method call
     *
     * @param responseCode The response code
     * @param lampID       The Lamp ID
     */
    virtual void SetLampNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }

    /**
     *  Indicates that the signal LampsNameChanged has been received
     *
     *  @param lampIDs   The Lamp IDs
     */
    virtual void LampsNameChangedCB(const LSF_ID_List& lampIDs) { }

    /**
     * Indicates that a reply has been received for the GetLampDetails method call
     *
     * @param responseCode The response code
     * @param lampID       The Lamp ID
     * @param lampDetails  The Lamp Details
     */
    virtual void GetLampDetailsReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const LampDetails& lampDetails) { }

    /**
     * Indicates that a reply has been received for the GetLampParameters method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param lampParameters  The Lamp Parameters
     */
    virtual void GetLampParametersReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const LampParameters& lampParameters) { }

    /**
     * Indicates that a reply has been received for the GetLampParametersField method call
     *
     * @param responseCode        The response code
     * @param lampID              The Lamp ID
     * @param parameterFieldName  Name of the Lamp Parameter field
     * @param value               Value of the Lamp Parameter field
     */
    virtual void GetLampParametersEnergyUsageMilliwattsFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const uint32_t& energyUsageMilliwatts) { }

    /**
     * Indicates that a reply has been received for the GetLampParametersField method call
     *
     * @param responseCode        The response code
     * @param lampID              The Lamp ID
     * @param parameterFieldName  Name of the Lamp Parameter field
     * @param value               Value of the Lamp Parameter field
     */
    virtual void GetLampParametersBrightnessLumensFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const uint32_t& brightnessLumens) { }

    /**
     * Indicates that a reply has been received for the GetLampState method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param lampState       Lamp State
     */
    virtual void GetLampStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const LampState& lampState) { }

    /**
     * Indicates that a reply has been received for the GetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  Name of the Lamp State field
     * @param value           Value of the Lamp State field
     */
    virtual void GetLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const bool& onOff) { }

    /**
     * Indicates that a reply has been received for the GetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  Name of the Lamp State field
     * @param value           Value of the Lamp State field
     */
    virtual void GetLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const uint32_t& hue) { }

    /**
     * Indicates that a reply has been received for the GetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  Name of the Lamp State field
     * @param value           Value of the Lamp State field
     */
    virtual void GetLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const uint32_t& saturation) { }

    /**
     * Indicates that a reply has been received for the GetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  Name of the Lamp State field
     * @param value           Value of the Lamp State field
     */
    virtual void GetLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const uint32_t& brightness) { }

    /**
     * Indicates that a reply has been received for the GetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  Name of the Lamp State field
     * @param value           Value of the Lamp State field
     */
    virtual void GetLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const uint32_t& colorTemp) { }

    /**
     * Indicates that a reply has been received for the ResetLampState method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void ResetLampStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }

    /**
     *  Indicates that the signal LampsStateChanged has been received
     *
     *  @param lampIDs   The Lamp IDs
     */
    virtual void LampsStateChangedCB(const LSF_ID_List& lampIDs) { }

    /**
     * Indicates that a reply has been received for the TransitionLampState method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  The Lamp State Field
     */
    virtual void TransitionLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  The Lamp State Field
     */
    virtual void TransitionLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  The Lamp State Field
     */
    virtual void TransitionLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  The Lamp State Field
     */
    virtual void TransitionLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  The Lamp State Field
     */
    virtual void TransitionLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }

    /**
     * Indicates that a reply has been received for the GetLampFaults method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param faultCodes      List of Lamp Fault Codes
     */
    virtual void GetLampFaultsReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const LampFaultCodeList& faultCodes) { }

    /**
     * Indicates that a reply has been received for the ClearLampFault method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param faultCode
     */
    virtual void ClearLampFaultReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const LampFaultCode& faultCode) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  The Lamp State Field
     */
    virtual void ResetLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  The Lamp State Field
     */
    virtual void ResetLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  The Lamp State Field
     */
    virtual void ResetLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  The Lamp State Field
     */
    virtual void ResetLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }

    /**
     * Indicates that a reply has been received for the ResetLampStateField method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     * @param stateFieldName  The Lamp State Field
     */
    virtual void ResetLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampStateToSavedState method call
     *
     * @param responseCode    The response code
     * @param lampID          The Lamp ID
     */
    virtual void TransitionLampStateToSavedStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) { }
};

/**
 * LampManager is used by the application to make the controller do useful things related to Lamps
 */
class LampManager : public Manager {

    friend class ControllerClient;

  public:
    LampManager(ControllerClient& controller, LampManagerCallback& callback);

    /**
     * Get the IDs of all visible Lamps
     * Response in  LampManagerCallback::GetAllLampIDsReplyCB
     */
    ControllerClientStatus GetAllLampIDs(void);

    /**
     * Get the name of the specified Lamp
     * Response in LampManagerCallback::GetLampNameReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus GetLampName(const LSF_ID& lampID);

    /**
     * Set an Lamp's Name
     * Response in LampManagerCallback::SetLampNameReplyCB
     *
     * @param lampID    The Lamp ID
     * @param lampName  The Lamp Name
     */
    ControllerClientStatus SetLampName(const LSF_ID& lampID, const LSF_Name& lampName);

    /**
     * Get the details of a Lamp
     * Return in LampManagerCallback::GetLampDetailsReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus GetLampDetails(const LSF_ID& lampID);

    /**
     * Get the parameters of a given Lamp
     * Response in LampManagerCallback::GetLampParametersReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus GetLampParameters(const LSF_ID& lampID);

    /**
     * Get a given parameter field from the Lamp
     * Response in LampManagerCallback::GetLampParametersFieldReplyCB
     *
     * @param lampID             The Lamp id
     * @param parameterFieldName The name of the Lamp Parameter Field
     */
    ControllerClientStatus GetLampParametersEnergyUsageMilliwattsField(const LSF_ID& lampID) {
        return GetLampParametersField(lampID, LSF_Name("Energy_Usage_Milliwatts"));
    }

    /**
     * Get a given parameter field from the Lamp
     * Response in LampManagerCallback::GetLampParametersFieldReplyCB
     *
     * @param lampID             The Lamp id
     * @param parameterFieldName The name of the Lamp Parameter Field
     */
    ControllerClientStatus GetLampParametersBrightnessLumensField(const LSF_ID& lampID) {
        return GetLampParametersField(lampID, LSF_Name("Brightness_Lumens"));
    }


    /**
     * Get the Lamp's full state
     * Response in LampManagerCallback::GetLampStateReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus GetLampState(const LSF_ID& lampID);

    /**
     * Get the Lamp's state param
     * Response in LampManagerCallback::GetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus GetLampStateOnOffField(const LSF_ID& lampID) {
        return GetLampStateField(lampID, LSF_Name("OnOff"));
    }

    /**
     * Get the Lamp's state param
     * Response in LampManagerCallback::GetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus GetLampStateHueField(const LSF_ID& lampID) {
        return GetLampStateField(lampID, LSF_Name("Hue"));
    }

    /**
     * Get the Lamp's state param
     * Response in LampManagerCallback::GetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus GetLampStateSaturationField(const LSF_ID& lampID) {
        return GetLampStateField(lampID, LSF_Name("Saturation"));
    }

    /**
     * Get the Lamp's state param
     * Response in LampManagerCallback::GetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus GetLampStateBrightnessField(const LSF_ID& lampID) {
        return GetLampStateField(lampID, LSF_Name("Brightness"));
    }

    /**
     * Get the Lamp's state param
     * Response in LampManagerCallback::GetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus GetLampStateColorTempField(const LSF_ID& lampID) {
        return GetLampStateField(lampID, LSF_Name("ColorTemp"));
    }

    /**
     * Reset the Lamp's state to the default
     * Response in LampManagerCallback::ResetLampStateReplyCB
     *
     * @param lampID    The Lamp id
     */
    ControllerClientStatus ResetLampState(const LSF_ID& lampID);

    /**
     * Reset the Lamp's state param
     * Response in LampManagerCallback::ResetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus ResetLampStateOnOffField(const LSF_ID& lampID) {
        return ResetLampStateField(lampID, LSF_Name("OnOff"));
    }

    /**
     * Reset the Lamp's state param
     * Response in LampManagerCallback::ResetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus ResetLampStateHueField(const LSF_ID& lampID) {
        return ResetLampStateField(lampID, LSF_Name("Hue"));
    }

    /**
     * Reset the Lamp's state param
     * Response in LampManagerCallback::ResetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus ResetLampStateSaturationField(const LSF_ID& lampID) {
        return ResetLampStateField(lampID, LSF_Name("Saturation"));
    }

    /**
     * Reset the Lamp's state param
     * Response in LampManagerCallback::ResetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus ResetLampStateBrightnessField(const LSF_ID& lampID) {
        return ResetLampStateField(lampID, LSF_Name("Brightness"));
    }

    /**
     * Reset the Lamp's state param
     * Response in LampManagerCallback::ResetLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus ResetLampStateColorTempField(const LSF_ID& lampID) {
        return ResetLampStateField(lampID, LSF_Name("ColorTemp"));
    }

    /**
     * Transition the Lamp to a given state
     * Response in LampManagerCallback::TransitionLampStateReplyCB
     *
     * @param lampID    The Lamp id
     * @param lampState The new Lamp state
     */
    ControllerClientStatus TransitionLampState(const LSF_ID& lampID, const LampState& lampState, const uint32_t& transitionPeriod = 0);

    /**
     * Set the Lamp's state param
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus TransitionLampStateOnOffField(const LSF_ID& lampID, const bool& onOff) {
        LSF_Name name("OnOff");
        return TransitionLampStateBooleanField(lampID, name, onOff);
    }

    /**
     * Set the Lamp's state param
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus TransitionLampStateHueField(const LSF_ID& lampID, const uint32_t& hue, const uint32_t& transitionPeriod = 0) {
        LSF_Name name("Hue");
        return TransitionLampStateIntegerField(lampID, name, hue, transitionPeriod);
    }

    /**
     * Set the Lamp's state param
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus TransitionLampStateSaturationField(const LSF_ID& lampID, const uint32_t& saturation, const uint32_t& transitionPeriod = 0) {
        LSF_Name name("Saturation");
        return TransitionLampStateIntegerField(lampID, name, saturation, transitionPeriod);
    }

    /**
     * Set the Lamp's state param
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus TransitionLampStateBrightnessField(const LSF_ID& lampID, const uint32_t& brightness, const uint32_t& transitionPeriod = 0) {
        LSF_Name name("Brightness");
        return TransitionLampStateIntegerField(lampID, name, brightness, transitionPeriod);
    }

    /**
     * Set the Lamp's state param
     * Response in LampManagerCallback::TransitionLampStateFieldReplyCB
     *
     * @param lampID    The Lamp id
     * @param stateFieldName The Lamp state field to fetch
     */
    ControllerClientStatus TransitionLampStateColorTempField(const LSF_ID& lampID, const uint32_t& colorTemp, const uint32_t& transitionPeriod = 0) {
        LSF_Name name("ColorTemp");
        return TransitionLampStateIntegerField(lampID, name, colorTemp, transitionPeriod);
    }

    /**
     * Transition the Lamp to a given saved state
     * Response in LampManagerCallback::TransitionLampStateToSavedStateReplyCB
     *
     * @param lampID    The id of the Lamp
     * @param savedStateID The id of the saved state
     */
    ControllerClientStatus TransitionLampStateToSavedState(const LSF_ID& lampID, const LSF_ID& savedStateID, const uint32_t& transitionPeriod = 0);

    /**
     * Get a list of the Lamp's fault codes
     * Response in LampManagerCallback::GetLampFaultReplyCB
     *
     * @param lampID    The id of the Lamp
     */
    ControllerClientStatus GetLampFaults(const LSF_ID& lampID);

    /**
     * Reset the Lamp's faults
     * Response in LampManagerCallback::ClearLampFaultReplyCB
     *
     * @param lampID    The id of the Lamp
     * @param faultCode Lamp fault code
     */
    ControllerClientStatus ClearLampFault(const LSF_ID& lampID, const LampFaultCode faultCode);

  private:

    ControllerClientStatus GetLampStateField(const LSF_ID& lampID, const LSF_Name& stateFieldName);
    ControllerClientStatus ResetLampStateField(const LSF_ID& lampID, const LSF_Name& stateFieldName);
    ControllerClientStatus TransitionLampStateIntegerField(const LSF_ID& lampID, const LSF_Name& stateFieldName, const uint32_t& value, const uint32_t& transitionPeriod = 0);
    ControllerClientStatus TransitionLampStateBooleanField(const LSF_ID& lampID, const LSF_Name& stateFieldName, const bool& value);
    ControllerClientStatus GetLampParametersField(const LSF_ID& lampID, const LSF_Name& stateFieldName);

    void LampsNameChanged(LSF_ID_List& idList) {
        callback.LampsNameChangedCB(idList);
    }
    void LampsStateChanged(LSF_ID_List& idList) {
        callback.LampsStateChangedCB(idList);
    }

    // method reply handlers
    void GetAllLampIDsReply(LSFResponseCode& responseCode, LSF_ID_List& idList) {
        callback.GetAllLampIDsReplyCB(responseCode, idList);
    }

    void GetLampNameReply(LSFResponseCode& responseCode, LSF_ID& lsfId, LSF_Name& lsfName) {
        callback.GetLampNameReplyCB(responseCode, lsfId, lsfName);
    }

    void SetLampNameReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.SetLampNameReplyCB(responseCode, lsfId);
    }

    void GetLampStateReply(ajn::Message& message);
    void GetLampStateFieldReply(ajn::Message& message);

    void ResetLampStateReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.ResetLampStateReplyCB(responseCode, lsfId);
    }

    void ResetLampStateFieldReply(LSFResponseCode& responseCode, LSF_ID& lsfId, LSF_Name& lsfName);

    void TransitionLampStateReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.TransitionLampStateReplyCB(responseCode, lsfId);
    }

    void TransitionLampStateFieldReply(LSFResponseCode& responseCode, LSF_ID& lsfId, LSF_Name& lsfName);

    void TransitionLampStateToSavedStateReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.TransitionLampStateToSavedStateReplyCB(responseCode, lsfId);
    }

    void GetLampFaultsReply(ajn::Message& message);
    void ClearLampFaultReply(ajn::Message& message);
    void GetLampDetailsReply(ajn::Message& message);
    void GetLampParametersReply(ajn::Message& message);
    void GetLampParametersFieldReply(ajn::Message& message);
    LampManagerCallback&    callback;
};

}

#endif
