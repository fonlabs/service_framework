#ifndef _SCENE_MANAGER_H_
#define _SCENE_MANAGER_H_
/**
 * \ingroup ControllerService
 */
/**
 * @file
 * This file provides definitions for scene manager
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

#include <Manager.h>
#include <LampGroupManager.h>

#include <Mutex.h>
#include <LSFTypes.h>

#include <string>
#include <map>

namespace lsf {

class MasterSceneManager;
class SceneObject;
/**
 * scene management class
 */
class SceneManager : public Manager {
    friend class MasterSceneManager;
    friend class SceneObject;
  public:
    /**
     * SceneManager CTOR
     */
    SceneManager(ControllerService& controllerSvc, LampGroupManager& lampGroupMgr, MasterSceneManager* masterSceneMgr, const std::string& sceneFile);
    /**
     * UnregsiterSceneEventActionObjects
     */
    void UnregsiterSceneEventActionObjects(void);
    /**
     * Clearing all scenesS
     */
    LSFResponseCode Reset(void);
    /**
     * is there scene that depends on present
     */
    LSFResponseCode IsDependentOnPreset(LSFString& presetID);
    /**
     * is there scene that depends on lamp group
     */
    LSFResponseCode IsDependentOnLampGroup(LSFString& lampGroupID);
    /**
     * Get All Scene IDs
     */
    void GetAllSceneIDs(ajn::Message& message);
    /**
     * Get Scene name
     */
    void GetSceneName(ajn::Message& message);
    /**
     * Set Scene name
     */
    void SetSceneName(ajn::Message& message);
    /**
     * Delete Scene
     */
    void DeleteScene(ajn::Message& message);
    /**
     * Create Scene
     */
    void CreateScene(ajn::Message& message);
    /**
     * Update Scene
     */
    void UpdateScene(ajn::Message& message);
    /**
     * Get Scene
     */
    void GetScene(ajn::Message& message);
    /**
     * Apply Scene
     */
    void ApplyScene(ajn::Message& message);
    /**
     * Send Scene Or Master Scene Applied Signal
     */
    void SendSceneOrMasterSceneAppliedSignal(LSFString& sceneorMasterSceneId);
    /**
     * Get All Scenes
     */
    LSFResponseCode GetAllScenes(SceneMap& sceneMap);
    /**
     * Read Write File
     */
    void ReadWriteFile();
    /**
     * Read Saved Data
     */
    void ReadSavedData();
    /**
     * Get Controller Service Scene Interface Version
     */
    uint32_t GetControllerServiceSceneInterfaceVersion(void);
    /**
     * Get string representation of scene objects
     * @param output - string representation of scene objects
     * @param checksum - of the output
     * @param timestamp - current time
     */
    virtual bool GetString(std::string& output, uint32_t& checksum, uint64_t& timestamp);
    /**
     * Get relevant file info
     */
    void GetBlobInfo(uint32_t& checksum, uint64_t& timestamp) {
        scenesLock.Lock();
        GetBlobInfoInternal(checksum, timestamp);
        scenesLock.Unlock();
    }
    /**
     * Reading scenes from string
     */
    void HandleReceivedBlob(const std::string& blob, uint32_t checksum, uint64_t timestamp);

  private:

    void ReplaceMap(std::istringstream& stream);

    LSFResponseCode ApplySceneInternal(ajn::Message message, LSFStringList& sceneList, LSFString sceneOrMasterSceneId);

    typedef std::map<LSFString, SceneObject*> SceneObjectMap;

    SceneObjectMap scenes;
    Mutex scenesLock;
    LampGroupManager& lampGroupManager;
    MasterSceneManager* masterSceneManager;
    size_t blobLength;

    std::string GetString(const SceneObjectMap& items);
    std::string GetString(const std::string& name, const std::string& id, const Scene& scene);
};

/**
 * SceneObject class - the bus object implementation
 */
class SceneObject : public BusObject, public Translator {
  public:
    /**
     * SceneObject CTOR
     * @param sceneMgr - SceneManager object reference
     * @param sceneid - the scene id related to the class object
     * @param tempScene
     * @param name  The scene name
     */
    SceneObject(SceneManager& sceneMgr, LSFString& sceneid, Scene& tempScene, LSFString& name);
    /**
     * SceneObject DTOR
     */
    ~SceneObject();

    /**
     * apply scene implementation
     */
    void ApplySceneHandler(const InterfaceDescription::Member* member, Message& msg);
    /**
     * Send scene applied signal
     */
    void SendSceneAppliedSignal(void);
    /**
     * Number of target languages
     * @return number
     */
    size_t NumTargetLanguages() {
        return 1;
    }
    /**
     * Get target language
     * @param index
     * @param ret - language returned
     */
    void GetTargetLanguage(size_t index, qcc::String& ret) {
        ret.assign("en");
    }
    /**
     * Translator class override member
     */
    const char* Translate(const char* sourceLanguage, const char* targetLanguage, const char* source, qcc::String& buffer);
    /**
     * BusObject override member
     */
    void ObjectRegistered(void);

    SceneManager& sceneManager; /**< Scene manager reference count */
    LSFString sceneId;          /**< Scene id */
    Scene scene;                /**< Scene object */
    Mutex sceneNameMutex;       /**< Scene name mutex */
    LSFString sceneName;        /**< Scene name */
    const InterfaceDescription::Member* appliedSceneMember;  /**< applied scene signal */
};

}

#endif
