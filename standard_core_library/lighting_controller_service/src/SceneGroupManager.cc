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

#include <SceneGroupManager.h>
#include <ControllerService.h>
#include <qcc/Debug.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "SCENE_GROUP_MANAGER"

SceneGroupManager::SceneGroupManager(ControllerService& controllerSvc, SceneManager& sceneMgr, const char* ifaceName) :
    Manager(controllerSvc), sceneManager(sceneMgr), interfaceName(ifaceName)
{

}

void SceneGroupManager::GetAllSceneGroupIDs(ajn::Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSF_ID_List idList;
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = sceneGroupsLock.Lock();
    if (ER_OK == status) {
        for (SceneGroupMap::iterator it = sceneGroups.begin(); it != sceneGroups.end(); ++it) {
            idList.push_back(it->first.c_str());
        }
        status = sceneGroupsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: sceneGroupsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: sceneGroupsLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(msg, responseCode, idList);
}

void SceneGroupManager::GetSceneGroupName(ajn::Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSF_Name name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSF_ID sceneGroupID(id);

    QStatus status = sceneGroupsLock.Lock();
    if (ER_OK == status) {
        SceneGroupMap::iterator it = sceneGroups.find(id);
        if (it != sceneGroups.end()) {
            name = it->second.first;
            responseCode = LSF_OK;
        }
        status = sceneGroupsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: sceneGroupsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: sceneGroupsLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeIDAndName(msg, responseCode, sceneGroupID, name);
}

void SceneGroupManager::SetSceneGroupName(ajn::Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSF_ID sceneGroupID(id);

    const char* name;
    args[1].Get("s", &name);

    QStatus status = sceneGroupsLock.Lock();
    if (ER_OK == status) {
        SceneGroupMap::iterator it = sceneGroups.find(id);
        if (it != sceneGroups.end()) {
            it->second.first = LSF_Name(name);
            responseCode = LSF_OK;

            LSF_ID_List idList;
            idList.push_back(sceneGroupID);
            controllerService.SendSignal(interfaceName, "SceneGroupsNameChanged", idList);
        }
        status = sceneGroupsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: sceneGroupsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: sceneGroupsLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, sceneGroupID);
}

void SceneGroupManager::DeleteSceneGroup(ajn::Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSF_Name name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSF_ID sceneGroupID(id);

    QStatus status = sceneGroupsLock.Lock();
    if (ER_OK == status) {
        SceneGroupMap::iterator it = sceneGroups.find(id);
        if (it != sceneGroups.end()) {
            sceneGroups.erase(it);
            responseCode = LSF_OK;

            LSF_ID_List idList;
            idList.push_back(sceneGroupID);
            controllerService.SendSignal(interfaceName, "SceneGroupsDeleted", idList);
        }
        status = sceneGroupsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: sceneGroupsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: sceneGroupsLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, sceneGroupID);
}

void SceneGroupManager::CreateSceneGroup(ajn::Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_OK;

    const ajn::MsgArg* inputArgs;
    size_t numInputArgs;
    msg->GetArgs(numInputArgs, inputArgs);

    SceneGroup sceneGroup(inputArgs[0]);
    LSF_ID sceneGroupID = GenerateUniqueID("SCENE_GROUP");

    QStatus status = sceneGroupsLock.Lock();
    if (ER_OK == status) {
        sceneGroups[sceneGroupID].first = sceneGroupID;
        sceneGroups[sceneGroupID].second = sceneGroup;

        LSF_ID_List idList;
        idList.push_back(sceneGroupID);
        controllerService.SendSignal(interfaceName, "SceneGroupsCreated", idList);

        status = sceneGroupsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: sceneGroupsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: sceneGroupsLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, sceneGroupID);
}

void SceneGroupManager::UpdateSceneGroup(ajn::Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSF_ID sceneGroupID(id);
    SceneGroup sceneGroup(args[1]);

    QStatus status = sceneGroupsLock.Lock();
    if (ER_OK == status) {
        SceneGroupMap::iterator it = sceneGroups.find(id);
        if (it != sceneGroups.end()) {
            sceneGroups[sceneGroupID].second = sceneGroup;
            responseCode = LSF_OK;

            LSF_ID_List idList;
            idList.push_back(sceneGroupID);
            controllerService.SendSignal(interfaceName, "SceneGroupsUpdated", idList);
        }
        status = sceneGroupsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: sceneGroupsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: sceneGroupsLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, sceneGroupID);
}

void SceneGroupManager::GetSceneGroup(ajn::Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    MsgArg outArgs[3];

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    QStatus status = sceneGroupsLock.Lock();
    if (ER_OK == status) {
        SceneGroupMap::iterator it = sceneGroups.find(id);
        if (it != sceneGroups.end()) {
            it->second.second.Get(&outArgs[2]);
            responseCode = LSF_OK;
        } else {
            outArgs[2].Set("as", 0, NULL);
        }
        status = sceneGroupsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: sceneGroupsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: sceneGroupsLock.Lock() failed", __FUNCTION__));
    }

    outArgs[0].Set("u", responseCode);
    outArgs[1].Set("s", id);

    controllerService.SendMethodReply(msg, outArgs, 3);
}


void SceneGroupManager::ApplySceneGroup(ajn::Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* sceneGroupID;
    args[0].Get("s", &sceneGroupID);
    LSFResponseCode rc = LSF_OK;

    sceneGroupsLock.Lock();
    SceneGroupMap::iterator sit = sceneGroups.find(sceneGroupID);

    if (sit != sceneGroups.end()) {

        const SceneGroup& sceneGroup = sit->second.second;

        // TODO: something with this!
    } else {
        rc = LSF_ERR_INVALID;
    }

    sceneGroupsLock.Unlock();
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSF_ID id;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, id);
    LSF_ID_List idList;
    idList.push_back("abc");
    controllerService.SendSignal(interfaceName, "SceneGroupsApplied", idList);
}
