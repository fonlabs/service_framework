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

#include <Manager.h>

#include <Mutex.h>
#include <LSFTypes.h>

#include <string>
#include <map>

namespace lsf {

class SceneManager;
/**
 * class manages the present of the lamps
 */
class PresetManager : public Manager {
    friend class LampManager;
  public:
    /**
     * class constructor
     */
    PresetManager(ControllerService& controllerSvc, SceneManager* sceneMgrPtr, const std::string& presetFile);
    /**
     * reset the object
     */
    LSFResponseCode Reset(void);
    /**
     * Reset to default state
     */
    LSFResponseCode ResetDefaultState(void);
    /**
     * Get all present ids
     */
    void GetAllPresetIDs(ajn::Message& msg);
    /**
     * Get present name
     */
    void GetPresetName(ajn::Message& msg);
    /**
     * Set present name
     */
    void SetPresetName(ajn::Message& msg);
    /**
     * Create present
     */
    void CreatePreset(ajn::Message& msg);
    /**
     * Update present
     */
    void UpdatePreset(ajn::Message& msg);
    /**
     * Delete present
     */
    void DeletePreset(ajn::Message& msg);
    /**
     * Get present
     */
    void GetPreset(ajn::Message& msg);
    /**
     * Get present internal
     */
    LSFResponseCode GetPresetInternal(const LSFString& presetID, LampState& state);
    /**
     * Get default lamp state
     */
    void GetDefaultLampState(ajn::Message& msg);
    /**
     * Set default lamp state
     */
    void SetDefaultLampState(ajn::Message& msg);
    /**
     * Get all presents
     */
    LSFResponseCode GetAllPresets(PresetMap& presetMap);
    /**
     * Get default lamp state internal
     */
    LSFResponseCode GetDefaultLampStateInternal(LampState& state);
    /**
     * Read saved data
     */
    void ReadSavedData(void);
    /**
     * Read write file
     */
    void ReadWriteFile(void);
    /**
     * Hansled received blob
     */
    void HandleReceivedBlob(const std::string& blob, uint32_t checksum, uint64_t timestamp);
    /**
     * Get Controller Service Preset Interface Version
     */
    uint32_t GetControllerServicePresetInterfaceVersion(void);
    /**
     * Get string
     */
    virtual bool GetString(std::string& output, uint32_t& checksum, uint64_t& timestamp);
    /**
     * Get blob info
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
