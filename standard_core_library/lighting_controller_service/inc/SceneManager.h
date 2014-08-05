#ifndef _SCENE_MANAGER_H_
#define _SCENE_MANAGER_H_
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

class SceneManager : public Manager {
    friend class MasterSceneManager;
    friend class SceneObject;
  public:
    SceneManager(ControllerService& controllerSvc, LampGroupManager& lampGroupMgr, MasterSceneManager* masterSceneMgr, const std::string& sceneFile);

    void UnregsiterSceneEventActionObjects(void);

    LSFResponseCode Reset(void);
    LSFResponseCode IsDependentOnPreset(LSFString& presetID);
    LSFResponseCode IsDependentOnLampGroup(LSFString& lampGroupID);

    void GetAllSceneIDs(ajn::Message& message);
    void GetSceneName(ajn::Message& message);
    void SetSceneName(ajn::Message& message);
    void DeleteScene(ajn::Message& message);
    void CreateScene(ajn::Message& message);
    void UpdateScene(ajn::Message& message);
    void GetScene(ajn::Message& message);
    void ApplyScene(ajn::Message& message);

    void SendSceneOrMasterSceneAppliedSignal(LSFString& sceneorMasterSceneId);

    LSFResponseCode GetAllScenes(SceneMap& sceneMap);

    void ReadWriteFile();
    void ReadSavedData();

    uint32_t GetControllerServiceSceneInterfaceVersion(void);

    virtual bool GetString(std::string& output, uint32_t& checksum, uint64_t& timestamp);

    void GetBlobInfo(uint32_t& checksum, uint64_t& timestamp) {
        scenesLock.Lock();
        GetBlobInfoInternal(checksum, timestamp);
        scenesLock.Unlock();
    }

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

class SceneObject : public BusObject, public Translator {
  public:
    SceneObject(SceneManager& sceneMgr, LSFString& sceneid, Scene& tempScene, LSFString& name);
    ~SceneObject();

    void ApplySceneHandler(const InterfaceDescription::Member* member, Message& msg);

    void SendSceneAppliedSignal(void);

    size_t NumTargetLanguages() {
        return 1;
    }

    void GetTargetLanguage(size_t index, qcc::String& ret) {
        ret.assign("en");
    }

    const char* Translate(const char* sourceLanguage, const char* targetLanguage, const char* source, qcc::String& buffer);

    void ObjectRegistered(void);

    SceneManager& sceneManager;
    LSFString sceneId;
    Scene scene;
    Mutex sceneNameMutex;
    LSFString sceneName;
    const InterfaceDescription::Member* appliedSceneMember;
};

}

#endif
