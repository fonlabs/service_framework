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
    Manager(controllerSvc, masterSceneFile), sceneManager(sceneMgr), blobLength(0)
{
    QCC_DbgTrace(("%s", __func__));
    masterScenes.clear();
}

LSFResponseCode MasterSceneManager::GetAllMasterScenes(MasterSceneMap& masterSceneMap)
{
    QCC_DbgTrace(("%s", __func__));
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        masterSceneMap = masterScenes;
        status = masterScenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __func__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __func__));
    }

    return responseCode;
}

LSFResponseCode MasterSceneManager::Reset(void)
{
    QCC_DbgPrintf(("%s", __func__));
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
        blobLength = 0;
        ScheduleFileWrite();
        tempStatus = masterScenesLock.Unlock();
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: masterScenesLock.Unlock() failed", __func__));
        }

        /*
         * Send the MasterScenes deleted signal
         */
        if (masterScenesList.size()) {
            tempStatus = controllerService.SendSignal(ControllerServiceMasterSceneInterfaceName, "MasterScenesDeleted", masterScenesList);
            if (ER_OK != tempStatus) {
                QCC_LogError(tempStatus, ("%s: Unable to send MasterScenesDeleted signal", __func__));
            }
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(tempStatus, ("%s: masterScenesLock.Lock() failed", __func__));
    }

    return responseCode;
}

LSFResponseCode MasterSceneManager::IsDependentOnScene(LSFString& sceneID)
{
    QCC_DbgTrace(("%s", __func__));
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
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __func__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __func__));
    }

    return responseCode;
}

void MasterSceneManager::GetAllMasterSceneIDs(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __func__, message->ToString().c_str()));

    LSFStringList idList;
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = masterScenesLock.Lock();
    if (ER_OK == status) {
        for (MasterSceneMap::iterator it = masterScenes.begin(); it != masterScenes.end(); ++it) {
            idList.push_back(it->first.c_str());
        }
        status = masterScenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __func__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __func__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(message, responseCode, idList);
}

void MasterSceneManager::GetMasterSceneName(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __func__, message->ToString().c_str()));
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
        QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __func__, language.c_str()));
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
                QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __func__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __func__));
        }
    }

    controllerService.SendMethodReplyWithResponseCodeIDLanguageAndName(message, responseCode, masterSceneID, language, name);
}

void MasterSceneManager::SetMasterSceneName(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __func__, message->ToString().c_str()));
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
        QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __func__, language.c_str()));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else if (name[0] == '\0') {
        QCC_LogError(ER_FAIL, ("%s: master scene name is empty", __func__));
        responseCode = LSF_ERR_EMPTY_NAME;
    } else {
        if (strlen(name) > LSF_MAX_NAME_LENGTH) {
            responseCode = LSF_ERR_INVALID_ARGS;
            QCC_LogError(ER_FAIL, ("%s: strlen(name) > LSF_MAX_NAME_LENGTH", __func__));
        } else {
            QStatus status = masterScenesLock.Lock();
            if (ER_OK == status) {
                MasterSceneMap::iterator it = masterScenes.find(uniqueId);
                if (it != masterScenes.end()) {
                    LSFString newName = name;
                    size_t newlen = blobLength - it->second.first.length() + newName.length();

                    if (newlen < MAX_FILE_LEN) {
                        it->second.first = newName;
                        responseCode = LSF_OK;
                        nameChanged = true;
                        ScheduleFileWrite();
                    } else {
                        responseCode = LSF_ERR_RESOURCES;
                    }
                }
                status = masterScenesLock.Unlock();
                if (ER_OK != status) {
                    QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __func__));
                }
            } else {
                responseCode = LSF_ERR_BUSY;
                QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __func__));
            }
        }
    }

    controllerService.SendMethodReplyWithResponseCodeIDAndName(message, responseCode, masterSceneID, language);

    if (nameChanged) {
        LSFStringList idList;
        idList.push_back(masterSceneID);
        controllerService.SendSignal(ControllerServiceMasterSceneInterfaceName, "MasterScenesNameChanged", idList);
    }
}

void MasterSceneManager::CreateMasterScene(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __func__, message->ToString().c_str()));

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
                QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __func__, language.c_str()));
                responseCode = LSF_ERR_INVALID_ARGS;
            } else if (name.empty()) {
                QCC_LogError(ER_FAIL, ("%s: scene name is empty", __func__));
                responseCode = LSF_ERR_EMPTY_NAME;
            } else {
                std::string newGroupStr = GetString(name, masterSceneID, masterScene);

                size_t newlen = blobLength + newGroupStr.length();
                if (newlen < MAX_FILE_LEN) {
                    blobLength = newlen;
                    masterScenes[masterSceneID].first = name;
                    masterScenes[masterSceneID].second = masterScene;
                    created = true;
                    ScheduleFileWrite();
                } else {
                    responseCode = LSF_ERR_RESOURCES;
                }
            }
        } else {
            QCC_LogError(ER_FAIL, ("%s: No slot for new MasterScene", __func__));
            responseCode = LSF_ERR_NO_SLOT;
        }
        status = masterScenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __func__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __func__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, masterSceneID);

    if (created) {
        LSFStringList idList;
        idList.push_back(masterSceneID);
        controllerService.SendSignal(ControllerServiceMasterSceneInterfaceName, "MasterScenesCreated", idList);
    }
}

void MasterSceneManager::UpdateMasterScene(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __func__, message->ToString().c_str()));
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
            size_t newlen = blobLength;
            // sub len of old group, add len of new group
            newlen -= GetString(it->second.first, masterSceneID, it->second.second).length();
            newlen += GetString(it->second.first, masterSceneID, masterScene).length();

            if (newlen < MAX_FILE_LEN) {
                blobLength = newlen;
                masterScenes[masterSceneID].second = masterScene;
                responseCode = LSF_OK;
                updated = true;
                ScheduleFileWrite();
            } else {
                responseCode = LSF_ERR_RESOURCES;
            }
        }
        status = masterScenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __func__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __func__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, masterSceneID);

    if (updated) {
        LSFStringList idList;
        idList.push_back(masterSceneID);
        controllerService.SendSignal(ControllerServiceMasterSceneInterfaceName, "MasterScenesUpdated", idList);
    }
}

void MasterSceneManager::DeleteMasterScene(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __func__, message->ToString().c_str()));
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
            blobLength -= GetString(it->second.first, uniqueId, it->second.second).length();
            masterScenes.erase(it);

            responseCode = LSF_OK;
            deleted = true;
            ScheduleFileWrite();
        }
        status = masterScenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __func__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __func__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, masterSceneID);

    if (deleted) {
        LSFStringList idList;
        idList.push_back(masterSceneID);
        controllerService.SendSignal(ControllerServiceMasterSceneInterfaceName, "MasterScenesDeleted", idList);
    }
}

void MasterSceneManager::GetMasterScene(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __func__, message->ToString().c_str()));

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
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __func__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __func__));
    }

    outArgs[0].Set("u", responseCode);
    outArgs[1].Set("s", uniqueId);

    controllerService.SendMethodReply(message, outArgs, 3);
}

void MasterSceneManager::ApplyMasterScene(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __func__, message->ToString().c_str()));
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
            QCC_LogError(status, ("%s: masterScenesLock.Unlock() failed", __func__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: masterScenesLock.Lock() failed", __func__));
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
    QCC_DbgTrace(("%s", __func__));
    std::istringstream stream;
    if (!ValidateFileAndRead(stream)) {
        /*
         * If there is no file present / CRC check failed on the file create a new
         * file with initialState entry
         */
        masterScenesLock.Lock();
        ScheduleFileWrite(false, true);
        masterScenesLock.Unlock();
        return;
    }

    blobLength = stream.str().size();
    ReplaceMap(stream);
}

void MasterSceneManager::ReplaceMap(std::istringstream& stream)
{
    QCC_DbgTrace(("%s", __func__));
    bool firstIteration = true;
    while (!stream.eof()) {
        std::string token = ParseString(stream);

        if (token == "MasterScene") {
            std::string id = ParseString(stream);
            std::string name = ParseString(stream);
            if (0 == strcmp(id.c_str(), resetID.c_str())) {
                QCC_DbgPrintf(("The file has a reset entry. Clearing the map"));
                masterScenes.clear();
            } else if (0 == strcmp(id.c_str(), initialStateID.c_str())) {
                QCC_DbgPrintf(("The file has a initialState entry. So we ignore it"));
            } else {
                if (firstIteration) {
                    masterScenes.clear();
                    firstIteration = false;
                }
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
}

std::string MasterSceneManager::GetString(const std::string& name, const std::string& id, const MasterScene& msc)
{
    std::ostringstream stream;
    stream << "MasterScene " << id << " \"" << name << "\"";

    for (LSFStringList::const_iterator sit = msc.scenes.begin(); sit != msc.scenes.end(); ++sit) {
        stream << " Scene " << *sit;
    }

    stream << " EndMasterScene" << std::endl;
    return stream.str();
}

std::string MasterSceneManager::GetString(const MasterSceneMap& items)
{
    QCC_DbgTrace(("%s", __func__));
    std::ostringstream stream;
    // (MasterScene id "name" (Scene id)* EndMasterScene)*
    if (0 == items.size()) {
        if (initialState) {
            QCC_DbgPrintf(("%s: This is the initial state entry", __func__));
            const LSFString& id = initialStateID;
            const LSFString& name = initialStateID;

            stream << "MasterScene " << id << " \"" << name << "\"";
            stream << " EndMasterScene" << std::endl;
        } else {
            const LSFString& id = resetID;
            const LSFString& name = resetID;

            stream << "MasterScene " << id << " \"" << name << "\"";
            stream << " EndMasterScene" << std::endl;
        }
    } else {
        for (MasterSceneMap::const_iterator it = items.begin(); it != items.end(); ++it) {
            const LSFString& id = it->first;
            const LSFString& name = it->second.first;
            const MasterScene& msc = it->second.second;
            stream << GetString(name, id, msc);
        }
    }

    return stream.str();
}

bool MasterSceneManager::GetString(std::string& output, uint32_t& checksum, uint64_t& timestamp)
{
    QCC_DbgTrace(("%s", __func__));
    MasterSceneMap mapCopy;
    mapCopy.clear();
    bool ret = false;
    output.clear();

    masterScenesLock.Lock();
    // we can't hold this lock for the entire time!
    if (updated) {
        mapCopy = masterScenes;
        updated = false;
        ret = true;
    }
    masterScenesLock.Unlock();

    if (ret) {
        output = GetString(mapCopy);
        masterScenesLock.Lock();
        if (blobUpdateCycle) {
            checksum = checkSum;
            timestamp = timeStamp;
            blobUpdateCycle = false;
        } else {
            if (initialState) {
                timeStamp = timestamp = 0UL;
                initialState = false;
            } else {
                timeStamp = timestamp = GetTimestamp64();
            }
            checkSum = checksum = GetChecksum(output);
        }
        masterScenesLock.Unlock();
    }

    return ret;
}

void MasterSceneManager::HandleReceivedBlob(const std::string& blob, uint32_t checksum, uint64_t timestamp)
{
    QCC_DbgPrintf(("%s", __func__));
    uint64_t currentTimestamp = GetTimestamp64();
    masterScenesLock.Lock();
    if (((timeStamp == 0) || ((currentTimestamp - timeStamp) > timestamp)) && (checkSum != checksum)) {
        std::istringstream stream(blob.c_str());
        ReplaceMap(stream);
        timeStamp = currentTimestamp;
        checkSum = checksum;
        ScheduleFileWrite(true);
    }
    masterScenesLock.Unlock();
}

void MasterSceneManager::ReadWriteFile()
{
    QCC_DbgPrintf(("%s", __func__));

    if (filePath.empty()) {
        return;
    }

    std::string output;
    uint32_t checksum;
    uint64_t timestamp;
    bool status = false;

    status = GetString(output, checksum, timestamp);

    if (status) {
        WriteFileWithChecksumAndTimestamp(output, checksum, timestamp);
        uint64_t currentTime = GetTimestamp64();
        controllerService.SendBlobUpdate(LSF_MASTER_SCENE, output, checksum, (currentTime - timestamp));
    }

    std::list<ajn::Message> tempMessageList;

    readMutex.Lock();
    if (read) {
        tempMessageList = readBlobMessages;
        readBlobMessages.clear();
        read = false;
    }
    readMutex.Unlock();

    if (tempMessageList.size() && !status) {
        std::istringstream stream;
        status = ValidateFileAndReadInternal(checksum, timestamp, stream);
        if (status) {
            output = stream.str();
        } else {
            QCC_LogError(ER_FAIL, ("%s: MasterScene persistent store corrupted", __func__));
        }
    }

    if (status) {
        while (!tempMessageList.empty()) {
            uint64_t currentTime = GetTimestamp64();
            controllerService.SendGetBlobReply(tempMessageList.front(), LSF_MASTER_SCENE, output, checksum, (currentTime - timestamp));
            tempMessageList.pop_front();
        }
    }
}

uint32_t MasterSceneManager::GetControllerServiceMasterSceneInterfaceVersion(void)
{
    QCC_DbgPrintf(("%s: controllerMasterSceneInterfaceVersion=%d", __func__, ControllerServiceMasterSceneInterfaceVersion));
    return ControllerServiceMasterSceneInterfaceVersion;
}
