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

class SceneManager : public Manager {
    friend class MasterSceneManager;
  public:
    SceneManager(ControllerService& controllerSvc, LampGroupManager& lampGroupMgr, MasterSceneManager* masterSceneMgr, const std::string& sceneFile);

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

    LSFResponseCode GetAllScenes(SceneMap& sceneMap);

    virtual void WriteFile();
    void ReadSavedData();

    uint32_t GetControllerServiceSceneInterfaceVersion(void);

  private:

    LSFResponseCode ApplySceneInternal(ajn::Message message, LSFStringList& sceneList);

    SceneMap scenes;
    Mutex scenesLock;
    LampGroupManager& lampGroupManager;
    MasterSceneManager* masterSceneManager;
};

}

#endif
