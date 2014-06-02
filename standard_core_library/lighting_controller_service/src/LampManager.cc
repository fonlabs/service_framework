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
#include <OEMConfig.h>

#include <ControllerService.h>

#include <alljoyn/Status.h>
#include <qcc/Debug.h>

#include <algorithm>


using namespace lsf;
using namespace ajn;

#define QCC_MODULE "LAMP_MANAGER"

static const uint32_t controllerLampInterfaceVersion = 1;

LampManager::LampManager(ControllerService& controllerSvc, PresetManager& presetMgr, const char* ifaceName)
    : Manager(controllerSvc), lampClients(controllerSvc), presetManager(presetMgr), interfaceName(ifaceName)
{

}

LampManager::~LampManager()
{

}

QStatus LampManager::Start(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    /*
     * Start the Lamp Clients
     */
    QStatus status = lampClients.Start();
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

    LampsAndPreset transitionToPresetComponent(lampList, presetID, transitionPeriod);

    ChangeLampStateAndField(message, NULL, &transitionToPresetComponent);
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

    LampsAndStateField stateFieldComponent(lampList, fieldName, *varArg, transitionPeriod);

    ChangeLampStateAndField(message, NULL, NULL, &stateFieldComponent);
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

    if (LSF_OK == responseCode) {
        LampsAndState transitionToStateComponent(lamps, defaultLampState, 0);
        ChangeLampStateAndField(message, &transitionToStateComponent, NULL, NULL, NULL, NULL, groupOperation);
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
        ChangeLampStateAndField(message, NULL, NULL, &stateFieldComponent, NULL, NULL, groupOperation);
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
                                          LampsAndState* transitionToStateComponent,
                                          LampsAndPreset* transitionToPresetComponent,
                                          LampsAndStateField* stateFieldComponent,
                                          PulseLampsWithState* pulseWithStateComponent,
                                          PulseLampsWithPreset* pulseWithPresetComponent,
                                          bool groupOperation)
{
    LSFResponseCode responseCode = LSF_ERR_FAILURE;

    QCC_DbgPrintf(("%s", __FUNCTION__));

    uint64_t startTimeStamp = 0;
    GetSyncTimeStamp(startTimeStamp);

    if (transitionToStateComponent) {
        MsgArg state;
        transitionToStateComponent->state.Get(&state);
        QCC_DbgPrintf(("%s: Applying transitionToStateComponent", __FUNCTION__));
        lampClients.TransitionLampState(message, transitionToStateComponent->lamps, startTimeStamp, state, 0, groupOperation);
    }

    if (transitionToPresetComponent) {
        LampState preset;
        responseCode = presetManager.GetPresetInternal(transitionToPresetComponent->presetID, preset);
        if (LSF_OK == responseCode) {
            MsgArg state;
            preset.Get(&state);
            QCC_DbgPrintf(("%s: Applying transitionToPresetComponent", __FUNCTION__));
            lampClients.TransitionLampState(message, transitionToPresetComponent->lamps, startTimeStamp, state, 0, groupOperation);
        } else {
            if (groupOperation) {
                size_t numArgs;
                const MsgArg* args;
                message->GetArgs(numArgs, args);
                LSFString lampGroupId = static_cast<LSFString>(args[0].v_string.str);
                controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, lampGroupId);
            } else {
                controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, transitionToPresetComponent->lamps.front());
            }
        }
    }

    if (stateFieldComponent) {
        QCC_DbgPrintf(("%s: Applying stateFieldComponent", __FUNCTION__));
        lampClients.TransitionLampStateField(message, stateFieldComponent->lamps, startTimeStamp, stateFieldComponent->stateFieldName.c_str(),
                                             stateFieldComponent->stateFieldValue, stateFieldComponent->transitionPeriod, groupOperation);
    }

    if (pulseWithStateComponent) {
        QCC_DbgPrintf(("%s: Applying pulseWithStateComponent", __FUNCTION__));
        MsgArg fromState;
        MsgArg toState;
        pulseWithStateComponent->fromState.Get(&fromState);
        pulseWithStateComponent->toState.Get(&toState);
        lampClients.PulseLampWithState(message, pulseWithStateComponent->lamps, fromState, toState, pulseWithStateComponent->period, pulseWithStateComponent->duration,
                                       pulseWithStateComponent->numPulses, startTimeStamp, groupOperation);
    }

    if (pulseWithPresetComponent) {
        LampState fromPreset;
        LampState toPreset;
        responseCode = presetManager.GetPresetInternal(pulseWithPresetComponent->fromPreset, fromPreset);
        if (LSF_OK == responseCode) {
            responseCode = presetManager.GetPresetInternal(pulseWithPresetComponent->toPreset, toPreset);
            if (LSF_OK == responseCode) {
                MsgArg fromState;
                MsgArg toState;
                fromPreset.Get(&fromState);
                toPreset.Get(&toState);
                QCC_DbgPrintf(("%s: Applying pulseWithPresetComponent", __FUNCTION__));
                lampClients.PulseLampWithState(message, pulseWithPresetComponent->lamps, fromState, toState, pulseWithPresetComponent->period, pulseWithPresetComponent->duration,
                                               pulseWithPresetComponent->numPulses, startTimeStamp, groupOperation);
            }
        } else {
            if (groupOperation) {
                size_t numArgs;
                const MsgArg* args;
                message->GetArgs(numArgs, args);
                LSFString lampGroupId = static_cast<LSFString>(args[0].v_string.str);
                controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, lampGroupId);
            } else {
                controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, pulseWithPresetComponent->lamps.front());
            }
        }
    }
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

    LampsAndState transitionToStateComponent(lampList, state, transitionPeriod);

    ChangeLampStateAndField(message, &transitionToStateComponent);
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

    PulseLampsWithState pulseWithStateComponent(lampList, fromLampState, toLampState, period, duration, numPulses);
    ChangeLampStateAndField(message, NULL, NULL, NULL, &pulseWithStateComponent);
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

    PulseLampsWithPreset pulseWithPresetComponent(lampList, fromPresetID, toPresetID, period, duration, numPulses);
    ChangeLampStateAndField(message, NULL, NULL, NULL, NULL, &pulseWithPresetComponent);
}

uint32_t LampManager::GetControllerLampInterfaceVersion(void)
{
    QCC_DbgPrintf(("%s: controllerLampInterfaceVersion=%d", __FUNCTION__, controllerLampInterfaceVersion));
    return controllerLampInterfaceVersion;
}
