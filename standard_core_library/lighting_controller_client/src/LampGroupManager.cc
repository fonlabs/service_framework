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

ControllerClientStatus LampGroupManager::GetLampGroupName(const LSF_ID& lampGroupID)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s", __FUNCTION__, lampGroupID.c_str()));
    MsgArg arg("s", lampGroupID.c_str());
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "GetLampGroupName",
               &arg,
               1);
}

ControllerClientStatus LampGroupManager::SetLampGroupName(const LSF_ID& lampGroupID, const LSF_Name& lampGroupName)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s lampGroupName=%s", __FUNCTION__, lampGroupID.c_str(), lampGroupName.c_str()));
    MsgArg args[2];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", lampGroupName.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "SetLampGroupName",
               args,
               2);
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

ControllerClientStatus LampGroupManager::UpdateLampGroup(const LSF_ID& lampGroupID, const LampGroup& lampGroup)
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

ControllerClientStatus LampGroupManager::GetLampGroup(const LSF_ID& lampGroupID)
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
    LSF_ID lampGroupID = static_cast<LSF_ID>(args[1].v_string.str);
    LampGroup lampGroup(args[2], args[3]);

    callback.GetLampGroupReplyCB(responseCode, lampGroupID, lampGroup);
}

ControllerClientStatus LampGroupManager::DeleteLampGroup(const LSF_ID& lampGroupID)
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

ControllerClientStatus LampGroupManager::ResetLampGroupState(const LSF_ID& lampGroupID)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s", __FUNCTION__, lampGroupID.c_str()));
    MsgArg arg("s", lampGroupID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "ResetLampGroupState",
               &arg,
               1);
}

ControllerClientStatus LampGroupManager::ResetLampGroupStateField(const LSF_ID& lampGroupID, const LSF_Name& stateFieldName)
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

ControllerClientStatus LampGroupManager::TransitionLampGroupState(const LSF_ID& lampGroupID, const LampState& state, const uint32_t& transitionPeriod)
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

ControllerClientStatus LampGroupManager::TransitionLampGroupStateIntegerField(const LSF_ID& lampGroupID, const LSF_Name& stateFieldName, const uint32_t& value, const uint32_t& transitionPeriod)
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

ControllerClientStatus LampGroupManager::TransitionLampGroupStateBooleanField(const LSF_ID& lampGroupID, const LSF_Name& stateFieldName, const bool& value)
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

ControllerClientStatus LampGroupManager::TransitionLampGroupStateToSavedState(const LSF_ID& lampGroupID, const LSF_ID& savedStateID, const uint32_t& transitionPeriod)
{
    QCC_DbgPrintf(("%s: lampGroupID=%s savedStateID=%s", __FUNCTION__, lampGroupID.c_str(), savedStateID.c_str()));
    MsgArg args[3];
    args[0].Set("s", lampGroupID.c_str());
    args[1].Set("s", savedStateID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceLampGroupInterfaceName.c_str(),
               "TransitionLampGroupStateToSavedState",
               args,
               3);
}

void LampGroupManager::ResetLampGroupFieldStateReply(LSFResponseCode& responseCode, LSF_ID& lsfId, LSF_Name& lsfName)
{

}

void LampGroupManager::TransitionLampGroupStateFieldReply(LSFResponseCode& responseCode, LSF_ID& lsfId, LSF_Name& lsfName)
{

}
