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

#include <MasterSceneManager.h>
#include <ControllerService.h>
#include <qcc/Debug.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "MASTER_SCENE_MANAGER"

MasterSceneManager::MasterSceneManager(ControllerService& controllerSvc, SceneManager& sceneMgr, const char* ifaceName) :
    Manager(controllerSvc), sceneManager(sceneMgr), interfaceName(ifaceName)
{

}

LSFResponseCode MasterSceneManager::Reset(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    LSFResponseCode responseCode = LSF_OK;
    QStatus tempStatus = masterScenesLock.Lock();
    if (ER_OK == tempStatus) {
        /*
         * Record the IDs of all the MasterScenes that are being deleted
         */
        LSFStringList masterScenesList;
        for (MasterSceneMap::iterator it = masterScenes.begin(); it != masterScenes.end(); ++it) {
            masterScenesList.push_back(it->first);
        }

        /*
         * Clear the MasterScenes
         */
        masterScenes.clear();
        tempStatus = masterScenesLock.Unlock();
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: masterScenesLock.Unlock() failed", __FUNCTION__));
        }

        /*
         * Send the MasterScenes deleted signal
         */
        if (masterScenesList.size()) {
            tempStatus = controllerService.SendSignal(interfaceName, "MasterScenesDeleted", masterScenesList);
            if (ER_OK != tempStatus) {
                QCC_LogError(tempStatus, ("%s: Unable to send MasterScenesDeleted signal", __FUNCTION__));
            }
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(tempStatus, ("%s: masterScenesLock.Lock() failed", __FUNCTION__));
    }

    return responseCode;
}

LSFResponseCode MasterSceneManager::IsDependentOnScene(LSFString& sceneID)
{
    return LSF_OK;
}

void MasterSceneManager::GetAllMasterSceneIDs(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFStringList idList;
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        for (MasterSceneMap::iterator it = masterScenes.begin(); it != masterScenes.end(); ++it) {
            idList.push_back(it->first.c_str());
        }
        status = masterScenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(msg, responseCode, idList);
}

void MasterSceneManager::GetMasterSceneName(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFString name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSFString masterSceneID(id);

    LSFString language = static_cast<LSFString>(args[1].v_string.str);
    if (0 != strcmp("en", language.c_str())) {
        QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __FUNCTION__, language.c_str()));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else {
        QStatus status = masterScenesLock.Lock();
        if (ER_OK == status) {
            MasterSceneMap::iterator it = masterScenes.find(id);
            if (it != masterScenes.end()) {
                name = it->second.first;
                responseCode = LSF_OK;
            }
            status = masterScenesLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __FUNCTION__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __FUNCTION__));
        }
    }

    controllerService.SendMethodReplyWithResponseCodeIDLanguageAndName(msg, responseCode, masterSceneID, language, name);
}

void MasterSceneManager::SetMasterSceneName(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool nameChanged = false;
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSFString masterSceneID(id);

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
            QStatus status = masterScenesLock.Lock();
            if (ER_OK == status) {
                MasterSceneMap::iterator it = masterScenes.find(id);
                if (it != masterScenes.end()) {
                    it->second.first = LSFString(name);
                    responseCode = LSF_OK;
                    nameChanged = true;
                }
                status = masterScenesLock.Unlock();
                if (ER_OK != status) {
                    QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __FUNCTION__));
                }
            } else {
                responseCode = LSF_ERR_BUSY;
                QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __FUNCTION__));
            }
        }
    }

    controllerService.SendMethodReplyWithResponseCodeIDAndName(msg, responseCode, masterSceneID, language);

    if (nameChanged) {
        LSFStringList idList;
        idList.push_back(masterSceneID);
        controllerService.SendSignal(interfaceName, "MasterScenesNameChanged", idList);
    }
}

void MasterSceneManager::CreateMasterScene(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_OK;

    bool created = false;
    LSFString masterSceneID;

    const ajn::MsgArg* inputArgs;
    size_t numInputArgs;
    msg->GetArgs(numInputArgs, inputArgs);

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        if (masterScenes.size() < MAX_SUPPORTED_NUM_LSF_ENTITY) {
            masterSceneID = GenerateUniqueID("MASTER_SCENE");
            MasterScene masterScene(inputArgs[0]);
            masterScenes[masterSceneID].first = masterSceneID;
            masterScenes[masterSceneID].second = masterScene;
            created = true;
        } else {
            QCC_LogError(ER_FAIL, ("%s: No slot for new MasterScene", __FUNCTION__));
            responseCode = LSF_ERR_NO_SLOT;
        }
        status = masterScenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, masterSceneID);

    if (created) {
        LSFStringList idList;
        idList.push_back(masterSceneID);
        controllerService.SendSignal(interfaceName, "MasterScenesCreated", idList);
    }
}

void MasterSceneManager::UpdateMasterScene(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool updated = false;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSFString masterSceneID(id);
    MasterScene masterScene(args[1]);

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        MasterSceneMap::iterator it = masterScenes.find(id);
        if (it != masterScenes.end()) {
            masterScenes[masterSceneID].second = masterScene;
            responseCode = LSF_OK;
            updated = true;
        }
        status = masterScenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, masterSceneID);

    if (updated) {
        LSFStringList idList;
        idList.push_back(masterSceneID);
        controllerService.SendSignal(interfaceName, "MasterScenesUpdated", idList);
    }
}

void MasterSceneManager::DeleteMasterScene(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFString name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool deleted = false;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSFString masterSceneID(id);

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        MasterSceneMap::iterator it = masterScenes.find(id);
        if (it != masterScenes.end()) {
            masterScenes.erase(it);
            responseCode = LSF_OK;
            deleted = true;
        }
        status = masterScenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, masterSceneID);

    if (deleted) {
        LSFStringList idList;
        idList.push_back(masterSceneID);
        controllerService.SendSignal(interfaceName, "MasterScenesDeleted", idList);
    }
}

void MasterSceneManager::GetMasterScene(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    MsgArg outArgs[3];

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        MasterSceneMap::iterator it = masterScenes.find(id);
        if (it != masterScenes.end()) {
            it->second.second.Get(&outArgs[2]);
            responseCode = LSF_OK;
        } else {
            outArgs[2].Set("as", 0, NULL);
        }
        status = masterScenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __FUNCTION__));
    }

    outArgs[0].Set("u", responseCode);
    outArgs[1].Set("s", id);

    controllerService.SendMethodReply(msg, outArgs, 3);
}

void MasterSceneManager::ApplyMasterScene(ajn::Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* masterSceneID;
    args[0].Get("s", &masterSceneID);
    LSFResponseCode rc = LSF_OK;

    masterScenesLock.Lock();
    MasterSceneMap::iterator sit = masterScenes.find(masterSceneID);

    if (sit != masterScenes.end()) {

        const MasterScene& masterScene = sit->second.second;

        // TODO: something with this!
    } else {
        rc = LSF_ERR_INVALID;
    }

    masterScenesLock.Unlock();
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFString id;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, id);
    LSFStringList idList;
    idList.push_back("abc");
    controllerService.SendSignal(interfaceName, "MasterScenesApplied", idList);
}

void MasterSceneManager::AddMasterScene(const LSFString& id, const std::string& name, const MasterScene& group)
{
    std::pair<LSFString, MasterScene> thePair(name, group);
    masterScenes[id] = thePair;
}
