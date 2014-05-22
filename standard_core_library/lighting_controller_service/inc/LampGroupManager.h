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

  public:

    LampGroupManager(ControllerService& controllerSvc, LampManager& lampMgr, const char* ifaceName, SceneManager* sceneMgrPtr);

    LSFResponseCode Reset(void);
    LSFResponseCode IsDependentOnLampGroup(LSFString& lampGroupID);

    void ResetLampGroupState(ajn::Message& msg);
    void TransitionLampGroupState(ajn::Message& msg);
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

    void TransitionLampGroupStateToPreset(ajn::Message& msg);
    void TransitionLampGroupStateField(ajn::Message& msg);
    void ResetLampGroupFieldState(ajn::Message& msg);
    void GetAllLampGroupIDs(ajn::Message& msg);
    void GetLampGroupName(ajn::Message& msg);
    void SetLampGroupName(ajn::Message& msg);
    void CreateLampGroup(ajn::Message& msg);
    void UpdateLampGroup(ajn::Message& msg);
    void DeleteLampGroup(ajn::Message& msg);
    void GetLampGroup(ajn::Message& msg);

    void AddLampGroup(const LSFString& id, const std::string& name, const LampGroup& group);

    LSFResponseCode GetAllLampGroups(LampGroupMap& lampGroupMap);

  protected:


    void GetAllGroupLamps(const LampGroup& group, LSFStringList& Lamps) const;

    typedef std::map<LSFString, bool> VisitedMap;
    bool IsGroupValid(const LampGroup& group) const;
    bool IsGroupValidHelper(const LSFString& id, VisitedMap& visited, VisitedMap& callStack) const;

    LampGroupMap lampGroups;
    Mutex lampGroupsLock;
    LampManager& lampManager;
    const char* interfaceName;
    SceneManager* sceneManagerPtr;
};

}

#endif
