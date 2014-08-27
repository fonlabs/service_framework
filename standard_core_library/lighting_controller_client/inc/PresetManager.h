#ifndef _PRESET_MANAGER_H_
#define _PRESET_MANAGER_H_
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

/**
 * a callback class which object delivered to the PresentManager and its object and methods delivered as a handle to get method replies and signals related to presence of the lamps.
 */
class PresetManagerCallback {
  public:
    virtual ~PresetManagerCallback() { }

    /**
     * Response to PresetManager::GetPreset
     *
     * @param responseCode    The return code
     * @param presetID    The id of the Preset
     * @param preset The state information
     */
    virtual void GetPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID, const LampState& preset) { }

    /**
     * Response to PresetManager::GetAllPresetIDs
     *
     * @param responseCode    The return code
     * @param presetIDs   A list of LSFString's
     */
    virtual void GetAllPresetIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& presetIDs) { }

    /**
     * Response to PresetManager::GetPresetName
     *
     * @param responseCode    The return code
     * @param presetID    The id of the Preset
     * @param presetName  The name of the Preset
     */
    virtual void GetPresetNameReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID, const LSFString& language, const LSFString& presetName) { }

    /**
     * Response to PresetManager::SetPresetName
     *
     * @param responseCode    The return code
     * @param presetID    The id of the Preset
     */
    virtual void SetPresetNameReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID, const LSFString& language) { }

    /**
     * A Preset has been renamed
     *
     * @param presetIDs    The id of the Preset
     */
    virtual void PresetsNameChangedCB(const LSFStringList& presetIDs) { }

    /**
     * Response to PresetManager::CreatePreset
     *
     * @param responseCode    The return code
     * @param presetID    The id of the new Preset
     */
    virtual void CreatePresetReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID) { }

    /**
     * A Preset has been created
     *
     * @param presetIDs    The id of the Preset
     */
    virtual void PresetsCreatedCB(const LSFStringList& presetIDs) { }

    /**
     * Response to PresetManager::UpdatePreset
     *
     * @param responseCode    The return code
     * @param presetID    The id of the new Preset
     */
    virtual void UpdatePresetReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID) { }

    /**
     * A Preset has been updated
     *
     * @param presetIDs    The id of the Preset
     */
    virtual void PresetsUpdatedCB(const LSFStringList& presetIDs) { }

    /**
     * Response to PresetManager::DeletePreset
     *
     * @param responseCode    The return code
     * @param presetID    The id of the Preset
     */
    virtual void DeletePresetReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID) { }

    /**
     * A Preset has been deleted
     *
     * @param presetIDs    The id of the Preset
     */
    virtual void PresetsDeletedCB(const LSFStringList& presetIDs) { }

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
     *  @param responseCode   The response code from the LightingControllerService
     */
    virtual void SetDefaultLampStateReplyCB(const LSFResponseCode& responseCode) { }

    /**
     * Indicates that a DefaultLampStateChanged signal has been received
     */
    virtual void DefaultLampStateChangedCB(void) { }
};

/**
 * a class manage the status of the lamps
 */
class PresetManager : public Manager {

    friend class ControllerClient;

  public:

    PresetManager(ControllerClient& controller, PresetManagerCallback& callback);

    /**
     * Get a list of all LSFString's
     * Response in PresetManagerCallback::GetAllPresetIDsReplyCB
     */
    ControllerClientStatus GetAllPresetIDs(void);

    /**
     * Get the preset
     * Response in PresetManagerCallback::GetPresetReplyCB
     *
     * @param presetID    The Preset id
     */
    ControllerClientStatus GetPreset(const LSFString& presetID);

    /**
     * Get the name of a Preset
     * Response in PresetManagerCallback::GetPresetNameReplyCB
     *
     * @param presetID    The id of the Preset
     */
    ControllerClientStatus GetPresetName(const LSFString& presetID, const LSFString& language = LSFString("en"));

    /**
     * Set the name of a Preset
     * Response in PresetManagerCallback::SetPresetNameReplyCB
     *
     * @param presetID    The id of the Preset
     * @param presetName  The new name of the Preset
     */
    ControllerClientStatus SetPresetName(const LSFString& presetID, const LSFString& presetName, const LSFString& language = LSFString("en"));

    /**
     * Create a new preset
     * Response in PresetManagerCallback::CreatePresetReplyCB
     *
     * @param preset The new state information
     */
    ControllerClientStatus CreatePreset(const LampState& preset, const LSFString& presetName, const LSFString& language = LSFString("en"));

    /**
     * Update a Preset
     * Response in PresetManagerCallback::UpdatePresetReplyCB
     *
     * @param presetID    The id of the Preset
     * @param preset The new state information
     */
    ControllerClientStatus UpdatePreset(const LSFString& presetID, const LampState& preset);

    /**
     * Delete a preset
     * Response in PresetManagerCallback::DeletePresetReplyCB
     *
     * @param presetID    The id of the Preset to delete
     */
    ControllerClientStatus DeletePreset(const LSFString& presetID);

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

    /**
     * Get the Preset Info and Name
     *
     * @param presetID    The ID of the master preset
     */
    ControllerClientStatus GetPresetDataSet(const LSFString& presetID, const LSFString& language = LSFString("en"));

  private:

    // signal handlers
    void PresetsNameChanged(LSFStringList& idList) {
        callback.PresetsNameChangedCB(idList);
    }

    void PresetsCreated(LSFStringList& idList) {
        callback.PresetsCreatedCB(idList);
    }

    void PresetsUpdated(LSFStringList& idList) {
        callback.PresetsUpdatedCB(idList);
    }

    void PresetsDeleted(LSFStringList& idList) {
        callback.PresetsDeletedCB(idList);
    }

    /**
     * Handler for the signal DefaultLampStateChanged
     */
    void DefaultLampStateChanged(void) {
        callback.DefaultLampStateChangedCB();
    }

    // method reply handlers
    void GetAllPresetIDsReply(LSFResponseCode& responseCode, LSFStringList& idList) {
        callback.GetAllPresetIDsReplyCB(responseCode, idList);
    }

    void GetPresetReply(ajn::Message& message);

    void GetPresetNameReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language, LSFString& lsfName) {
        callback.GetPresetNameReplyCB(responseCode, lsfId, language, lsfName);
    }

    void SetPresetNameReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language) {
        callback.SetPresetNameReplyCB(responseCode, lsfId, language);
    }

    void CreatePresetReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.CreatePresetReplyCB(responseCode, lsfId);
    }

    void UpdatePresetReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.UpdatePresetReplyCB(responseCode, lsfId);
    }

    void DeletePresetReply(LSFResponseCode& responseCode, LSFString& lsfId) {
        callback.DeletePresetReplyCB(responseCode, lsfId);
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

    PresetManagerCallback& callback;
};

}

#endif
