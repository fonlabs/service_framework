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

#include <SavedStateManager.h>
#include <ControllerClient.h>
#include <qcc/Debug.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "SAVED_STATE_MANAGER"

SavedStateManager::SavedStateManager(ControllerClient& controllerClient, SavedStateManagerCallback& callback) :
    Manager(controllerClient),
    callback(callback)
{
    controllerClient.savedStateManagerPtr = this;
}

ControllerClientStatus SavedStateManager::GetAllSavedStateIDs(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndListOfIDs(
               ControllerClient::ControllerServiceSavedStateInterfaceName.c_str(),
               "GetAllSavedStateIDs");
}

ControllerClientStatus SavedStateManager::GetSavedState(const LSF_ID& savedStateID)
{
    QCC_DbgPrintf(("%s: savedStateID=%s", __FUNCTION__, savedStateID.c_str()));
    MsgArg arg;
    arg.Set("s", savedStateID.c_str());

    ControllerClientStatus status = controllerClient.MethodCallAsync(
        ControllerClient::ControllerServiceSavedStateInterfaceName.c_str(),
        "GetSavedState",
        this,
        &SavedStateManager::GetSavedStateReply,
        &arg,
        1);

    return status;
}

void SavedStateManager::GetSavedStateReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSF_ID savedStateID = static_cast<LSF_ID>(args[1].v_string.str);
    LampState savedState(args[2]);
    callback.GetSavedStateReplyCB(responseCode, savedStateID, savedState);
}

ControllerClientStatus SavedStateManager::GetSavedStateName(const LSF_ID& savedStateID)
{
    QCC_DbgPrintf(("%s: savedStateID=%s", __FUNCTION__, savedStateID.c_str()));
    MsgArg arg("s", savedStateID.c_str());
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerClient::ControllerServiceSavedStateInterfaceName.c_str(),
               "GetSavedStateName",
               &arg,
               1);
}

ControllerClientStatus SavedStateManager::SetSavedStateName(const LSF_ID& savedStateID, const LSF_Name& savedStateName)
{
    QCC_DbgPrintf(("%s: savedStateID=%s savedStateName=%s", __FUNCTION__, savedStateID.c_str(), savedStateName.c_str()));
    MsgArg args[2];
    args[0].Set("s", savedStateID.c_str());
    args[1].Set("s", savedStateName.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSavedStateInterfaceName.c_str(),
               "SetSavedStateName",
               args,
               2);
}

ControllerClientStatus SavedStateManager::CreateSavedState(const LampState& savedState)
{
    QCC_DbgPrintf(("%s: savedState=%s", __FUNCTION__, savedState.c_str()));

    MsgArg arg;
    savedState.Get(&arg);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSavedStateInterfaceName.c_str(),
               "CreateSavedState",
               &arg,
               1);
}

ControllerClientStatus SavedStateManager::UpdateSavedState(const LSF_ID& savedStateID, const LampState& savedState)
{
    QCC_DbgPrintf(("%s: savedStateID=%s savedState=%s", __FUNCTION__, savedStateID.c_str(), savedState.c_str()));
    MsgArg args[2];
    args[0].Set("s", savedStateID.c_str());
    savedState.Get(&args[1]);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSavedStateInterfaceName.c_str(),
               "UpdateSavedState",
               args,
               2);
}

ControllerClientStatus SavedStateManager::DeleteSavedState(const LSF_ID& savedStateID)
{
    QCC_DbgPrintf(("%s: savedStateID=%s", __FUNCTION__, savedStateID.c_str()));
    MsgArg arg;
    arg.Set("s", savedStateID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSavedStateInterfaceName.c_str(),
               "DeleteSavedState",
               &arg,
               1);
}

ControllerClientStatus SavedStateManager::GetDefaultLampState(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    return controllerClient.MethodCallAsync(
               ControllerClient::ControllerServiceSavedStateInterfaceName.c_str(),
               "GetDefaultLampState",
               this,
               &SavedStateManager::GetDefaultLampStateReply);
}

void SavedStateManager::GetDefaultLampStateReply(ajn::Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LampState defaultLampState(args[1]);

    callback.GetDefaultLampStateReplyCB(responseCode, defaultLampState);
}

ControllerClientStatus SavedStateManager::SetDefaultLampState(const LampState& defaultLampState)
{
    QCC_DbgPrintf(("%s: defaultLampState=%s", __FUNCTION__, defaultLampState.c_str()));

    MsgArg arg;
    defaultLampState.Get(&arg);

    return controllerClient.MethodCallAsyncForReplyWithUint32Value(
               ControllerClient::ControllerServiceSavedStateInterfaceName.c_str(),
               "SetDefaultLampState",
               &arg,
               1);
}
