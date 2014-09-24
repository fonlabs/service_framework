#ifndef _MASTER_SCENE_MANAGER_H_
#define _MASTER_SCENE_MANAGER_H_
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
#include <SceneManager.h>

#include <Mutex.h>
#include <LSFTypes.h>

#include <string>
#include <map>

namespace lsf {
/**
 * Master Scene Manager
 */
class MasterSceneManager : public Manager {

  public:
    /**
     * MasterSceneManager constructor
     */
    MasterSceneManager(ControllerService& controllerSvc, SceneManager& sceneMgr, const std::string& masterSceneFile);
    /**
     * Reset object
     */
    LSFResponseCode Reset(void);
    /**
     * Is Dependent On Scene
     */
    LSFResponseCode IsDependentOnScene(LSFString& sceneID);
    /**
     * Get All Master Scene IDs
     */
    void GetAllMasterSceneIDs(ajn::Message& message);
    /**
     * Get Master Scene Name
     */
    void GetMasterSceneName(ajn::Message& message);
    /**
     * Set Master Scene Name
     */
    void SetMasterSceneName(ajn::Message& message);
    /**
     * Delete Master Scene
     */
    void DeleteMasterScene(ajn::Message& message);
    /**
     * Create Master Scene
     */
    void CreateMasterScene(ajn::Message& message);
    /**
     * Update Master Scene
     */
    void UpdateMasterScene(ajn::Message& message);
    /**
     * Get Master Scene
     */
    void GetMasterScene(ajn::Message& message);
    /**
     * Apply Master Scene
     */
    void ApplyMasterScene(ajn::Message& message);
    /**
     * Send Master Scene Applied Signal
     */
    void SendMasterSceneAppliedSignal(LSFString& sceneorMasterSceneId);
    /**
     * Get All Master Scenes
     */
    LSFResponseCode GetAllMasterScenes(MasterSceneMap& masterSceneMap);
    /**
     * Read Saved Data
     */
    void ReadSavedData();
    /**
     * Read Write File
     */
    void ReadWriteFile();
    /**
     * Get Controller Service Master Scene Interface Version
     */
    uint32_t GetControllerServiceMasterSceneInterfaceVersion(void);
    /**
     * Get String
     */
    virtual bool GetString(std::string& output, uint32_t& checksum, uint64_t& timestamp);
    /**
     * Get Blob Info
     */
    void GetBlobInfo(uint32_t& checksum, uint64_t& timestamp) {
        masterScenesLock.Lock();
        GetBlobInfoInternal(checksum, timestamp);
        masterScenesLock.Unlock();
    }
    /**
     * Handle Received Blob
     */
    void HandleReceivedBlob(const std::string& blob, uint32_t checksum, uint64_t timestamp);

  private:

    void ReplaceMap(std::istringstream& stream);

    MasterSceneMap masterScenes;
    Mutex masterScenesLock;
    SceneManager& sceneManager;
    size_t blobLength;

    std::string GetString(const MasterSceneMap& items);
    std::string GetString(const std::string& name, const std::string& id, const MasterScene& msc);
};

}

#endif
