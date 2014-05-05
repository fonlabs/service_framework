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

#include <SceneGroupManager.h>
#include <ControllerClient.h>

#include <qcc/Debug.h>

#include <utility>

#define QCC_MODULE "LAMP_GROUP_MANAGER"

using namespace lsf;
using namespace ajn;

SceneGroupManager::SceneGroupManager(ControllerClient& controllerClient, SceneGroupManagerCallback& callback) :
    Manager(controllerClient),
    callback(callback)
{
    controllerClient.sceneGroupManagerPtr = this;
}

ControllerClientStatus SceneGroupManager::GetAllSceneGroupIDs(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndListOfIDs(
               ControllerClient::ControllerServiceSceneGroupInterfaceName.c_str(),
               "GetAllSceneGroupIDs");
}

ControllerClientStatus SceneGroupManager::GetSceneGroupName(const LSF_ID& sceneGroupID)
{
    QCC_DbgPrintf(("%s: sceneGroupID=%s", __FUNCTION__, sceneGroupID.c_str()));
    MsgArg arg("s", sceneGroupID.c_str());
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerClient::ControllerServiceSceneGroupInterfaceName.c_str(),
               "GetSceneGroupName",
               &arg,
               1);
}

ControllerClientStatus SceneGroupManager::SetSceneGroupName(const LSF_ID& sceneGroupID, const LSF_Name& sceneGroupName)
{
    QCC_DbgPrintf(("%s: sceneGroupID=%s sceneGroupName=%s", __FUNCTION__, sceneGroupID.c_str(), sceneGroupName.c_str()));
    MsgArg args[2];
    args[0].Set("s", sceneGroupID.c_str());
    args[1].Set("s", sceneGroupName.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSceneGroupInterfaceName.c_str(),
               "SetSceneGroupName",
               args,
               2);
}

ControllerClientStatus SceneGroupManager::CreateSceneGroup(const SceneGroup& sceneGroup)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    MsgArg arg;
    sceneGroup.Get(&arg);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSceneGroupInterfaceName.c_str(),
               "CreateSceneGroup",
               &arg,
               1);
}

ControllerClientStatus SceneGroupManager::UpdateSceneGroup(const LSF_ID& sceneGroupID, const SceneGroup& sceneGroup)
{
    QCC_DbgPrintf(("%s: sceneGroupID=%s", __FUNCTION__, sceneGroupID.c_str()));
    MsgArg args[2];
    args[0].Set("s", sceneGroupID.c_str());
    sceneGroup.Get(&args[1]);

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSceneGroupInterfaceName.c_str(),
               "UpdateSceneGroup",
               args,
               2);
}

ControllerClientStatus SceneGroupManager::GetSceneGroup(const LSF_ID& sceneGroupID)
{
    QCC_DbgPrintf(("%s: sceneGroupID=%s", __FUNCTION__, sceneGroupID.c_str()));
    MsgArg arg;
    arg.Set("s", sceneGroupID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerClient::ControllerServiceSceneGroupInterfaceName.c_str(),
               "GetSceneGroup",
               this,
               &SceneGroupManager::GetSceneGroupReply,
               &arg,
               1);
}

void SceneGroupManager::GetSceneGroupReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    LSFResponseCode responseCode = static_cast<LSFResponseCode>(args[0].v_uint32);
    LSF_ID sceneGroupID = static_cast<LSF_ID>(args[1].v_string.str);
    SceneGroup sceneGroup(args[2]);

    callback.GetSceneGroupReplyCB(responseCode, sceneGroupID, sceneGroup);
}

ControllerClientStatus SceneGroupManager::DeleteSceneGroup(const LSF_ID& sceneGroupID)
{
    QCC_DbgPrintf(("%s: sceneGroupID=%s", __FUNCTION__, sceneGroupID.c_str()));
    MsgArg arg;
    arg.Set("s", sceneGroupID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSceneGroupInterfaceName.c_str(),
               "DeleteSceneGroup",
               &arg,
               1);
}

ControllerClientStatus SceneGroupManager::ApplySceneGroup(const LSF_ID& sceneGroupID)
{
    QCC_DbgPrintf(("%s: sceneGroupID=%s", __FUNCTION__, sceneGroupID.c_str()));
    MsgArg arg;
    arg.Set("s", sceneGroupID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSceneGroupInterfaceName.c_str(),
               "ApplySceneGroup",
               &arg,
               1);
}
