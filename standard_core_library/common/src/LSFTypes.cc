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

#include <LSFTypes.h>
#include <LampValues.h>
#include <qcc/Debug.h>
#include <algorithm>

using namespace ajn;

#define QCC_MODULE "LSF_TYPES"

namespace lsf {

const LSFString CurrentStateIdentifier = LSFString("CURRENT_STATE");

const char* ControllerServiceObjectPath = "/org/allseen/LSF/ControllerService";
const char* ControllerServiceInterfaceName = "org.allseen.LSF.ControllerService";
const char* ControllerServiceLampInterfaceName = "org.allseen.LSF.ControllerService.Lamp";
const char* ControllerServiceLampGroupInterfaceName = "org.allseen.LSF.ControllerService.LampGroup";
const char* ControllerServicePresetInterfaceName = "org.allseen.LSF.ControllerService.Preset";
const char* ControllerServiceSceneInterfaceName = "org.allseen.LSF.ControllerService.Scene";
const char* ControllerServiceMasterSceneInterfaceName = "org.allseen.LSF.ControllerService.MasterScene";
ajn::SessionPort ControllerServiceSessionPort = 43;

const uint32_t ControllerServiceInterfaceVersion = 1;
const uint32_t ControllerServiceLampInterfaceVersion = 1;
const uint32_t ControllerServiceLampGroupInterfaceVersion = 1;
const uint32_t ControllerServicePresetInterfaceVersion = 1;
const uint32_t ControllerServiceSceneInterfaceVersion = 1;
const uint32_t ControllerServiceMasterSceneInterfaceVersion = 1;
const uint32_t LeaderElectionAndStateSyncInterfaceVersion = 1;

const char* LampServiceObjectPath = "/org/allseen/LSF/Lamp";
const char* LampServiceInterfaceName = "org.allseen.LSF.LampService";
const char* LampServiceStateInterfaceName = "org.allseen.LSF.LampState";
const char* LampServiceParametersInterfaceName = "org.allseen.LSF.LampParameters";
const char* LampServiceDetailsInterfaceName = "org.allseen.LSF.LampDetails";
ajn::SessionPort LampServiceSessionPort = 42;

const char* ConfigServiceObjectPath = "/Config";
const char* ConfigServiceInterfaceName = "org.alljoyn.Config";

const char* OnboardingServiceObjectPath = "/Onboarding";
const char* OnboardingServiceInterfaceName = "org.alljoyn.Onboarding";

const char* AboutObjectPath = "/About";
const char* AboutInterfaceName = "org.alljoyn.About";

const char* AboutIconObjectPath = "/About/DeviceIcon";
const char* AboutIconInterfaceName = "org.alljoyn.Icon";

const char* LeaderElectionAndStateSyncObjectPath = "/org/allseen/LeaderElectionAndStateSync";
const char* LeaderElectionAndStateSyncInterfaceName = "org.allseen.LeaderElectionAndStateSync";
ajn::SessionPort LeaderElectionAndStateSyncSessionPort = 44;

const char* ApplySceneEventActionInterfaceName = "org.allseen.LSF.ControllerService.ApplySceneEventAction.";

const char* ApplySceneEventActionObjectPath = "/org/allseen/LSF/ControllerService/ApplySceneEventAction/";

void CreateUniqueList(LSFStringList& uniqueList, LSFStringList& fromList)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    for (LSFStringList::iterator lampIt = fromList.begin(); lampIt != fromList.end(); lampIt++) {
        if (std::find(uniqueList.begin(), uniqueList.end(), *lampIt) == uniqueList.end()) {
            uniqueList.push_back(*lampIt);
            QCC_DbgPrintf(("%s: lampId = %s", __FUNCTION__, (*lampIt).c_str()));
        } else {
            QCC_DbgPrintf(("%s: lampId = %s already in the list", __FUNCTION__, (*lampIt).c_str()));
        }
    }
}

void CreateUniqueList(LSFStringList& uniqueList, ajn::MsgArg* idsArray, size_t idsSize)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    for (size_t i = 0; i < idsSize; i++) {
        char* gid;
        idsArray[i].Get("s", &gid);
        if ((std::find(uniqueList.begin(), uniqueList.end(), LSFString(gid))) == uniqueList.end()) {
            uniqueList.push_back(LSFString(gid));
        } else {
            QCC_DbgPrintf(("%s: lampId = %s already in the list", __FUNCTION__, gid));
        }
    }

}

LampState::LampState() :
    onOff(false),
    hue(0),
    saturation(0),
    colorTemp(0),
    brightness(0),
    nullState(true)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampState::LampState(bool onOff, uint32_t hue, uint32_t saturation, uint32_t colorTemp, uint32_t brightness) :
    onOff(onOff),
    hue(hue),
    saturation(saturation),
    colorTemp(colorTemp),
    brightness(brightness),
    nullState(false)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampState::LampState(const ajn::MsgArg& arg)
{
    Set(arg);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampState::LampState(const LampState& other) :
    onOff(other.onOff),
    hue(other.hue),
    saturation(other.saturation),
    colorTemp(other.colorTemp),
    brightness(other.brightness),
    nullState(other.nullState)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampState& LampState::operator=(const LampState& other)
{
    onOff = other.onOff;
    hue = other.hue;
    saturation = other.saturation;
    colorTemp = other.colorTemp;
    brightness = other.brightness;
    nullState = other.nullState;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

const char* LampState::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    if (nullState) {
        ret += qcc::String("nullState") + qcc::String("\n");
    } else {
        ret = qcc::String("LampState") + qcc::String("\n");
        ret += qcc::String("OnOff=") + U32ToString(onOff) + qcc::String("\n");
        ret += qcc::String("Hue=") + U32ToString(hue) + qcc::String("\n");
        ret += qcc::String("Saturation=") + U32ToString(saturation) + qcc::String("\n");
        ret += qcc::String("Brightness=") + U32ToString(brightness) + qcc::String("\n");
        ret += qcc::String("ColorTemp=") + U32ToString(colorTemp) + qcc::String("\n");
    }
    return ret.c_str();
}

void LampState::Set(const ajn::MsgArg& arg)
{
    MsgArg* args;
    size_t numArgs;
    arg.Get("a{sv}", &numArgs, &args);

    QCC_DbgPrintf(("%s: numArgs=%d", __FUNCTION__, numArgs));

    if (numArgs) {
        for (size_t i = 0; i < numArgs; i++) {
            char* field;
            MsgArg* value;
            args[i].Get("{sv}", &field, &value);

            if (0 == strcmp(field, "OnOff")) {
                value->Get("b", &onOff);
            } else if (0 == strcmp(field, "Hue")) {
                value->Get("u", &hue);
            } else if (0 == strcmp(field, "Saturation")) {
                value->Get("u", &saturation);
            } else if (0 == strcmp(field, "Brightness")) {
                value->Get("u", &brightness);
            } else if (0 == strcmp(field, "ColorTemp")) {
                value->Get("u", &colorTemp);
            }
        }
        nullState = false;
    } else {
        nullState = true;
    }

    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void LampState::Get(ajn::MsgArg* arg) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    if (nullState) {
        arg->Set("a{sv}", (size_t)0, NULL);
    } else {
        const char* str[] = { "OnOff", "Hue", "Saturation", "Brightness", "ColorTemp" };
        MsgArg* dict = new MsgArg[5];

        MsgArg* var = new MsgArg("b", onOff);
        dict[0].Set("{sv}", str[0], var);
        dict[0].SetOwnershipFlags(MsgArg::OwnsArgs, true);

        var = new MsgArg("u", hue);
        dict[1].Set("{sv}", str[1], var);
        dict[1].SetOwnershipFlags(MsgArg::OwnsArgs, true);

        var = new MsgArg("u", saturation);
        dict[2].Set("{sv}", str[2], var);
        dict[2].SetOwnershipFlags(MsgArg::OwnsArgs, true);

        var = new MsgArg("u", brightness);
        dict[3].Set("{sv}", str[3], var);
        dict[3].SetOwnershipFlags(MsgArg::OwnsArgs, true);

        var = new MsgArg("u", colorTemp);
        dict[4].Set("{sv}", str[4], var);
        dict[4].SetOwnershipFlags(MsgArg::OwnsArgs, true);

        arg->Set("a{sv}", (size_t)5, dict);
    }
}

LampParameters::LampParameters() :
    energyUsageMilliwatts(0),
    lumens(0)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampParameters::LampParameters(const ajn::MsgArg& arg)
{
    Set(arg);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampParameters::LampParameters(const LampParameters& other) :
    energyUsageMilliwatts(other.energyUsageMilliwatts),
    lumens(other.lumens)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampParameters& LampParameters::operator=(const LampParameters& other)
{
    energyUsageMilliwatts = other.energyUsageMilliwatts;
    lumens = other.lumens;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

const char* LampParameters::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret = qcc::String("LampParameters") + qcc::String("\n");
    ret += qcc::String("EnergyUsageMilliwatts=") + U32ToString(energyUsageMilliwatts) + qcc::String("\n");
    ret += qcc::String("Lumens=") + U32ToString(lumens) + qcc::String("\n");
    return ret.c_str();
}

void LampParameters::Set(const ajn::MsgArg& arg)
{
    MsgArg* args;
    size_t numArgs;
    arg.Get("a{sv}", &numArgs, &args);

    for (size_t i = 0; i < numArgs; i++) {
        char* field;
        MsgArg* value;
        args[i].Get("{sv}", &field, &value);

        if (0 == strcmp(field, "Energy_Usage_Milliwatts")) {
            value->Get("u", &energyUsageMilliwatts);
        } else if (0 == strcmp(field, "Brightness_Lumens")) {
            value->Get("u", &lumens);
        }
    }
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void LampParameters::Get(ajn::MsgArg* arg) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    const char* str[] = { "Energy_Usage_Milliwatts", "Brightness_Lumens" };
    MsgArg* dict = new MsgArg[2];

    MsgArg* var = new MsgArg("u", energyUsageMilliwatts);
    dict[0].Set("{sv}", str[0], var);
    dict[0].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", lumens);
    dict[1].Set("{sv}", str[1], var);
    dict[1].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    arg->Set("a{sv}", (size_t)2, dict);
}

LampDetails::LampDetails()
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampDetails::LampDetails(const ajn::MsgArg& arg)
{
    Set(arg);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampDetails::LampDetails(const LampDetails& other) :
    make(other.make),
    model(other.model),
    type(other.type),
    lampType(other.lampType),
    lampBaseType(other.lampBaseType),
    lampBeamAngle(other.lampBeamAngle),
    dimmable(other.dimmable),
    color(other.color),
    variableColorTemp(other.variableColorTemp),
    hasEffects(other.hasEffects),
    maxVoltage(other.maxVoltage),
    minVoltage(other.minVoltage),
    wattage(other.wattage),
    incandescentEquivalent(other.incandescentEquivalent),
    maxLumens(other.maxLumens),
    minTemperature(other.minTemperature),
    maxTemperature(other.maxTemperature),
    colorRenderingIndex(other.colorRenderingIndex),
    lampID(other.lampID)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampDetails& LampDetails::operator=(const LampDetails& other)
{
    make = other.make;
    model = other.model;
    type = other.type;
    lampType = other.lampType;
    lampBaseType = other.lampBaseType;
    lampBeamAngle = other.lampBeamAngle;
    dimmable = other.dimmable;
    color = other.color;
    variableColorTemp = other.variableColorTemp;
    hasEffects = other.hasEffects;
    maxVoltage = other.maxVoltage;
    minVoltage = other.minVoltage;
    wattage = other.wattage;
    incandescentEquivalent = other.incandescentEquivalent;
    maxLumens = other.maxLumens;
    minTemperature = other.minTemperature;
    maxTemperature = other.maxTemperature;
    colorRenderingIndex = other.colorRenderingIndex;
    lampID = other.lampID;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

const char* LampDetails::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret = qcc::String("LampDetails") + qcc::String("\n");
    ret += qcc::String("make=") + U32ToString(make) + qcc::String("\n");
    ret += qcc::String("model=") + U32ToString(model) + qcc::String("\n");
    ret += qcc::String("type=") + U32ToString(type) + qcc::String("\n");
    ret += qcc::String("lampType=") + U32ToString(lampType) + qcc::String("\n");
    ret += qcc::String("lampBaseType=") + U32ToString(lampBaseType) + qcc::String("\n");
    ret += qcc::String("lampBeamAngle=") + U32ToString(lampBeamAngle) + qcc::String("\n");
    ret += qcc::String("dimmable=") + U32ToString(dimmable) + qcc::String("\n");
    ret += qcc::String("color=") + U32ToString(color) + qcc::String("\n");
    ret += qcc::String("variableColorTemp=") + U32ToString(variableColorTemp) + qcc::String("\n");
    ret += qcc::String("hasEffects=") + U32ToString(hasEffects) + qcc::String("\n");
    ret += qcc::String("maxVoltage=") + U32ToString(maxVoltage) + qcc::String("\n");
    ret += qcc::String("minVoltage=") + U32ToString(minVoltage) + qcc::String("\n");
    ret += qcc::String("wattage=") + U32ToString(wattage) + qcc::String("\n");
    ret += qcc::String("incandescentEquivalent=") + U32ToString(incandescentEquivalent) + qcc::String("\n");
    ret += qcc::String("maxLumens=") + U32ToString(maxLumens) + qcc::String("\n");
    ret += qcc::String("minTemperature=") + U32ToString(minTemperature) + qcc::String("\n");
    ret += qcc::String("maxTemperature=") + U32ToString(maxTemperature) + qcc::String("\n");
    ret += qcc::String("colorRenderingIndex=") + U32ToString(colorRenderingIndex) + qcc::String("\n");
    ret += qcc::String("lampID=") + qcc::String(lampID.c_str()) + qcc::String("\n");
    return ret.c_str();
}

void LampDetails::Set(const ajn::MsgArg& arg)
{
    MsgArg* args;
    size_t numArgs;
    arg.Get("a{sv}", &numArgs, &args);

    for (size_t i = 0; i < numArgs; i++) {
        char* field;
        MsgArg* value;
        args[i].Get("{sv}", &field, &value);

        if (0 == strcmp(field, "Make")) {
            value->Get("u", &make);
        } else if (0 == strcmp(field, "Model")) {
            value->Get("u", &model);
        } else if (0 == strcmp(field, "Type")) {
            value->Get("u", &type);
        } else if (0 == strcmp(field, "LampType")) {
            value->Get("u", &lampType);
        } else if (0 == strcmp(field, "LampBaseType")) {
            value->Get("u", &lampBaseType);
        } else if (0 == strcmp(field, "LampBeamAngle")) {
            value->Get("u", &lampBeamAngle);
        } else if (0 == strcmp(field, "Dimmable")) {
            value->Get("b", &dimmable);
        } else if (0 == strcmp(field, "Color")) {
            value->Get("b", &color);
        } else if (0 == strcmp(field, "VariableColorTemp")) {
            value->Get("b", &variableColorTemp);
        } else if (0 == strcmp(field, "HasEffects")) {
            value->Get("b", &hasEffects);
        } else if (0 == strcmp(field, "MaxVoltage")) {
            value->Get("u", &maxVoltage);
        } else if (0 == strcmp(field, "MinVoltage")) {
            value->Get("u", &minVoltage);
        } else if (0 == strcmp(field, "Wattage")) {
            value->Get("u", &wattage);
        } else if (0 == strcmp(field, "IncandescentEquivalent")) {
            value->Get("u", &incandescentEquivalent);
        } else if (0 == strcmp(field, "MaxLumens")) {
            value->Get("u", &maxLumens);
        } else if (0 == strcmp(field, "MinTemperature")) {
            value->Get("u", &minTemperature);
        } else if (0 == strcmp(field, "MaxTemperature")) {
            value->Get("u", &maxTemperature);
        } else if (0 == strcmp(field, "ColorRenderingIndex")) {
            value->Get("u", &colorRenderingIndex);
        } else if (0 == strcmp(field, "LampID")) {
            char* temp;
            value->Get("s", &temp);
            lampID = LSFString(temp);
        }
    }
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void LampDetails::Get(ajn::MsgArg* arg) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    const char* str[] = {
        "Make",
        "Model",
        "Type",
        "LampType",
        "LampBaseType",
        "LampBeamAngle",
        "Dimmable",
        "Color",
        "VariableColorTemp",
        "HasEffects",
        "MaxVoltage",
        "MinVoltage",
        "Wattage",
        "IncandescentEquivalent",
        "MaxLumens",
        "MinTemperature",
        "MaxTemperature",
        "ColorRenderingIndex",
        "LampID"
    };

    MsgArg* dict = new MsgArg[19];

    MsgArg* var = new MsgArg("u", make);
    dict[0].Set("{sv}", str[0], var);
    dict[0].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", model);
    dict[1].Set("{sv}", str[1], var);
    dict[1].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", type);
    dict[2].Set("{sv}", str[2], var);
    dict[2].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", lampType);
    dict[3].Set("{sv}", str[3], var);
    dict[3].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", lampBaseType);
    dict[4].Set("{sv}", str[4], var);
    dict[4].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", lampBeamAngle);
    dict[5].Set("{sv}", str[5], var);
    dict[5].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("b", dimmable);
    dict[6].Set("{sv}", str[6], var);
    dict[6].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("b", color);
    dict[7].Set("{sv}", str[7], var);
    dict[7].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("b", variableColorTemp);
    dict[8].Set("{sv}", str[8], var);
    dict[8].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("b", hasEffects);
    dict[9].Set("{sv}", str[9], var);
    dict[9].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", maxVoltage);
    dict[10].Set("{sv}", str[10], var);
    dict[10].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", minVoltage);
    dict[11].Set("{sv}", str[11], var);
    dict[11].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", wattage);
    dict[12].Set("{sv}", str[12], var);
    dict[12].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", incandescentEquivalent);
    dict[13].Set("{sv}", str[13], var);
    dict[13].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", maxLumens);
    dict[14].Set("{sv}", str[14], var);
    dict[14].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", minTemperature);
    dict[15].Set("{sv}", str[15], var);
    dict[15].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", maxTemperature);
    dict[16].Set("{sv}", str[16], var);
    dict[16].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", colorRenderingIndex);
    dict[17].Set("{sv}", str[17], var);
    dict[17].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("s", lampID.c_str());
    dict[18].Set("{sv}", str[18], var);
    dict[18].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    arg->Set("a{sv}", (size_t)19, dict);
}

LampGroup::LampGroup()
{
    lamps.clear();
    lampGroups.clear();
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampGroup::LampGroup(const ajn::MsgArg& lampList, const ajn::MsgArg& lampGroupList)
{
    Set(lampList, lampGroupList);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampGroup::LampGroup(LSFStringList lampList, LSFStringList lampGroupList) :
    lamps(lampList), lampGroups(lampGroupList)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampGroup::LampGroup(const LampGroup& other)
{
    lamps.clear();
    lampGroups.clear();
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

const char* LampGroup::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret = qcc::String("LampGroup::Lamps:") + qcc::String("\n");
    for (LSFStringList::const_iterator it = lamps.begin(); it != lamps.end(); it++) {
        ret += qcc::String((*it).c_str()) + qcc::String("\n");
    }
    ret += qcc::String("LampGroup::Lamp Groups:") + qcc::String("\n");
    for (LSFStringList::const_iterator it = lampGroups.begin(); it != lampGroups.end(); it++) {
        ret += qcc::String((*it).c_str()) + qcc::String("\n");
    }

    return ret.c_str();
}

LampGroup& LampGroup::operator=(const LampGroup& other)
{
    lamps.clear();
    lampGroups.clear();
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void LampGroup::Set(const ajn::MsgArg& lampList, const ajn::MsgArg& lampGroupList)
{
    lamps.clear();
    lampGroups.clear();

    MsgArg* idsArray;
    size_t idsSize;
    lampList.Get("as", &idsSize, &idsArray);
    CreateUniqueList(lamps, idsArray, idsSize);

    MsgArg* gidsArray;
    size_t gidsSize;
    lampGroupList.Get("as", &gidsSize, &gidsArray);
    CreateUniqueList(lampGroups, gidsArray, gidsSize);

    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void LampGroup::Get(ajn::MsgArg* lampList, ajn::MsgArg* lampGroupList) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    size_t idsVecSize = lamps.size();

    if (idsVecSize) {
        const char** idsVec = new const char*[idsVecSize];
        size_t i = 0;
        for (LSFStringList::const_iterator it = lamps.begin(); it != lamps.end(); it++) {
            idsVec[i++] = it->c_str();
        }
        lampList->Set("as", idsVecSize, idsVec);
        lampList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        lampList->Set("as", 0, NULL);
    }

    size_t gidsVecSize = lampGroups.size();
    if (gidsVecSize) {
        const char** gidsVec = new const char*[gidsVecSize];
        size_t i = 0;
        for (LSFStringList::const_iterator it = lampGroups.begin(); it != lampGroups.end(); it++) {
            gidsVec[i++] = it->c_str();
        }
        lampGroupList->Set("as", gidsVecSize, gidsVec);
        lampGroupList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        lampGroupList->Set("as", 0, NULL);
    }
}

LSFResponseCode LampGroup::IsDependentLampGroup(LSFString& lampGroupID)
{
    LSFResponseCode responseCode = LSF_OK;

    LSFStringList::iterator findIt = std::find(lampGroups.begin(), lampGroups.end(), lampGroupID);
    if (findIt != lampGroups.end()) {
        responseCode = LSF_ERR_DEPENDENCY;
    }

    return responseCode;
}

TransitionLampsLampGroupsToState::TransitionLampsLampGroupsToState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& lampState, uint32_t& transPeriod) :
    lamps(lampList), lampGroups(lampGroupList), state(lampState), transitionPeriod(transPeriod), invalidArgs(false)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

TransitionLampsLampGroupsToState::TransitionLampsLampGroupsToState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& lampState) :
    lamps(lampList), lampGroups(lampGroupList), state(lampState), transitionPeriod(0), invalidArgs(false)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

TransitionLampsLampGroupsToState::TransitionLampsLampGroupsToState(const ajn::MsgArg& component)
{
    Set(component);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

const char* TransitionLampsLampGroupsToState::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret = qcc::String("TransitionLampsLampGroupsToState::Lamps:") + qcc::String("\n");
    for (LSFStringList::const_iterator it = lamps.begin(); it != lamps.end(); it++) {
        ret += qcc::String((*it).c_str()) + qcc::String("\n");
    }
    ret += qcc::String("TransitionLampsLampGroupsToState::Lamp Groups:") + qcc::String("\n");
    for (LSFStringList::const_iterator it = lampGroups.begin(); it != lampGroups.end(); it++) {
        ret += qcc::String((*it).c_str()) + qcc::String("\n");
    }

    ret += qcc::String("TransitionLampsLampGroupsToState::State:") + qcc::String(state.c_str()) + qcc::String("\n");
    ret += qcc::String("TransitionLampsLampGroupsToState::TransitionPeriod:") + U32ToString(transitionPeriod) + qcc::String("\n");

    return ret.c_str();
}

TransitionLampsLampGroupsToState::TransitionLampsLampGroupsToState(const TransitionLampsLampGroupsToState& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), state(other.state), transitionPeriod(other.transitionPeriod), invalidArgs(false)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

TransitionLampsLampGroupsToState& TransitionLampsLampGroupsToState::operator=(const TransitionLampsLampGroupsToState& other)
{
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    state = other.state;
    transitionPeriod = other.transitionPeriod;
    invalidArgs = false;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void TransitionLampsLampGroupsToState::Set(const ajn::MsgArg& component)
{
    MsgArg* lampList;
    size_t numLamps;
    MsgArg* lampGroupList;
    size_t numLampGroups;
    MsgArg* stateArgs;
    size_t stateArgsSize;
    invalidArgs =  false;

    component.Get("(asasa{sv}u)", &numLamps, &lampList, &numLampGroups, &lampGroupList, &stateArgsSize, &stateArgs, &transitionPeriod);
    CreateUniqueList(lamps, lampList, numLamps);
    CreateUniqueList(lampGroups, lampGroupList, numLampGroups);

    MsgArg arg;
    arg.Set("a{sv}", stateArgsSize, stateArgs);
    state.Set(arg);
    if (state.nullState) {
        QCC_LogError(ER_FAIL, ("%s: TransitionLampsLampGroupsToState cannot include NULL state", __FUNCTION__));
        invalidArgs =  true;
    }
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void TransitionLampsLampGroupsToState::Get(ajn::MsgArg* component) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    size_t numLamps = lamps.size();
    const char** lampList = NULL;
    if (numLamps) {
        lampList = new const char*[numLamps];
        size_t j = 0;
        for (LSFStringList::const_iterator nit = lamps.begin(); nit != lamps.end(); nit++) {
            lampList[j++] = nit->c_str();
        }
    }

    size_t numLampGroups = lampGroups.size();
    const char** lampGroupList = NULL;
    if (numLampGroups) {
        lampGroupList = new const char*[numLampGroups];
        size_t j = 0;
        for (LSFStringList::const_iterator nit = lampGroups.begin(); nit != lampGroups.end(); nit++) {
            lampGroupList[j++] = nit->c_str();
        }
    }

    size_t stateArgsSize;
    MsgArg* stateArgs;
    MsgArg stateArg;

    state.Get(&stateArg);
    stateArg.Get("a{sv}", &stateArgsSize, &stateArgs);

    component->Set("(asasa{sv}u)", numLamps, lampList, numLampGroups, lampGroupList, stateArgsSize, stateArgs, transitionPeriod);
    component->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
}

TransitionLampsLampGroupsToPreset::TransitionLampsLampGroupsToPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& presetID, uint32_t& transPeriod) :
    lamps(lampList), lampGroups(lampGroupList), presetID(presetID), transitionPeriod(transPeriod), invalidArgs(false)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

TransitionLampsLampGroupsToPreset::TransitionLampsLampGroupsToPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& presetID) :
    lamps(lampList), lampGroups(lampGroupList), presetID(presetID), transitionPeriod(0), invalidArgs(false)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

TransitionLampsLampGroupsToPreset::TransitionLampsLampGroupsToPreset(const ajn::MsgArg& component)
{
    Set(component);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

const char* TransitionLampsLampGroupsToPreset::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret = qcc::String("TransitionLampsLampGroupsToPreset::Lamps:") + qcc::String("\n");
    for (LSFStringList::const_iterator it = lamps.begin(); it != lamps.end(); it++) {
        ret += qcc::String((*it).c_str()) + qcc::String("\n");
    }
    ret += qcc::String("TransitionLampsLampGroupsToPreset::Lamp Groups:") + qcc::String("\n");
    for (LSFStringList::const_iterator it = lampGroups.begin(); it != lampGroups.end(); it++) {
        ret += qcc::String((*it).c_str()) + qcc::String("\n");
    }

    ret += qcc::String("TransitionLampsLampGroupsToPreset::Preset:") + qcc::String(presetID.c_str()) + qcc::String("\n");
    ret += qcc::String("TransitionLampsLampGroupsToPreset::TransitionPeriod:") + U32ToString(transitionPeriod) + qcc::String("\n");

    return ret.c_str();
}

TransitionLampsLampGroupsToPreset::TransitionLampsLampGroupsToPreset(const TransitionLampsLampGroupsToPreset& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), presetID(other.presetID), transitionPeriod(other.transitionPeriod), invalidArgs(false)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

TransitionLampsLampGroupsToPreset& TransitionLampsLampGroupsToPreset::operator=(const TransitionLampsLampGroupsToPreset& other)
{
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    presetID = other.presetID;
    transitionPeriod = other.transitionPeriod;
    invalidArgs = false;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void TransitionLampsLampGroupsToPreset::Set(const ajn::MsgArg& component)
{
    MsgArg* lampList;
    size_t numLamps;
    MsgArg* lampGroupList;
    size_t numLampGroups;
    const char* presetId;
    invalidArgs =  false;

    component.Get("(asassu)", &numLamps, &lampList, &numLampGroups, &lampGroupList, &presetId, &transitionPeriod);
    CreateUniqueList(lamps, lampList, numLamps);
    CreateUniqueList(lampGroups, lampGroupList, numLampGroups);

    presetID = LSFString(presetId);

    if (0 == strcmp(presetID.c_str(), CurrentStateIdentifier.c_str())) {
        QCC_LogError(ER_FAIL, ("%s: TransitionLampsLampGroupsToPreset cannot include current state", __FUNCTION__));
        invalidArgs =  true;
    }

    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void TransitionLampsLampGroupsToPreset::Get(ajn::MsgArg* component) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    size_t numLamps = lamps.size();
    const char** lampList = NULL;
    if (numLamps) {
        lampList = new const char*[numLamps];
        size_t j = 0;
        for (LSFStringList::const_iterator nit = lamps.begin(); nit != lamps.end(); nit++) {
            lampList[j++] = nit->c_str();
        }
    }

    size_t numLampGroups = lampGroups.size();
    const char** lampGroupList = NULL;
    if (numLampGroups) {
        lampGroupList = new const char*[numLampGroups];
        size_t j = 0;
        for (LSFStringList::const_iterator nit = lampGroups.begin(); nit != lampGroups.end(); nit++) {
            lampGroupList[j++] = nit->c_str();
        }
    }

    component->Set("(asassu)", numLamps, lampList, numLampGroups, lampGroupList, presetID.c_str(), transitionPeriod);
    component->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
}

PulseLampsLampGroupsWithState::PulseLampsLampGroupsWithState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& fromLampState, LampState& toLampState, uint32_t& period, uint32_t& duration, uint32_t& numOfPulses) :
    lamps(lampList), lampGroups(lampGroupList), fromState(fromLampState), toState(toLampState), pulsePeriod(period), pulseDuration(duration), numPulses(numOfPulses), invalidArgs(false)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

PulseLampsLampGroupsWithState::PulseLampsLampGroupsWithState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& toLampState, uint32_t& period, uint32_t& duration, uint32_t& numOfPulses) :
    lamps(lampList), lampGroups(lampGroupList), fromState(LampState()), toState(toLampState), pulsePeriod(period), pulseDuration(duration), numPulses(numOfPulses), invalidArgs(false)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

PulseLampsLampGroupsWithState::PulseLampsLampGroupsWithState(const ajn::MsgArg& component)
{
    Set(component);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

const char* PulseLampsLampGroupsWithState::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret = qcc::String("PulseLampsLampGroupsWithState::Lamps:") + qcc::String("\n");
    for (LSFStringList::const_iterator it = lamps.begin(); it != lamps.end(); it++) {
        ret += qcc::String((*it).c_str()) + qcc::String("\n");
    }
    ret += qcc::String("PulseLampsLampGroupsWithState::Lamp Groups:") + qcc::String("\n");
    for (LSFStringList::const_iterator it = lampGroups.begin(); it != lampGroups.end(); it++) {
        ret += qcc::String((*it).c_str()) + qcc::String("\n");
    }

    ret += qcc::String("PulseLampsLampGroupsWithState::FromState:") + qcc::String(fromState.c_str()) + qcc::String("\n");
    ret += qcc::String("PulseLampsLampGroupsWithState::ToState:") + qcc::String(toState.c_str()) + qcc::String("\n");
    ret += qcc::String("PulseLampsLampGroupsWithState::PulsePeriod:") + U32ToString(pulsePeriod) + qcc::String("\n");
    ret += qcc::String("PulseLampsLampGroupsWithState::PulseDuration:") + U32ToString(pulseDuration) + qcc::String("\n");
    ret += qcc::String("PulseLampsLampGroupsWithState::NumPulses:") + U32ToString(numPulses) + qcc::String("\n");

    return ret.c_str();
}

PulseLampsLampGroupsWithState::PulseLampsLampGroupsWithState(const PulseLampsLampGroupsWithState& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), fromState(other.fromState), toState(other.toState), pulsePeriod(other.pulsePeriod), pulseDuration(other.pulseDuration), numPulses(other.numPulses), invalidArgs(false)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

PulseLampsLampGroupsWithState& PulseLampsLampGroupsWithState::operator=(const PulseLampsLampGroupsWithState& other)
{
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    fromState = other.fromState;
    toState = other.toState;
    pulsePeriod = other.pulsePeriod;
    pulseDuration = other.pulseDuration;
    numPulses = other.numPulses;
    invalidArgs = false;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void PulseLampsLampGroupsWithState::Set(const ajn::MsgArg& component)
{
    MsgArg* lampList;
    size_t numLamps;
    MsgArg* lampGroupList;
    size_t numLampGroups;
    MsgArg* fromStateArgs;
    size_t fromStateArgsSize;
    MsgArg* toStateArgs;
    size_t toStateArgsSize;
    invalidArgs =  false;

    component.Get("(asasa{sv}a{sv}uuu)", &numLamps, &lampList, &numLampGroups, &lampGroupList, &fromStateArgsSize, &fromStateArgs, &toStateArgsSize, &toStateArgs, &pulsePeriod, &pulseDuration, &numPulses);
    CreateUniqueList(lamps, lampList, numLamps);
    CreateUniqueList(lampGroups, lampGroupList, numLampGroups);

    MsgArg fromArg;
    fromArg.Set("a{sv}", fromStateArgsSize, fromStateArgs);
    fromState.Set(fromArg);

    MsgArg toArg;
    toArg.Set("a{sv}", toStateArgsSize, toStateArgs);
    toState.Set(toArg);

    if (toState.nullState) {
        QCC_LogError(ER_FAIL, ("%s: PulseLampsLampGroupsWithState cannot include to state as NULL state", __FUNCTION__));
        invalidArgs =  true;
    }

    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void PulseLampsLampGroupsWithState::Get(ajn::MsgArg* component) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    size_t numLamps = lamps.size();
    const char** lampList = NULL;
    if (numLamps) {
        lampList = new const char*[numLamps];
        size_t j = 0;
        for (LSFStringList::const_iterator nit = lamps.begin(); nit != lamps.end(); nit++) {
            lampList[j++] = nit->c_str();
        }
    }

    size_t numLampGroups = lampGroups.size();
    const char** lampGroupList = NULL;
    if (numLampGroups) {
        lampGroupList = new const char*[numLampGroups];
        size_t j = 0;
        for (LSFStringList::const_iterator nit = lampGroups.begin(); nit != lampGroups.end(); nit++) {
            lampGroupList[j++] = nit->c_str();
        }
    }

    size_t fromStateArgsSize;
    MsgArg* fromStateArgs;
    MsgArg fromStateArg;

    fromState.Get(&fromStateArg);
    fromStateArg.Get("a{sv}", &fromStateArgsSize, &fromStateArgs);

    size_t toStateArgsSize;
    MsgArg* toStateArgs;
    MsgArg toStateArg;

    toState.Get(&toStateArg);
    toStateArg.Get("a{sv}", &toStateArgsSize, &toStateArgs);

    component->Set("(asasa{sv}a{sv}uuu)", numLamps, lampList, numLampGroups, lampGroupList, fromStateArgsSize, fromStateArgs, toStateArgsSize, toStateArgs, pulsePeriod, pulseDuration, numPulses);
    component->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
}

PulseLampsLampGroupsWithPreset::PulseLampsLampGroupsWithPreset(LSFStringList& lampList, LSFStringList& lampGroupList,  LSFString& fromPreset, LSFString& toPreset, uint32_t& period, uint32_t& duration, uint32_t& numOfPulses) :
    lamps(lampList), lampGroups(lampGroupList), fromPreset(fromPreset), toPreset(toPreset), pulsePeriod(period), pulseDuration(duration), numPulses(numOfPulses), invalidArgs(false)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

PulseLampsLampGroupsWithPreset::PulseLampsLampGroupsWithPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& toPreset, uint32_t& period, uint32_t& duration, uint32_t& numOfPulses) :
    lamps(lampList), lampGroups(lampGroupList), fromPreset(CurrentStateIdentifier), toPreset(toPreset), pulsePeriod(period), pulseDuration(duration), numPulses(numOfPulses), invalidArgs(false)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

PulseLampsLampGroupsWithPreset::PulseLampsLampGroupsWithPreset(const ajn::MsgArg& component)
{
    Set(component);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

const char* PulseLampsLampGroupsWithPreset::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret = qcc::String("PulseLampsLampGroupsWithPreset::Lamps:") + qcc::String("\n");
    for (LSFStringList::const_iterator it = lamps.begin(); it != lamps.end(); it++) {
        ret += qcc::String((*it).c_str()) + qcc::String("\n");
    }
    ret += qcc::String("PulseLampsLampGroupsWithPreset::Lamp Groups:") + qcc::String("\n");
    for (LSFStringList::const_iterator it = lampGroups.begin(); it != lampGroups.end(); it++) {
        ret += qcc::String((*it).c_str()) + qcc::String("\n");
    }

    ret += qcc::String("PulseLampsLampGroupsWithPreset::FromPreset:") + qcc::String(fromPreset.c_str()) + qcc::String("\n");
    ret += qcc::String("PulseLampsLampGroupsWithPreset::ToPreset:") + qcc::String(toPreset.c_str()) + qcc::String("\n");
    ret += qcc::String("PulseLampsLampGroupsWithPreset::PulsePeriod:") + U32ToString(pulsePeriod) + qcc::String("\n");
    ret += qcc::String("PulseLampsLampGroupsWithPreset::PulseDuration:") + U32ToString(pulseDuration) + qcc::String("\n");
    ret += qcc::String("PulseLampsLampGroupsWithPreset::NumPulses:") + U32ToString(numPulses) + qcc::String("\n");

    return ret.c_str();
}

PulseLampsLampGroupsWithPreset::PulseLampsLampGroupsWithPreset(const PulseLampsLampGroupsWithPreset& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), fromPreset(other.fromPreset), toPreset(other.toPreset), pulsePeriod(other.pulsePeriod), pulseDuration(other.pulseDuration), numPulses(other.numPulses), invalidArgs(false)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

PulseLampsLampGroupsWithPreset& PulseLampsLampGroupsWithPreset::operator=(const PulseLampsLampGroupsWithPreset& other)
{
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    fromPreset = other.fromPreset;
    toPreset = other.toPreset;
    pulsePeriod = other.pulsePeriod;
    pulseDuration = other.pulseDuration;
    numPulses = other.numPulses;
    invalidArgs = false;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void PulseLampsLampGroupsWithPreset::Set(const ajn::MsgArg& component)
{
    MsgArg* lampList;
    size_t numLamps;
    MsgArg* lampGroupList;
    size_t numLampGroups;
    const char* fromPresetId;
    const char* toPresetId;
    invalidArgs =  false;

    component.Get("(asasssuuu)", &numLamps, &lampList, &numLampGroups, &lampGroupList, &fromPresetId, &toPresetId, &pulsePeriod, &pulseDuration, &numPulses);
    CreateUniqueList(lamps, lampList, numLamps);
    CreateUniqueList(lampGroups, lampGroupList, numLampGroups);

    fromPreset = LSFString(fromPresetId);
    toPreset = LSFString(toPresetId);

    if (0 == strcmp(toPreset.c_str(), CurrentStateIdentifier.c_str())) {
        QCC_LogError(ER_FAIL, ("%s: PulseLampsLampGroupsWithPreset cannot include to state as the current state", __FUNCTION__));
        invalidArgs =  true;
    }

    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void PulseLampsLampGroupsWithPreset::Get(ajn::MsgArg* component) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    size_t numLamps = lamps.size();
    const char** lampList = NULL;
    if (numLamps) {
        lampList = new const char*[numLamps];
        size_t j = 0;
        for (LSFStringList::const_iterator nit = lamps.begin(); nit != lamps.end(); nit++) {
            lampList[j++] = nit->c_str();
        }
    }

    size_t numLampGroups = lampGroups.size();
    const char** lampGroupList = NULL;
    if (numLampGroups) {
        lampGroupList = new const char*[numLampGroups];
        size_t j = 0;
        for (LSFStringList::const_iterator nit = lampGroups.begin(); nit != lampGroups.end(); nit++) {
            lampGroupList[j++] = nit->c_str();
        }
    }

    component->Set("(asasssuuu)", numLamps, lampList, numLampGroups, lampGroupList, fromPreset.c_str(), toPreset.c_str(), pulsePeriod, pulseDuration, numPulses);
    component->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
}

Scene::Scene() :
    invalidArgs(false)
{
    transitionToStateComponent.clear();
    transitionToPresetComponent.clear();
    pulseWithStateComponent.clear();
    pulseWithPresetComponent.clear();
    //QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

Scene::Scene(const ajn::MsgArg& transitionToStateComponentList, const ajn::MsgArg& transitionToPresetComponentList,
             const ajn::MsgArg& pulseWithStateComponentList, const ajn::MsgArg& pulseWithPresetComponentList) :
    invalidArgs(false)
{
    Set(transitionToStateComponentList, transitionToPresetComponentList, pulseWithStateComponentList, pulseWithPresetComponentList);
    //QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

Scene::Scene(TransitionLampsLampGroupsToStateList& transitionToStateComponentList, TransitionLampsLampGroupsToPresetList& transitionToPresetComponentList,
             PulseLampsLampGroupsWithStateList& pulseWithStateComponentList, PulseLampsLampGroupsWithPresetList& pulseWithPresetComponentList) :
    invalidArgs(false)
{
    transitionToStateComponent.clear();
    transitionToPresetComponent.clear();
    pulseWithStateComponent.clear();
    pulseWithPresetComponent.clear();
    transitionToStateComponent = transitionToStateComponentList;
    transitionToPresetComponent = transitionToPresetComponentList;
    pulseWithStateComponent = pulseWithStateComponentList;
    pulseWithPresetComponent = pulseWithPresetComponentList;
    //QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

Scene::Scene(const Scene& other)
{
    transitionToStateComponent.clear();
    transitionToPresetComponent.clear();
    pulseWithStateComponent.clear();
    pulseWithPresetComponent.clear();
    transitionToStateComponent = other.transitionToStateComponent;
    transitionToPresetComponent = other.transitionToPresetComponent;
    pulseWithStateComponent = other.pulseWithStateComponent;
    pulseWithPresetComponent = other.pulseWithPresetComponent;
    invalidArgs = false;
    //QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

Scene& Scene::operator=(const Scene& other)
{
    transitionToStateComponent.clear();
    transitionToPresetComponent.clear();
    pulseWithStateComponent.clear();
    pulseWithPresetComponent.clear();
    transitionToStateComponent = other.transitionToStateComponent;
    transitionToPresetComponent = other.transitionToPresetComponent;
    pulseWithStateComponent = other.pulseWithStateComponent;
    pulseWithPresetComponent = other.pulseWithPresetComponent;
    invalidArgs = false;
    //QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

const char* Scene::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret = qcc::String("Scene::TransitionToState Components:") + qcc::String("\n");
    for (TransitionLampsLampGroupsToStateList::const_iterator it = transitionToStateComponent.begin(); it != transitionToStateComponent.end(); it++) {
        ret += qcc::String(it->c_str()) + qcc::String("\n");
    }
    ret += qcc::String("Scene::TransitionToPreset Components:") + qcc::String("\n");
    for (TransitionLampsLampGroupsToPresetList::const_iterator it = transitionToPresetComponent.begin(); it != transitionToPresetComponent.end(); it++) {
        ret += qcc::String(it->c_str()) + qcc::String("\n");
    }
    ret += qcc::String("Scene::PulseWithState Components:") + qcc::String("\n");
    for (PulseLampsLampGroupsWithStateList::const_iterator it = pulseWithStateComponent.begin(); it != pulseWithStateComponent.end(); it++) {
        ret += qcc::String(it->c_str()) + qcc::String("\n");
    }
    ret += qcc::String("Scene::PulseWithPreset Components:") + qcc::String("\n");
    for (PulseLampsLampGroupsWithPresetList::const_iterator it = pulseWithPresetComponent.begin(); it != pulseWithPresetComponent.end(); it++) {
        ret += qcc::String(it->c_str()) + qcc::String("\n");
    }
    return ret.c_str();
}

void Scene::Set(const ajn::MsgArg& transitionToStateComponentList, const ajn::MsgArg& transitionToPresetComponentList, const ajn::MsgArg& pulseWithStateComponentList,
                const ajn::MsgArg& pulseWithPresetComponentList)
{
    transitionToStateComponent.clear();
    transitionToPresetComponent.clear();
    pulseWithStateComponent.clear();
    pulseWithPresetComponent.clear();

    QCC_DbgPrintf(("%s", transitionToStateComponentList.ToString().c_str()));
    QCC_DbgPrintf(("%s", transitionToPresetComponentList.ToString().c_str()));
    QCC_DbgPrintf(("%s", pulseWithStateComponentList.ToString().c_str()));
    QCC_DbgPrintf(("%s", pulseWithPresetComponentList.ToString().c_str()));

    MsgArg* transitionToStateComponentArray;
    size_t transitionToStateComponentSize;
    transitionToStateComponentList.Get("a(asasa{sv}u)", &transitionToStateComponentSize, &transitionToStateComponentArray);

    for (size_t i = 0; i < transitionToStateComponentSize; i++) {
        TransitionLampsLampGroupsToState tempStateComponent(transitionToStateComponentArray[i]);
        if (tempStateComponent.invalidArgs) {
            QCC_LogError(ER_FAIL, ("%s: TransitionLampsLampGroupsToState invalid", __FUNCTION__));
            invalidArgs = true;
            return;
        } else {
            transitionToStateComponent.push_back(tempStateComponent);
        }
    }

    MsgArg* transitionToPresetComponentArray;
    size_t transitionToPresetComponentSize;
    transitionToPresetComponentList.Get("a(asassu)", &transitionToPresetComponentSize, &transitionToPresetComponentArray);
    for (size_t i = 0; i < transitionToPresetComponentSize; i++) {
        TransitionLampsLampGroupsToPreset tempPresetComponent(transitionToPresetComponentArray[i]);
        if (tempPresetComponent.invalidArgs) {
            QCC_LogError(ER_FAIL, ("%s: TransitionLampsLampGroupsToPreset invalid", __FUNCTION__));
            invalidArgs = true;
            return;
        } else {
            transitionToPresetComponent.push_back(tempPresetComponent);
        }
    }

    MsgArg* pulseWithStateComponentArray;
    size_t pulseWithStateComponentSize;
    pulseWithStateComponentList.Get("a(asasa{sv}a{sv}uuu)", &pulseWithStateComponentSize, &pulseWithStateComponentArray);
    for (size_t i = 0; i < pulseWithStateComponentSize; i++) {
        PulseLampsLampGroupsWithState tempPulseComponent(pulseWithStateComponentArray[i]);
        if (tempPulseComponent.invalidArgs) {
            QCC_LogError(ER_FAIL, ("%s: PulseLampsLampGroupsWithState invalid", __FUNCTION__));
            invalidArgs = true;
            return;
        } else {
            pulseWithStateComponent.push_back(tempPulseComponent);
        }
    }

    MsgArg* pulseWithPresetComponentArray;
    size_t pulseWithPresetComponentSize;
    pulseWithPresetComponentList.Get("a(asasssuuu)", &pulseWithPresetComponentSize, &pulseWithPresetComponentArray);
    for (size_t i = 0; i < pulseWithPresetComponentSize; i++) {
        PulseLampsLampGroupsWithPreset tempPulseComponent(pulseWithPresetComponentArray[i]);
        if (tempPulseComponent.invalidArgs) {
            QCC_LogError(ER_FAIL, ("%s: PulseLampsLampGroupsWithPreset invalid", __FUNCTION__));
            invalidArgs = true;
            return;
        } else {
            pulseWithPresetComponent.push_back(tempPulseComponent);
        }
    }

    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void Scene::Get(ajn::MsgArg* transitionToStateComponentList, ajn::MsgArg* transitionToPresetComponentList, ajn::MsgArg* pulseWithStateComponentList,
                ajn::MsgArg* pulseWithPresetComponentList) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    size_t transitionToStateComponentArraySize = transitionToStateComponent.size();
    if (transitionToStateComponentArraySize) {
        MsgArg* transitionToStateComponentArray = new MsgArg[transitionToStateComponentArraySize];
        size_t i = 0;
        for (TransitionLampsLampGroupsToStateList::const_iterator it = transitionToStateComponent.begin(); it != transitionToStateComponent.end(); it++, i++) {
            it->Get(&transitionToStateComponentArray[i]);
        }
        transitionToStateComponentList->Set("a(asasa{sv}u)", transitionToStateComponentArraySize, transitionToStateComponentArray);
        transitionToStateComponentList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        transitionToStateComponentList->Set("a(asasa{sv}u)", 0, NULL);
    }

    size_t transitionToPresetComponentArraySize = transitionToPresetComponent.size();
    if (transitionToPresetComponentArraySize) {
        MsgArg* transitionToPresetComponentArray = new MsgArg[transitionToPresetComponentArraySize];
        size_t i = 0;
        for (TransitionLampsLampGroupsToPresetList::const_iterator it = transitionToPresetComponent.begin(); it != transitionToPresetComponent.end(); it++, i++) {
            it->Get(&transitionToPresetComponentArray[i]);
        }
        transitionToPresetComponentList->Set("a(asassu)", transitionToPresetComponentArraySize, transitionToPresetComponentArray);
        transitionToPresetComponentList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        transitionToPresetComponentList->Set("a(asassu)", 0, NULL);
    }

    size_t pulseWithStateComponentArraySize = pulseWithStateComponent.size();
    if (pulseWithStateComponentArraySize) {
        MsgArg* pulseWithStateComponentArray = new MsgArg[pulseWithStateComponentArraySize];
        size_t i = 0;
        for (PulseLampsLampGroupsWithStateList::const_iterator it = pulseWithStateComponent.begin(); it != pulseWithStateComponent.end(); it++, i++) {
            it->Get(&pulseWithStateComponentArray[i]);
        }
        pulseWithStateComponentList->Set("a(asasa{sv}a{sv}uuu)", pulseWithStateComponentArraySize, pulseWithStateComponentArray);
        pulseWithStateComponentList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        pulseWithStateComponentList->Set("a(asasa{sv}a{sv}uuu)", 0, NULL);
    }

    size_t pulseWithPresetComponentArraySize = pulseWithPresetComponent.size();
    if (pulseWithPresetComponentArraySize) {
        MsgArg* pulseWithPresetComponentArray = new MsgArg[pulseWithPresetComponentArraySize];
        size_t i = 0;
        for (PulseLampsLampGroupsWithPresetList::const_iterator it = pulseWithPresetComponent.begin(); it != pulseWithPresetComponent.end(); it++, i++) {
            it->Get(&pulseWithPresetComponentArray[i]);
        }
        pulseWithPresetComponentList->Set("a(asasssuuu)", pulseWithPresetComponentArraySize, pulseWithPresetComponentArray);
        pulseWithPresetComponentList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        pulseWithPresetComponentList->Set("a(asasssuuu)", 0, NULL);
    }
}

LSFResponseCode Scene::IsDependentOnPreset(LSFString& presetID)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    LSFResponseCode responseCode = LSF_OK;

    for (TransitionLampsLampGroupsToPresetList::iterator it = transitionToPresetComponent.begin(); it != transitionToPresetComponent.end(); it++) {
        if (0 == strcmp(it->presetID.c_str(), presetID.c_str())) {
            responseCode = LSF_ERR_DEPENDENCY;
        }
    }

    if (LSF_OK == responseCode) {
        for (PulseLampsLampGroupsWithPresetList::iterator it = pulseWithPresetComponent.begin(); it != pulseWithPresetComponent.end(); it++) {
            if (0 == strcmp(it->fromPreset.c_str(), presetID.c_str())) {
                responseCode = LSF_ERR_DEPENDENCY;
            } else if (0 == strcmp(it->toPreset.c_str(), presetID.c_str())) {
                responseCode = LSF_ERR_DEPENDENCY;
            }
        }
    }

    return responseCode;
}

LSFResponseCode Scene::IsDependentOnLampGroup(LSFString& lampGroupID)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    LSFResponseCode responseCode = LSF_OK;

    for (TransitionLampsLampGroupsToStateList::iterator it = transitionToStateComponent.begin(); it != transitionToStateComponent.end(); it++) {
        LSFStringList::iterator findIt = std::find(it->lampGroups.begin(), it->lampGroups.end(), lampGroupID);
        if (findIt != it->lampGroups.end()) {
            responseCode = LSF_ERR_DEPENDENCY;
        }
    }

    if (LSF_OK == responseCode) {
        for (PulseLampsLampGroupsWithStateList::iterator it = pulseWithStateComponent.begin(); it != pulseWithStateComponent.end(); it++) {
            LSFStringList::iterator findIt = std::find(it->lampGroups.begin(), it->lampGroups.end(), lampGroupID);
            if (findIt != it->lampGroups.end()) {
                responseCode = LSF_ERR_DEPENDENCY;
            }
        }
    }

    if (LSF_OK == responseCode) {
        for (TransitionLampsLampGroupsToPresetList::iterator it = transitionToPresetComponent.begin(); it != transitionToPresetComponent.end(); it++) {
            LSFStringList::iterator findIt = std::find(it->lampGroups.begin(), it->lampGroups.end(), lampGroupID);
            if (findIt != it->lampGroups.end()) {
                responseCode = LSF_ERR_DEPENDENCY;
            }
        }
    }

    if (LSF_OK == responseCode) {
        for (PulseLampsLampGroupsWithPresetList::iterator it = pulseWithPresetComponent.begin(); it != pulseWithPresetComponent.end(); it++) {
            LSFStringList::iterator findIt = std::find(it->lampGroups.begin(), it->lampGroups.end(), lampGroupID);
            if (findIt != it->lampGroups.end()) {
                responseCode = LSF_ERR_DEPENDENCY;
            }
        }
    }

    return responseCode;
}

MasterScene::MasterScene()
{
    scenes.clear();
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

MasterScene::MasterScene(const ajn::MsgArg& sceneList)
{
    Set(sceneList);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

MasterScene::MasterScene(LSFStringList sceneList) :
    scenes(sceneList)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

MasterScene::MasterScene(const MasterScene& other)
{
    scenes.clear();
    scenes = other.scenes;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

const char* MasterScene::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret = qcc::String("MasterScene::Scenes:") + qcc::String("\n");
    for (LSFStringList::const_iterator it = scenes.begin(); it != scenes.end(); it++) {
        ret += qcc::String((*it).c_str()) + qcc::String("\n");
    }
    return ret.c_str();
}

MasterScene& MasterScene::operator=(const MasterScene& other)
{
    scenes.clear();
    scenes = other.scenes;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void MasterScene::Set(const ajn::MsgArg& sceneList)
{
    scenes.clear();

    MsgArg* gidsArray;
    size_t gidsSize;
    sceneList.Get("as", &gidsSize, &gidsArray);
    CreateUniqueList(scenes, gidsArray, gidsSize);

    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void MasterScene::Get(ajn::MsgArg* sceneList) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    size_t gidsVecSize = scenes.size();
    if (gidsVecSize) {
        const char** gidsVec = new const char*[gidsVecSize];
        size_t i = 0;
        for (LSFStringList::const_iterator it = scenes.begin(); it != scenes.end(); it++) {
            gidsVec[i++] = it->c_str();
        }
        sceneList->Set("as", gidsVecSize, gidsVec);
        sceneList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        sceneList->Set("as", 0, NULL);
    }
}

LSFResponseCode MasterScene::IsDependentOnScene(LSFString& sceneID)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    LSFResponseCode responseCode = LSF_OK;

    LSFStringList::iterator findIt = std::find(scenes.begin(), scenes.end(), sceneID);
    if (findIt != scenes.end()) {
        responseCode = LSF_ERR_DEPENDENCY;
    }

    return responseCode;
}

}
