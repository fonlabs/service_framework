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

#include <ControllerService.h>

#include <alljoyn/Status.h>
#include <qcc/Debug.h>

#include <algorithm>


using namespace lsf;
using namespace ajn;

#define QCC_MODULE "LAMP_MANAGER"

LampManager::LampManager(ControllerService& controllerSvc, PresetManager& presetMgr, const char* ifaceName)
    : Manager(controllerSvc), lampClients(controllerSvc, *this), presetManager(presetMgr), interfaceName(ifaceName)
{

}

LampManager::~LampManager()
{

}

QStatus LampManager::Start(void)
{
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
    /*
     * Stop the Lamp Clients
     */
    QStatus status = lampClients.Stop();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to stop the Lamp Clients", __FUNCTION__));
    }

    return status;
}

void LampManager::GetAllLampIDs(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));

    LSFStringList idList;
    LSFResponseCode responseCode = lampClients.GetAllLampIDs(idList);

    controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(message, responseCode, idList);
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

void LampManager::GetLampFaultsReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode responseCode)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg replyArgs[3];
    replyArgs[0].Set("u", responseCode);
    replyArgs[1] = args[0];
    replyArgs[2] = replyMsg;

    controllerService.SendMethodReply(origMsg, replyArgs, 3);
}

void LampManager::GetLampVersionReplyCB(ajn::Message& origMsg, LSFResponseCode responseCode, uint32_t version)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg replyArgs[3];
    replyArgs[0].Set("u", responseCode);
    replyArgs[1] = args[0];
    replyArgs[2].Set("u", version);

    controllerService.SendMethodReply(origMsg, replyArgs, 3);
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

void LampManager::ClearLampFaultReplyCB(ajn::Message& origMsg, LSFResponseCode responseCode, LampFaultCode code)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg replyArgs[3];
    replyArgs[0].Set("u", responseCode);
    replyArgs[1] = args[0]; // id
    replyArgs[2].Set("u", code);

    controllerService.SendMethodReply(origMsg, replyArgs, 3);
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

void LampManager::GetLampSupportedLanguagesReplyCB(ajn::Message& origMsg, const MsgArg& arg, LSFResponseCode responseCode)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg outArgs[3];
    outArgs[0].Set("u", responseCode);
    outArgs[1] = args[0]; // lamp id
    outArgs[2] = arg;
    controllerService.SendMethodReply(origMsg, outArgs, 3);
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

void LampManager::GetLampManufacturerReplyCB(ajn::Message& origMsg, const char* manufacturer, LSFResponseCode responseCode)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg outArgs[4];
    outArgs[0].Set("u", responseCode);
    outArgs[1] = args[0]; // lamp id
    outArgs[2] = args[1];
    outArgs[3].Set("s", manufacturer ? manufacturer : "");

    controllerService.SendMethodReply(origMsg, outArgs, 4);
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

void LampManager::GetLampNameReplyCB(ajn::Message& origMsg, const char* name, LSFResponseCode responseCode)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg outArgs[4];
    outArgs[0].Set("u", responseCode);
    outArgs[1] = args[0]; // lamp id
    outArgs[2] = args[1];
    outArgs[3].Set("s", name ? name : "");

    controllerService.SendMethodReply(origMsg, outArgs, 4);
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

void LampManager::SetLampNameReplyCB(ajn::Message& origMsg, LSFResponseCode responseCode)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg outArgs[3];
    outArgs[0].Set("u", responseCode);
    outArgs[1] = args[0]; // lamp id
    outArgs[2] = args[2];

    QCC_DbgPrintf(("Reply with %s\n", MsgArg::ToString(outArgs, 3).c_str()));

    controllerService.SendMethodReply(origMsg, outArgs, 3);
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

void LampManager::GetLampDetailsReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode responseCode)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg replyArgs[3];
    replyArgs[0].Set("u", responseCode);
    replyArgs[1] = args[0];
    replyArgs[2] = replyMsg;

    controllerService.SendMethodReply(origMsg, replyArgs, 3);
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

void LampManager::GetLampParametersReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode responseCode)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg replyArgs[3];
    replyArgs[0].Set("u", responseCode);
    replyArgs[1] = args[0];
    replyArgs[2] = replyMsg;

    controllerService.SendMethodReply(origMsg, replyArgs, 3);
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

void LampManager::GetLampParametersFieldReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode responseCode)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg replyArgs[4];
    replyArgs[0].Set("u", responseCode);
    replyArgs[1] = args[0];
    replyArgs[2] = args[1];
    replyArgs[3] = replyMsg;
    controllerService.SendMethodReply(origMsg, replyArgs, 4);
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

void LampManager::GetLampStateReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode responseCode)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg replyArgs[3];
    replyArgs[0].Set("u", responseCode);
    replyArgs[1] = args[0];
    replyArgs[2] = replyMsg;

    controllerService.SendMethodReply(origMsg, replyArgs, 3);
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

void LampManager::GetLampStateFieldReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode responseCode)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg replyArgs[4];
    replyArgs[0].Set("u", responseCode);
    replyArgs[1] = args[0];
    replyArgs[2] = args[1];
    replyArgs[3] = replyMsg;
    controllerService.SendMethodReply(origMsg, replyArgs, 4);
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

    TransitionLampStateAndFieldInternal(message, NULL, &transitionToPresetComponent, NULL);
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

    TransitionLampStateAndFieldInternal(message, NULL, NULL, &stateFieldComponent);
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

LSFResponseCode LampManager::ResetLampStateInternal(ajn::Message& message, LSFStringList lamps)
{
    LampState defaultLampState;

    QCC_DbgPrintf(("%s", __FUNCTION__));

    LSFResponseCode responseCode = presetManager.GetDefaultLampStateInternal(defaultLampState);

    if (LSF_OK == responseCode) {
        LampsAndState transitionToStateComponent(lamps, defaultLampState, 0);
        responseCode = TransitionLampStateAndFieldInternal(message, &transitionToStateComponent, NULL, NULL);
    } else {
        //TODO: Add support for Lamp Groups
        QCC_LogError(ER_FAIL, ("%s: Error getting the default lamp state", __FUNCTION__));
        controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, lamps.front());
    }

    return responseCode;
}

LSFResponseCode LampManager::ResetLampStateFieldInternal(ajn::Message& message, LSFStringList lamps, LSFString stateFieldName)
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
        responseCode = TransitionLampStateAndFieldInternal(message, NULL, NULL, &stateFieldComponent);
    } else {
        //TODO: Add support for Lamp Groups
        QCC_LogError(ER_FAIL, ("%s: Error getting the default lamp state", __FUNCTION__));
        controllerService.SendMethodReplyWithResponseCodeIDAndName(message, responseCode, lamps.front(), stateFieldName);
    }

    return responseCode;
}

LSFResponseCode LampManager::TransitionLampStateAndFieldInternal(ajn::Message& message,
                                                                 LampsAndState* transitionToStateComponent,
                                                                 LampsAndPreset* transitionToPresetComponent,
                                                                 LampsAndStateField* stateFieldComponent)
{
    LSFResponseCode responseCode = LSF_ERR_FAILURE;

    QCC_DbgPrintf(("%s", __FUNCTION__));

    if (transitionToStateComponent) {
        MsgArg state;
        transitionToStateComponent->state.Get(&state);
        QCC_DbgPrintf(("%s: Applying transitionToStateComponent", __FUNCTION__));
        lampClients.TransitionLampState(message, transitionToStateComponent->lamps, 0x5555555555555555, state, 0);
    }

    if (transitionToPresetComponent) {
        LampState preset;
        responseCode = presetManager.GetPresetInternal(transitionToPresetComponent->presetID, preset);
        if (LSF_OK == responseCode) {
            MsgArg state;
            preset.Get(&state);
            QCC_DbgPrintf(("%s: Applying transitionToPresetComponent", __FUNCTION__));
            lampClients.TransitionLampState(message, transitionToPresetComponent->lamps, 0x6666666666666666, state, 0);
        } else {
            //TODO: Add support for LampGroups
            controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, transitionToPresetComponent->lamps.front());
        }
    }

    if (stateFieldComponent) {
        QCC_DbgPrintf(("%s: Applying stateFieldComponent", __FUNCTION__));
        lampClients.TransitionLampStateField(message, stateFieldComponent->lamps, 0x7777777777777777, stateFieldComponent->stateFieldName.c_str(), stateFieldComponent->stateFieldValue, stateFieldComponent->transitionPeriod);
    }

    return responseCode;
}

void LampManager::TransitionLampStateFieldReplyCB(ajn::Message& origMsg, LSFResponseCode responseCode)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg replyArgs[3];
    replyArgs[0].Set("u", responseCode);
    replyArgs[1] = args[0];
    replyArgs[2] = args[1];
    controllerService.SendMethodReply(origMsg, replyArgs, 3);
}

void LampManager::TransitionLampState(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    uint32_t transitionPeriod = static_cast<uint32_t>(args[2].v_uint32);

    LSFStringList lampList;
    lampList.push_back(lampID);
    lampClients.TransitionLampState(message, lampList, 0UL, args[1], transitionPeriod);
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

    lampClients.PulseLampWithState(message, lampList, args[1], args[2], period, duration, numPulses, 0x8888888888888888);
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

    LampState fromPreset;
    LampState toPreset;
    LSFResponseCode responseCode = presetManager.GetPresetInternal(fromPresetID, fromPreset);
    if (LSF_OK == responseCode) {
        responseCode = presetManager.GetPresetInternal(toPresetID, toPreset);

        if (LSF_OK == responseCode) {
            LSFStringList lampList;
            lampList.push_back(lampID);

            MsgArg fromArg;
            fromPreset.Get(&fromArg);

            MsgArg toArg;
            toPreset.Get(&toArg);

            lampClients.PulseLampWithState(message, lampList, fromArg, toArg, period, duration, numPulses, 0x9999999999999999);
        }
    }

    if (LSF_OK != responseCode) {
        controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, lampID);
    }
}

void LampManager::ChangeLampStateReplyCB(ajn::Message& origMsg, LSFResponseCode responseCode)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);


    MsgArg replyArgs[2];
    replyArgs[0].Set("u", responseCode);
    replyArgs[1] = args[0];
    controllerService.SendMethodReply(origMsg, replyArgs, 2);
}
