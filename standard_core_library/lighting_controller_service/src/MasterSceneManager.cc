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
#include <OEM_CS_Config.h>
#include <FileParser.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "MASTER_SCENE_MANAGER"

MasterSceneManager::MasterSceneManager(ControllerService& controllerSvc, SceneManager& sceneMgr, const std::string& masterSceneFile) :
    Manager(controllerSvc, masterSceneFile), sceneManager(sceneMgr)
{

}

LSFResponseCode MasterSceneManager::GetAllMasterScenes(MasterSceneMap& masterSceneMap)
{
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        masterSceneMap = masterScenes;
        status = masterScenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __FUNCTION__));
    }

    return responseCode;
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
            tempStatus = controllerService.SendSignal(ControllerServiceMasterSceneInterfaceName, "MasterScenesDeleted", masterScenesList);
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
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        for (MasterSceneMap::iterator it = masterScenes.begin(); it != masterScenes.end(); ++it) {
            responseCode = it->second.second.IsDependentOnScene(sceneID);
            if (LSF_OK != responseCode) {
                break;
            }
        }
        status = masterScenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __FUNCTION__));
    }

    return responseCode;
}

void MasterSceneManager::GetAllMasterSceneIDs(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));

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

    controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(message, responseCode, idList);
}

void MasterSceneManager::GetMasterSceneName(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    LSFString name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    LSFString masterSceneID(uniqueId);

    LSFString language = static_cast<LSFString>(args[1].v_string.str);
    if (0 != strcmp("en", language.c_str())) {
        QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __FUNCTION__, language.c_str()));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else {
        QStatus status = masterScenesLock.Lock();
        if (ER_OK == status) {
            MasterSceneMap::iterator it = masterScenes.find(uniqueId);
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

    controllerService.SendMethodReplyWithResponseCodeIDLanguageAndName(message, responseCode, masterSceneID, language, name);
}

void MasterSceneManager::SetMasterSceneName(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool nameChanged = false;
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    LSFString masterSceneID(uniqueId);

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
                MasterSceneMap::iterator it = masterScenes.find(uniqueId);
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

    controllerService.SendMethodReplyWithResponseCodeIDAndName(message, responseCode, masterSceneID, language);

    if (nameChanged) {
        ScheduleFileUpdate();
        LSFStringList idList;
        idList.push_back(masterSceneID);
        controllerService.SendSignal(ControllerServiceMasterSceneInterfaceName, "MasterScenesNameChanged", idList);
    }
}

void MasterSceneManager::CreateMasterScene(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));

    LSFResponseCode responseCode = LSF_OK;

    bool created = false;
    LSFString masterSceneID;

    const ajn::MsgArg* inputArgs;
    size_t numInputArgs;
    message->GetArgs(numInputArgs, inputArgs);

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        if (masterScenes.size() < MAX_SUPPORTED_NUM_LSF_ENTITY) {
            masterSceneID = GenerateUniqueID("MASTER_SCENE");
            MasterScene masterScene(inputArgs[0]);

            LSFString name = static_cast<LSFString>(inputArgs[1].v_string.str);
            LSFString language = static_cast<LSFString>(inputArgs[2].v_string.str);

            if (0 != strcmp("en", language.c_str())) {
                QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __FUNCTION__, language.c_str()));
                responseCode = LSF_ERR_INVALID_ARGS;
            } else {
                masterScenes[masterSceneID].first = name;
                masterScenes[masterSceneID].second = masterScene;
                created = true;
            }
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

    controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, masterSceneID);

    if (created) {
        ScheduleFileUpdate();
        LSFStringList idList;
        idList.push_back(masterSceneID);
        controllerService.SendSignal(ControllerServiceMasterSceneInterfaceName, "MasterScenesCreated", idList);
    }
}

void MasterSceneManager::UpdateMasterScene(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool updated = false;

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    LSFString masterSceneID(uniqueId);
    MasterScene masterScene(args[1]);

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        MasterSceneMap::iterator it = masterScenes.find(uniqueId);
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

    controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, masterSceneID);

    if (updated) {
        ScheduleFileUpdate();
        LSFStringList idList;
        idList.push_back(masterSceneID);
        controllerService.SendSignal(ControllerServiceMasterSceneInterfaceName, "MasterScenesUpdated", idList);
    }
}

void MasterSceneManager::DeleteMasterScene(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    LSFString name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool deleted = false;

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    LSFString masterSceneID(uniqueId);

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        MasterSceneMap::iterator it = masterScenes.find(uniqueId);
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

    controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, masterSceneID);

    if (deleted) {
        ScheduleFileUpdate();
        LSFStringList idList;
        idList.push_back(masterSceneID);
        controllerService.SendSignal(ControllerServiceMasterSceneInterfaceName, "MasterScenesDeleted", idList);
    }
}

void MasterSceneManager::GetMasterScene(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    MsgArg outArgs[3];

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        MasterSceneMap::iterator it = masterScenes.find(uniqueId);
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
    outArgs[1].Set("s", uniqueId);

    controllerService.SendMethodReply(message, outArgs, 3);
}

void MasterSceneManager::ApplyMasterScene(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    const char* masterSceneId;
    args[0].Get("s", &masterSceneId);

    LSFString uniqueId(masterSceneId);

    LSFStringList scenes;
    LSFStringList appliedList;
    appliedList.push_back(uniqueId);

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        MasterSceneMap::iterator it = masterScenes.find(uniqueId);
        if (it != masterScenes.end()) {
            scenes = it->second.second.scenes;
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

    if (LSF_OK == responseCode) {
        responseCode = sceneManager.ApplySceneInternal(message, scenes);
    }

    if (LSF_OK == responseCode) {
        controllerService.SendSignal(ControllerServiceMasterSceneInterfaceName, "MasterScenesApplied", appliedList);
    } else {
        controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, uniqueId);
    }
}

// Saved scenes have the format:
// (MasterScene id "name" (Scene id)* EndMasterScene)*
void MasterSceneManager::ReadSavedData()
{
    std::istringstream stream;
    if (!ValidateFileAndRead(stream)) {
        return;
    }

    while (!stream.eof()) {
        std::string token = ParseString(stream);

        if (token == "MasterScene") {
            std::string id = ParseString(stream);
            std::string name = ParseString(stream);
            LSFStringList subScenes;

            do {
                token = ParseString(stream);

                if (token == "Scene") {
                    std::string scene = ParseString(stream);
                    subScenes.push_back(scene);
                }
            } while (token != "EndMasterScene");

            MasterScene msc(subScenes);
            std::pair<LSFString, MasterScene> thePair(name, msc);
            masterScenes[id] = thePair;
        }
    }
}

std::string MasterSceneManager::GetString(const MasterSceneMap& items)
{
    std::ostringstream stream;
    // (MasterScene id "name" (Scene id)* EndMasterScene)*
    for (MasterSceneMap::const_iterator it = items.begin(); it != items.end(); ++it) {
        const LSFString& id = it->first;
        const LSFString& name = it->second.first;
        const MasterScene& msc = it->second.second;

        stream << "MasterScene " << id << " \"" << name << "\"";

        for (LSFStringList::const_iterator sit = msc.scenes.begin(); sit != msc.scenes.end(); ++sit) {
            stream << " Scene " << *sit;
        }

        stream << " EndMasterScene" << std::endl;
    }

    return stream.str();
}

std::string MasterSceneManager::GetString()
{
    masterScenesLock.Lock();
    // we can't hold this lock for the entire time!
    MasterSceneMap mapCopy = masterScenes;
    updated = false;
    masterScenesLock.Unlock();

    return GetString(mapCopy);
}

void MasterSceneManager::WriteFile()
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    if (!updated) {
        return;
    }

    if (filePath.empty()) {
        return;
    }

    std::string output = GetString();
    uint32_t checksum = GetChecksum(output);
    WriteFileWithChecksum(output, checksum);
    controllerService.SendBlobUpdate(LSF_MASTER_SCENE, checksum, 0UL);
}

uint32_t MasterSceneManager::GetControllerServiceMasterSceneInterfaceVersion(void)
{
    QCC_DbgPrintf(("%s: controllerMasterSceneInterfaceVersion=%d", __FUNCTION__, ControllerServiceMasterSceneInterfaceVersion));
    return ControllerServiceMasterSceneInterfaceVersion;
}
