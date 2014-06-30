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

#include <LampManager.h>
#include <LSFResponseCodes.h>
#include <OEM_CS_Config.h>

#include <ControllerService.h>

#include <alljoyn/Status.h>
#include <qcc/Debug.h>

#include <algorithm>


using namespace lsf;
using namespace ajn;

#define QCC_MODULE "LAMP_MANAGER"

LampManager::LampManager(ControllerService& controllerSvc, PresetManager& presetMgr)
    : Manager(controllerSvc), lampClients(controllerSvc), presetManager(presetMgr)
{

}

LampManager::~LampManager()
{

}

QStatus LampManager::Start(const char* keyStoreFileLocation)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    /*
     * Start the Lamp Clients
     */
    QStatus status = lampClients.Start(keyStoreFileLocation);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to start the Lamp Clients", __FUNCTION__));
    }

    return status;
}

QStatus LampManager::Stop(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    return lampClients.Join();
}

void LampManager::GetAllLampIDs(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));

    lampClients.RequestAllLampIDs(message);
}

void LampManager::GetLampFaults(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    QCC_DbgPrintf(("lampID=%s", lampID.c_str()));

    lampClients.GetLampFaults(lampID, message);
}

void LampManager::ClearLampFault(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    LampFaultCode faultCode = static_cast<LampFaultCode>(args[1].v_uint32);
    QCC_DbgPrintf(("lampID=%s faultCode=%d", lampID.c_str(), faultCode));

    lampClients.ClearLampFault(lampID, faultCode, message);
}

void LampManager::GetLampServiceVersion(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    QCC_DbgPrintf(("lampID=%s", lampID.c_str()));

    lampClients.GetLampVersion(lampID, message);
}

void LampManager::GetLampSupportedLanguages(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    QCC_DbgPrintf(("lampID=%s", lampID.c_str()));

    lampClients.GetLampSupportedLanguages(lampID, message);
}

void LampManager::GetLampManufacturer(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    LSFString language = static_cast<LSFString>(args[1].v_string.str);
    QCC_DbgPrintf(("lampID=%s language=%s", lampID.c_str(), language.c_str()));

    lampClients.GetLampManufacturer(lampID, language, message);
}

void LampManager::GetLampName(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    LSFString language = static_cast<LSFString>(args[1].v_string.str);

    QCC_DbgPrintf(("lampID=%s language=%s", lampID.c_str(), language.c_str()));

    lampClients.GetLampName(lampID, language, message);
}

void LampManager::SetLampName(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    LSFString lampName = static_cast<LSFString>(args[1].v_string.str);
    LSFString language = static_cast<LSFString>(args[2].v_string.str);

    QCC_DbgPrintf(("lampID=%s lampName=%s language=%s", lampID.c_str(), lampName.c_str(), language.c_str()));

    lampClients.SetLampName(lampID, lampName, language, message);
}

void LampManager::GetLampDetails(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    QCC_DbgPrintf(("lampID=%s", lampID.c_str()));

    lampClients.GetLampDetails(lampID, message);
}

void LampManager::GetLampParameters(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    QCC_DbgPrintf(("lampID=%s", lampID.c_str()));


    lampClients.GetLampParameters(lampID, message);
}

void LampManager::GetLampParametersField(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    LSFString fieldName = static_cast<LSFString>(args[1].v_string.str);
    QCC_DbgPrintf(("lampID=%s fieldName=%s", lampID.c_str(), fieldName.c_str()));

    lampClients.GetLampParametersField(lampID, fieldName, message);
}

void LampManager::GetLampState(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    QCC_DbgPrintf(("lampID=%s", lampID.c_str()));

    lampClients.GetLampState(lampID, message);
}

void LampManager::GetLampStateField(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    LSFString fieldName = static_cast<LSFString>(args[1].v_string.str);
    QCC_DbgPrintf(("lampID=%s fieldName=%s", lampID.c_str(), fieldName.c_str()));

    lampClients.GetLampStateField(lampID, fieldName, message);
}

void LampManager::TransitionLampStateToPreset(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    LSFString presetID = static_cast<LSFString>(args[1].v_string.str);
    uint32_t transitionPeriod = static_cast<uint32_t>(args[2].v_uint32);
    QCC_DbgPrintf(("lampID=%s presetID=%s transitionPeriod=%d", lampID.c_str(), presetID.c_str(), transitionPeriod));

    LSFStringList lampList;
    lampList.push_back(lampID);

    LampsAndStateList transitionToState;
    LampsAndPresetList transitionToPreset;
    LampsAndStateFieldList stateField;
    PulseLampsWithStateList pulseWithState;
    PulseLampsWithPresetList pulseWithPreset;

    LampsAndPreset transitionToPresetComponent(lampList, presetID, transitionPeriod);
    transitionToPreset.push_back(transitionToPresetComponent);

    ChangeLampStateAndField(message, transitionToState, transitionToPreset, stateField, pulseWithState, pulseWithPreset);
}

void LampManager::TransitionLampStateField(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    LSFString fieldName = static_cast<LSFString>(args[1].v_string.str);
    MsgArg* varArg;
    args[2].Get("v", &varArg);
    uint32_t transitionPeriod = static_cast<uint32_t>(args[3].v_uint32);
    QCC_DbgPrintf(("lampID=%s fieldName=%s transitionPeriod=%d", lampID.c_str(), fieldName.c_str(), transitionPeriod));

    LSFStringList lampList;
    lampList.push_back(lampID);

    LampsAndStateList transitionToState;
    LampsAndPresetList transitionToPreset;
    LampsAndStateFieldList stateField;
    PulseLampsWithStateList pulseWithState;
    PulseLampsWithPresetList pulseWithPreset;

    LampsAndStateField stateFieldComponent(lampList, fieldName, *varArg, transitionPeriod);
    stateField.push_back(stateFieldComponent);

    ChangeLampStateAndField(message, transitionToState, transitionToPreset, stateField, pulseWithState, pulseWithPreset);
}

void LampManager::ResetLampState(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    QCC_DbgPrintf(("lampID=%s", lampID.c_str()));

    LSFStringList lampList;
    lampList.push_back(lampID);

    ResetLampStateInternal(message, lampList);
}

void LampManager::ResetLampStateField(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    LSFString fieldName = static_cast<LSFString>(args[1].v_string.str);
    QCC_DbgPrintf(("%s: lampID=%s fieldName=%s", __FUNCTION__, lampID.c_str(), fieldName.c_str()));

    LSFStringList lampList;
    lampList.push_back(lampID);

    ResetLampStateFieldInternal(message, lampList, fieldName);
}

void LampManager::ResetLampStateInternal(ajn::Message& message, LSFStringList lamps, bool groupOperation)
{
    LampState defaultLampState;

    QCC_DbgPrintf(("%s", __FUNCTION__));

    LSFResponseCode responseCode = presetManager.GetDefaultLampStateInternal(defaultLampState);

    LampsAndStateList transitionToState;
    LampsAndPresetList transitionToPreset;
    LampsAndStateFieldList stateField;
    PulseLampsWithStateList pulseWithState;
    PulseLampsWithPresetList pulseWithPreset;

    if (LSF_OK == responseCode) {
        LampsAndState transitionToStateComponent(lamps, defaultLampState, 0);
        transitionToState.push_back(transitionToStateComponent);
        ChangeLampStateAndField(message, transitionToState, transitionToPreset, stateField, pulseWithState, pulseWithPreset, groupOperation);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Error getting the default lamp state", __FUNCTION__));
        if (groupOperation) {
            size_t numArgs;
            const MsgArg* args;
            message->GetArgs(numArgs, args);
            LSFString lampGroupId = static_cast<LSFString>(args[0].v_string.str);
            controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, lampGroupId);
        } else {
            controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, lamps.front());
        }
    }
}

void LampManager::ResetLampStateFieldInternal(ajn::Message& message, LSFStringList lamps, LSFString stateFieldName, bool groupOperation)
{
    LampState defaultLampState;

    QCC_DbgPrintf(("%s", __FUNCTION__));

    LSFResponseCode responseCode = presetManager.GetDefaultLampStateInternal(defaultLampState);
    MsgArg arg;

    LampsAndStateList transitionToState;
    LampsAndPresetList transitionToPreset;
    LampsAndStateFieldList stateField;
    PulseLampsWithStateList pulseWithState;
    PulseLampsWithPresetList pulseWithPreset;

    if (LSF_OK == responseCode) {
        QCC_DbgPrintf(("%s: defaultLampState=%s", __FUNCTION__, defaultLampState.c_str()));
        if (0 == strcmp(stateFieldName.c_str(), "OnOff")) {
            arg.Set("b", defaultLampState.onOff);
        } else if (0 == strcmp(stateFieldName.c_str(), "Hue")) {
            arg.Set("u", defaultLampState.hue);
        } else if (0 == strcmp(stateFieldName.c_str(), "Saturation")) {
            arg.Set("u", defaultLampState.saturation);
        } else if (0 == strcmp(stateFieldName.c_str(), "Brightness")) {
            arg.Set("u", defaultLampState.brightness);
        } else if (0 == strcmp(stateFieldName.c_str(), "ColorTemp")) {
            arg.Set("u", defaultLampState.colorTemp);
        }

        LampsAndStateField stateFieldComponent(lamps, stateFieldName, arg, 0);
        stateField.push_back(stateFieldComponent);
        ChangeLampStateAndField(message, transitionToState, transitionToPreset, stateField, pulseWithState, pulseWithPreset, groupOperation);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Error getting the default lamp state", __FUNCTION__));
        controllerService.SendMethodReplyWithResponseCodeIDAndName(message, responseCode, lamps.front(), stateFieldName);
        if (groupOperation) {
            size_t numArgs;
            const MsgArg* args;
            message->GetArgs(numArgs, args);
            LSFString lampGroupId = static_cast<LSFString>(args[0].v_string.str);
            controllerService.SendMethodReplyWithResponseCodeIDAndName(message, responseCode, lampGroupId, stateFieldName);
        } else {
            controllerService.SendMethodReplyWithResponseCodeIDAndName(message, responseCode, lamps.front(), stateFieldName);
        }
    }
}

void LampManager::ChangeLampStateAndField(Message& message,
                                          LampsAndStateList& transitionToStateComponent,
                                          LampsAndPresetList& transitionToPresetComponent,
                                          LampsAndStateFieldList& stateFieldComponent,
                                          PulseLampsWithStateList& pulseWithStateComponent,
                                          PulseLampsWithPresetList& pulseWithPresetComponent,
                                          bool groupOperation,
                                          bool sceneOperation)
{
    LSFResponseCode responseCode = LSF_ERR_FAILURE;

    QCC_DbgPrintf(("%s", __FUNCTION__));

    uint64_t timestamp = 0;
    GetSyncTimeStamp(timestamp);

    TransitionStateParamsList stateParamsList;
    TransitionStateFieldParamsList stateFieldParamsList;
    PulseStateParamsList pulseParamsList;

    stateParamsList.clear();
    stateFieldParamsList.clear();
    pulseParamsList.clear();

    while (transitionToStateComponent.size()) {
        LampsAndState transitionToStateComp = transitionToStateComponent.front();
        MsgArg state;
        transitionToStateComp.state.Get(&state);
        QCC_DbgPrintf(("%s: Applying transitionToStateComponent", __FUNCTION__));
        TransitionStateParams params(transitionToStateComp.lamps, timestamp, state, transitionToStateComp.transitionPeriod);
        stateParamsList.push_back(params);
        transitionToStateComponent.pop_front();
    }

    while (transitionToPresetComponent.size()) {
        LampsAndPreset transitionToPresetComp = transitionToPresetComponent.front();
        LampState preset;
        responseCode = presetManager.GetPresetInternal(transitionToPresetComp.presetID, preset);
        if (LSF_OK == responseCode) {
            MsgArg state;
            preset.Get(&state);
            QCC_DbgPrintf(("%s: Applying transitionToPresetComponent", __FUNCTION__));
            TransitionStateParams params(transitionToPresetComp.lamps, timestamp, state, transitionToPresetComp.transitionPeriod);
            stateParamsList.push_back(params);
        } else {
            if (groupOperation || (sceneOperation && (0 == strcmp(ControllerServiceSceneInterfaceName, message->GetInterface())))) {
                size_t numArgs;
                const MsgArg* args;
                message->GetArgs(numArgs, args);
                LSFString uniqueID = static_cast<LSFString>(args[0].v_string.str);
                controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, uniqueID);
            } else {
                controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, transitionToPresetComp.lamps.front());
            }
            return;
        }
        transitionToPresetComponent.pop_front();
    }

    while (stateFieldComponent.size()) {
        LampsAndStateField stateFieldComp = stateFieldComponent.front();
        QCC_DbgPrintf(("%s: Applying stateFieldComponent", __FUNCTION__));
        TransitionStateFieldParams params(stateFieldComp.lamps, timestamp, stateFieldComp.stateFieldName.c_str(),
                                          stateFieldComp.stateFieldValue, stateFieldComp.transitionPeriod);
        stateFieldParamsList.push_back(params);
        stateFieldComponent.pop_front();
    }

    while (pulseWithStateComponent.size()) {
        QCC_DbgPrintf(("%s: Applying pulseWithStateComponent", __FUNCTION__));
        PulseLampsWithState pulseWithStateComp = pulseWithStateComponent.front();
        MsgArg fromState;
        MsgArg toState;
        pulseWithStateComp.fromState.Get(&fromState);
        pulseWithStateComp.toState.Get(&toState);
        PulseStateParams params(pulseWithStateComp.lamps, fromState, toState, pulseWithStateComp.period, pulseWithStateComp.duration,
                                pulseWithStateComp.numPulses, timestamp);
        pulseParamsList.push_back(params);
        pulseWithStateComponent.pop_front();
    }

    while (pulseWithPresetComponent.size()) {
        PulseLampsWithPreset pulseWithPresetComp = pulseWithPresetComponent.front();
        LampState fromPreset;
        LampState toPreset;
        responseCode = presetManager.GetPresetInternal(pulseWithPresetComp.fromPreset, fromPreset);
        if (LSF_OK == responseCode) {
            responseCode = presetManager.GetPresetInternal(pulseWithPresetComp.toPreset, toPreset);
            if (LSF_OK == responseCode) {
                MsgArg fromState;
                MsgArg toState;
                fromPreset.Get(&fromState);
                toPreset.Get(&toState);
                QCC_DbgPrintf(("%s: Applying pulseWithPresetComponent", __FUNCTION__));
                PulseStateParams params(pulseWithPresetComp.lamps, fromState, toState, pulseWithPresetComp.period, pulseWithPresetComp.duration,
                                        pulseWithPresetComp.numPulses, timestamp);
                pulseParamsList.push_back(params);
            }
        } else {
            if (groupOperation || (sceneOperation && (0 == strcmp(ControllerServiceSceneInterfaceName, message->GetInterface())))) {
                size_t numArgs;
                const MsgArg* args;
                message->GetArgs(numArgs, args);
                LSFString uniqueID = static_cast<LSFString>(args[0].v_string.str);
                controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, uniqueID);
            } else {
                controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, pulseWithPresetComp.lamps.front());
            }
            return;
        }
        pulseWithPresetComponent.pop_front();
    }

    lampClients.ChangeLampState(message, groupOperation, sceneOperation, stateParamsList, stateFieldParamsList, pulseParamsList);
}

void LampManager::TransitionLampState(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    LampState state(args[1]);
    uint32_t transitionPeriod = static_cast<uint32_t>(args[2].v_uint32);
    QCC_DbgPrintf(("lampID=%s state=%s transitionPeriod=%d", lampID.c_str(), state.c_str(), transitionPeriod));

    LSFStringList lampList;
    lampList.push_back(lampID);

    LampsAndStateList transitionToState;
    LampsAndPresetList transitionToPreset;
    LampsAndStateFieldList stateField;
    PulseLampsWithStateList pulseWithState;
    PulseLampsWithPresetList pulseWithPreset;

    LampsAndState transitionToStateComponent(lampList, state, transitionPeriod);
    transitionToState.push_back(transitionToStateComponent);

    ChangeLampStateAndField(message, transitionToState, transitionToPreset, stateField, pulseWithState, pulseWithPreset);
}

void LampManager::PulseLampWithState(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    LampState fromLampState(args[1]);
    LampState toLampState(args[2]);
    uint32_t period = static_cast<uint32_t>(args[3].v_uint32);
    uint32_t duration = static_cast<uint32_t>(args[4].v_uint32);
    uint32_t numPulses = static_cast<uint32_t>(args[5].v_uint32);
    QCC_DbgPrintf(("%s: lampID=%s, fromLampState=%s, period=%d, duration=%d, numPulses=%d",
                   __FUNCTION__, lampID.c_str(), fromLampState.c_str(), period, duration, numPulses));
    QCC_DbgPrintf(("%s: toLampState=%s", __FUNCTION__, toLampState.c_str()));

    LSFStringList lampList;
    lampList.push_back(lampID);

    LampsAndStateList transitionToState;
    LampsAndPresetList transitionToPreset;
    LampsAndStateFieldList stateField;
    PulseLampsWithStateList pulseWithState;
    PulseLampsWithPresetList pulseWithPreset;

    PulseLampsWithState pulseWithStateComponent(lampList, fromLampState, toLampState, period, duration, numPulses);
    pulseWithState.push_back(pulseWithStateComponent);
    ChangeLampStateAndField(message, transitionToState, transitionToPreset, stateField, pulseWithState, pulseWithPreset);
}

void LampManager::PulseLampWithPreset(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    LSFString fromPresetID = static_cast<LSFString>(args[1].v_string.str);
    LSFString toPresetID = static_cast<LSFString>(args[2].v_string.str);
    uint32_t period = static_cast<uint32_t>(args[3].v_uint32);
    uint32_t duration = static_cast<uint32_t>(args[4].v_uint32);
    uint32_t numPulses = static_cast<uint32_t>(args[5].v_uint32);
    QCC_DbgPrintf(("%s: lampID=%s, fromPresetID=%s, toPresetID=%s, period=%d, duration=%d, numPulses=%d",
                   __FUNCTION__, lampID.c_str(), fromPresetID.c_str(), toPresetID.c_str(), period, duration, numPulses));

    LSFStringList lampList;
    lampList.push_back(lampID);

    LampsAndStateList transitionToState;
    LampsAndPresetList transitionToPreset;
    LampsAndStateFieldList stateField;
    PulseLampsWithStateList pulseWithState;
    PulseLampsWithPresetList pulseWithPreset;

    PulseLampsWithPreset pulseWithPresetComponent(lampList, fromPresetID, toPresetID, period, duration, numPulses);
    pulseWithPreset.push_back(pulseWithPresetComponent);
    ChangeLampStateAndField(message, transitionToState, transitionToPreset, stateField, pulseWithState, pulseWithPreset);
}

uint32_t LampManager::GetControllerServiceLampInterfaceVersion(void)
{
    QCC_DbgPrintf(("%s: controllerLampInterfaceVersion=%d", __FUNCTION__, ControllerServiceLampInterfaceVersion));
    return ControllerServiceLampInterfaceVersion;
}
