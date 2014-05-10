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
    lampClients.GetAllLampIDs(idList);

    MsgArg replyArgs[2];
    replyArgs[0].Set("u", LSF_OK);

    size_t arraySize = idList.size();
    if (arraySize) {
        const char** ids = new const char*[arraySize];
        size_t i = 0;
        for (LSFStringList::const_iterator it = idList.begin(); it != idList.end(); ++it, ++i) {
            ids[i] = it->c_str();
        }

        replyArgs[1].Set("as", arraySize, ids);
        replyArgs[1].SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        replyArgs[1].Set("as", 0, NULL);
    }

    controllerService.SendMethodReply(message, replyArgs, 2);
}

void LampManager::GetLampFaults(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    QCC_DbgPrintf(("lampID=%s", lampID.c_str()));

    LSFResponseCode responseCode = lampClients.GetLampFaults(lampID, message);
    if (responseCode != LSF_OK) {
        QCC_DbgPrintf(("%s: Error", __FUNCTION__));
        MsgArg replyArgs[3];
        replyArgs[0].Set("u", responseCode);
        replyArgs[1] = args[0];
        replyArgs[2].Set("au", 0, NULL);
        controllerService.SendMethodReply(message, replyArgs, 3);
    }
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

void LampManager::ClearLampFault(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    LampFaultCode faultCode = static_cast<LampFaultCode>(args[1].v_uint32);
    QCC_DbgPrintf(("lampID=%s", lampID.c_str()));

    LSFResponseCode responseCode = lampClients.ClearLampFault(lampID, message);
    if (responseCode != LSF_OK) {
        MsgArg replyArgs[3];
        replyArgs[0].Set("u", responseCode);
        replyArgs[1] = args[0];
        replyArgs[2] = args[1];
        controllerService.SendMethodReply(message, replyArgs, 3);
    }
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

void LampManager::GetLampSupportedLanguages(Message& message)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, message->ToString().c_str()));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);
    LSFString lampID = static_cast<LSFString>(args[0].v_string.str);
    QCC_DbgPrintf(("lampID=%s", lampID.c_str()));

    //TODO: Call in to LampClients and get reply
    LSFResponseCode responseCode = LSF_ERR_FAILURE;
    if (responseCode != LSF_OK) {
        MsgArg outArgs[3];
        outArgs[0].Set("u", responseCode);
        outArgs[1] = args[0]; // lamp id
        outArgs[2].Set("as", 0, NULL);
        controllerService.SendMethodReply(message, outArgs, 3);
    }
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

    LSFResponseCode responseCode = lampClients.GetLampManufacturer(lampID, message);
    if (responseCode != LSF_OK) {
        MsgArg outArgs[4];
        outArgs[0].Set("u", responseCode);
        outArgs[1] = args[0]; // lamp id
        outArgs[2] = args[1]; // language
        outArgs[3].Set("s", "");
        controllerService.SendMethodReply(message, outArgs, 4);
    }
}

void LampManager::GetLampManufacturerReplyCB(ajn::Message& origMsg, const char* manufacturer, LSFResponseCode rc)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg outArgs[4];
    outArgs[0].Set("u", rc);
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

    QCC_DbgPrintf(("lampID=%s language=%s", lampID.c_str()));

    LSFResponseCode responseCode = lampClients.GetLampName(lampID, message);
    if (responseCode != LSF_OK) {
        MsgArg outArgs[4];
        outArgs[0].Set("u", responseCode);
        outArgs[1] = args[0]; // lamp id
        outArgs[2] = args[1]; // language
        outArgs[3].Set("s", ""); // lamp name
        controllerService.SendMethodReply(message, outArgs, 4);
    }
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
    const char* lampName;
    args[1].Get("s", &lampName);

    LSFString language = static_cast<LSFString>(args[2].v_string.str);

    QCC_DbgPrintf(("lampID=%s lampName=%s language=%s", lampID.c_str(), lampName, language.c_str()));

    LSFResponseCode responseCode = LSF_ERR_FAILURE; //lampClients.SetLampName(lampID, lampName, message);
    if (responseCode != LSF_OK) {
        controllerService.SendMethodReplyWithResponseCodeIDAndName(message, responseCode, lampID, language);
    }
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

    LSFResponseCode responseCode = lampClients.GetLampDetails(lampID, message);
    if (responseCode != LSF_OK) {
        MsgArg replyArgs[3];
        replyArgs[0].Set("u", responseCode);
        replyArgs[1] = args[0];
        replyArgs[2].Set("a{sv}", 0, NULL);
        controllerService.SendMethodReply(message, replyArgs, 3);
    }
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


    LSFResponseCode responseCode = lampClients.GetLampParameters(lampID, message);
    if (responseCode != LSF_OK) {
        MsgArg replyArgs[3];
        replyArgs[0].Set("u", responseCode);
        replyArgs[1] = args[0];
        replyArgs[2].Set("a{sv}", 0, NULL);
        controllerService.SendMethodReply(message, replyArgs, 3);
    }
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

    LSFResponseCode responseCode = lampClients.GetLampParametersField(lampID, fieldName, message);
    if (responseCode != LSF_OK) {
        MsgArg replyArgs[4];
        replyArgs[0].Set("u", responseCode);
        replyArgs[1] = args[0];
        replyArgs[2] = args[1];
        replyArgs[3].Set("v", "s", "dummyvalue");
        controllerService.SendMethodReply(message, replyArgs, 4);
    }
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

    LSFResponseCode responseCode = lampClients.GetLampState(lampID, message);
    if (responseCode != LSF_OK) {
        MsgArg replyArgs[3];
        replyArgs[0].Set("u", responseCode);
        replyArgs[1] = args[0];
        replyArgs[2].Set("a{sv}", 0, NULL);
        controllerService.SendMethodReply(message, replyArgs, 3);
    }
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

    LSFResponseCode responseCode = lampClients.GetLampStateField(lampID, fieldName, message);
    if (responseCode != LSF_OK) {
        MsgArg replyArgs[4];
        replyArgs[0].Set("u", responseCode);
        replyArgs[1] = args[0];
        replyArgs[2] = args[1];
        replyArgs[3].Set("v", "s", "dummyvalue");
        replyArgs[3].Set("v", new MsgArg("u", 0));
        replyArgs[3].SetOwnershipFlags(MsgArg::OwnsArgs, true);
        controllerService.SendMethodReply(message, replyArgs, 4);
    }
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

    LampsAndPreset presetComponent(lampList, presetID, transitionPeriod);

    LSFResponseCode responseCode = TransitionLampStateAndFieldInternal(message, NULL, &presetComponent, NULL);

    if (LSF_OK != responseCode) {
        controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, lampID);
    }
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

    LSFResponseCode responseCode = TransitionLampStateAndFieldInternal(message, NULL, NULL, &stateFieldComponent);

    if (LSF_OK != responseCode) {
        controllerService.SendMethodReplyWithResponseCodeIDAndName(message, responseCode, lampID, fieldName);
    }
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

    LSFResponseCode responseCode = ResetLampStateInternal(message, lampList);

    if (LSF_OK != responseCode) {
        controllerService.SendMethodReplyWithResponseCodeAndID(message, responseCode, lampID);
    }
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

    LSFResponseCode responseCode = ResetLampStateFieldInternal(message, lampList, fieldName);

    if (LSF_OK != responseCode) {
        controllerService.SendMethodReplyWithResponseCodeIDAndName(message, responseCode, lampID, fieldName);
    }
}

LSFResponseCode LampManager::ResetLampStateInternal(ajn::Message& message, LSFStringList lamps)
{
    LampState defaultLampState;

    QCC_DbgPrintf(("%s", __FUNCTION__));

    LSFResponseCode responseCode = presetManager.GetDefaultLampStateInternal(defaultLampState);

    if (LSF_OK == responseCode) {
        LampsAndState stateComponent(lamps, defaultLampState, 0);
        responseCode = TransitionLampStateAndFieldInternal(message, &stateComponent, NULL, NULL);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Error getting the default lamp state", __FUNCTION__));
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
        QCC_LogError(ER_FAIL, ("%s: Error getting the default lamp state", __FUNCTION__));
    }

    return responseCode;
}

LSFResponseCode LampManager::TransitionLampStateAndFieldInternal(ajn::Message& message,
                                                                 LampsAndState* stateComponent,
                                                                 LampsAndPreset* presetComponent,
                                                                 LampsAndStateField* stateFieldComponent)
{
    LSFResponseCode responseCode = LSF_ERR_FAILURE;

    QCC_DbgPrintf(("%s", __FUNCTION__));

    if (stateComponent) {
        MsgArg state;
        stateComponent->state.Get(&state);
        QCC_DbgPrintf(("%s: Applying stateComponent", __FUNCTION__));
        responseCode = lampClients.TransitionLampState(message, stateComponent->lamps, 0UL, state, 0);
    }

    if (presetComponent) {
        LampState preset;
        responseCode = presetManager.GetPresetInternal(presetComponent->presetID, preset);
        if (LSF_OK == responseCode) {
            MsgArg state;
            preset.Get(&state);
            QCC_DbgPrintf(("%s: Applying presetComponent", __FUNCTION__));
            responseCode = lampClients.TransitionLampState(message, presetComponent->lamps, 0UL, state, 0);
        }
    }

    if (stateFieldComponent) {
        QCC_DbgPrintf(("%s: Applying stateFieldComponent", __FUNCTION__));
        responseCode = lampClients.TransitionLampFieldState(message, stateFieldComponent->lamps, 0UL, stateFieldComponent->stateFieldName.c_str(), stateFieldComponent->stateFieldValue, stateFieldComponent->transitionPeriod);
    }

    return responseCode;
}

void LampManager::TransitionLampStateFieldReplyCB(ajn::Message& origMsg, const char* field, LSFResponseCode rc)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);

    MsgArg replyArgs[3];
    replyArgs[0].Set("u", rc);
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
    LSFResponseCode responseCode = lampClients.TransitionLampState(message, lampList, 0UL, args[1], transitionPeriod);
    if (responseCode != LSF_OK) {
        MsgArg replyArgs[2];
        replyArgs[0].Set("u", responseCode);
        replyArgs[1] = args[0];
        controllerService.SendMethodReply(message, replyArgs, 2);
    }
}

void LampManager::TransitionLampStateReplyCB(ajn::Message& origMsg, LSFResponseCode rc)
{
    size_t numArgs;
    const MsgArg* args;
    origMsg->GetArgs(numArgs, args);


    MsgArg replyArgs[2];
    replyArgs[0].Set("u", rc);
    replyArgs[1] = args[0];
    controllerService.SendMethodReply(origMsg, replyArgs, 2);
}