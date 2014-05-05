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
#include <ControllerClient.h>

#include <qcc/Debug.h>

#include <vector>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "LAMP_MANAGER"

LampManager::LampManager(ControllerClient& controllerClient, LampManagerCallback& callback) :
    Manager(controllerClient),
    callback(callback)
{
    controllerClient.lampManagerPtr = this;
}

ControllerClientStatus LampManager::GetAllLampIDs(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndListOfIDs(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "GetAllLampIDs");
}

ControllerClientStatus LampManager::GetLampName(const LSF_ID& lampID)
{
    QCC_DbgPrintf(("%s: lampID=%s", __FUNCTION__, lampID.c_str()));
    MsgArg arg("s", lampID.c_str());
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "GetLampName",
               &arg,
               1);
}

ControllerClientStatus LampManager::SetLampName(const LSF_ID& lampID, const LSF_Name& lampName)
{
    QCC_DbgPrintf(("%s: lampID=%s lampName=%s,", __FUNCTION__, lampID.c_str(), lampName.c_str()));

    MsgArg args[2];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", lampName.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "SetLampName",
               args,
               2);
}

ControllerClientStatus LampManager::GetLampDetails(const LSF_ID& lampID)
{
    QCC_DbgPrintf(("%s: lampID=%s", __FUNCTION__, lampID.c_str()));

    MsgArg arg;
    arg.Set("s", lampID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "GetLampDetails",
               this,
               &LampManager::GetLampDetailsReply,
               &arg,
               1);
}

void LampManager::GetLampDetailsReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSF_ID lampID = static_cast<LSF_ID>(args[1].v_string.str);
    LampDetails details(args[2]);

    callback.GetLampDetailsReplyCB(responseCode, lampID, details);
}

ControllerClientStatus LampManager::GetLampParameters(const LSF_ID& lampID)
{
    QCC_DbgPrintf(("\n%s: lampID=%s\n", __FUNCTION__, lampID.c_str()));

    MsgArg arg;
    arg.Set("s", lampID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "GetLampParameters",
               this,
               &LampManager::GetLampParametersReply,
               &arg,
               1);
}

void LampManager::GetLampParametersReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSF_ID lampID = static_cast<LSF_ID>(args[1].v_string.str);
    LampParameters parameters(args[2]);

    callback.GetLampParametersReplyCB(responseCode, lampID, parameters);
}

ControllerClientStatus LampManager::GetLampState(const LSF_ID& lampID)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));

    MsgArg arg;
    arg.Set("s", lampID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "GetLampState",
               this,
               &LampManager::GetLampStateReply,
               &arg,
               1);
}

void LampManager::GetLampStateReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSF_ID lampID = static_cast<LSF_ID>(args[1].v_string.str);
    LampState state(args[2]);

    callback.GetLampStateReplyCB(responseCode, lampID, state);
}

ControllerClientStatus LampManager::ResetLampState(const LSF_ID& lampID)
{
    QCC_DbgPrintf(("\n%s: %s\n", __FUNCTION__, lampID.c_str()));

    MsgArg arg;
    arg.Set("s", lampID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "ResetLampState",
               &arg,
               1);
}

ControllerClientStatus LampManager::TransitionLampState(
    const LSF_ID& lampID,
    const LampState& state,
    const uint32_t& transitionPeriod)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));

    MsgArg args[3];
    args[0].Set("s", lampID.c_str());
    state.Get(&args[1]);
    args[2].Set("u", transitionPeriod);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "TransitionLampState",
               args,
               3);
}

ControllerClientStatus LampManager::TransitionLampStateToSavedState(const LSF_ID& lampID, const LSF_ID& savedStateID, const uint32_t& transitionPeriod)
{
    QCC_DbgPrintf(("\n%s: %s %s\n", __FUNCTION__, lampID.c_str(), savedStateID.c_str()));

    MsgArg args[3];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", savedStateID.c_str());
    args[2].Set("u", transitionPeriod);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "TransitionLampStateToSavedState",
               args,
               3);
}

ControllerClientStatus LampManager::GetLampFaults(const LSF_ID& lampID)
{
    QCC_DbgPrintf(("\n%s: %s\n", __FUNCTION__, lampID.c_str()));

    MsgArg arg("s", lampID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "GetLampFaults",
               this,
               &LampManager::GetLampFaultsReply,
               &arg,
               1);
}

void LampManager::GetLampFaultsReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSF_ID lampID = static_cast<LSF_ID>(args[1].v_string.str);

    LampFaultCodeList codes;
    uint32_t* codeList;
    size_t numCodes;
    args[2].Get("au", &numCodes, &codeList);

    for (size_t i = 0; i < numCodes; i++) {
        codes.push_back(codeList[i]);
        QCC_DbgPrintf(("%s: code=%d", __FUNCTION__, codeList[i]));
    }

    callback.GetLampFaultsReplyCB(responseCode, lampID, codes);
}

ControllerClientStatus LampManager::ClearLampFault(const LSF_ID& lampID, LampFaultCode faultCode)
{
    QCC_DbgPrintf(("\n%s: lampID=%s faultCode=%d\n", __FUNCTION__, lampID.c_str(), faultCode));

    MsgArg args[2];
    args[0].Set("s", lampID.c_str());
    args[1].Set("u", faultCode);

    return controllerClient.MethodCallAsync(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "ClearLampFault",
               this,
               &LampManager::ClearLampFaultReply,
               args,
               2);
}

void LampManager::ClearLampFaultReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSF_ID lampID = static_cast<LSF_ID>(args[1].v_string.str);
    LampFaultCode code = static_cast<LampFaultCode>(args[2].v_uint32);

    callback.ClearLampFaultReplyCB(responseCode, lampID, code);
}

ControllerClientStatus LampManager::GetLampStateField(const LSF_ID& lampID, const LSF_Name& stateFieldName)
{
    QCC_DbgPrintf(("\n%s: lampID=%s stateFieldName=%s\n", __FUNCTION__, lampID.c_str(), stateFieldName.c_str()));

    MsgArg args[2];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", stateFieldName.c_str());

    return controllerClient.MethodCallAsync(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "GetLampStateField",
               this,
               &LampManager::GetLampStateFieldReply,
               args,
               2);
}

void LampManager::GetLampStateFieldReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSF_ID lampID = static_cast<LSF_ID>(args[1].v_string.str);
    LSF_Name fieldName = static_cast<LSF_Name>(args[2].v_string.str);

    MsgArg* varArg;
    args[3].Get("v", &varArg);

    if (0 == strcmp("OnOff", fieldName.c_str())) {
        bool onOff = static_cast<bool>(varArg->v_bool);
        callback.GetLampStateOnOffFieldReplyCB(responseCode, lampID, onOff);
    } else {
        uint32_t value = static_cast<uint32_t>(varArg->v_uint32);

        if (0 == strcmp("Hue", fieldName.c_str())) {
            callback.GetLampStateHueFieldReplyCB(responseCode, lampID, value);
        } else if (0 == strcmp("Saturation", fieldName.c_str())) {
            callback.GetLampStateSaturationFieldReplyCB(responseCode, lampID, value);
        } else if (0 == strcmp("Brightness", fieldName.c_str())) {
            callback.GetLampStateBrightnessFieldReplyCB(responseCode, lampID, value);
        } else if (0 == strcmp("ColorTemp", fieldName.c_str())) {
            callback.GetLampStateColorTempFieldReplyCB(responseCode, lampID, value);
        }
    }
}

ControllerClientStatus LampManager::ResetLampStateField(const LSF_ID& lampID, const LSF_Name& stateFieldName)
{
    QCC_DbgPrintf(("\n%s: lampID=%s stateFieldName=%s\n", __FUNCTION__, lampID.c_str(), stateFieldName.c_str()));

    MsgArg args[2];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", stateFieldName.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "ResetLampStateField",
               args,
               2);
}

void LampManager::ResetLampStateFieldReply(LSFResponseCode& responseCode, LSF_ID& lsfId, LSF_Name& lsfName)
{
    QCC_DbgPrintf(("\n%s: %s %s %s\n", __FUNCTION__, LSFResponseCodeText(responseCode), lsfId.c_str(), lsfName.c_str()));

    if (0 == strcmp("OnOff", lsfName.c_str())) {
        callback.ResetLampStateOnOffFieldReplyCB(responseCode, lsfId);
    } else {
        if (0 == strcmp("Hue", lsfName.c_str())) {
            callback.ResetLampStateHueFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("Saturation", lsfName.c_str())) {
            callback.ResetLampStateSaturationFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("Brightness", lsfName.c_str())) {
            callback.ResetLampStateBrightnessFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("ColorTemp", lsfName.c_str())) {
            callback.ResetLampStateColorTempFieldReplyCB(responseCode, lsfId);
        }
    }
}

ControllerClientStatus LampManager::TransitionLampStateIntegerField(
    const LSF_ID& lampID,
    const LSF_Name& stateFieldName,
    const uint32_t& value,
    const uint32_t& transitionPeriod)
{
    QCC_DbgPrintf(("\n%s: lampID=%s stateFieldName=%s value=%d transitionPeriod=%d\n", __FUNCTION__,
                   lampID.c_str(), stateFieldName.c_str(), value, transitionPeriod));

    MsgArg args[4];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", stateFieldName.c_str());
    MsgArg arg("u", value);
    args[2].Set("v", &arg);
    args[3].Set("u", transitionPeriod);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "TransitionLampStateField",
               args,
               4);
}

ControllerClientStatus LampManager::TransitionLampStateBooleanField(
    const LSF_ID& lampID,
    const LSF_Name& stateFieldName,
    const bool& value)
{
    QCC_DbgPrintf(("\n%s: lampID=%s stateFieldName=%s\n", __FUNCTION__, lampID.c_str(), stateFieldName.c_str()));

    MsgArg args[4];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", stateFieldName.c_str());
    MsgArg arg("b", value);
    args[2].Set("v", &arg);
    args[3].Set("u", 0);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "TransitionLampStateField",
               args,
               4);
}

void LampManager::TransitionLampStateFieldReply(LSFResponseCode& responseCode, LSF_ID& lsfId, LSF_Name& lsfName)
{
    QCC_DbgPrintf(("\n%s: %s %s %s\n", __FUNCTION__, LSFResponseCodeText(responseCode), lsfId.c_str(), lsfName.c_str()));

    if (0 == strcmp("OnOff", lsfName.c_str())) {
        callback.TransitionLampStateOnOffFieldReplyCB(responseCode, lsfId);
    } else {
        if (0 == strcmp("Hue", lsfName.c_str())) {
            callback.TransitionLampStateHueFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("Saturation", lsfName.c_str())) {
            callback.TransitionLampStateSaturationFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("Brightness", lsfName.c_str())) {
            callback.TransitionLampStateBrightnessFieldReplyCB(responseCode, lsfId);
        } else if (0 == strcmp("ColorTemp", lsfName.c_str())) {
            callback.TransitionLampStateColorTempFieldReplyCB(responseCode, lsfId);
        }
    }
}

ControllerClientStatus LampManager::GetLampParametersField(const LSF_ID& lampID, const LSF_Name& fieldName)
{
    QCC_DbgPrintf(("\n%s\n", __FUNCTION__));

    MsgArg args[2];
    args[0].Set("s", lampID.c_str());
    args[1].Set("s", fieldName.c_str());

    return controllerClient.MethodCallAsync(
               ControllerClient::ControllerServiceLampInterfaceName.c_str(),
               "GetLampParametersField",
               this,
               &LampManager::GetLampParametersFieldReply,
               args,
               2);
}

void LampManager::GetLampParametersFieldReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSF_ID lampID = static_cast<LSF_ID>(args[1].v_string.str);
    LSF_Name fieldName = static_cast<LSF_Name>(args[2].v_string.str);

    MsgArg* varArg;
    args[3].Get("v", &varArg);
    uint32_t value = static_cast<uint32_t>(varArg->v_uint32);

    if (0 == strcmp("Energy_Usage_Milliwatts", fieldName.c_str())) {
        callback.GetLampParametersEnergyUsageMilliwattsFieldReplyCB(responseCode, lampID, value);
    } else if (0 == strcmp("Brightness_Lumens", fieldName.c_str())) {
        callback.GetLampParametersBrightnessLumensFieldReplyCB(responseCode, lampID, value);
    }
}
