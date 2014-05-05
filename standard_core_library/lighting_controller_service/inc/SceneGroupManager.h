#ifndef _SCENE_GROUP_MANAGER_H_
#define _SCENE_GROUP_MANAGER_H_
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

class SceneGroupManager : public Manager {

  public:
    SceneGroupManager(ControllerService& controllerSvc, SceneManager& sceneMgr, const char* ifaceName);

    void GetAllSceneGroupIDs(ajn::Message& msg);
    void GetSceneGroupName(ajn::Message& msg);
    void SetSceneGroupName(ajn::Message& msg);
    void DeleteSceneGroup(ajn::Message& msg);
    void CreateSceneGroup(ajn::Message& msg);
    void UpdateSceneGroup(ajn::Message& msg);
    void GetSceneGroup(ajn::Message& msg);
    void ApplySceneGroup(ajn::Message& msg);

  private:

    typedef std::map<LSF_ID, std::pair<LSF_Name, SceneGroup> > SceneGroupMap;

    SceneGroupMap sceneGroups;
    Mutex sceneGroupsLock;
    SceneManager& sceneManager;
    const char* interfaceName;
};

}

#endif
