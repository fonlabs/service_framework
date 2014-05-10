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

  public:
    SceneManager(ControllerService& controllerSvc, LampGroupManager& lampGroupMgr, const char* ifaceName, MasterSceneManager* masterSceneMgr);

    LSFResponseCode Reset(void);
    LSFResponseCode IsDependentOnPreset(LSFString& presetID);
    LSFResponseCode IsDependentOnLampGroup(LSFString& lampGroupID);

    void GetAllSceneIDs(ajn::Message& msg);
    void GetSceneName(ajn::Message& msg);
    void SetSceneName(ajn::Message& msg);
    void DeleteScene(ajn::Message& msg);
    void CreateScene(ajn::Message& msg);
    void UpdateScene(ajn::Message& msg);
    void GetScene(ajn::Message& msg);
    void ApplyScene(ajn::Message& msg);

    void AddScene(const LSFString& id, const std::string& name, const Scene& scene);

  private:

    typedef std::map<LSFString, std::pair<LSFString, Scene> > SceneMap;

    SceneMap scenes;
    Mutex scenesLock;
    LampGroupManager& lampGroupManager;
    const char* interfaceName;
    MasterSceneManager* masterSceneManager;
};

}

#endif
