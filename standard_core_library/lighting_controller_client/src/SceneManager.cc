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
#include <ControllerClient.h>
#include <qcc/Debug.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "SCENE_MANAGER"

SceneManager::SceneManager(ControllerClient& controllerClient, SceneManagerCallback& callback) :
    Manager(controllerClient),
    callback(callback)
{
    controllerClient.sceneManagerPtr = this;
}

ControllerClientStatus SceneManager::GetAllSceneIDs(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndListOfIDs(
               ControllerClient::ControllerServiceSceneInterfaceName.c_str(),
               "GetAllSceneIDs");
}

ControllerClientStatus SceneManager::GetSceneName(const LSF_ID& sceneID)
{
    QCC_DbgPrintf(("%s: sceneID=%s", __FUNCTION__, sceneID.c_str()));
    MsgArg arg("s", sceneID.c_str());
    return controllerClient.MethodCallAsyncForReplyWithResponseCodeIDAndName(
               ControllerClient::ControllerServiceSceneInterfaceName.c_str(),
               "GetSceneName",
               &arg,
               1);
}

ControllerClientStatus SceneManager::SetSceneName(const LSF_ID& sceneID, const LSF_Name& sceneName)
{
    QCC_DbgPrintf(("%s: sceneID=%s sceneName=%s", __FUNCTION__, sceneID.c_str(), sceneName.c_str()));
    MsgArg args[2];
    args[0].Set("s", sceneID.c_str());
    args[1].Set("s", sceneName.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSceneInterfaceName.c_str(),
               "SetSceneName",
               args,
               2);
}

ControllerClientStatus SceneManager::GetScene(const LSF_ID& sceneID)
{
    QCC_DbgPrintf(("%s: sceneID=%s", __FUNCTION__, sceneID.c_str()));
    MsgArg arg;
    arg.Set("s", sceneID.c_str());

    return controllerClient.MethodCallAsync(
               ControllerClient::ControllerServiceSceneInterfaceName.c_str(),
               "GetScene",
               this,
               &SceneManager::GetSceneReply,
               &arg,
               1);
}

void SceneManager::GetSceneReply(Message& message)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    LSFResponseCode rc = static_cast<LSFResponseCode>(args[5].v_uint32);

    Scene scene;
    callback.GetSceneReplyCB(rc, "", scene);
}

ControllerClientStatus SceneManager::CreateScene(const Scene& scene)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    MsgArg args[3];
    args[0].Set("s", "");

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSceneInterfaceName.c_str(),
               "CreateScene",
               args,
               6);
}

ControllerClientStatus SceneManager::UpdateScene(const LSF_ID& sceneID, const Scene& scene)
{
    QCC_DbgPrintf(("%s: sceneID=%s", __FUNCTION__, sceneID.c_str()));
    MsgArg args[3];
    args[0].Set("s", sceneID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSceneInterfaceName.c_str(),
               "UpdateScene",
               args,
               6);
}

ControllerClientStatus SceneManager::DeleteScene(const LSF_ID& sceneID)
{
    QCC_DbgPrintf(("%s: sceneID=%s", __FUNCTION__, sceneID.c_str()));
    MsgArg arg;
    arg.Set("s", sceneID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSceneInterfaceName.c_str(),
               "DeleteScene",
               &arg,
               1);
}

ControllerClientStatus SceneManager::ApplyScene(const LSF_ID& sceneID)
{
    QCC_DbgPrintf(("%s: sceneID=%s", __FUNCTION__, sceneID.c_str()));
    MsgArg arg;
    arg.Set("s", sceneID.c_str());

    return controllerClient.MethodCallAsyncForReplyWithResponseCodeAndID(
               ControllerClient::ControllerServiceSceneInterfaceName.c_str(),
               "ApplyScene",
               &arg,
               1);
}
