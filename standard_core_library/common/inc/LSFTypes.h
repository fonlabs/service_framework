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

using namespace qcc;

#define LSF_CASE(_case) case _case: return # _case

#define MAX_SUPPORTED_NUM_LSF_ENTITY 100

namespace lsf {

typedef std::string LSFString;
typedef std::list<LSFString> LSFStringList;
typedef std::list<LampFaultCode> LampFaultCodeList;

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
    void Get(size_t& size, ajn::MsgArg* arg) const;

    bool onOff;
    uint32_t hue;
    uint32_t saturation;
    uint32_t colorTemp;
    uint32_t brightness;

};

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
    uint32_t brightness_lumens;

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

    uint32_t hardwareVersion;
    uint32_t firmwareVersion;
    LampMake make;
    LampModel model;
    DeviceType type;
    LampMake lampType;
    uint32_t lampBaseType;
    uint32_t lampBeamAngle;
    bool dimmable;
    bool color;
    bool variableColorTemp;
    bool hasEffects;
    uint32_t voltage;
    uint32_t wattage;
    uint32_t wattageEquivalent;
    uint32_t maxOutput;
    uint32_t minTemperature;
    uint32_t maxTemperature;
    uint32_t colorRenderingIndex;
    uint32_t lifespan;
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

    LSFStringList lamps;
    LSFStringList lampGroups;
};

class LampsLampGroupsAndState {
  public:
    LampsLampGroupsAndState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& lampState, uint32_t& transPeriod);

    LampsLampGroupsAndState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& lampState);

    LampsLampGroupsAndState(const ajn::MsgArg& component);

    const char* c_str(void) const;

    LampsLampGroupsAndState(const LampsLampGroupsAndState& other);
    LampsLampGroupsAndState& operator=(const LampsLampGroupsAndState& other);

    void Set(const ajn::MsgArg& component);
    void Get(ajn::MsgArg* component) const;

    LSFStringList lamps;
    LSFStringList lampGroups;
    LampState state;
    uint32_t transitionPeriod;
};

class LampsLampGroupsAndPreset {
  public:
    LampsLampGroupsAndPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& presetID, uint32_t& transPeriod);

    LampsLampGroupsAndPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& presetID);

    LampsLampGroupsAndPreset(const ajn::MsgArg& component);

    const char* c_str(void) const;

    LampsLampGroupsAndPreset(const LampsLampGroupsAndPreset& other);
    LampsLampGroupsAndPreset& operator=(const LampsLampGroupsAndPreset& other);

    void Set(const ajn::MsgArg& component);
    void Get(ajn::MsgArg* component) const;

    LSFStringList lamps;
    LSFStringList lampGroups;
    LSFString presetID;
    uint32_t transitionPeriod;
};

typedef std::list<LampsLampGroupsAndState> LampsLampGroupsAndStateList;
typedef std::list<LampsLampGroupsAndPreset> LampsLampGroupsAndPresetList;

class Scene {
  public:
    Scene();
    Scene(const ajn::MsgArg& stateList, const ajn::MsgArg& presetList);
    Scene(LampsLampGroupsAndStateList& stateList, LampsLampGroupsAndPresetList& presetList);

    Scene(const Scene& other);
    Scene& operator=(const Scene& other);

    const char* c_str(void) const;

    void Set(const ajn::MsgArg& stateList, const ajn::MsgArg& presetList);
    void Get(ajn::MsgArg* stateList, ajn::MsgArg* presetList) const;

    LampsLampGroupsAndStateList stateComponent;
    LampsLampGroupsAndPresetList presetComponent;
};

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

    LSFStringList scenes;
};

}

#endif
