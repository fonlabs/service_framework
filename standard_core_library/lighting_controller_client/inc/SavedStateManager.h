#ifndef _SAVED_STATE_MANAGER_H_
#define _SAVED_STATE_MANAGER_H_
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

#include <list>
#include <LSFResponseCodes.h>

namespace lsf {

class ControllerClient;

class SavedStateManagerCallback {
  public:
    virtual ~SavedStateManagerCallback() { }

    /**
     * Response to SavedStateManager::GetSavedState
     *
     * @param responseCode    The return code
     * @param savedStateID    The id of the SavedState
     * @param savedState The state information
     */
    virtual void GetSavedStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& savedStateID, const LampState& savedState) { }

    /**
     * Response to SavedStateManager::GetAllSavedStateIDs
     *
     * @param responseCode    The return code
     * @param savedStateIDs   A list of LSF_ID's
     */
    virtual void GetAllSavedStateIDsReplyCB(const LSFResponseCode& responseCode, const LSF_ID_List& savedStateIDs) { }

    /**
     * Response to SavedStateManager::GetSavedStateName
     *
     * @param responseCode    The return code
     * @param savedStateID    The id of the SavedState
     * @param savedStateName  The name of the SavedState
     */
    virtual void GetSavedStateNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& savedStateID, const LSF_Name& savedStateName) { }

    /**
     * Response to SavedStateManager::SetSavedStateName
     *
     * @param responseCode    The return code
     * @param savedStateID    The id of the SavedState
     */
    virtual void SetSavedStateNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& savedStateID) { }

    /**
     * A SavedState has been renamed
     *
     * @param savedStateIDs    The id of the SavedState
     */
    virtual void SavedStatesNameChangedCB(const LSF_ID_List& savedStateIDs) { }

    /**
     * Response to SavedStateManager::CreateSavedState
     *
     * @param responseCode    The return code
     * @param savedStateID    The id of the new SavedState
     */
    virtual void CreateSavedStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& savedStateID) { }

    /**
     * A SavedState has been created
     *
     * @param savedStateIDs    The id of the SavedState
     */
    virtual void SavedStatesCreatedCB(const LSF_ID_List& savedStateIDs) { }

    /**
     * Response to SavedStateManager::UpdateSavedState
     *
     * @param responseCode    The return code
     * @param savedStateID    The id of the new SavedState
     */
    virtual void UpdateSavedStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& savedStateID) { }

    /**
     * A SavedState has been updated
     *
     * @param savedStateIDs    The id of the SavedState
     */
    virtual void SavedStatesUpdatedCB(const LSF_ID_List& savedStateIDs) { }

    /**
     * Response to SavedStateManager::DeleteSavedState
     *
     * @param responseCode    The return code
     * @param savedStateID    The id of the SavedState
     */
    virtual void DeleteSavedStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& savedStateID) { }

    /**
     * A SavedState has been deleted
     *
     * @param savedStateIDs    The id of the SavedState
     */
    virtual void SavedStatesDeletedCB(const LSF_ID_List& savedStateIDs) { }

    /**
     *  Indicates that a reply has been received for the GetDefaultLampState method call
     *
     *  @param responseCode    The response code
     *  @param state           The default LampState
     */
    virtual void GetDefaultLampStateReplyCB(const LSFResponseCode& responseCode, const LampState& defaultLampState) { }

    /**
     *  Indicates that a reply has been received for the SetDefaultLampState method call
     *
     *  @param responseCode   The response code from the LampControllerService
     */
    virtual void SetDefaultLampStateReplyCB(const LSFResponseCode& responseCode) { }

    /**
     * Indicates that a DefaultLampStateChanged signal has been received
     */
    virtual void DefaultLampStateChangedCB(void) { }
};


class SavedStateManager : public Manager {

    friend class ControllerClient;

  public:

    SavedStateManager(ControllerClient& controller, SavedStateManagerCallback& callback);

    /**
     * Get a list of all LSF_ID's
     * Response in SavedStateManagerCallback::GetAllSavedStateIDsReplyCB
     */
    ControllerClientStatus GetAllSavedStateIDs(void);

    /**
     * Get the saved state
     * Response in SavedStateManagerCallback::GetSavedStateReplyCB
     *
     * @param savedStateID    The SavedState id
     */
    ControllerClientStatus GetSavedState(const LSF_ID& savedStateID);

    /**
     * Get the name of a SavedState
     * Response in SavedStateManagerCallback::GetSavedStateNameReplyCB
     *
     * @param savedStateID    The id of the SavedState
     */
    ControllerClientStatus GetSavedStateName(const LSF_ID& savedStateID);

    /**
     * Set the name of a SavedState
     * Response in SavedStateManagerCallback::SetSavedStateNameReplyCB
     *
     * @param savedStateID    The id of the SavedState
     * @param savedStateName  The new name of the SavedState
     */
    ControllerClientStatus SetSavedStateName(const LSF_ID& savedStateID, const LSF_Name& savedStateName);

    /**
     * Create a new saved state
     * Response in SavedStateManagerCallback::CreateSavedStateReplyCB
     *
     * @param savedState The new state information
     */
    ControllerClientStatus CreateSavedState(const LampState& savedState);

    /**
     * Update a SavedState
     * Response in SavedStateManagerCallback::UpdateSavedStateReplyCB
     *
     * @param savedStateID    The id of the SavedState
     * @param savedState The new state information
     */
    ControllerClientStatus UpdateSavedState(const LSF_ID& savedStateID, const LampState& savedState);

    /**
     * Delete a saved state
     * Response in SavedStateManagerCallback::DeleteSavedStateReplyCB
     *
     * @param savedStateID    The id of the SavedState to delete
     */
    ControllerClientStatus DeleteSavedState(const LSF_ID& savedStateID);

    /**
     * Get the default Lamp State
     * Response comes in LampManagerCallback::GetDefaultLampStateReplyCB
     *
     * @param None
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     */
    ControllerClientStatus GetDefaultLampState(void);

    /**
     * Set the default state of new Lamps
     * Response comes in LampManagerCallback::SetDefaultLampStateReplyCB
     *
     * @param  defaultLampState The Lamp state
     * @return
     *      - CONTROLLER_CLIENT_OK if successful
     *      - An error status otherwise
     */
    ControllerClientStatus SetDefaultLampState(const LampState& defaultLampState);

  private:

    // signal handlers
    void SavedStatesNameChanged(LSF_ID_List& idList) {
        callback.SavedStatesNameChangedCB(idList);
    }

    void SavedStatesCreated(LSF_ID_List& idList) {
        callback.SavedStatesCreatedCB(idList);
    }

    void SavedStatesUpdated(LSF_ID_List& idList) {
        callback.SavedStatesUpdatedCB(idList);
    }

    void SavedStatesDeleted(LSF_ID_List& idList) {
        callback.SavedStatesDeletedCB(idList);
    }

    /**
     * Handler for the signal DefaultLampStateChanged
     */
    void DefaultLampStateChanged(void) {
        callback.DefaultLampStateChangedCB();
    }

    // method reply handlers
    void GetAllSavedStateIDsReply(LSFResponseCode& responseCode, LSF_ID_List& idList) {
        callback.GetAllSavedStateIDsReplyCB(responseCode, idList);
    }

    void GetSavedStateReply(ajn::Message& message);

    void GetSavedStateNameReply(LSFResponseCode& responseCode, LSF_ID& lsfId, LSF_Name& lsfName) {
        callback.GetSavedStateNameReplyCB(responseCode, lsfId, lsfName);
    }

    void SetSavedStateNameReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.SetSavedStateNameReplyCB(responseCode, lsfId);
    }

    void CreateSavedStateReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.CreateSavedStateReplyCB(responseCode, lsfId);
    }

    void UpdateSavedStateReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.UpdateSavedStateReplyCB(responseCode, lsfId);
    }

    void DeleteSavedStateReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.DeleteSavedStateReplyCB(responseCode, lsfId);
    }

    /**
     * Method Reply Handler for the signal GetDefaultLampState
     */
    void GetDefaultLampStateReply(ajn::Message& message);

    /**
     * Method Reply Handler for the signal GetDefaultLampState
     */
    void SetDefaultLampStateReply(uint32_t& responseCode) {
        callback.SetDefaultLampStateReplyCB((LSFResponseCode &)responseCode);
    }

    SavedStateManagerCallback& callback;
};

}

#endif
