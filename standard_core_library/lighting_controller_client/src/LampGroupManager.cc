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

#include <LampGroupManager.h>
#include <ControllerClient.h>

#include <qcc/Debug.h>

#include <utility>

#define QCC_MODULE "LAMP_GROUP_MANAGER"

using namespace lsf;
using namespace ajn;

LampGroupManager::LampGroupManager(ControllerClient& controllerClient, LampGroupManagerCallback& callback) :
    Manager(controllerClient),
    callback(callback)
{
    controllerClient.lampGroupManagerPtr = this;
}

ControllerClientStatus LampGroupManager::GetAllLampGroupIDs(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndListOfIDs(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "GetAllLampGroupIDs");
}

ControllerClientStatus LampGroupManager::GetLampGroupName(const LSFString& lampGroupID, const LSFString& language)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s", __FUNCTION__, lampGroupID.c_str()));
    MsgArg args[2];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", language.c_str());
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDLanguageAndName(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "GetLampGroupName",
               args,
               2);
}

ControllerClientStatus LampGroupManager::SetLampGroupName(const LSFString& lampGroupID, const LSFString& lampGroupName, const LSFString& language)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s lampGroupName=%s language=%s", __FUNCTION__, lampGroupID.c_str(), lampGroupName.c_str(), language.c_str()));

    MsgArg args[3];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", lampGroupName.c_str());
    args[2].Set("s", language.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "SetLampGroupName",
               args,
               3);
}

ControllerClientStatus LampGroupManager::CreateLampGroup(const LampGroup& lampGroup)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    MsgArg args[2];
    lampGroup.Get(&args[0], &args[1]);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "CreateLampGroup",
               args,
               2);
}

ControllerClientStatus LampGroupManager::UpdateLampGroup(const LSFString& lampGroupID, const LampGroup& lampGroup)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s", __FUNCTION__, lampGroupID.c_str()));
    MsgArg args[3];
    args[0].Set("s", lampGroupID.c_str());
    lampGroup.Get(&args[1], &args[2]);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "UpdateLampGroup",
               args,
               3);
}

ControllerClientStatus LampGroupManager::GetLampGroup(const LSFString& lampGroupID)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s", __FUNCTION__, lampGroupID.c_str()));
    MsgArg arg;
    arg.Set("s", lampGroupID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "GetLampGroup",
               this,
               &LampGroupManager::GetLampGroupReply,
               &arg,
               1);
}

void LampGroupManager::GetLampGroupReply(Message& message)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSFString lampGroupID = static_cast<LSFString>(args[1].v_string.str);
    LampGroup lampGroup(args[2], args[3]);

    callback.GetLampGroupReplyCB(responseCode, lampGroupID, lampGroup);
}

ControllerClientStatus LampGroupManager::DeleteLampGroup(const LSFString& lampGroupID)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s", __FUNCTION__, lampGroupID.c_str()));
    MsgArg arg;
    arg.Set("s", lampGroupID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "DeleteLampGroup",
               &arg,
               1);
}

ControllerClientStatus LampGroupManager::ResetLampGroupState(const LSFString& lampGroupID)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s", __FUNCTION__, lampGroupID.c_str()));
    MsgArg arg("s", lampGroupID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "ResetLampGroupState",
               &arg,
               1);
}

ControllerClientStatus LampGroupManager::ResetLampGroupStateField(const LSFString& lampGroupID, const LSFString& stateFieldName)
{
    QCC_DbgPrintf(("\n%s: lampGroupID=%s stateFieldName=%s\n", __FUNCTION__, lampGroupID.c_str(), stateFieldName.c_str()));

    MsgArg args[2];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", stateFieldName.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "ResetLampGroupFieldState",
               args,
               2);
}

ControllerClientStatus LampGroupManager::TransitionLampGroupState(const LSFString& lampGroupID, const LampState& state, const uint32_t& transitionPeriod)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s state=%s", __FUNCTION__, lampGroupID.c_str(), state.c_str()));
    MsgArg args[3];
    args[0].Set("s", lampGroupID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "TransitionLampGroupState",
               args,
               3);
}

ControllerClientStatus LampGroupManager::PulseLampGroupWithState(
    const LSFString& lampGroupID,
    const LampState& fromLampGroupState,
    const LampState& toLampGroupState,
    const uint32_t& period,
    const uint32_t& duration,
    const uint32_t& numPulses)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));

    MsgArg args[6];
    args[0].Set("s", lampGroupID.c_str());
    fromLampGroupState.Get(&args[1]);
    toLampGroupState.Get(&args[2]);
    args[3].Set("u", period);
    args[4].Set("u", duration);
    args[5].Set("u", numPulses);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "PulseLampGroupWithState",
               args,
               6);
}

ControllerClientStatus LampGroupManager::StrobeLampGroupWithState(
    const LSFString& lampGroupID,
    const LampState& fromLampGroupState,
    const LampState& toLampGroupState,
    const uint32_t& period,
    const uint32_t& numStrobes)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));

    MsgArg args[5];
    args[0].Set("s", lampGroupID.c_str());
    fromLampGroupState.Get(&args[1]);
    toLampGroupState.Get(&args[2]);
    args[3].Set("u", period);
    args[4].Set("u", numStrobes);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "StrobeLampGroupWithState",
               args,
               5);
}

ControllerClientStatus LampGroupManager::CycleLampGroupWithState(
    const LSFString& lampGroupID,
    const LampState& lampGroupStateA,
    const LampState& lampGroupStateB,
    const uint32_t& period,
    const uint32_t& duration,
    const uint32_t& numCycles)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));

    MsgArg args[6];
    args[0].Set("s", lampGroupID.c_str());
    lampGroupStateA.Get(&args[1]);
    lampGroupStateB.Get(&args[2]);
    args[3].Set("u", period);
    args[4].Set("u", duration);
    args[5].Set("u", numCycles);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "CycleLampGroupWithState",
               args,
               6);
}

ControllerClientStatus LampGroupManager::PulseLampGroupWithPreset(
    const LSFString& lampGroupID,
    const LSFString& fromPresetID,
    const LSFString& toPresetID,
    const uint32_t& period,
    const uint32_t& duration,
    const uint32_t& numPulses)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));

    MsgArg args[6];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", fromPresetID.c_str());
    args[2].Set("s", toPresetID.c_str());
    args[3].Set("u", period);
    args[4].Set("u", duration);
    args[5].Set("u", numPulses);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "PulseLampGroupWithPreset",
               args,
               6);
}

ControllerClientStatus LampGroupManager::StrobeLampGroupWithPreset(
    const LSFString& lampGroupID,
    const LSFString& fromPresetID,
    const LSFString& toPresetID,
    const uint32_t& period,
    const uint32_t& numStrobes)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));

    MsgArg args[5];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", fromPresetID.c_str());
    args[2].Set("s", toPresetID.c_str());
    args[3].Set("u", period);
    args[4].Set("u", numStrobes);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "StrobeLampGroupWithPreset",
               args,
               5);
}

ControllerClientStatus LampGroupManager::CycleLampGroupWithPreset(
    const LSFString& lampGroupID,
    const LSFString& presetIdA,
    const LSFString& presetIdB,
    const uint32_t& period,
    const uint32_t& duration,
    const uint32_t& numCycles)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));

    MsgArg args[6];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", presetIdA.c_str());
    args[2].Set("s", presetIdB.c_str());
    args[3].Set("u", period);
    args[4].Set("u", duration);
    args[5].Set("u", numCycles);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "CycleLampGroupWithPreset",
               args,
               6);
}

ControllerClientStatus LampGroupManager::TransitionLampGroupStateIntegerField(const LSFString& lampGroupID, const LSFString& stateFieldName, const uint32_t& value, const uint32_t& transitionPeriod)
{
    QCC_DbgPrintf(("\n%s: lampGroupID=%s stateFieldName=%s\n", __FUNCTION__, lampGroupID.c_str(), stateFieldName.c_str()));

    MsgArg args[3];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", stateFieldName.c_str());
    args[2].Set("v", &value);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "TransitionLampGroupStateField",
               args,
               3);
}

ControllerClientStatus LampGroupManager::TransitionLampGroupStateBooleanField(const LSFString& lampGroupID, const LSFString& stateFieldName, const bool& value)
{
    QCC_DbgPrintf(("\n%s: lampGroupID=%s stateFieldName=%s\n", __FUNCTION__, lampGroupID.c_str(), stateFieldName.c_str()));

    MsgArg args[3];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", stateFieldName.c_str());
    args[2].Set("v", &value);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "TransitionLampGroupStateField",
               args,
               3);
}

ControllerClientStatus LampGroupManager::TransitionLampGroupStateToPreset(const LSFString& lampGroupID, const LSFString& presetID, const uint32_t& transitionPeriod)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s presetID=%s", __FUNCTION__, lampGroupID.c_str(), presetID.c_str()));
    MsgArg args[3];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", presetID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "TransitionLampGroupStateToPreset",
               args,
               3);
}

void LampGroupManager::ResetLampGroupFieldStateReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName)
{

}

void LampGroupManager::TransitionLampGroupStateFieldReply(LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName)
{

}
