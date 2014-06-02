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

class MasterSceneManager : public Manager {

  public:
    MasterSceneManager(ControllerService& controllerSvc, SceneManager& sceneMgr, const char* ifaceName, const std::string& masterSceneFile);

    LSFResponseCode Reset(void);
    LSFResponseCode IsDependentOnScene(LSFString& sceneID);

    void GetAllMasterSceneIDs(ajn::Message& msg);
    void GetMasterSceneName(ajn::Message& msg);
    void SetMasterSceneName(ajn::Message& msg);
    void DeleteMasterScene(ajn::Message& msg);
    void CreateMasterScene(ajn::Message& msg);
    void UpdateMasterScene(ajn::Message& msg);
    void GetMasterScene(ajn::Message& msg);
    void ApplyMasterScene(ajn::Message& msg);

    LSFResponseCode GetAllMasterScenes(MasterSceneMap& masterSceneMap);

    void ReadSavedData();
    virtual void WriteFile();

    uint32_t GetControllerMasterSceneInterfaceVersion(void);

  private:

    MasterSceneMap masterScenes;
    Mutex masterScenesLock;
    SceneManager& sceneManager;
    const char* interfaceName;
};

}

#endif
