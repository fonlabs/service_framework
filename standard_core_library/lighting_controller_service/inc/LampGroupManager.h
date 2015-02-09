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
 * Copyright AllSeen Alliance. All rights reserved.
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
 * LampGroupManager class. \n
 * The class is handling groups of lamps. \n
 */
class LampGroupManager : public Manager {

    friend class SceneManager;

  public:
    /**
     * LampGroupManager constructor
     */
    LampGroupManager(ControllerService& controllerSvc, LampManager& lampMgr, SceneManager* sceneMgrPtr, const std::string& lampGroupFile);
    /**
     * Clear all lamp groups from LampGroupManager. \n
     * sends signal to the controller client 'org.allseen.LSF.ControllerService.LampGroup' 'LampGroupsDeleted' indicating all the deleted groups. \n
     * @return LSF_OK on success
     */
    LSFResponseCode Reset(void);
    /**
     * Is Dependent On Lamp Group - is there other lamp group or scene that depends on that group. \n
     * @param lampGroupID - lamp group unique identifier
     * @return LSF_OK if not depend
     */
    LSFResponseCode IsDependentOnLampGroup(LSFString& lampGroupID);
    /**
     * Reset Lamp Group State. \n
     * Go to each lamp in the specified group and reset its state. \n
     * @param message contains MsgArg with group id.
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success. \n
     */
    void ResetLampGroupState(ajn::Message& message);
    /**
     * Transition Lamp Group State. \n
     * Calls to transition state for all lamps in the group. \n
     * @param message contains MsgArg with lamp group id, requested state, transition period. \n
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success
     */
    void TransitionLampGroupState(ajn::Message& message);
    /**
     * pulse Lamp Group State. \n
     * Calls to pulse state for all lamps in the group. \n
     * @param message contains MsgArg with lamp group id, from requested state, to requested state, period, duration, number of pulses. \n
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success
     */
    void PulseLampGroupWithState(ajn::Message& message);
    /**
     * pulse Lamp Group State with preset states. \n
     * Calls to pulse state for all lamps in the group. \n
     * @param message contains MsgArg with lamp group id, from preset state, to preset state, period, duration, number of pulses. \n
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success
     */
    void PulseLampGroupWithPreset(ajn::Message& message);
    /**
     * Transit Lamp Group State to preset state. \n
     * Calls to transit state for all lamps in the group. \n
     * @param message contains MsgArg with lamp group id, to preset state, transition period. \n
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success
     */
    void TransitionLampGroupStateToPreset(ajn::Message& message);
    /**
     * Transit Lamp Group State field. \n
     * Calls to transit field in state for all lamps in the group. \n
     * @param message contains MsgArg with lamp group id, field name, transition period. \n
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success
     */
    void TransitionLampGroupStateField(ajn::Message& message);
    /**
     * Reset Lamp Group State field. \n
     * Calls to reset field in state for all lamps in the group. \n
     * @param message contains MsgArg with lamp group id, field name. \n
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success
     */
    void ResetLampGroupStateField(ajn::Message& message);
    /**
     * Get All Lamp Group IDs. \n
     * Return asynchronously list of ids for all lamps in the group. \n
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success
     */
    void GetAllLampGroupIDs(ajn::Message& message);
    /**
     * Get Lamp Group name. \n
     * Return asynchronously the lamp group name, relevant language, lamp group id. \n
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success
     */
    void GetLampGroupName(ajn::Message& message);
    /**
     * Set Lamp Group name. \n
     * Send signal to the controller client "org.allseen.LSF.ControllerService.LampGroup" "LampGroupsNameChanged". \n
     * @param message contains MsgArg with lamp group id, group name. \n
     * Return asynchronously the lamp group name, relevant language, lamp group id. \n
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success
     */
    void SetLampGroupName(ajn::Message& message);
    /**
     * Create Lamp Group. \n
     * Send signal to the controller client "org.allseen.LSF.ControllerService.LampGroup" "LampGroupsCreated". \n
     * @param message contains MsgArg with lamp group details, group name, relevant language. \n
     * Return asynchronously the lamp group id. \n
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success
     */
    void CreateLampGroup(ajn::Message& message);
    /**
     * Update Lamp Group - fill the group with other details. \n
     * Send signal to the controller client "org.allseen.LSF.ControllerService.LampGroup" "LampGroupsUpdated". \n
     * @param message contains MsgArg with lamp group details, lamp group id. \n
     * Return asynchronously the lamp group id. \n
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success
     */
    void UpdateLampGroup(ajn::Message& message);
    /**
     * Delete Lamp Group. \n
     * Send signal to the controller client "org.allseen.LSF.ControllerService.LampGroup" "LampGroupsDeleted". \n
     * @param message contains MsgArg with lamp group id. \n
     * Return asynchronously the lamp group id of the deleted group. \n
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success
     */
    void DeleteLampGroup(ajn::Message& message);
    /**
     * Get Lamp Group details. \n
     * @param message contains MsgArg with lamp group id. \n
     * Return asynchronously the lamp group id and the lamps details. \n
     * Return asynchronously response code and lamp group id. \n
     *   LSF_OK - on success
     */
    void GetLampGroup(ajn::Message& message);
    /**
     * Get All Lamp Groups synchronously to the parameter. \n
     * @param lampGroupMap - details of all lamp groups. \n
     * @return LSF_OK - on success
     */
    LSFResponseCode GetAllLampGroups(LampGroupMap& lampGroupMap);
    /**
     * Update persistent data
     */
    void ReadWriteFile();
    /**
     * Read Saved Data from persistent data
     */
    void ReadSavedData();
    /**
     * Get Controller ServiceLamp Group Interface Version
     * @return 32 bit unsigned integer of the version
     */
    uint32_t GetControllerServiceLampGroupInterfaceVersion(void);
    /**
     * Get Blob Info about checksum and timestamp
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
     * Get all lamps in the mentioned groups
     * @param lampGroupList - groups ids of those who needed to be searched.
     * @param lamps - the output list of lamps
     * @param refList - a subset of lampGroupList which is a unique list of the actual groups exists.
     * @return LSF_OK on success
     */
    LSFResponseCode GetAllGroupLampsInternal(LSFStringList& lampGroupList, LSFStringList& lamps, LSFStringList& refList);
    /**
     * Get all lamps in the mentioned groups
     * @param lampGroupList - groups ids of those who needed to be searched.
     * @param lamps - the output list of lamps
     * @return LSF_OK on success
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
