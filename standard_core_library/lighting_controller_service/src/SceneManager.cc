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
#include <qcc/atomic.h>
#include <qcc/Debug.h>
#include <MasterSceneManager.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "SCENE_MANAGER"

SceneManager::SceneManager(ControllerService& controllerSvc, LampGroupManager& lampGroupMgr, const char* ifaceName, MasterSceneManager* masterSceneMgr) :
    Manager(controllerSvc), lampGroupManager(lampGroupMgr), interfaceName(ifaceName), masterSceneManager(masterSceneMgr)
{
}

LSFResponseCode SceneManager::Reset(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    LSFResponseCode responseCode = LSF_OK;
    QStatus tempStatus = scenesLock.Lock();
    if (ER_OK == tempStatus) {
        /*
         * Record the IDs of all the Scenes that are being deleted
         */
        LSFStringList scenesList;
        for (SceneMap::iterator it = scenes.begin(); it != scenes.end(); ++it) {
            scenesList.push_back(it->first);
        }

        /*
         * Clear the Scenes
         */
        scenes.clear();
        tempStatus = scenesLock.Unlock();
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: scenesLock.Unlock() failed", __FUNCTION__));
        }

        /*
         * Send the Scenes deleted signal
         */
        if (scenesList.size()) {
            tempStatus = controllerService.SendSignal(interfaceName, "ScenesDeleted", scenesList);
            if (ER_OK != tempStatus) {
                QCC_LogError(tempStatus, ("%s: Unable to send ScenesDeleted signal", __FUNCTION__));
            }
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(tempStatus, ("%s: scenesLock.Lock() failed", __FUNCTION__));
    }

    return responseCode;
}

LSFResponseCode SceneManager::IsDependentOnPreset(LSFString& presetID)
{
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = scenesLock.Lock();
    if (ER_OK == status) {
        for (SceneMap::iterator it = scenes.begin(); it != scenes.end(); ++it) {
            responseCode = it->second.second.IsDependentOnPreset(presetID);
            if (LSF_OK != responseCode) {
                break;
            }
        }
        status = scenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: scenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: scenesLock.Lock() failed", __FUNCTION__));
    }

    return responseCode;
}

LSFResponseCode SceneManager::IsDependentOnLampGroup(LSFString& lampGroupID)
{
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = scenesLock.Lock();
    if (ER_OK == status) {
        for (SceneMap::iterator it = scenes.begin(); it != scenes.end(); ++it) {
            responseCode = it->second.second.IsDependentOnLampGroup(lampGroupID);
            if (LSF_OK != responseCode) {
                break;
            }
        }
        status = scenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: scenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: scenesLock.Lock() failed", __FUNCTION__));
    }

    return responseCode;
}

void SceneManager::GetAllSceneIDs(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFStringList idList;
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = scenesLock.Lock();
    if (ER_OK == status) {
        for (SceneMap::iterator it = scenes.begin(); it != scenes.end(); ++it) {
            idList.push_back(it->first.c_str());
        }
        status = scenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: scenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: scenesLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(msg, responseCode, idList);
}

void SceneManager::GetSceneName(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFString name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSFString sceneID(id);

    LSFString language = static_cast<LSFString>(args[1].v_string.str);
    if (0 != strcmp("en", language.c_str())) {
        QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __FUNCTION__, language.c_str()));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else {
        QStatus status = scenesLock.Lock();
        if (ER_OK == status) {
            SceneMap::iterator it = scenes.find(id);
            if (it != scenes.end()) {
                name = it->second.first;
                responseCode = LSF_OK;
            }
            status = scenesLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: scenesLock.Unlock() failed", __FUNCTION__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: scenesLock.Lock() failed", __FUNCTION__));
        }
    }

    controllerService.SendMethodReplyWithResponseCodeIDLanguageAndName(msg, responseCode, sceneID, language, name);
}

void SceneManager::SetSceneName(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool nameChanged = false;
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSFString sceneID(id);

    const char* name;
    args[1].Get("s", &name);

    LSFString language = static_cast<LSFString>(args[2].v_string.str);
    if (0 != strcmp("en", language.c_str())) {
        QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __FUNCTION__, language.c_str()));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else {
        if (strlen(name) > LSF_MAX_NAME_LENGTH) {
            responseCode = LSF_ERR_INVALID_ARGS;
            QCC_LogError(ER_FAIL, ("%s: strlen(name) > LSF_MAX_NAME_LENGTH", __FUNCTION__));
        } else {
            QStatus status = scenesLock.Lock();
            if (ER_OK == status) {
                SceneMap::iterator it = scenes.find(id);
                if (it != scenes.end()) {
                    it->second.first = LSFString(name);
                    responseCode = LSF_OK;
                    nameChanged = true;
                }
                status = scenesLock.Unlock();
                if (ER_OK != status) {
                    QCC_LogError(status, ("%s: scenesLock.Unlock() failed", __FUNCTION__));
                }
            } else {
                responseCode = LSF_ERR_BUSY;
                QCC_LogError(status, ("%s: scenesLock.Lock() failed", __FUNCTION__));
            }
        }
    }

    controllerService.SendMethodReplyWithResponseCodeIDAndName(msg, responseCode, sceneID, language);

    if (nameChanged) {
        LSFStringList idList;
        idList.push_back(sceneID);
        controllerService.SendSignal(interfaceName, "ScenesNameChanged", idList);
    }
}

void SceneManager::CreateScene(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_OK;

    bool created = false;
    LSFString sceneID;

    const ajn::MsgArg* inputArgs;
    size_t numInputArgs;
    msg->GetArgs(numInputArgs, inputArgs);

    QStatus status = scenesLock.Lock();
    if (ER_OK == status) {
        if (scenes.size() < MAX_SUPPORTED_NUM_LSF_ENTITY) {
            sceneID = GenerateUniqueID("SCENE");
            Scene scene(inputArgs[0], inputArgs[1], inputArgs[2], inputArgs[3], inputArgs[4], inputArgs[5], inputArgs[6], inputArgs[7]);
            scenes[sceneID].first = sceneID;
            scenes[sceneID].second = scene;
            created = true;
        } else {
            QCC_LogError(ER_FAIL, ("%s: No slot for new Scene", __FUNCTION__));
            responseCode = LSF_ERR_NO_SLOT;
        }
        status = scenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: scenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: scenesLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, sceneID);

    if (created) {
        LSFStringList idList;
        idList.push_back(sceneID);
        controllerService.SendSignal(interfaceName, "ScenesCreated", idList);
    }
}

void SceneManager::UpdateScene(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool updated = false;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSFString sceneID(id);
    Scene scene(args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8]);

    QStatus status = scenesLock.Lock();
    if (ER_OK == status) {
        SceneMap::iterator it = scenes.find(id);
        if (it != scenes.end()) {
            scenes[sceneID].second = scene;
            responseCode = LSF_OK;
            updated = true;
        }
        status = scenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: scenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: scenesLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, sceneID);

    if (updated) {
        LSFStringList idList;
        idList.push_back(sceneID);
        controllerService.SendSignal(interfaceName, "ScenesUpdated", idList);
    }
}

void SceneManager::DeleteScene(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_OK;

    bool deleted = false;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* sceneId;
    args[0].Get("s", &sceneId);

    LSFString sceneID(sceneId);

    responseCode = masterSceneManager->IsDependentOnScene(sceneID);

    if (LSF_OK == responseCode) {
        QStatus status = scenesLock.Lock();
        if (ER_OK == status) {
            SceneMap::iterator it = scenes.find(sceneId);
            if (it != scenes.end()) {
                scenes.erase(it);
                deleted = true;
            } else {
                responseCode = LSF_ERR_NOT_FOUND;
            }
            status = scenesLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: scenesLock.Unlock() failed", __FUNCTION__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: scenesLock.Lock() failed", __FUNCTION__));
        }
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, sceneID);

    if (deleted) {
        LSFStringList idList;
        idList.push_back(sceneID);
        controllerService.SendSignal(interfaceName, "ScenesDeleted", idList);
    }
}

void SceneManager::GetScene(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    MsgArg outArgs[10];

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    QStatus status = scenesLock.Lock();
    if (ER_OK == status) {
        SceneMap::iterator it = scenes.find(id);
        if (it != scenes.end()) {
            it->second.second.Get(&outArgs[2], &outArgs[3], &outArgs[4], &outArgs[5], &outArgs[6], &outArgs[7], &outArgs[8], &outArgs[9]);
            responseCode = LSF_OK;
        } else {
            outArgs[2].Set("a(asasa{sv}u)", 0, NULL);
            outArgs[3].Set("a(asassu)", 0, NULL);
            outArgs[4].Set("a(asasa{sv}a{sv}uuu)", 0, NULL);
            outArgs[5].Set("a(asasssuuu)", 0, NULL);
            outArgs[6].Set("a(asasa{sv}a{sv}uu)", 0, NULL);
            outArgs[7].Set("a(asasssuu)", 0, NULL);
            outArgs[8].Set("a(asasa{sv}a{sv}uuu)", 0, NULL);
            outArgs[9].Set("a(asasssuuu)", 0, NULL);
        }
        status = scenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: scenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: scenesLock.Lock() failed", __FUNCTION__));
    }

    outArgs[0].Set("u", responseCode);
    outArgs[1].Set("s", id);

    controllerService.SendMethodReply(msg, outArgs, 10);
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

    scenesLock.Unlock();
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFString id;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, id);
    LSFStringList idList;
    idList.push_back("abc");
    controllerService.SendSignal(interfaceName, "ScenesApplied", idList);
}

void SceneManager::AddScene(const LSFString& id, const std::string& name, const Scene& scene)
{
    std::pair<LSFString, Scene> thePair(name, scene);
    scenes[id] = thePair;
}
