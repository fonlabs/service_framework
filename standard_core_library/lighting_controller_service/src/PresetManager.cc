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
#include <OEMConfig.h>
#include <FileParser.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "PRESET_MANAGER"

static const uint32_t controllerPresetInterfaceVersion = 1;

PresetManager::PresetManager(ControllerService& controllerSvc, const char* ifaceName, SceneManager* sceneMgrPtr, const std::string& presetFile) :
    Manager(controllerSvc, presetFile), interfaceName(ifaceName), sceneManagerPtr(sceneMgrPtr)
{
    GetFactorySetDefaultLampState(defaultLampState);
}

LSFResponseCode PresetManager::GetAllPresets(PresetMap& presetMap)
{
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = presetsLock.Lock();
    if (ER_OK == status) {
        presetMap = presets;
        status = presetsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: presetsLock.Lock() failed", __FUNCTION__));
    }

    return responseCode;
}

LSFResponseCode PresetManager::Reset(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    LSFResponseCode responseCode = LSF_OK;
    QStatus tempStatus = presetsLock.Lock();
    if (ER_OK == tempStatus) {
        /*
         * Record the IDs of all the Presets that are being deleted
         */
        LSFStringList presetsList;
        for (PresetMap::iterator it = presets.begin(); it != presets.end(); ++it) {
            presetsList.push_back(it->first);
        }

        /*
         * Clear the Presets
         */
        presets.clear();
        tempStatus = presetsLock.Unlock();
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: presetsLock.Unlock() failed", __FUNCTION__));
        }

        /*
         * Send the Presets deleted signal
         */
        if (presetsList.size()) {
            tempStatus = controllerService.SendSignal(interfaceName, "PresetsDeleted", presetsList);
            if (ER_OK != tempStatus) {
                QCC_LogError(tempStatus, ("%s: Unable to send PresetsDeleted signal", __FUNCTION__));
            }
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(tempStatus, ("%s: presetsLock.Lock() failed", __FUNCTION__));
    }

    return responseCode;
}

LSFResponseCode PresetManager::ResetDefaultState(void)
{
    LampState state(false, 0x100, 0x100, 0x100, 0x100);
    return SetDefaultLampStateInternal(state);
}

LSFResponseCode PresetManager::GetPresetInternal(const LSFString& presetID, LampState& preset)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    //TODO: Change this later if required
    if (0 == strcmp(presetID.c_str(), "CURRENT_STATE")) {
        QCC_DbgPrintf(("%s: NULL STATE", __FUNCTION__));
        preset = LampState();
        responseCode = LSF_OK;
    } else {
        QStatus status = presetsLock.Lock();
        if (ER_OK == status) {
            PresetMap::iterator it = presets.find(presetID);
            if (it != presets.end()) {
                preset = it->second.second;
                responseCode = LSF_OK;
            }
            status = presetsLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __FUNCTION__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: presetsLock.Lock() failed", __FUNCTION__));
        }
    }
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, LSFResponseCodeText(responseCode)));
    return responseCode;
}

void PresetManager::GetAllPresetIDs(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFStringList idList;
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = presetsLock.Lock();
    if (ER_OK == status) {
        for (PresetMap::iterator it = presets.begin(); it != presets.end(); ++it) {
            idList.push_back(it->first.c_str());
        }
        status = presetsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: presetsLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(msg, responseCode, idList);
}

void PresetManager::GetPresetName(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFString name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    LSFString presetID(uniqueId);

    LSFString language = static_cast<LSFString>(args[1].v_string.str);
    if (0 != strcmp("en", language.c_str())) {
        QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __FUNCTION__, language.c_str()));
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
                QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __FUNCTION__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: presetsLock.Lock() failed", __FUNCTION__));
        }
    }

    controllerService.SendMethodReplyWithResponseCodeIDLanguageAndName(msg, responseCode, presetID, language, name);
}

void PresetManager::SetPresetName(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool nameChanged = false;
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    LSFString presetID(uniqueId);

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
            QStatus status = presetsLock.Lock();
            if (ER_OK == status) {
                PresetMap::iterator it = presets.find(uniqueId);
                if (it != presets.end()) {
                    it->second.first = LSFString(name);
                    responseCode = LSF_OK;
                    nameChanged = true;
                }
                status = presetsLock.Unlock();
                if (ER_OK != status) {
                    QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __FUNCTION__));
                }
            } else {
                responseCode = LSF_ERR_BUSY;
                QCC_LogError(status, ("%s: presetsLock.Lock() failed", __FUNCTION__));
            }
        }
    }

    controllerService.SendMethodReplyWithResponseCodeIDAndName(msg, responseCode, presetID, language);

    if (nameChanged) {
        ScheduleFileUpdate();
        LSFStringList idList;
        idList.push_back(presetID);
        controllerService.SendSignal(interfaceName, "PresetsNameChanged", idList);
    }
}

void PresetManager::CreatePreset(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_OK;

    bool created = false;
    LSFString presetID;

    const ajn::MsgArg* inputArgs;
    size_t numInputArgs;
    msg->GetArgs(numInputArgs, inputArgs);

    QStatus status = presetsLock.Lock();
    if (ER_OK == status) {
        if (presets.size() < MAX_SUPPORTED_NUM_LSF_ENTITY) {
            presetID = GenerateUniqueID("PRESET");
            LampState preset(inputArgs[0]);
            presets[presetID].first = presetID;
            presets[presetID].second = preset;
            created = true;
        } else {
            QCC_LogError(ER_FAIL, ("%s: No slot for new Preset", __FUNCTION__));
            responseCode = LSF_ERR_NO_SLOT;
        }
        status = presetsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: presetsLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, presetID);

    if (created) {
        ScheduleFileUpdate();
        LSFStringList idList;
        idList.push_back(presetID);
        controllerService.SendSignal(interfaceName, "PresetsCreated", idList);
    }
}

void PresetManager::UpdatePreset(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool updated = false;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* presetId;
    args[0].Get("s", &presetId);

    LSFString presetID(presetId);
    LampState preset(args[1]);

    QStatus status = presetsLock.Lock();
    if (ER_OK == status) {
        PresetMap::iterator it = presets.find(presetId);
        if (it != presets.end()) {
            presets[presetID].second = preset;
            responseCode = LSF_OK;
            updated = true;
        }
        status = presetsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: presetsLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, presetID);

    if (updated) {
        ScheduleFileUpdate();
        LSFStringList idList;
        idList.push_back(presetID);
        controllerService.SendSignal(interfaceName, "PresetsUpdated", idList);
    }
}

void PresetManager::DeletePreset(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_OK;

    bool deleted = false;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* presetId;
    args[0].Get("s", &presetId);

    LSFString presetID(presetId);

    responseCode = sceneManagerPtr->IsDependentOnPreset(presetID);

    if (LSF_OK == responseCode) {
        QStatus status = presetsLock.Lock();
        if (ER_OK == status) {
            PresetMap::iterator it = presets.find(presetId);
            if (it != presets.end()) {
                presets.erase(it);
                deleted = true;
            } else {
                responseCode = LSF_ERR_NOT_FOUND;
            }
            status = presetsLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: presetsLock.Unlock() failed", __FUNCTION__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: presetsLock.Lock() failed", __FUNCTION__));
        }
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, presetID);

    if (deleted) {
        ScheduleFileUpdate();
        LSFStringList idList;
        idList.push_back(presetID);
        controllerService.SendSignal(interfaceName, "PresetsDeleted", idList);
    }
}

void PresetManager::GetPreset(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    MsgArg outArgs[3];

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

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
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

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
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_BUSY;

    const ajn::MsgArg* inputArg;
    size_t numInputArgs;
    msg->GetArgs(numInputArgs, inputArg);

    MsgArg arg;

    LampState preset(*inputArg);
    if (LSF_OK == SetDefaultLampStateInternal(preset)) {
        responseCode = LSF_OK;
    }

    controllerService.SendMethodReplyWithUint32Value(msg, (uint32_t &)responseCode);
}

LSFResponseCode PresetManager::GetDefaultLampStateInternal(LampState& preset)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    LSFResponseCode responseCode = LSF_ERR_FAILURE;
    QStatus status = defaultLampStateLock.Lock();
    if (ER_OK == status) {
        preset = defaultLampState;
        status = defaultLampStateLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: defaultLampStateLock.Unlock() failed", __FUNCTION__));
        }
        responseCode = LSF_OK;
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: defaultLampStateLock.Lock() failed", __FUNCTION__));
    }
    return responseCode;
}

LSFResponseCode PresetManager::SetDefaultLampStateInternal(LampState& preset)
{
    QCC_DbgPrintf(("%s: preset=%s", __FUNCTION__, preset.c_str()));
    LSFResponseCode responseCode = LSF_OK;
    QStatus tempStatus = defaultLampStateLock.Lock();
    if (ER_OK == tempStatus) {
        /*
         * Set the defaultLampState to factory settings
         * TODO: Provision for factory setting
         */
        defaultLampState = preset;
        tempStatus = defaultLampStateLock.Unlock();
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: defaultLampStateLock.Unlock() failed", __FUNCTION__));
        }

        /*
         * Send the DefaultLampStateChangedSignal
         */
        QCC_DbgPrintf(("%s: Sending the DefaultLampStateChangedSignal", __FUNCTION__));
        tempStatus = controllerService.SendSignalWithoutArg(interfaceName, "DefaultLampStateChanged");
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: Unable to send DefaultLampStateChanged signal", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(tempStatus, ("%s: defaultLampStateLock.Lock() failed", __FUNCTION__));
    }

    return responseCode;
}

// never call this when the ControllerService is up; it isn't thread-safe!
// Presets follow the form:
// (Preset id "name" on/off hue saturation colortemp brightness)*
void PresetManager::ReadSavedData()
{
    if (filePath.empty()) {
        return;
    }

    std::ifstream stream(filePath.c_str());
    if (!stream.is_open()) {
        QCC_DbgPrintf(("File not found: %s\n", filePath.c_str()));
        return;
    }

    while (!stream.eof()) {
        std::string type = ParseString(stream);

        // keep searching for "Preset", which indicates the beginning of a saved Preset state
        if (type == "Preset") {
            std::string presetId = ParseString(stream);
            std::string presetName = ParseString(stream);

            //printf("SavedState id=%s, name=[%s]\n", id.c_str(), name.c_str());
            LampState state;
            ParseLampState(stream, state);

            std::pair<LSFString, LampState> thePair(presetName, state);
            presets[presetId] = thePair;
        }
    }

    stream.close();
}

void PresetManager::WriteFile()
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    if (!updated) {
        return;
    }

    if (filePath.empty()) {
        return;
    }

    presetsLock.Lock();

    // we can't hold this lock for the entire time!
    PresetMap mapCopy = presets;
    updated = false;
    presetsLock.Unlock();



    std::ofstream stream(filePath.c_str(), std::ios_base::out);
    if (!stream.is_open()) {
        QCC_DbgPrintf(("File not found: %s\n", filePath.c_str()));
        return;
    }

    // // (Preset id "name" on/off hue saturation colortemp brightness)*
    for (PresetMap::const_iterator it = mapCopy.begin(); it != mapCopy.end(); ++it) {
        const LSFString& id = it->first;
        const LSFString& name = it->second.first;
        const LampState& state = it->second.second;

        stream << "Preset " << id << " \"" << name << "\" "
               << (state.onOff ? 1 : 0) << ' '
               << state.hue << ' ' << state.saturation << ' '
               << state.colorTemp << ' ' << state.brightness << '\n';
    }

    stream.close();
}

uint32_t PresetManager::GetControllerPresetInterfaceVersion(void)
{
    QCC_DbgPrintf(("%s: controllerPresetInterfaceVersion=%d", __FUNCTION__, controllerPresetInterfaceVersion));
    return controllerPresetInterfaceVersion;
}
