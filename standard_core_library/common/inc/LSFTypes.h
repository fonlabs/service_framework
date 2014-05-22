#ifndef _LSF_TYPES_H_
#define _LSF_TYPES_H_
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

#include <memory>
#include <string>
#include <string.h>
#include <map>
#include <list>
#include <set>

#include <alljoyn/MsgArg.h>
#include <qcc/StringUtil.h>

#include <LampValues.h>
#include <LSFResponseCodes.h>

using namespace qcc;

#define LSF_CASE(_case) case _case: return # _case

#define MAX_SUPPORTED_NUM_LSF_ENTITY 100

namespace lsf {

typedef std::string LSFString;
typedef std::list<LSFString> LSFStringList;
typedef std::list<LampFaultCode> LampFaultCodeList;

typedef std::map<LSFString, LSFString> LampNameMap;

class LampState {

  public:
    LampState();
    LampState(bool onOff, uint32_t hue, uint32_t saturation, uint32_t colorTemp, uint32_t brightness);
    LampState(const ajn::MsgArg& arg);

    LampState(const LampState& other);
    LampState& operator=(const LampState& other);

    const char* c_str(void) const;

    void Set(const ajn::MsgArg& arg);
    void Get(ajn::MsgArg* arg) const;

    bool onOff;
    uint32_t hue;
    uint32_t saturation;
    uint32_t colorTemp;
    uint32_t brightness;
    bool nullState;

};

/*
 * std::map<presetID, std::pair<presetName, LampState>>
 */
typedef std::map<LSFString, std::pair<LSFString, LampState> > PresetMap;

class LampParameters {

  public:

    LampParameters();
    LampParameters(const ajn::MsgArg& arg);

    LampParameters(const LampParameters& other);
    LampParameters& operator=(const LampParameters& other);

    const char* c_str(void) const;

    void Set(const ajn::MsgArg& arg);
    void Get(ajn::MsgArg* arg) const;

    uint32_t energy_usage_milliwatts;
    uint32_t lumens;

};

class LampDetails {

  public:

    LampDetails();
    LampDetails(const ajn::MsgArg& arg);

    LampDetails(const LampDetails& other);
    LampDetails& operator=(const LampDetails& other);

    const char* c_str(void) const;

    void Set(const ajn::MsgArg& arg);
    void Get(ajn::MsgArg* arg) const;

    LampMake make;
    LampModel model;
    DeviceType type;
    LampType lampType;
    BaseType lampBaseType;
    uint32_t lampBeamAngle;
    bool dimmable;
    bool color;
    bool variableColorTemp;
    bool hasEffects;
    uint32_t maxVoltage;
    uint32_t minVoltage;
    uint32_t wattage;
    uint32_t incandescentEquivalent;
    uint32_t maxLumens;
    uint32_t minTemperature;
    uint32_t maxTemperature;
    uint32_t colorRenderingIndex;
    LSFString lampID;
};

class LampGroup {
  public:

    LampGroup();
    LampGroup(const ajn::MsgArg& lampList, const ajn::MsgArg& lampGroupList);
    LampGroup(LSFStringList lampList, LSFStringList lampGroupList);

    ~LampGroup() {
        lamps.clear();
        lampGroups.clear();
    }

    const char* c_str(void) const;

    LampGroup(const LampGroup& other);
    LampGroup& operator=(const LampGroup& other);

    void Set(const ajn::MsgArg& lampList, const ajn::MsgArg& lampGroupList);
    void Get(ajn::MsgArg* lampList, ajn::MsgArg* lampGroupList) const;

    LSFResponseCode IsDependentLampGroup(LSFString& lampGroupID);

    LSFStringList lamps;
    LSFStringList lampGroups;
};

/*
 * std::map<lampGroupID, std::pair<lampGroupName, LampGroup>>
 */
typedef std::map<LSFString, std::pair<LSFString, LampGroup> > LampGroupMap;

class TransitionLampsLampGroupsToState {
  public:
    TransitionLampsLampGroupsToState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& lampState, uint32_t& transPeriod);

    TransitionLampsLampGroupsToState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& lampState);

    TransitionLampsLampGroupsToState(const ajn::MsgArg& component);

    const char* c_str(void) const;

    TransitionLampsLampGroupsToState(const TransitionLampsLampGroupsToState& other);
    TransitionLampsLampGroupsToState& operator=(const TransitionLampsLampGroupsToState& other);

    void Set(const ajn::MsgArg& component);
    void Get(ajn::MsgArg* component) const;

    LSFStringList lamps;
    LSFStringList lampGroups;
    LampState state;
    uint32_t transitionPeriod;
};

class TransitionLampsLampGroupsToPreset {
  public:
    TransitionLampsLampGroupsToPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& presetID, uint32_t& transPeriod);

    TransitionLampsLampGroupsToPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& presetID);

    TransitionLampsLampGroupsToPreset(const ajn::MsgArg& component);

    const char* c_str(void) const;

    TransitionLampsLampGroupsToPreset(const TransitionLampsLampGroupsToPreset& other);
    TransitionLampsLampGroupsToPreset& operator=(const TransitionLampsLampGroupsToPreset& other);

    void Set(const ajn::MsgArg& component);
    void Get(ajn::MsgArg* component) const;

    LSFStringList lamps;
    LSFStringList lampGroups;
    LSFString presetID;
    uint32_t transitionPeriod;
};

class PulseLampsLampGroupsWithState {
  public:
    PulseLampsLampGroupsWithState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& fromLampState, LampState& toLampState, uint32_t& period, uint32_t& duration, uint32_t& numOfPulses);

    PulseLampsLampGroupsWithState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& toLampState, uint32_t& period, uint32_t& duration, uint32_t& numOfPulses);

    PulseLampsLampGroupsWithState(const ajn::MsgArg& component);

    const char* c_str(void) const;

    PulseLampsLampGroupsWithState(const PulseLampsLampGroupsWithState& other);
    PulseLampsLampGroupsWithState& operator=(const PulseLampsLampGroupsWithState& other);

    void Set(const ajn::MsgArg& component);
    void Get(ajn::MsgArg* component) const;

    LSFStringList lamps;
    LSFStringList lampGroups;
    LampState fromState;
    LampState toState;
    uint32_t pulsePeriod;
    uint32_t pulseDuration;
    uint32_t numPulses;
};

class PulseLampsLampGroupsWithPreset {
  public:
    PulseLampsLampGroupsWithPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& fromPreset, LSFString& toPreset, uint32_t& period, uint32_t& duration, uint32_t& numOfPulses);

    PulseLampsLampGroupsWithPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& toPreset, uint32_t& period, uint32_t& duration, uint32_t& numOfPulses);

    PulseLampsLampGroupsWithPreset(const ajn::MsgArg& component);

    const char* c_str(void) const;

    PulseLampsLampGroupsWithPreset(const PulseLampsLampGroupsWithPreset& other);
    PulseLampsLampGroupsWithPreset& operator=(const PulseLampsLampGroupsWithPreset& other);

    void Set(const ajn::MsgArg& component);
    void Get(ajn::MsgArg* component) const;

    LSFStringList lamps;
    LSFStringList lampGroups;
    LSFString fromPreset;
    LSFString toPreset;
    uint32_t pulsePeriod;
    uint32_t pulseDuration;
    uint32_t numPulses;
};

typedef std::list<TransitionLampsLampGroupsToState> TransitionLampsLampGroupsToStateList;
typedef std::list<TransitionLampsLampGroupsToPreset> TransitionLampsLampGroupsToPresetList;
typedef std::list<PulseLampsLampGroupsWithState> PulseLampsLampGroupsWithStateList;
typedef std::list<PulseLampsLampGroupsWithPreset> PulseLampsLampGroupsWithPresetList;

class Scene {
  public:
    Scene();
    Scene(const ajn::MsgArg& transitionToStateComponentList, const ajn::MsgArg& transitionToPresetComponentList,
          const ajn::MsgArg& pulseWithStateComponentList, const ajn::MsgArg& pulseWithPresetComponentList);
    Scene(TransitionLampsLampGroupsToStateList& transitionToStateComponentList, TransitionLampsLampGroupsToPresetList& transitionToPresetComponentList,
          PulseLampsLampGroupsWithStateList& pulseWithStateComponentList, PulseLampsLampGroupsWithPresetList& pulseWithPresetComponentList);

    Scene(const Scene& other);
    Scene& operator=(const Scene& other);

    const char* c_str(void) const;

    void Set(const ajn::MsgArg& transitionToStateComponentList, const ajn::MsgArg& transitionToPresetComponentList,
             const ajn::MsgArg& pulseWithStateComponentList, const ajn::MsgArg& pulseWithPresetComponentList);
    void Get(ajn::MsgArg* transitionToStateComponentList, ajn::MsgArg* transitionToPresetComponentList,
             ajn::MsgArg* pulseWithStateComponentList, ajn::MsgArg* pulseWithPresetComponentList) const;

    LSFResponseCode IsDependentOnPreset(LSFString& presetID);
    LSFResponseCode IsDependentOnLampGroup(LSFString& lampGroupID);

    TransitionLampsLampGroupsToStateList transitionToStateComponent;
    TransitionLampsLampGroupsToPresetList transitionToPresetComponent;
    PulseLampsLampGroupsWithStateList pulseWithStateComponent;
    PulseLampsLampGroupsWithPresetList pulseWithPresetComponent;
};

/*
 * std::map<sceneID, std::pair<sceneName, Scene>>
 */
typedef std::map<LSFString, std::pair<LSFString, Scene> > SceneMap;

class MasterScene {
  public:
    MasterScene();
    MasterScene(const ajn::MsgArg& sceneList);
    MasterScene(LSFStringList sceneList);

    ~MasterScene() { scenes.clear(); }

    const char* c_str(void) const;

    MasterScene(const MasterScene& other);
    MasterScene& operator=(const MasterScene& other);

    void Set(const ajn::MsgArg& sceneList);
    void Get(ajn::MsgArg* sceneList) const;

    LSFResponseCode IsDependentOnScene(LSFString& sceneID);

    LSFStringList scenes;
};

/*
 * std::map<masterSceneID, std::pair<masterSceneName, masterScene>>
 */
typedef std::map<LSFString, std::pair<LSFString, MasterScene> > MasterSceneMap;

}

#endif
