#ifndef _LAMP_GROUP_MANAGER_H_
#define _LAMP_GROUP_MANAGER_H_
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

class LampGroupManager : public Manager {

    friend class SceneManager;

  public:

    LampGroupManager(ControllerService& controllerSvc, LampManager& lampMgr, SceneManager* sceneMgrPtr, const std::string& lampGroupFile);

    LSFResponseCode Reset(void);
    LSFResponseCode IsDependentOnLampGroup(LSFString& lampGroupID);

    void ResetLampGroupState(ajn::Message& message);
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

    void TransitionLampGroupStateToPreset(ajn::Message& message);
    void TransitionLampGroupStateField(ajn::Message& message);
    void ResetLampGroupStateField(ajn::Message& message);
    void GetAllLampGroupIDs(ajn::Message& message);
    void GetLampGroupName(ajn::Message& message);
    void SetLampGroupName(ajn::Message& message);
    void CreateLampGroup(ajn::Message& message);
    void UpdateLampGroup(ajn::Message& message);
    void DeleteLampGroup(ajn::Message& message);
    void GetLampGroup(ajn::Message& message);

    LSFResponseCode GetAllLampGroups(LampGroupMap& lampGroupMap);

    virtual void WriteFile();
    void ReadSavedData();

    uint32_t GetControllerServiceLampGroupInterfaceVersion(void);

  protected:

    LSFResponseCode GetAllGroupLampsInternal(LSFStringList& lampGroupList, LSFStringList& lamps, LSFStringList& refList);

    LSFResponseCode GetAllGroupLamps(LSFStringList& lampGroupList, LSFStringList& lamps) {
        LSFStringList internalList;
        internalList.clear();
        return GetAllGroupLampsInternal(lampGroupList, lamps, internalList);
    }

    LSFResponseCode ChangeLampGroupStateAndField(ajn::Message& message,
                                                 TransitionLampsLampGroupsToStateList& transitionToStateComponent,
                                                 TransitionLampsLampGroupsToPresetList& transitionToPresetComponent,
                                                 PulseLampsLampGroupsWithStateList& pulseWithStateComponent,
                                                 PulseLampsLampGroupsWithPresetList& pulseWithPresetComponent,
                                                 bool groupOperation = false);

    LampGroupMap lampGroups;
    Mutex lampGroupsLock;
    LampManager& lampManager;
    SceneManager* sceneManagerPtr;
};

}

#endif
