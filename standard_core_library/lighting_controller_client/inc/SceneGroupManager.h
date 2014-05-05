#ifndef _SCENE_GROUP_MANAGER_H_
#define _SCENE_GROUP_MANAGER_H_
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

#include <list>

#include <Manager.h>
#include <ControllerClientDefs.h>

namespace lsf {

class ControllerClient;

class SceneGroupManagerCallback {
  public:
    virtual ~SceneGroupManagerCallback() { }

    /**
     * Response to SceneGroupManager::GetAllSceneGroupIDs
     *
     * @param responseCode        The response code
     * @param sceneGroupList    The sceneGroup ID's
     */
    virtual void GetAllSceneGroupIDsReplyCB(const LSFResponseCode& responseCode, const LSF_ID_List& sceneGroupList) { }

    /**
     * Response to SceneGroupManager::GetSceneGroupName
     *
     * @param responseCode    The response code
     * @param sceneGroupID    The sceneGroup id
     * @param sceneGroupName  The sceneGroup sceneGroupName
     */
    virtual void GetSceneGroupNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneGroupID, const LSF_Name& sceneGroupName) { }

    /**
     * Response to SceneGroupManager::SetSceneGroupName
     *
     * @param responseCode    The response code
     * @param sceneGroupID    The Lamp sceneGroup id
     */
    virtual void SetSceneGroupNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneGroupID) { }

    /**
     * A sceneGroup has had its sceneGroupName set
     *
     * @param sceneGroupID    The sceneGroup id
     */
    virtual void SceneGroupsNameChangedCB(const LSF_ID_List& sceneGroupIDs) { }

    /**
     * Response to SceneGroupManager::CreateSceneGroup
     *
     * @param responseCode    The response code
     * @param sceneGroupID    The Lamp sceneGroup id
     * @param sceneGroup The Lamp sceneGroup
     */
    virtual void CreateSceneGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneGroupID) { }

    /**
     *  A sceneGroup has been created
     *
     *  @param sceneGroupID   The sceneGroup id
     */
    virtual void SceneGroupsCreatedCB(const LSF_ID_List& sceneGroupIDs) { }

    /**
     * Response to SceneGroupManager::GetSceneGroup
     *
     * @param responseCode    The response code
     * @param sceneGroupID    The Lamp sceneGroup id
     * @parma sceneGroup The GroupID
     */
    virtual void GetSceneGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneGroupID, const SceneGroup& sceneGroup) { }

    /**
     * Response to SceneGroupManager::DeleteSceneGroup
     *
     * @param responseCode    The response code
     * @param sceneGroupID    The Lamp sceneGroup id
     */
    virtual void DeleteSceneGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneGroupID) { }

    /**
     *  A sceneGroup has been deleted
     *
     *  @param sceneGroupID   The sceneGroup id
     */
    virtual void SceneGroupsDeletedCB(const LSF_ID_List& sceneGroupIDs) { }

    /**
     * Response to SceneGroupManager::UpdateSceneGroup
     *
     * @param responseCode    The response code
     * @param sceneGroupID    The Lamp sceneGroup id
     */
    virtual void UpdateSceneGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneGroupID) { }

    /**
     * A Lamp sceneGroup has been updated
     *
     * @param sceneGroupID    The id of the Lamp sceneGroup
     */
    virtual void SceneGroupsUpdatedCB(const LSF_ID_List& sceneGroupIDs) { }

    /**
     * Response to SceneGroupManager::ApplySceneGroup
     *
     * @param responseCode    The response code
     * @param sceneGroupID    The id of the scene sceneGroup
     */
    virtual void ApplySceneGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneGroupID) { }

    /**
     * A scene sceneGroup has been applied
     *
     * @param sceneGroupID    The id of the scene sceneGroup
     */
    virtual void SceneGroupsAppliedCB(const LSF_ID_List& sceneGroupIDs) { }
};


class SceneGroupManager : public Manager {

    friend class ControllerClient;

  public:
    SceneGroupManager(ControllerClient& controller, SceneGroupManagerCallback& callback);

    /**
     * Get the IDs of all sceneGroupList
     * Response in SceneGroupManagerCallback::GetAllSceneGroupIDsReplyCB
     */
    ControllerClientStatus GetAllSceneGroupIDs(void);

    /**
     * Get the names of the sceneGroup
     * Response in SceneGroupManagerCallback::GetSceneGroupNameCB
     *
     * @param sceneGroupID    The sceneGroup id
     */
    ControllerClientStatus GetSceneGroupName(const LSF_ID& sceneGroupID);

    /**
     * Set the sceneGroupName of the specified sceneGroup
     * Response in SceneGroupManagerCallback::SetSceneGroupNameReplyCB
     *
     * @param sceneGroupID    The id of the sceneGroup
     * @param sceneGroupName  The Scene Group Name
     */
    ControllerClientStatus SetSceneGroupName(const LSF_ID& sceneGroupID, const LSF_Name& sceneGroupName);

    /**
     * Create a new Scene sceneGroup
     * Response in SceneGroupManagerCallback::CreateSceneGroupReplyCB
     *
     */
    ControllerClientStatus CreateSceneGroup(const SceneGroup& sceneGroup);

    /**
     * Modify a sceneGroup id
     * Response in SceneGroupManagerCallback::UpdateSceneGroupReplyCB
     *
     * @param sceneGroupID    The id of the sceneGroup to modify
     * @param lgids   The list of Lamp ID's that will be in the sceneGroup
     * @param gids  The list of sceneGroupList to add to this sceneGroup
     */
    ControllerClientStatus UpdateSceneGroup(const LSF_ID& sceneGroupID, const SceneGroup& sceneGroup);

    /**
     * Get the information about the sceneGroup
     * Response in SceneGroupManagerCallback::GetSceneGroupReplyCB
     *
     * @param sceneGroupID    Group id to get
     */
    ControllerClientStatus GetSceneGroup(const LSF_ID& sceneGroupID);

    /**
     * Delete a Lamp sceneGroup
     * Response in SceneGroupManagerCallback::DeleteSceneGroupReplyCB
     *
     * @param sceneGroupID    The id of the sceneGroup to delete
     */
    ControllerClientStatus DeleteSceneGroup(const LSF_ID& sceneGroupID);

    /**
     * Apply a scene sceneGroup
     * Response in SceneGroupManagerCallback::ApplySceneGroupReplyCB
     *
     * @param sceneGroupID    The ID of the scene to apply
     */
    ControllerClientStatus ApplySceneGroup(const LSF_ID& sceneGroupID);

  private:

    // signal handlers
    void SceneGroupsNameChanged(LSF_ID_List& idList) {
        callback.SceneGroupsNameChangedCB(idList);
    }

    void SceneGroupsCreated(LSF_ID_List& idList) {
        callback.SceneGroupsCreatedCB(idList);
    }

    void SceneGroupsDeleted(LSF_ID_List& idList) {
        callback.SceneGroupsDeletedCB(idList);
    }

    void SceneGroupsUpdated(LSF_ID_List& idList) {
        callback.SceneGroupsUpdatedCB(idList);
    }

    void SceneGroupsApplied(LSF_ID_List& idList) {
        callback.SceneGroupsAppliedCB(idList);
    }

    // method response handlers
    void GetAllSceneGroupIDsReply(LSFResponseCode& responseCode, LSF_ID_List& idList) {
        callback.GetAllSceneGroupIDsReplyCB(responseCode, idList);
    }

    void GetSceneGroupNameReply(LSFResponseCode& responseCode, LSF_ID& lsfId, LSF_Name& lsfName) {
        callback.GetSceneGroupNameReplyCB(responseCode, lsfId, lsfName);
    }

    void ApplySceneGroupReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.ApplySceneGroupReplyCB(responseCode, lsfId);
    }

    void SetSceneGroupNameReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.SetSceneGroupNameReplyCB(responseCode, lsfId);
    }

    void CreateSceneGroupReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.CreateSceneGroupReplyCB(responseCode, lsfId);
    }

    void UpdateSceneGroupReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.UpdateSceneGroupReplyCB(responseCode, lsfId);
    }

    void GetSceneGroupReply(ajn::Message& message);

    void DeleteSceneGroupReply(LSFResponseCode& responseCode, LSF_ID& lsfId) {
        callback.DeleteSceneGroupReplyCB(responseCode, lsfId);
    }

    SceneGroupManagerCallback&   callback;
};


}

#endif
