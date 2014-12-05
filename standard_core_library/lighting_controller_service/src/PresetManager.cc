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

#include <PresetManager.h>
#include <ControllerService.h>
#include <qcc/Debug.h>
#include <SceneManager.h>
#include <OEM_CS_Config.h>
#include <FileParser.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "PRESET_MANAGER"

LSFString defaultLampStateID = "DefaultLampState";

PresetManager::PresetManager(ControllerService& controllerSvc, SceneManager* sceneMgrPtr, const std::string& presetFile) :
    Manager(controllerSvc, presetFile), sceneManagerPtr(sceneMgrPtr), blobLength(0)
{
    QCC_DbgTrace(("%s", __func__));
    presets.clear();
}

LSFResponseCode PresetManager::GetAllPresets(PresetMap& presetMap)
{
    QCC_DbgTrace(("%s", __func__));
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = presetsLock.Lock();
    if (ER_OK == status) {
        presetMap = presets;
        status = presetsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __func__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: presetsLock.Lock() failed", __func__));
    }

    return responseCode;
}

LSFResponseCode PresetManager::Reset(void)
{
    QCC_DbgPrintf(("%s", __func__));
    LSFResponseCode responseCode = LSF_OK;

    if (!controllerService.UpdatesAllowed()) {
        return LSF_ERR_BUSY;
    }

    QStatus tempStatus = presetsLock.Lock();
    if (ER_OK == tempStatus) {
        /*
         * Record the IDs of all the Presets that are being deleted
         */
        LSFStringList presetsList;
        for (PresetMap::iterator it = presets.begin(); it != presets.end(); ++it) {
            if (0 != strcmp(it->first.c_str(), defaultLampStateID.c_str())) {
                presetsList.push_back(it->first);
            }
        }

        /*
         * Clear the Presets
         */
        presets.clear();
        blobLength = 0;
        ScheduleFileWrite();
        tempStatus = presetsLock.Unlock();
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: presetsLock.Unlock() failed", __func__));
        }

        /*
         * Send the Presets deleted signal
         */
        if (presetsList.size()) {
            tempStatus = controllerService.SendSignal(ControllerServicePresetInterfaceName, "PresetsDeleted", presetsList);
            if (ER_OK != tempStatus) {
                QCC_LogError(tempStatus, ("%s: Unable to send PresetsDeleted signal", __func__));
            }
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(tempStatus, ("%s: presetsLock.Lock() failed", __func__));
    }

    return responseCode;
}

LSFResponseCode PresetManager::ResetDefaultState(void)
{
    QCC_DbgTrace(("%s", __func__));
    bool erased = false;
    presetsLock.Lock();
    PresetMap::iterator it = presets.find(defaultLampStateID);
    if (it != presets.end()) {
        QCC_DbgPrintf(("%s: Removing the default lamp state entry", __func__));
        blobLength -= GetString(it->second.first, defaultLampStateID, it->second.second).length();
        presets.erase(it);
        erased = true;
    }
    presetsLock.Unlock();
    if (erased) {
        /*
         * Send the DefaultLampStateChangedSignal
         */
        QCC_DbgPrintf(("%s: Sending the DefaultLampStateChangedSignal", __func__));
        QStatus status = controllerService.SendSignalWithoutArg(ControllerServicePresetInterfaceName, "DefaultLampStateChanged");
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: Unable to send DefaultLampStateChanged signal", __func__));
        }
    }
    return LSF_OK;
}

LSFResponseCode PresetManager::GetPresetInternal(const LSFString& presetID, LampState& preset)
{
    QCC_DbgPrintf(("%s", __func__));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    //TODO: Change this later if required
    if (0 == strcmp(presetID.c_str(), CurrentStateIdentifier.c_str())) {
        QCC_DbgPrintf(("%s: NULL STATE", __func__));
        preset = LampState();
        responseCode = LSF_OK;
    } else {
        QStatus status = presetsLock.Lock();
        if (ER_OK == status) {
            PresetMap::iterator it = presets.find(presetID);
            if (it != presets.end()) {
                preset = it->second.second;
                QCC_DbgPrintf(("%s: Found Preset %s", __func__, preset.c_str()));
                responseCode = LSF_OK;
            }
            status = presetsLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __func__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: presetsLock.Lock() failed", __func__));
        }
    }
    QCC_DbgPrintf(("%s: %s", __func__, LSFResponseCodeText(responseCode)));
    return responseCode;
}

void PresetManager::GetAllPresetIDs(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __func__, msg->ToString().c_str()));

    LSFStringList idList;
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = presetsLock.Lock();
    if (ER_OK == status) {
        for (PresetMap::iterator it = presets.begin(); it != presets.end(); ++it) {
            if (0 != strcmp(it->first.c_str(), defaultLampStateID.c_str())) {
                idList.push_back(it->first.c_str());
            }
        }
        status = presetsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __func__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: presetsLock.Lock() failed", __func__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(msg, responseCode, idList);
}

void PresetManager::GetPresetName(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __func__, msg->ToString().c_str()));
    LSFString name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    if (controllerService.CheckNumArgsInMessage(numArgs, 2)  != LSF_OK) {
        return;
    }

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    LSFString presetID(uniqueId);

    LSFString language = static_cast<LSFString>(args[1].v_string.str);
    if (0 != strcmp("en", language.c_str())) {
        QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __func__, language.c_str()));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else {
        QStatus status = presetsLock.Lock();
        if (ER_OK == status) {
            PresetMap::iterator it = presets.find(uniqueId);
            if (it != presets.end()) {
                name = it->second.first;
                responseCode = LSF_OK;
            }
            status = presetsLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __func__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: presetsLock.Lock() failed", __func__));
        }
    }

    controllerService.SendMethodReplyWithResponseCodeIDLanguageAndName(msg, responseCode, presetID, language, name);
}

void PresetManager::SetPresetName(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __func__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool nameChanged = false;
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    if (controllerService.CheckNumArgsInMessage(numArgs, 3)  != LSF_OK) {
        return;
    }

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    LSFString presetID(uniqueId);

    const char* name;
    args[1].Get("s", &name);

    LSFString language = static_cast<LSFString>(args[2].v_string.str);

    if (!controllerService.UpdatesAllowed()) {
        controllerService.SendMethodReplyWithResponseCodeIDAndName(msg, LSF_ERR_BUSY, presetID, language);
        return;
    }

    if (0 != strcmp("en", language.c_str())) {
        QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __func__, language.c_str()));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else if (name[0] == '\0') {
        QCC_LogError(ER_FAIL, ("%s: preset name is empty", __func__));
        responseCode = LSF_ERR_EMPTY_NAME;
    } else {
        if (strlen(name) > LSF_MAX_NAME_LENGTH) {
            responseCode = LSF_ERR_INVALID_ARGS;
            QCC_LogError(ER_FAIL, ("%s: strlen(name) > LSF_MAX_NAME_LENGTH", __func__));
        } else {
            QStatus status = presetsLock.Lock();
            if (ER_OK == status) {
                PresetMap::iterator it = presets.find(uniqueId);
                if (it != presets.end()) {
                    LSFString newName = name;
                    size_t newlen = blobLength - it->second.first.length() + newName.length();

                    if (newlen < MAX_FILE_LEN) {
                        blobLength = newlen;
                        it->second.first = newName;
                        responseCode = LSF_OK;
                        nameChanged = true;
                        ScheduleFileWrite();
                    } else {
                        responseCode = LSF_ERR_RESOURCES;
                    }
                }
                status = presetsLock.Unlock();
                if (ER_OK != status) {
                    QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __func__));
                }
            } else {
                responseCode = LSF_ERR_BUSY;
                QCC_LogError(status, ("%s: presetsLock.Lock() failed", __func__));
            }
        }
    }

    controllerService.SendMethodReplyWithResponseCodeIDAndName(msg, responseCode, presetID, language);

    if (nameChanged) {
        LSFStringList idList;
        idList.push_back(presetID);
        controllerService.SendSignal(ControllerServicePresetInterfaceName, "PresetsNameChanged", idList);
    }
}

void PresetManager::CreatePreset(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __func__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_OK;

    LSFString presetID;

    if (!controllerService.UpdatesAllowed()) {
        controllerService.SendMethodReplyWithResponseCodeAndID(msg, LSF_ERR_BUSY, presetID);
        return;
    }

    presetID = GenerateUniqueID("PRESET");

    bool created = false;

    const ajn::MsgArg* inputArgs;
    size_t numInputArgs;
    msg->GetArgs(numInputArgs, inputArgs);

    if (controllerService.CheckNumArgsInMessage(numInputArgs, 3)  != LSF_OK) {
        return;
    }

    LampState preset(inputArgs[0]);
    LSFString name = static_cast<LSFString>(inputArgs[1].v_string.str);
    LSFString language = static_cast<LSFString>(inputArgs[2].v_string.str);

    if (0 != strcmp("en", language.c_str())) {
        QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __func__, language.c_str()));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else if (name.empty()) {
        QCC_LogError(ER_FAIL, ("%s: scene name is empty", __func__));
        responseCode = LSF_ERR_EMPTY_NAME;
    } else if (preset.nullState) {
        QCC_LogError(ER_FAIL, ("%s: Cannot save NULL state as a Preset", __func__, language.c_str()));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else if (name.length() > LSF_MAX_NAME_LENGTH) {
        QCC_LogError(ER_FAIL, ("%s: name length exceeds %d", __func__, LSF_MAX_NAME_LENGTH));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else {
        QStatus status = presetsLock.Lock();
        if (ER_OK == status) {
            if (presets.size() < OEM_CS_MAX_SUPPORTED_NUM_LSF_ENTITY) {
                std::string newPresetStr = GetString(name, presetID, preset);
                size_t newlen = blobLength + newPresetStr.length();
                if (newlen < MAX_FILE_LEN) {
                    blobLength = newlen;
                    presets[presetID].first = name;
                    presets[presetID].second = preset;
                    created = true;
                    ScheduleFileWrite();
                } else {
                    responseCode = LSF_ERR_RESOURCES;
                }
            } else {
                QCC_LogError(ER_FAIL, ("%s: No slot for new Preset", __func__));
                responseCode = LSF_ERR_NO_SLOT;
            }
            status = presetsLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __func__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: presetsLock.Lock() failed", __func__));
        }
    }

    if (!created) {
        presetID.clear();
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, presetID);

    if (created) {
        LSFStringList idList;
        idList.push_back(presetID);
        controllerService.SendSignal(ControllerServicePresetInterfaceName, "PresetsCreated", idList);
    }
}

void PresetManager::UpdatePreset(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __func__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool updated = false;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    if (controllerService.CheckNumArgsInMessage(numArgs, 2)  != LSF_OK) {
        return;
    }

    const char* presetId;
    args[0].Get("s", &presetId);

    LSFString presetID(presetId);

    if (!controllerService.UpdatesAllowed()) {
        controllerService.SendMethodReplyWithResponseCodeAndID(msg, LSF_ERR_BUSY, presetID);
        return;
    }

    LampState preset(args[1]);

    if (preset.nullState) {
        QCC_LogError(ER_FAIL, ("%s: Empty state", __func__));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else {
        QStatus status = presetsLock.Lock();
        if (ER_OK == status) {
            PresetMap::iterator it = presets.find(presetId);
            if (it != presets.end()) {
                size_t newlen = blobLength;
                // sub len of old group, add len of new group
                newlen -= GetString(it->second.first, presetID, it->second.second).length();
                newlen += GetString(it->second.first, presetID, preset).length();

                if (newlen < MAX_FILE_LEN) {
                    blobLength = newlen;
                    presets[presetID].second = preset;
                    responseCode = LSF_OK;
                    updated = true;
                    ScheduleFileWrite();
                } else {
                    responseCode = LSF_ERR_RESOURCES;
                }
            }
            status = presetsLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __func__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: presetsLock.Lock() failed", __func__));
        }
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, presetID);

    if (updated) {
        LSFStringList idList;
        idList.push_back(presetID);
        controllerService.SendSignal(ControllerServicePresetInterfaceName, "PresetsUpdated", idList);
    }
}

void PresetManager::DeletePreset(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __func__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_OK;

    bool deleted = false;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    if (controllerService.CheckNumArgsInMessage(numArgs, 1)  != LSF_OK) {
        return;
    }

    const char* presetId;
    args[0].Get("s", &presetId);

    LSFString presetID(presetId);

    if (!controllerService.UpdatesAllowed()) {
        controllerService.SendMethodReplyWithResponseCodeAndID(msg, LSF_ERR_BUSY, presetID);
        return;
    }

    responseCode = sceneManagerPtr->IsDependentOnPreset(presetID);

    if (LSF_OK == responseCode) {
        QStatus status = presetsLock.Lock();
        if (ER_OK == status) {
            PresetMap::iterator it = presets.find(presetId);
            if (it != presets.end()) {
                blobLength -= GetString(it->second.first, presetId, it->second.second).length();
                presets.erase(it);
                deleted = true;
                ScheduleFileWrite();
            } else {
                responseCode = LSF_ERR_NOT_FOUND;
            }
            status = presetsLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __func__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: presetsLock.Lock() failed", __func__));
        }
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, presetID);

    if (deleted) {
        LSFStringList idList;
        idList.push_back(presetID);
        controllerService.SendSignal(ControllerServicePresetInterfaceName, "PresetsDeleted", idList);
    }
}

void PresetManager::GetPreset(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __func__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    MsgArg outArgs[3];

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    if (controllerService.CheckNumArgsInMessage(numArgs, 1)  != LSF_OK) {
        return;
    }

    const char* presetId;
    args[0].Get("s", &presetId);

    LampState preset;

    responseCode = GetPresetInternal(presetId, preset);

    if (LSF_OK == responseCode) {
        preset.Get(&outArgs[2]);
    } else {
        outArgs[2].Set("a{sv}", 0, NULL);
    }

    outArgs[0].Set("u", responseCode);
    outArgs[1].Set("s", presetId);

    controllerService.SendMethodReply(msg, outArgs, 3);
}

void PresetManager::GetDefaultLampState(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __func__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_BUSY;
    LampState preset;

    MsgArg outArgs[2];

    if (LSF_OK == GetDefaultLampStateInternal(preset)) {
        preset.Get(&outArgs[1]);
        responseCode = LSF_OK;
    } else {
        outArgs[1].Set("a{sv}", 0, NULL);
    }

    outArgs[0].Set("u", responseCode);

    controllerService.SendMethodReply(msg, outArgs, 2);
}

void PresetManager::SetDefaultLampState(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __func__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_BUSY;

    const ajn::MsgArg* inputArg;
    size_t numInputArgs;
    msg->GetArgs(numInputArgs, inputArg);

    if (controllerService.CheckNumArgsInMessage(numInputArgs, 1)  != LSF_OK) {
        return;
    }

    MsgArg arg;

    if (!controllerService.UpdatesAllowed()) {
        controllerService.SendMethodReplyWithUint32Value(msg, LSF_ERR_BUSY);
    } else {
        LampState preset(*inputArg);
        if (LSF_OK == SetDefaultLampStateInternal(preset)) {
            responseCode = LSF_OK;
        }

        controllerService.SendMethodReplyWithUint32Value(msg, (uint32_t &)responseCode);
    }
}

LSFResponseCode PresetManager::GetDefaultLampStateInternal(LampState& preset)
{
    QCC_DbgPrintf(("%s", __func__));

    LSFResponseCode responseCode = GetPresetInternal(defaultLampStateID, preset);

    if (responseCode == LSF_ERR_NOT_FOUND) {
        OEM_CS_GetFactorySetDefaultLampState(preset);
        responseCode = LSF_OK;
    }

    return responseCode;
}

LSFResponseCode PresetManager::SetDefaultLampStateInternal(LampState& preset)
{
    QCC_DbgPrintf(("%s: preset=%s", __func__, preset.c_str()));
    LSFResponseCode responseCode = LSF_OK;
    QStatus tempStatus = presetsLock.Lock();
    LSFString presetID = defaultLampStateID;
    if (ER_OK == tempStatus) {

        PresetMap::iterator it = presets.find(presetID);
        if (it != presets.end()) {
            size_t newlen = blobLength;
            newlen -= GetString(it->second.first, presetID, it->second.second).length();
            newlen += GetString(it->second.first, presetID, preset).length();

            if (newlen < MAX_FILE_LEN) {
                blobLength = newlen;
                it->second.second = preset;
                ScheduleFileWrite();
            } else {
                responseCode = LSF_ERR_RESOURCES;
            }
        } else {
            size_t newlen = blobLength + GetString(presetID, presetID, preset).length();

            if (newlen < MAX_FILE_LEN) {
                blobLength = newlen;
                presets[presetID] = std::make_pair(presetID, preset);
                ScheduleFileWrite();
            } else {
                responseCode = LSF_ERR_RESOURCES;
            }
        }


        tempStatus = presetsLock.Unlock();
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: presetsLock.Unlock() failed", __func__));
        }

        /*
         * Send the DefaultLampStateChangedSignal
         */
        QCC_DbgPrintf(("%s: Sending the DefaultLampStateChangedSignal", __func__));
        tempStatus = controllerService.SendSignalWithoutArg(ControllerServicePresetInterfaceName, "DefaultLampStateChanged");
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: Unable to send DefaultLampStateChanged signal", __func__));
        }

    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(tempStatus, ("%s: presetsLock.Lock() failed", __func__));
    }

    return responseCode;
}

// never call this when the ControllerService is up; it isn't thread-safe!
// Presets follow the form:
// (Preset id "name" on/off hue saturation colortemp brightness)*
void PresetManager::ReadSavedData(void)
{
    QCC_DbgTrace(("%s", __func__));
    std::istringstream stream;
    if (!ValidateFileAndRead(stream)) {
        /*
         * If there is no file present / CRC check failed on the file create a new
         * file with initialState entry
         */
        presetsLock.Lock();
        ScheduleFileWrite(false, true);
        presetsLock.Unlock();
        return;
    }

    blobLength = stream.str().size();
    ReplaceMap(stream);
}

void PresetManager::HandleReceivedBlob(const std::string& blob, uint32_t checksum, uint64_t timestamp)
{
    QCC_DbgPrintf(("%s", __func__));
    uint64_t currentTimestamp = GetTimestampInMs();
    presetsLock.Lock();
    if (((timeStamp == 0) || ((currentTimestamp - timeStamp) > timestamp)) && (checkSum != checksum)) {
        std::istringstream stream(blob.c_str());
        ReplaceMap(stream);
        timeStamp = currentTimestamp;
        checkSum = checksum;
        ScheduleFileWrite(true);
    }
    presetsLock.Unlock();
}

void PresetManager::ReplaceMap(std::istringstream& stream)
{
    QCC_DbgTrace(("%s", __func__));
    bool firstIteration = true;
    while (!stream.eof()) {
        std::string token = ParseString(stream);

        // keep searching for "Preset", which indicates the beginning of a saved Preset state
        if (token == "Preset") {
            std::string presetId = ParseString(stream);
            std::string presetName = ParseString(stream);

            QCC_DbgPrintf(("Preset id=%s, name=[%s]\n", presetId.c_str(), presetName.c_str()));

            if (0 == strcmp(presetId.c_str(), resetID.c_str())) {
                QCC_DbgPrintf(("The file has a reset entry. Clearing the map"));
                presets.clear();
            } else if (0 == strcmp(presetId.c_str(), initialStateID.c_str())) {
                QCC_DbgPrintf(("The file has a initialState entry. So we ignore it"));
            } else {
                if (firstIteration) {
                    presets.clear();
                    firstIteration = false;
                }
                LampState state;
                ParseLampState(stream, state);

                std::pair<LSFString, LampState> thePair = std::make_pair(presetName, state);
                presets[presetId] = thePair;
                QCC_DbgPrintf(("%s: Added ID=%s, Name=%s, State=%s", __func__, presetId.c_str(), presets[presetId].first.c_str(), presets[presetId].second.c_str()));
            }
        }
    }
}

std::string PresetManager::GetString(const std::string& name, const std::string& id, const LampState& state)
{
    std::ostringstream stream;
    if (!state.nullState) {
        stream << "Preset " << id << " \"" << name << "\" "
               << (state.nullState ? 1 : 0) << ' '
               << (state.onOff ? 1 : 0) << ' '
               << state.hue << ' ' << state.saturation << ' '
               << state.colorTemp << ' ' << state.brightness << '\n';
    } else {
        QCC_DbgPrintf(("%s: Saving NULL state as preset", __func__));
        stream << "Preset " << id << " \"" << name << "\" "
               << (state.nullState ? 1 : 0) << '\n';
    }
    return stream.str();
}

std::string PresetManager::GetString(const PresetMap& items)
{
    std::ostringstream stream;

    if (0 == items.size()) {
        if (initialState) {
            QCC_DbgPrintf(("%s: This is the initial state entry", __func__));
            const LSFString& id = initialStateID;
            const LSFString& name = initialStateID;

            stream << "Preset " << id << " \"" << name << "\" " << '\n';
        } else {
            QCC_DbgPrintf(("%s: File is being reset", __func__));
            const LSFString& id = resetID;
            const LSFString& name = resetID;

            stream << "Preset " << id << " \"" << name << "\" " << '\n';
        }
    } else {
        // (Preset id "name" on/off hue saturation colortemp brightness)*
        for (PresetMap::const_iterator it = items.begin(); it != items.end(); ++it) {
            const LSFString& id = it->first;
            const LSFString& name = it->second.first;
            const LampState& state = it->second.second;

            stream << GetString(name, id, state);
        }
    }

    return stream.str();
}

bool PresetManager::GetString(std::string& output, uint32_t& checksum, uint64_t& timestamp)
{
    PresetMap mapCopy;
    mapCopy.clear();
    bool ret = false;
    output.clear();

    presetsLock.Lock();
    // we can't hold this lock for the entire time!
    if (updated) {
        mapCopy = presets;
        updated = false;
        ret = true;
    }
    presetsLock.Unlock();

    if (ret) {
        output = GetString(mapCopy);
        presetsLock.Lock();
        if (blobUpdateCycle) {
            checksum = checkSum;
            timestamp = timeStamp;
            blobUpdateCycle = false;
        } else {
            if (initialState) {
                timeStamp = timestamp = 0UL;
                initialState = false;
            } else {
                timeStamp = timestamp = GetTimestampInMs();
            }
            checkSum = checksum = GetChecksum(output);
        }
        presetsLock.Unlock();
    }

    return ret;
}

void PresetManager::ReadWriteFile()
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
        if (timestamp != 0UL) {
            uint64_t currentTime = GetTimestampInMs();
            controllerService.SendBlobUpdate(LSF_PRESET, output, checksum, (currentTime - timestamp));
        }
    }

    std::list<ajn::Message> tempMessageList;

    readMutex.Lock();
    if (read) {
        tempMessageList = readBlobMessages;
        readBlobMessages.clear();
        read = false;
    }
    readMutex.Unlock();

    if ((tempMessageList.size() || sendUpdate) && !status) {
        std::istringstream stream;
        status = ValidateFileAndReadInternal(checksum, timestamp, stream);
        if (status) {
            output = stream.str();
        } else {
            QCC_LogError(ER_FAIL, ("%s: Preset persistent store corrupted", __func__));
        }
    }

    if (status) {
        while (tempMessageList.size()) {
            uint64_t currentTime = GetTimestampInMs();
            controllerService.SendGetBlobReply(tempMessageList.front(), LSF_PRESET, output, checksum, (currentTime - timestamp));
            tempMessageList.pop_front();
        }
    }

    if (sendUpdate) {
        sendUpdate = false;
        uint64_t currentTime = GetTimestampInMs();
        controllerService.GetLeaderElectionObj().SendBlobUpdate(LSF_PRESET, output, checksum, (currentTime - timestamp));
    }
}

uint32_t PresetManager::GetControllerServicePresetInterfaceVersion(void)
{
    QCC_DbgPrintf(("%s: controllerPresetInterfaceVersion=%d", __func__, ControllerServicePresetInterfaceVersion));
    return ControllerServicePresetInterfaceVersion;
}
