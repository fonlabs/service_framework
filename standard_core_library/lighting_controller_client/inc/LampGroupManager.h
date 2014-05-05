#ifndef _LAMP_GROUP_MANAGER_H_
#define _LAMP_GROUP_MANAGER_H_

/**
 * @file
 * This file provides definitions for the Lamp Group Manager
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

#include <list>

#include <Manager.h>
#include <ControllerClientDefs.h>


namespace lsf {

class ControllerClient;

/**
 * Abstract base class implemented by User Application Developers.
 * The callbacks defined in this class allow the User Application
 * to be informed when Lamp Groups specific specific AllJoyn method
 * replies or signals are received from the Lighting Controller
 * Service
 */
class LampGroupManagerCallback {
  public:

    /**
     * Destructor
     */
    virtual ~LampGroupManagerCallback() { }

    /**
     * Indicates that a reply has been received for the method call GetAllLampGroupIDs method call
     *
     * @param responseCode   The response code
     * @param lampGroupIDs   Lamp Group IDs
     */
    virtual void GetAllLampGroupIDsReplyCB(const LSFResponseCode& responseCode, const LSF_ID_List& lampGroupIDs) { }

    /**
     * Indicates that a reply has been received for the method call GetLampGroupName method call
     *
     * @param responseCode   The response code
     * @param lampGroupID    The Lamp Group ID
     * @param lampGroupName  The Lamp Group Name
     */
    virtual void GetLampGroupNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID, const LSF_Name& lampGroupName) { }

    /**
     * Indicates that a reply has been received for the SetLampGroupName method call
     *
     * @param responseCode   The response code
     * @param lampGroupID    The Lamp Group ID
     */
    virtual void SetLampGroupNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that the signal LampGroupsNameChanged has been received
     *
     * @param lampGroupIDs    The Lamp Group IDs
     */
    virtual void LampGroupsNameChangedCB(const LSF_ID_List& lampGroupIDs) { }

    /**
     * Indicates that a reply has been received for the CreateLampGroup method call
     *
     * @param responseCode   The response code
     * @param lampGroupID    The Lamp Group ID
     */
    virtual void CreateLampGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     *  Indicates that the signal LampGroupsCreated has been received
     *
     *  @param lampGroupIDs   The Lamp Group IDs
     */
    virtual void LampGroupsCreatedCB(const LSF_ID_List& lampGroupIDs) { }

    /**
     * Indicates that a reply has been received for the GetLampGroup method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     * @parma lampGroup       The Lamp Group
     */
    virtual void GetLampGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID, const LampGroup& lampGroup) { }

    /**
     * Indicates that a reply has been received for the DeleteLampGroup method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     */
    virtual void DeleteLampGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     *  Indicates that the signal LampGroupsDeleted has been received
     *
     *  @param lampGroupIDs   The Lamp Group IDs
     */
    virtual void LampGroupsDeletedCB(const LSF_ID_List& lampGroupIDs) { }

    /**
     * Indicates that a reply has been received for the TransitionLampGroupState method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     */
    virtual void TransitionLampGroupStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampGroupStateField method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     * @param stateFieldName
     */
    virtual void TransitionLampGroupStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampGroupStateField method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     * @param stateFieldName
     */
    virtual void TransitionLampGroupStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampGroupStateField method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     * @param stateFieldName
     */
    virtual void TransitionLampGroupStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampGroupStateField method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     * @param stateFieldName
     */
    virtual void TransitionLampGroupStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that a reply has been received for the TransitionLampGroupStateField method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     * @param stateFieldName
     */
    virtual void TransitionLampGroupStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that a reply has been received for the ResetLampGroupState method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     */
    virtual void ResetLampGroupStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that a reply has been received for the ResetLampGroupStateField method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     * @param stateFieldName
     */
    virtual void ResetLampGroupStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that a reply has been received for the ResetLampGroupStateField method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     * @param stateFieldName
     */
    virtual void ResetLampGroupStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that a reply has been received for the ResetLampGroupStateField method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     * @param stateFieldName
     */
    virtual void ResetLampGroupStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that a reply has been received for the ResetLampGroupStateField method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     * @param stateFieldName
     */
    virtual void ResetLampGroupStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that a reply has been received for the ResetLampGroupStateField method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     * @param stateFieldName
     */
    virtual void ResetLampGroupStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that a reply has been received for the UpdateLampGroup method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     */
    virtual void UpdateLampGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }

    /**
     * Indicates that the signal LampGroupsUpdated has been received
     *
     * @param lampGroupIDs    The Lamp Group IDs
     */
    virtual void LampGroupsUpdatedCB(const LSF_ID_List& lampGroupIDs) { }

    /**
     * Indicates that a reply has been received for the TransitionLampGroupStateToSavedState method call
     *
     * @param responseCode    The response code
     * @param lampGroupID     The Lamp Group ID
     */
    virtual void TransitionLampGroupStateToSavedStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) { }
};

/**
 * This class exposes the APIs that the User Application can use to manage
 * Lamp Groups
 */
class LampGroupManager : public Manager {

    friend class ControllerClient;

  public:

    /**
     * Constructor
     */
    LampGroupManager(ControllerClient& controller, LampGroupManagerCallback& callback);

    /**
     * Get the IDs of all the Lamp Groups
     * Response in LampGroupManagerCallback::GetAllLampGroupIDsReplyCB
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus GetAllLampGroupIDs(void);

    /**
     * Get the name of a Lamp Group
     * Response in LampGroupManagerCallback::GetLampGroupNameCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus GetLampGroupName(const LSF_ID& lampGroupID);

    /**
     * Set the name of the specified Lamp Group
     * Response in LampGroupManagerCallback::SetLampGroupNameReplyCB
     *
     * @param lampGroupID The Lamp Group ID
     * @param lampGroupName        The Lamp Group Name
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus SetLampGroupName(const LSF_ID& lampGroupID, const LSF_Name& lampGroupName);

    /**
     * Create a new Lamp Group
     * Response in LampGroupManagerCallback::CreateLampGroupReplyCB
     *
     * @param lampGroup   Lamp Group
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus CreateLampGroup(const LampGroup& lampGroup);

    /**
     * Modify a Lamp Group
     * Response in LampGroupManagerCallback::UpdateLampGroupReplyCB
     *
     * @param lampGroupID  The Lamp Group ID
     * @param lampGroup    Lamp Group
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus UpdateLampGroup(const LSF_ID& lampGroupID, const LampGroup& lampGroup);

    /**
     * Get the information about a Lamp Group
     * Response in LampGroupManagerCallback::GetLampGroupReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus GetLampGroup(const LSF_ID& lampGroupID);

    /**
     * Delete a Lamp Group
     * Response in LampGroupManagerCallback::DeleteLampGroupReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus DeleteLampGroup(const LSF_ID& lampGroupID);

    /**
     * Transition a Lamp Group to a new state
     * Response in LampGroupManagerCallback::TransitionLampGroupStateReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @param lampGroupState The new state
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus TransitionLampGroupState(const LSF_ID& lampGroupID, const LampState& lampGroupState, const uint32_t& transitionPeriod = 0);

    /**
     * Transition a Lamp Group's field to a given value
     * Response in LampGroupManagerCallback::TransitionLampGroupStateFieldReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @param stateFieldName The name of the state stateFieldName to transition
     * @param value          The new value of the state stateFieldName
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus TransitionLampGroupStateOnOffField(const LSF_ID& lampGroupID, const bool& onOff) {
        LSF_Name name("OnOff");
        return TransitionLampGroupStateBooleanField(lampGroupID, name, onOff);
    }

    /**
     * Transition a Lamp Group's field to a given value
     * Response in LampGroupManagerCallback::TransitionLampGroupStateFieldReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @param stateFieldName The name of the state stateFieldName to transition
     * @param value          The new value of the state stateFieldName
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus TransitionLampGroupStateHueField(const LSF_ID& lampGroupID, const uint32_t& hue, const uint32_t& transitionPeriod = 0) {
        LSF_Name name("Hue");
        return TransitionLampGroupStateIntegerField(lampGroupID, name, hue, transitionPeriod);
    }

    /**
     * Transition a Lamp Group's field to a given value
     * Response in LampGroupManagerCallback::TransitionLampGroupStateFieldReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @param stateFieldName The name of the state stateFieldName to transition
     * @param value          The new value of the state stateFieldName
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus TransitionLampGroupStateSaturationField(const LSF_ID& lampGroupID, const uint32_t& saturation, const uint32_t& transitionPeriod = 0) {
        LSF_Name name("Saturation");
        return TransitionLampGroupStateIntegerField(lampGroupID, name, saturation, transitionPeriod);
    }

    /**
     * Transition a Lamp Group's field to a given value
     * Response in LampGroupManagerCallback::TransitionLampGroupStateFieldReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @param stateFieldName The name of the state stateFieldName to transition
     * @param value          The new value of the state stateFieldName
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus TransitionLampGroupStateBrightnessField(const LSF_ID& lampGroupID, const uint32_t& brightness, const uint32_t& transitionPeriod = 0) {
        LSF_Name name("Brightness");
        return TransitionLampGroupStateIntegerField(lampGroupID, name, brightness, transitionPeriod);
    }

    /**
     * Transition a Lamp Group's field to a given value
     * Response in LampGroupManagerCallback::TransitionLampGroupStateFieldReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @param stateFieldName The name of the state stateFieldName to transition
     * @param value          The new value of the state stateFieldName
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus TransitionLampGroupStateColorTempField(const LSF_ID& lampGroupID, const uint32_t& colorTemp, const uint32_t& transitionPeriod = 0) {
        LSF_Name name("ColorTemp");
        return TransitionLampGroupStateIntegerField(lampGroupID, name, colorTemp, transitionPeriod);
    }

    /**
     * Transition a Lamp Group to a given Saved State
     * Response in LampGroupManagerCallback::TransitionLampGroupStateToSavedStateReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @param savedStateID   The Saved State ID
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus TransitionLampGroupStateToSavedState(const LSF_ID& lampGroupID, const LSF_ID& savedStateID, const uint32_t& transitionPeriod = 0);

    /**
     * Reset a Lamp Group's state
     * Response in LampGroupManagerCallback::ResetLampGroupStateReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus ResetLampGroupState(const LSF_ID& lampGroupID);

    /**
     * Reset a Lamp Group's field to a given value
     * Response in LampGroupManagerCallback::ResetLampGroupStateFieldReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @param stateFieldName The name of the state stateFieldName to transition
     * @param value          The new value of the state stateFieldName
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus ResetLampGroupStateOnOffField(const LSF_ID& lampGroupID) {
        LSF_Name name("OnOff");
        return ResetLampGroupStateField(lampGroupID, name);
    }

    /**
     * Reset a Lamp Group's field to a given value
     * Response in LampGroupManagerCallback::ResetLampGroupStateFieldReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @param stateFieldName The name of the state stateFieldName to transition
     * @param value          The new value of the state stateFieldName
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus ResetLampGroupStateHueField(const LSF_ID& lampGroupID) {
        LSF_Name name("Hue");
        return ResetLampGroupStateField(lampGroupID, name);
    }

    /**
     * Reset a Lamp Group's field to a given value
     * Response in LampGroupManagerCallback::ResetLampGroupStateFieldReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @param stateFieldName The name of the state stateFieldName to transition
     * @param value          The new value of the state stateFieldName
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus ResetLampGroupStateSaturationField(const LSF_ID& lampGroupID) {
        LSF_Name name("Saturation");
        return ResetLampGroupStateField(lampGroupID, name);
    }

    /**
     * Reset a Lamp Group's field to a given value
     * Response in LampGroupManagerCallback::ResetLampGroupStateFieldReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @param stateFieldName The name of the state stateFieldName to transition
     * @param value          The new value of the state stateFieldName
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus ResetLampGroupStateBrightnessField(const LSF_ID& lampGroupID) {
        LSF_Name name("Brightness");
        return ResetLampGroupStateField(lampGroupID, name);
    }

    /**
     * Reset a Lamp Group's field to a given value
     * Response in LampGroupManagerCallback::ResetLampGroupStateFieldReplyCB
     *
     * @param lampGroupID    The Lamp Group ID
     * @param stateFieldName The name of the state stateFieldName to transition
     * @param value          The new value of the state stateFieldName
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     *
     */
    ControllerClientStatus ResetLampGroupStateColorTempField(const LSF_ID& lampGroupID) {
        LSF_Name name("ColorTemp");
        return ResetLampGroupStateField(lampGroupID, name);
    }

  private:

    ControllerClientStatus TransitionLampGroupStateIntegerField(const LSF_ID& lampGroupID, const LSF_Name& stateFieldName, const uint32_t& value, const uint32_t& transitionPeriod = 0);
    ControllerClientStatus TransitionLampGroupStateBooleanField(const LSF_ID& lampGroupID, const LSF_Name& stateFieldName, const bool& value);
    ControllerClientStatus ResetLampGroupStateField(const LSF_ID& lampGroupID, const LSF_Name& stateFieldName);

    /**
     * Handler for the signal LampGroupsNameChanged
     */
    void LampGroupsNameChanged(LSF_ID_List& idList) {
        callback.LampGroupsNameChangedCB(idList);
    }

    /**
     * Handler for the signal LampGroupsCreated
     */
    void LampGroupsCreated(LSF_ID_List& idList) {
        callback.LampGroupsCreatedCB(idList);
    }

    /**
     * Handler for the signal LampGroupsDeleted
     */
    void LampGroupsDeleted(LSF_ID_List& idList) {
        callback.LampGroupsDeletedCB(idList);
    }

    /**
     * Handler for the signal LampGroupsUpdated
     */
    void LampGroupsUpdated(LSF_ID_List& idList) {
        callback.LampGroupsUpdatedCB(idList);
    }
    /**
     * Method Reply Handler for the signal GetAllLampGroupIDs
     */
    void GetAllLampGroupIDsReply(LSFResponseCode& responseCode, LSF_ID_List& idList) {
        callback.GetAllLampGroupIDsReplyCB(responseCode, idList);
    }

    /**
     * Method Reply Handler for the signal GetLampGroupName
     */
    void GetLampGroupNameReply(LSFResponseCode& responseCode, LSF_ID& lsfId, LSF_Name& lsfName) {
        callback.GetLampGroupNameReplyCB(responseCode, lsfId, lsfName);
    }

    /**
     * Method Reply Handler for the signal SetLampGroupName
     */
    void SetLampGroupNameReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.SetLampGroupNameReplyCB(responseCode, lsfId);
    }

    /**
     * Method Reply Handler for the signal CreateLampGroup
     */
    void CreateLampGroupReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.CreateLampGroupReplyCB(responseCode, lsfId);
    }

    /**
     * Method Reply Handler for the signal UpdateLampGroup
     */
    void UpdateLampGroupReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.UpdateLampGroupReplyCB(responseCode, lsfId);
    }

    /**
     * Method Reply Handler for the signal GetLampGroup
     */
    void GetLampGroupReply(ajn::Message& message);

    /**
     * Method Reply Handler for the signal DeleteLampGroup
     */
    void DeleteLampGroupReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.DeleteLampGroupReplyCB(responseCode, lsfId);
    }

    /**
     * Method Reply Handler for the signal ResetLampGroupFieldState
     */
    void ResetLampGroupFieldStateReply(LSFResponseCode& responseCode, LSF_ID& lsfId, LSF_Name& lsfName);

    /**
     * Method Reply Handler for the signal ResetLampGroupState
     */
    void ResetLampGroupStateReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.ResetLampGroupStateReplyCB(responseCode, lsfId);
    }

    /**
     * Method Reply Handler for the signal TransitionLampGroupState
     */
    void TransitionLampGroupStateReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.TransitionLampGroupStateReplyCB(responseCode, lsfId);
    }

    /**
     * Method Reply Handler for the signal TransitionLampGroupStateField
     */
    void TransitionLampGroupStateFieldReply(LSFResponseCode& responseCode, LSF_ID& lsfId, LSF_Name& lsfName);

    /**
     * Method Reply Handler for the signal TransitionLampGroupStateToSavedState
     */
    void TransitionLampGroupStateToSavedStateReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.TransitionLampGroupStateToSavedStateReplyCB(responseCode, lsfId);
    }

    /**
     * Callback used to send Lamp Group specific signals and method replies
     * to the User Application
     */
    LampGroupManagerCallback&   callback;
};


}

#endif
