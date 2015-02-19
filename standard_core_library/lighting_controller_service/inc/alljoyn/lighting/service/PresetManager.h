#ifndef _PRESET_MANAGER_H_
#define _PRESET_MANAGER_H_
/**
 * \ingroup ControllerService
 */
/**
 * @file
 * This file provides definitions for present manager
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

#include <alljoyn/lighting/service/Manager.h>

#include <Mutex.h>
#include <alljoyn/lighting/LSFTypes.h>

#include <string>
#include <map>

namespace lsf {

class SceneManager;
/**
 * class manages the pre-set of the lamps. \n
 * Pre-set is the ability to save lamp states and to use them when required later on.
 */
class PresetManager : public Manager {
    friend class LampManager;
  public:
    /**
     * class constructor. \n
     * @param controllerSvc - reference to controller service instance
     * @param sceneMgrPtr - pointer to scene manager
     * @param presetFile - The full path of pre-set file to be the persistent data
     */
    PresetManager(ControllerService& controllerSvc, SceneManager* sceneMgrPtr, const std::string& presetFile);
    /**
     * Clears the pre-sets data. \n
     * Send signal to the controller clients 'org.allseen.LSF.ControllerService.Preset' 'PresetsDeleted'. \n
     * @return LSF_OK on success. \n
     */
    LSFResponseCode Reset(void);
    /**
     * Reset to default state. \n
     * Send signal to the controller clients 'org.allseen.LSF.ControllerService.Preset' 'DefaultLampStateChanged'. \n
     * Removing pre-set with id 'DefaultLampState' \n
     * response code LSF_OK on success. \n
     */
    LSFResponseCode ResetDefaultState(void);
    /**
     * Get all pre-set ids. \n
     * Return asynchronous all pre-set ids which are not the default lamp state. \n
     * response code LSF_OK on success. \n
     */
    void GetAllPresetIDs(ajn::Message& msg);
    /**
     * Get pre-set name. \n
     * @param msg type Message with MsgArg unique id. \n
     * Return asynchronously the pre-set name, id, language and response code. \n
     * response code LSF_OK on success. \n
     */
    void GetPresetName(ajn::Message& msg);
    /**
     * Set pre-set name. \n
     * @param msg type Message with MsgArg pre-set unique id. \n
     * Return asynchronously the pre-set new name, id, language and response code. \n
     * Send signal to the controller clients 'org.allseen.LSF.ControllerService.Preset' 'PresetsNameChanged'. \n
     * response code LSF_OK on success. \n
     *      LSF_ERR_INVALID_ARGS - language not supported, name is too long. \n
     *      LSF_ERR_EMPTY_NAME - preset name is empty. \n
     *      LSF_ERR_RESOURCES - blob is too big. \n
     */
    void SetPresetName(ajn::Message& msg);
    /**
     * Create new pre-set. \n
     * @param msg type Message with MsgArgs: LampState, name, language. \n
     * Return asynchronously the pre-set response code and auto generated unique id. \n
     * Send signal to the controller clients 'org.allseen.LSF.ControllerService.Preset' 'PresetsCreated'. \n
     * response code LSF_OK on success. \n
     *      LSF_ERR_INVALID_ARGS - language not supported, name is too long. \n
     *      LSF_ERR_EMPTY_NAME - preset name is empty. \n
     *      LSF_ERR_RESOURCES - blob is too big. \n
     *
     */
    void CreatePreset(ajn::Message& msg);
    /**
     * Update existing pre-set. \n
     * @param msg type Message with MsgArgs: pre-set id. \n
     * Return asynchronously the pre-set response code and unique id. \n
     * Send signal to the controller clients 'org.allseen.LSF.ControllerService.Preset' 'PresetsUpdated'. \n
     * response code LSF_OK on success. \n
     *      LSF_ERR_INVALID_ARGS - language not supported, name is too long. \n
     *      LSF_ERR_RESOURCES - blob is too big. \n
     */
    void UpdatePreset(ajn::Message& msg);
    /**
     * Delete existing pre-set. \n
     * @param msg type Message with MsgArgs: pre-set id. \n
     * Return asynchronously the pre-set response code and unique id. \n
     * Send signal to the controller clients 'org.allseen.LSF.ControllerService.Preset' 'PresetsDeleted'. \n
     * response code LSF_OK on success. \n
     *      LSF_ERR_NOT_FOUND - pre-set with requested id is not found. \n
     */
    void DeletePreset(ajn::Message& msg);
    /**
     * Get existing pre-set. \n
     * @param msg type Message with MsgArgs: pre-set id. \n
     * Return asynchronously the pre-set response code, unique id and requested lamp state \n
     * response code LSF_OK on success. \n
     *      LSF_ERR_NOT_FOUND - pre-set with requested id is not found. \n
     */
    void GetPreset(ajn::Message& msg);
    /**
     * Get existing pre-set. \n
     * @param presetID - The pre-set unique id. \n
     * @param state - The requested lamp state filled synchronously as reference to object. \n
     * @return \n
     * response code LSF_OK on success. \n
     *      LSF_ERR_NOT_FOUND - pre-set with requested id is not found. \n
     */
    LSFResponseCode GetPresetInternal(const LSFString& presetID, LampState& state);
    /**
     * Get default pre-set. \n
     * Return asynchronously the pre-set response code and lamp state which id is 'DefaultLampState'. \n
     * response code LSF_OK on success. \n
     */
    void GetDefaultLampState(ajn::Message& msg);
    /**
     * Set default lamp state. \n
     * Fill the preset with id 'DefaultLampState' new lamp state value. \n
     * Creating it if it is not already exists. In this case the name and the id are the same. \n
     * @param msg type Message with MsgArg: lamp state. \n
     * Send signal to the controller clients 'org.allseen.LSF.ControllerService.Preset' 'DefaultLampStateChanged'. \n
     * Return asynchronously the pre-set response code. \n
     * response code LSF_OK on success. \n
     *      LSF_ERR_RESOURCES - blob is too big. \n
     */
    void SetDefaultLampState(ajn::Message& msg);
    /**
     * Get all presents. \n
     * @param presetMap - the requested presets filled synchronously as reference to LampState. \n
     * response code LSF_OK on success. \n
     */
    LSFResponseCode GetAllPresets(PresetMap& presetMap);
    /**
     * Get default lamp state who's preset name is 'DefaultLampState'. \n
     * @param state - Requested information  of type LampState. Filled synchronously.
     * @return ER_OK on success. \n
     *      LSF_ERR_NOT_FOUND in case it not exist. \n
     */
    LSFResponseCode GetDefaultLampStateInternal(LampState& state);
    /**
     * Read saved persistent data to cache
     */
    void ReadSavedData(void);
    /**
     * Write to cache persistent data
     */
    void ReadWriteFile(void);
    /**
     * Handle received blob. \n
     * Getting the blob string and wrting it to the file. \n
     */
    void HandleReceivedBlob(const std::string& blob, uint32_t checksum, uint64_t timestamp);
    /**
     * Get Controller Service Preset Interface Version. \n
     * @return 32 unsigned integer version. \n
     */
    uint32_t GetControllerServicePresetInterfaceVersion(void);
    /**
     * Get the presets information as a string. \n
     * @return true if data is written to file
     */
    virtual bool GetString(std::string& output, uint32_t& checksum, uint64_t& timestamp);
    /**
     * Get blob information about checksum and time stamp.
     */
    void GetBlobInfo(uint32_t& checksum, uint64_t& timestamp) {
        presetsLock.Lock();
        GetBlobInfoInternal(checksum, timestamp);
        presetsLock.Unlock();
    }

  private:

    void ReplaceMap(std::istringstream& stream);

    LSFResponseCode SetDefaultLampStateInternal(LampState& state);

    PresetMap presets;
    Mutex presetsLock;
    SceneManager* sceneManagerPtr;
    size_t blobLength;

    std::string GetString(const PresetMap& items);
    std::string GetString(const std::string& name, const std::string& id, const LampState& preset);
};

}


#endif
