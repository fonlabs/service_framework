#ifndef _LAMP_GROUP_MANAGER_H_
#define _LAMP_GROUP_MANAGER_H_
/**
 * \ingroup ControllerService
 */
/**
 * @file
 * This file provides definitions for lamp group manager
 */

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

#include <Manager.h>
#include <LampManager.h>

#include <LSFTypes.h>
#include <Mutex.h>

#include <string>

namespace lsf {

class SceneManager;
/**
 * LampGroupManager class
 */
class LampGroupManager : public Manager {

    friend class SceneManager;

  public:
    /**
     * LampGroupManager constructor
     */
    LampGroupManager(ControllerService& controllerSvc, LampManager& lampMgr, SceneManager* sceneMgrPtr, const std::string& lampGroupFile);
    /**
     * Reset the lamp group
     */
    LSFResponseCode Reset(void);
    /**
     * Is Dependent On Lamp Group
     */
    LSFResponseCode IsDependentOnLampGroup(LSFString& lampGroupID);
    /**
     * Reset Lamp Group State
     */
    void ResetLampGroupState(ajn::Message& message);
    /**
     * Transition Lamp Group State
     */
    void TransitionLampGroupState(ajn::Message& message);
    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.TransitionLampGroupState
     *
     * @param message   The params
     */
    void PulseLampGroupWithState(ajn::Message& message);
    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.TransitionLampGroupPreset
     *
     * @param message   The params
     */
    void PulseLampGroupWithPreset(ajn::Message& message);
    /**
     * Transition Lamp Group State To Preset
     */
    void TransitionLampGroupStateToPreset(ajn::Message& message);
    /**
     * Transition Lamp Group State Field
     */
    void TransitionLampGroupStateField(ajn::Message& message);
    /**
     * Reset Lamp Group State Field
     */
    void ResetLampGroupStateField(ajn::Message& message);
    /**
     * Get All Lamp Group IDs
     */
    void GetAllLampGroupIDs(ajn::Message& message);
    /**
     * Get All Lamp Group IDs
     */
    void GetLampGroupName(ajn::Message& message);
    /**
     * Set Lamp Group Name
     */
    void SetLampGroupName(ajn::Message& message);
    /**
     * Create Lamp Group
     */
    void CreateLampGroup(ajn::Message& message);
    /**
     * Update Lamp Group
     */
    void UpdateLampGroup(ajn::Message& message);
    /**
     * Delete Lamp Group
     */
    void DeleteLampGroup(ajn::Message& message);
    /**
     * Get Lamp Group
     */
    void GetLampGroup(ajn::Message& message);
    /**
     * Get All Lamp Groups
     */
    LSFResponseCode GetAllLampGroups(LampGroupMap& lampGroupMap);
    /**
     * Read Write File
     */
    void ReadWriteFile();
    /**
     * Read Saved Data
     */
    void ReadSavedData();
    /**
     * Get Controller ServiceLamp Group Interface Version
     */
    uint32_t GetControllerServiceLampGroupInterfaceVersion(void);
    /**
     * Get Blob Info
     */
    void GetBlobInfo(uint32_t& checksum, uint64_t& timestamp) {
        lampGroupsLock.Lock();
        GetBlobInfoInternal(checksum, timestamp);
        lampGroupsLock.Unlock();
    }
    /**
     * Handle Received Blob
     */
    void HandleReceivedBlob(const std::string& blob, uint32_t checksum, uint64_t timestamp);

  protected:
    /**
     * Replace Map
     */
    void ReplaceMap(std::istringstream& stream);
    /**
     * Get String
     */
    virtual bool GetString(std::string& output, uint32_t& checksum, uint64_t& timestamp);
    /**
     * Get All Group Lamps Internal
     */
    LSFResponseCode GetAllGroupLampsInternal(LSFStringList& lampGroupList, LSFStringList& lamps, LSFStringList& refList);
    /**
     * Get All Group Lamps
     */
    LSFResponseCode GetAllGroupLamps(LSFStringList& lampGroupList, LSFStringList& lamps) {
        LSFStringList internalList;
        internalList.clear();
        return GetAllGroupLampsInternal(lampGroupList, lamps, internalList);
    }
    /**
     * Change Lamp Group State And Field
     */
    LSFResponseCode ChangeLampGroupStateAndField(ajn::Message& message,
                                                 TransitionLampsLampGroupsToStateList& transitionToStateComponent,
                                                 TransitionLampsLampGroupsToPresetList& transitionToPresetComponent,
                                                 PulseLampsLampGroupsWithStateList& pulseWithStateComponent,
                                                 PulseLampsLampGroupsWithPresetList& pulseWithPresetComponent,
                                                 bool groupOperation = true, LSFString sceneOrMasterSceneID = LSFString());

    LampGroupMap lampGroups;        /**< lamp groups */
    Mutex lampGroupsLock;           /**< lamp groups lock */
    LampManager& lampManager;       /**< lamp manager */
    SceneManager* sceneManagerPtr;  /**< scene manager pointer */
    size_t blobLength;              /**< blob length */
    /**
     * get lamp group string
     */
    std::string GetString(const LampGroupMap& items);
    /**
     * get lamp group string
     */
    std::string GetString(const std::string& name, const std::string& id, const LampGroup& group);
};

}

#endif
