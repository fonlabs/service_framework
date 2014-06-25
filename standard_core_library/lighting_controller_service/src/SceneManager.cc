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
#include <OEM_CS_Config.h>
#include <FileParser.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "SCENE_MANAGER"

SceneManager::SceneManager(ControllerService& controllerSvc, LampGroupManager& lampGroupMgr, MasterSceneManager* masterSceneMgr, const std::string& sceneFile) :
    Manager(controllerSvc, sceneFile), lampGroupManager(lampGroupMgr), masterSceneManager(masterSceneMgr)
{
}

LSFResponseCode SceneManager::GetAllScenes(SceneMap& sceneMap)
{
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = scenesLock.Lock();
    if (ER_OK == status) {
        sceneMap = scenes;
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
            tempStatus = controllerService.SendSignal(ControllerServiceSceneInterfaceName, "ScenesDeleted", scenesList);
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

void SceneManager::GetAllSceneIDs(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));

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

    controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(message, responseCode, idList);
}

void SceneManager::GetSceneName(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    LSFString name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    LSFString sceneID(uniqueId);

    LSFString language = static_cast<LSFString>(args[1].v_string.str);
    if (0 != strcmp("en", language.c_str())) {
        QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __FUNCTION__, language.c_str()));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else {
        QStatus status = scenesLock.Lock();
        if (ER_OK == status) {
            SceneMap::iterator it = scenes.find(uniqueId);
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

    controllerService.SendMethodReplyWithResponseCodeIDLanguageAndName(message, responseCode, sceneID, language, name);
}

void SceneManager::SetSceneName(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool nameChanged = false;
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    LSFString sceneID(uniqueId);

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
                SceneMap::iterator it = scenes.find(uniqueId);
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

    controllerService.SendMethodReplyWithResponseCodeIDAndName(message, responseCode, sceneID, language);

    if (nameChanged) {
        ScheduleFileUpdate();
        LSFStringList idList;
        idList.push_back(sceneID);
        controllerService.SendSignal(ControllerServiceSceneInterfaceName, "ScenesNameChanged", idList);
    }
}

void SceneManager::CreateScene(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));

    LSFResponseCode responseCode = LSF_OK;

    bool created = false;
    LSFString sceneID;

    const ajn::MsgArg* inputArgs;
    size_t numInputArgs;
    message->GetArgs(numInputArgs, inputArgs);

    QStatus status = scenesLock.Lock();
    if (ER_OK == status) {
        if (scenes.size() < MAX_SUPPORTED_NUM_LSF_ENTITY) {
            sceneID = GenerateUniqueID("SCENE");
            Scene scene(inputArgs[0], inputArgs[1], inputArgs[2], inputArgs[3]);
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

    controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, sceneID);

    if (created) {
        ScheduleFileUpdate();
        LSFStringList idList;
        idList.push_back(sceneID);
        controllerService.SendSignal(ControllerServiceSceneInterfaceName, "ScenesCreated", idList);
    }
}

void SceneManager::UpdateScene(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool updated = false;

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    LSFString sceneID(uniqueId);
    Scene scene(args[1], args[2], args[3], args[4]);

    QStatus status = scenesLock.Lock();
    if (ER_OK == status) {
        SceneMap::iterator it = scenes.find(uniqueId);
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

    controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, sceneID);

    if (updated) {
        ScheduleFileUpdate();
        LSFStringList idList;
        idList.push_back(sceneID);
        controllerService.SendSignal(ControllerServiceSceneInterfaceName, "ScenesUpdated", idList);
    }
}

void SceneManager::DeleteScene(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    LSFResponseCode responseCode = LSF_OK;

    bool deleted = false;

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

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

    controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, sceneID);

    if (deleted) {
        ScheduleFileUpdate();
        LSFStringList idList;
        idList.push_back(sceneID);
        controllerService.SendSignal(ControllerServiceSceneInterfaceName, "ScenesDeleted", idList);
    }
}

void SceneManager::GetScene(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    MsgArg outArgs[6];

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    QStatus status = scenesLock.Lock();
    if (ER_OK == status) {
        SceneMap::iterator it = scenes.find(uniqueId);
        if (it != scenes.end()) {
            it->second.second.Get(&outArgs[2], &outArgs[3], &outArgs[4], &outArgs[5]);
            responseCode = LSF_OK;
        } else {
            outArgs[2].Set("a(asasa{sv}u)", 0, NULL);
            outArgs[3].Set("a(asassu)", 0, NULL);
            outArgs[4].Set("a(asasa{sv}a{sv}uuu)", 0, NULL);
            outArgs[5].Set("a(asasssuuu)", 0, NULL);
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
    outArgs[1].Set("s", uniqueId);

    controllerService.SendMethodReply(message, outArgs, 6);
}

void SceneManager::ApplyScene(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    LSFResponseCode responseCode = LSF_OK;

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    const char* sceneId;
    args[0].Get("s", &sceneId);

    LSFString sceneID(sceneId);

    QCC_DbgPrintf(("%s: sceneID=%s", __FUNCTION__, sceneId));

    LSFStringList scenes;
    scenes.push_back(sceneID);

    responseCode = ApplySceneInternal(message, scenes);

    if (LSF_OK == responseCode) {
        controllerService.SendSignal(ControllerServiceSceneInterfaceName, "ScenesApplied", scenes);
    } else {
        controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, sceneID);
    }
}

LSFResponseCode SceneManager::ApplySceneInternal(ajn::Message message, LSFStringList& sceneList)
{
    QCC_DbgPrintf(("%s: sceneList.size() = %d", __FUNCTION__, sceneList.size()));
    LSFResponseCode responseCode = LSF_OK;
    bool invokeChangeState = false;

    uint8_t notfound = 0;

    std::list<Scene> sceneInfoList;
    sceneInfoList.clear();

    QStatus status = scenesLock.Lock();
    if (ER_OK == status) {
        while (sceneList.size()) {
            LSFString sceneId = sceneList.front();
            SceneMap::iterator it = scenes.find(sceneId);
            if (it != scenes.end()) {
                QCC_DbgPrintf(("%s: Found sceneID=%s", __FUNCTION__, sceneId.c_str()));
                sceneInfoList.push_back(it->second.second);
            } else {
                QCC_DbgPrintf(("%s: Scene %s not found", __FUNCTION__, sceneId.c_str()));
                notfound++;
            }
            sceneList.pop_front();
        }
        status = scenesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: scenesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: scenesLock.Lock() failed", __FUNCTION__));
    }

    TransitionLampsLampGroupsToStateList transitionToStateComponent;
    TransitionLampsLampGroupsToPresetList transitionToPresetComponent;
    PulseLampsLampGroupsWithStateList pulseWithStateComponent;
    PulseLampsLampGroupsWithPresetList pulseWithPresetComponent;

    if (sceneInfoList.size() == 0) {
        responseCode = LSF_ERR_NOT_FOUND;
    } else {
        if (notfound) {
            responseCode = LSF_ERR_PARTIAL;
        }

        while (sceneInfoList.size()) {
            Scene scene = sceneInfoList.front();
            for (TransitionLampsLampGroupsToStateList::iterator it = scene.transitionToStateComponent.begin(); it != scene.transitionToStateComponent.end(); it++) {
                transitionToStateComponent.push_back(*it);
            }
            for (TransitionLampsLampGroupsToPresetList::iterator it = scene.transitionToPresetComponent.begin(); it != scene.transitionToPresetComponent.end(); it++) {
                transitionToPresetComponent.push_back(*it);
            }
            for (PulseLampsLampGroupsWithStateList::iterator it = scene.pulseWithStateComponent.begin(); it != scene.pulseWithStateComponent.end(); it++) {
                pulseWithStateComponent.push_back(*it);
            }
            for (PulseLampsLampGroupsWithPresetList::iterator it = scene.pulseWithPresetComponent.begin(); it != scene.pulseWithPresetComponent.end(); it++) {
                pulseWithPresetComponent.push_back(*it);
            }
            sceneInfoList.pop_front();
            invokeChangeState = true;
        }

        if (invokeChangeState) {
            LSFResponseCode tempResponseCode = lampGroupManager.ChangeLampGroupStateAndField(message, transitionToStateComponent, transitionToPresetComponent, pulseWithStateComponent, pulseWithPresetComponent);
            if (tempResponseCode != LSF_OK) {
                responseCode = LSF_ERR_FAILURE;
            }
        }
    }

    return responseCode;
}

void SceneManager::ReadSavedData()
{
    std::istringstream stream;
    if (!ValidateFileAndRead(stream)) {
        return;
    }

    while (!stream.eof()) {
        std::string token;
        std::string id;
        std::string name;
        LSFStringList subScenes;
        Scene scene;

        token = ParseString(stream);
        if (token == "Scene") {
            id = ParseString(stream);
            name = ParseString(stream);

            do {
                token = ParseString(stream);
                if (token == "TransitionLampsLampGroupsToState") {
                    LSFStringList lampList;
                    LSFStringList lampGroupList;
                    LampState lampState;
                    uint32_t period = 0;

                    do {
                        token = ParseString(stream);
                        if (token == "Lamp") {
                            std::string id = ParseString(stream);
                            lampList.push_back(id);
                        } else if (token == "LampGroup") {
                            std::string id = ParseString(stream);
                            lampGroupList.push_back(id);
                        } else if (token == "LampState") {
                            ParseLampState(stream, lampState);
                        } else if (token == "Period") {
                            period = ParseValue<uint32_t>(stream);
                        }
                    } while (token != "EndTransitionLampsLampGroupsToState");
                    scene.transitionToStateComponent.push_back(TransitionLampsLampGroupsToState(lampList, lampGroupList, lampState, period));
                } else if (token == "TransitionLampsLampGroupsToPreset") {
                    LSFStringList lampList;
                    LSFStringList lampGroupList;
                    LSFString preset;
                    uint32_t period = 0;

                    do {
                        token = ParseString(stream);

                        if (token == "Lamp") {
                            std::string id = ParseString(stream);
                            lampList.push_back(id);
                        } else if (token == "LampGroup") {
                            std::string id = ParseString(stream);
                            lampGroupList.push_back(id);
                        } else if (token == "LampState") {
                            preset = ParseString(stream);
                        } else if (token == "Period") {
                            period = ParseValue<uint32_t>(stream);
                        }
                    } while (token != "EndTransitionLampsLampGroupsToPreset");
                    scene.transitionToPresetComponent.push_back(TransitionLampsLampGroupsToPreset(lampList, lampGroupList, preset, period));
                } else if (token == "PulseLampsLampGroupsWithState") {
                    LSFStringList lampList;
                    LSFStringList lampGroupList;
                    LampState fromState, toState;
                    uint32_t period = 0, duration = 0, pulses = 0;

                    do {
                        token = ParseString(stream);
                        if (token == "Lamp") {
                            std::string id = ParseString(stream);
                            lampList.push_back(id);
                        } else if (token == "LampGroup") {
                            std::string id = ParseString(stream);
                            lampGroupList.push_back(id);
                        } else if (token == "FromState") {
                            ParseLampState(stream, fromState);
                        } else if (token == "ToState") {
                            ParseLampState(stream, toState);
                        } else if (token == "Period") {
                            period = ParseValue<uint32_t>(stream);
                        } else if (token == "Duration") {
                            duration = ParseValue<uint32_t>(stream);
                        } else if (token == "Pulses") {
                            pulses = ParseValue<uint32_t>(stream);
                        }
                    } while (token != "EndPulseLampsLampGroupsWithState");
                    scene.pulseWithStateComponent.push_back(PulseLampsLampGroupsWithState(lampList, lampGroupList, fromState, toState, period, duration, pulses));
                } else if (token == "PulseLampsLampGroupsWithPreset") {
                    LSFStringList lampList;
                    LSFStringList lampGroupList;
                    LSFString fromState, toState;
                    uint32_t period = 0, duration = 0, pulses = 0;

                    do {
                        token = ParseString(stream);
                        if (token == "Lamp") {
                            std::string id = ParseString(stream);
                            lampList.push_back(id);
                        } else if (token == "LampGroup") {
                            std::string id = ParseString(stream);
                            lampGroupList.push_back(id);
                        } else if (token == "FromState") {
                            fromState = ParseString(stream);
                        } else if (token == "ToState") {
                            toState = ParseString(stream);
                        } else if (token == "Period") {
                            period = ParseValue<uint32_t>(stream);
                        } else if (token == "Duration") {
                            duration = ParseValue<uint32_t>(stream);
                        } else if (token == "Pulses") {
                            pulses = ParseValue<uint32_t>(stream);
                        }
                    } while (token != "EndPulseLampsLampGroupsWithPreset");
                    scene.pulseWithPresetComponent.push_back(PulseLampsLampGroupsWithPreset(lampList, lampGroupList, fromState, toState, period, duration, pulses));
                }
            } while (token != "EndScene");

            std::pair<LSFString, Scene> thePair(name, scene);
            scenes[id] = thePair;
        }
    }
}

static void OutputLamps(std::ostream& stream, const std::string& name, const LSFStringList& list)
{
    for (LSFStringList::const_iterator it = list.begin(); it != list.end(); ++it) {
        stream << "\t\t" << name << ' ' << *it << '\n';
    }
}

static void OutputState(std::ostream& stream, const std::string& name, const LampState& state)
{
    stream << "\t\t" << name << ' '
           << (state.onOff ? 1 : 0) << ' '
           << state.hue << ' ' << state.saturation << ' '
           << state.colorTemp << ' ' << state.brightness << '\n';
}

std::string SceneManager::GetString(const SceneMap& items)
{
    std::ostringstream stream;
    for (SceneMap::const_iterator it = items.begin(); it != items.end(); ++it) {
        const LSFString& id = it->first;
        const LSFString& name = it->second.first;
        const Scene& scene = it->second.second;

        stream << "Scene " << id << " \"" << name << "\"\n";

        // TransitionLampsLampGroupsToState
        if (!scene.transitionToStateComponent.empty()) {
            stream << "\tTransitionLampsLampGroupsToState\n";
            for (TransitionLampsLampGroupsToStateList::const_iterator cit = scene.transitionToStateComponent.begin(); cit != scene.transitionToStateComponent.end(); ++cit) {
                const TransitionLampsLampGroupsToState& comp = *cit;

                OutputLamps(stream, "Lamp", comp.lamps);
                OutputLamps(stream, "LampGroup", comp.lampGroups);
                OutputState(stream, "LampState", comp.state);
                stream << "\t\tPeriod " << comp.transitionPeriod << '\n';
            }

            stream << "\tEndTransitionLampsLampGroupsToState\n";
        }

        if (!scene.transitionToPresetComponent.empty()) {
            stream << "\tTransitionLampsLampGroupsToPreset\n";
            for (TransitionLampsLampGroupsToPresetList::const_iterator cit = scene.transitionToPresetComponent.begin(); cit != scene.transitionToPresetComponent.end(); ++cit) {
                const TransitionLampsLampGroupsToPreset& comp = *cit;

                OutputLamps(stream, "Lamp", comp.lamps);
                OutputLamps(stream, "LampGroup", comp.lampGroups);
                stream << "\t\tLampState " << comp.presetID << "\n\t\tPeriod " << comp.transitionPeriod << '\n';
            }

            stream << "\tEndTransitionLampsLampGroupsToPreset\n";
        }


        if (!scene.pulseWithStateComponent.empty()) {
            stream << "\tPulseLampsLampGroupsWithState\n";
            for (PulseLampsLampGroupsWithStateList::const_iterator cit = scene.pulseWithStateComponent.begin(); cit != scene.pulseWithStateComponent.end(); ++cit) {
                const PulseLampsLampGroupsWithState& comp = *cit;

                OutputLamps(stream, "Lamp", comp.lamps);
                OutputLamps(stream, "LampGroup", comp.lampGroups);
                OutputState(stream, "FromState", comp.fromState);
                OutputState(stream, "ToState", comp.toState);
                stream << "\t\tPeriod " << comp.pulsePeriod << "\n\t\tDuration " << comp.pulseDuration << "\n\t\tPulses " << comp.numPulses << '\n';
            }

            stream << "\tEndPulseLampsLampGroupsWithState\n";
        }

        if (!scene.pulseWithPresetComponent.empty()) {
            stream << "\tPulseLampsLampGroupsWithPreset\n";
            for (PulseLampsLampGroupsWithPresetList::const_iterator cit = scene.pulseWithPresetComponent.begin(); cit != scene.pulseWithPresetComponent.end(); ++cit) {
                const PulseLampsLampGroupsWithPreset& comp = *cit;

                OutputLamps(stream, "Lamp", comp.lamps);
                OutputLamps(stream, "LampGroup", comp.lampGroups);
                stream << "\t\tFromState " << comp.fromPreset << "\n\t\tToState " << comp.toPreset
                       << "\n\t\tPeriod " << comp.pulsePeriod << "\n\t\tDuration " << comp.pulseDuration << "\n\t\tPulses " << comp.numPulses << '\n';
            }

            stream << "\tEndPulseLampsLampGroupsWithPreset\n";
        }
    }

    stream << "EndScene\n";
    return stream.str();
}

std::string SceneManager::GetString()
{
    scenesLock.Lock();
    // we can't hold this lock for the entire time!
    SceneMap mapCopy = scenes;
    updated = false;
    scenesLock.Unlock();
    return GetString(mapCopy);
}

void SceneManager::WriteFile()
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
    controllerService.SendBlobUpdate(LSF_SCENE, checksum, 0UL);
}

uint32_t SceneManager::GetControllerServiceSceneInterfaceVersion(void)
{
    QCC_DbgPrintf(("%s: controllerSceneInterfaceVersion=%d", __FUNCTION__, ControllerServiceSceneInterfaceVersion));
    return ControllerServiceSceneInterfaceVersion;
}
