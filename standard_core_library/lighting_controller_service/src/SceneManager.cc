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

#include <SceneManager.h>
#include <ControllerService.h>
#include <qcc/Debug.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "SCENE_MANAGER"

SceneManager::SceneManager(ControllerService& controllerSvc, LampGroupManager& lampGroupMgr, const char* ifaceName) :
    Manager(controllerSvc), lampGroupManager(lampGroupMgr), interfaceName(ifaceName)
{

}

void SceneManager::GetAllSceneIDs(ajn::Message& msg)
{
#if 0
    LSFResponseCode rc = LSF_OK;
    MsgArg outArgs[2];
    outArgs[1].Set("u", rc);

    scenesLock.Lock();
    size_t arraySize = scenes.size();
    if (arraySize) {
        const char** array = new const char*[arraySize];
        size_t i = 0;
        for (SceneMap::iterator it = scenes.begin(); it != scenes.end(); ++it, ++i) {
            array[i] = it->first.c_str();
        }

        outArgs[0].Set("as", arraySize, array);
        outArgs[0].SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        outArgs[0].Set("as", 0, NULL);
    }
    scenesLock.Unlock();

    controllerService.SendMethodReply(msg, outArgs, 2);
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSF_ID_List idList;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(msg, responseCode, idList);
}

void SceneManager::GetSceneName(ajn::Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSF_Name name;
    LSFResponseCode rc = LSF_OK;
    scenesLock.Lock();
    SceneMap::iterator it = scenes.find(id);
    if (it != scenes.end()) {
        name = it->second.first;
    } else {
        rc = LSF_ERR_INVALID;
    }
    scenesLock.Unlock();

    MsgArg outArgs[2];
    outArgs[0].Set("s", name.c_str());
    outArgs[1].Set("u", rc);
    controllerService.SendMethodReply(msg, outArgs, 2);
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSF_ID id;
    LSF_Name name;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeIDAndName(msg, responseCode, id, name);
}

void SceneManager::SetSceneName(ajn::Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    const char* name;
    args[1].Get("s", &name);

    LSFResponseCode rc = LSF_OK;
    scenesLock.Lock();
    SceneMap::iterator it = scenes.find(id);
    if (it != scenes.end()) {
        it->second.first = LSF_Name(name);
    } else {
        rc = LSF_ERR_INVALID;
    }
    scenesLock.Unlock();

    MsgArg outArg("u", rc);
    controllerService.SendMethodReply(msg, &outArg, 1);
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSF_ID id;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, id);
    LSF_ID_List idList;
    idList.push_back("abc");
    controllerService.SendSignal(interfaceName, "ScenesNameChanged", idList);
}

void SceneManager::DeleteScene(ajn::Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    scenesLock.Lock();
    SceneMap::iterator it = scenes.find(id);
    LSFResponseCode rc = LSF_OK;
    if (it != scenes.end()) {
        scenes.erase(it);
    } else {
        rc = LSF_ERR_INVALID;
    }
    scenesLock.Unlock();

    MsgArg outArg("u", rc);
    controllerService.SendMethodReply(msg, &outArg, 1);
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSF_ID id;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, id);
    LSF_ID_List idList;
    idList.push_back("abc");
    controllerService.SendSignal(interfaceName, "ScenesDeleted", idList);
}

void SceneManager::CreateScene(ajn::Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* oldID;
    args[0].Get("s", &oldID);
    LSF_ID id = oldID;

    bool updated = false;
    bool created = false;

    MsgArg outArgs[2];
    LSFResponseCode rc = LSF_OK;
    // this is an update!
    if (!id.empty()) {
        scenesLock.Lock();
        SceneMap::iterator it = scenes.find(id);
        if (it != scenes.end()) {
            Scene scene;
            it->second.second = scene;;
            created = true;
        } else {
            rc = LSF_ERR_NOT_FOUND;
        }
        scenesLock.Unlock();
    } else {
        //id = GenerateUniqueID("SCENE");
        scenesLock.Lock();
        assert(scenes.find(id) == scenes.end());
        Scene scene;
        scenes[id].second = scene;
        scenesLock.Unlock();
        updated = true;
    }

    outArgs[1].Set("s", id.c_str());
    outArgs[5].Set("u", rc);
    controllerService.SendMethodReply(msg, outArgs, 2);
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSF_ID id;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, id);
    LSF_ID_List idList;
    idList.push_back("abc");
    controllerService.SendSignal(interfaceName, "ScenesCreated", idList);
}

void SceneManager::UpdateScene(ajn::Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* oldID;
    args[0].Get("s", &oldID);
    LSF_ID id = oldID;

    bool updated = false;
    bool created = false;

    MsgArg outArgs[2];
    LSFResponseCode rc = LSF_OK;
    // this is an update!
    if (!id.empty()) {
        scenesLock.Lock();
        SceneMap::iterator it = scenes.find(id);
        if (it != scenes.end()) {
            Scene scene;
            it->second.second = scene;;
            created = true;
        } else {
            rc = LSF_ERR_NOT_FOUND;
        }
        scenesLock.Unlock();
    } else {
        //id = GenerateUniqueID("SCENE");
        scenesLock.Lock();
        assert(scenes.find(id) == scenes.end());
        Scene scene;
        scenes[id].second = scene;
        scenesLock.Unlock();
        updated = true;
    }

    outArgs[1].Set("s", id.c_str());
    outArgs[5].Set("u", rc);
    controllerService.SendMethodReply(msg, outArgs, 2);
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSF_ID id;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, id);
    LSF_ID_List idList;
    idList.push_back("abc");
    controllerService.SendSignal(interfaceName, "ScenesUpdated", idList);
}

void SceneManager::GetScene(ajn::Message& msg)
{
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    MsgArg outArgs[2];
    LSFResponseCode rc = LSF_OK;

    scenesLock.Lock();
    SceneMap::iterator it = scenes.find(id);
    if (it != scenes.end()) {
        it->second.second;
    } else {
        rc = LSF_ERR_INVALID;
        outArgs[0].Set("a(asasa{sv}(ua{sv}))", 0, NULL);
        outArgs[1].Set("a(asass(ua{sv}))", 0, NULL);
    }
    scenesLock.Unlock();

    outArgs[5].Set("u", rc);
    controllerService.SendMethodReply(msg, outArgs, 6);
}


void SceneManager::ApplyScene(ajn::Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* sceneID;
    args[0].Get("s", &sceneID);
    LSFResponseCode rc = LSF_OK;

    scenesLock.Lock();
    SceneMap::iterator sit = scenes.find(sceneID);

    if (sit != scenes.end()) {

        const Scene& scene = sit->second.second;

        // TODO: something with this!
    } else {
        rc = LSF_ERR_INVALID;
    }
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSF_ID id;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, id);
    LSF_ID_List idList;
    idList.push_back("abc");
    controllerService.SendSignal(interfaceName, "ScenesApplied", idList);
}
