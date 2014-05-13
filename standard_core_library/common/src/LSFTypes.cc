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

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "LSF_TYPES"

LampState::LampState() :
    onOff(false),
    hue(0),
    saturation(0),
    colorTemp(0),
    brightness(0)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampState::LampState(bool onOff, uint32_t hue, uint32_t saturation, uint32_t colorTemp, uint32_t brightness) :
    onOff(onOff),
    hue(hue),
    saturation(saturation),
    colorTemp(colorTemp),
    brightness(brightness)
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
    brightness(other.brightness)
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
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

const char* LampState::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret.append("\nonOff=");
    ret.append(qcc::U32ToString(onOff));
    ret.append("\nhue=");
    ret.append(qcc::U32ToString(hue));
    ret.append("\nsaturation=");
    ret.append(qcc::U32ToString(saturation));
    ret.append("\nbrightness=");
    ret.append(qcc::U32ToString(brightness));
    ret.append("\ncolorTemp=");
    ret.append(qcc::U32ToString(colorTemp));
    return ret.c_str();
}

void LampState::Set(const ajn::MsgArg& arg)
{
    MsgArg* args;
    size_t numArgs;
    arg.Get("a{sv}", &numArgs, &args);

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
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void LampState::Get(ajn::MsgArg* arg) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
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

LampParameters::LampParameters() :
    energy_usage_milliwatts(0),
    brightness_lumens(0)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampParameters::LampParameters(const ajn::MsgArg& arg)
{
    Set(arg);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampParameters::LampParameters(const LampParameters& other) :
    energy_usage_milliwatts(other.energy_usage_milliwatts),
    brightness_lumens(other.brightness_lumens)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampParameters& LampParameters::operator=(const LampParameters& other)
{
    energy_usage_milliwatts = other.energy_usage_milliwatts;
    brightness_lumens = other.brightness_lumens;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

const char* LampParameters::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret.append("\nenergy_usage_milliwatts=");
    ret.append(qcc::U32ToString(energy_usage_milliwatts));
    ret.append("\nbrightness_lumens=");
    ret.append(qcc::U32ToString(brightness_lumens));
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
            value->Get("u", &energy_usage_milliwatts);
        } else if (0 == strcmp(field, "Brightness_Lumens")) {
            value->Get("u", &brightness_lumens);
        }
    }
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void LampParameters::Get(ajn::MsgArg* arg) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    const char* str[] = { "Energy_Usage_Milliwatts", "Brightness_Lumens" };
    MsgArg* dict = new MsgArg[2];

    MsgArg* var = new MsgArg("u", energy_usage_milliwatts);
    dict[0].Set("{sv}", str[0], var);
    dict[0].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", brightness_lumens);
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
    hardwareVersion(other.hardwareVersion),
    firmwareVersion(other.firmwareVersion),
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
    voltage(other.voltage),
    wattage(other.wattage),
    wattageEquivalent(other.wattageEquivalent),
    maxOutput(other.maxOutput),
    minTemperature(other.minTemperature),
    maxTemperature(other.maxTemperature),
    colorRenderingIndex(other.colorRenderingIndex),
    lifespan(other.lifespan),
    lampID(other.lampID)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampDetails& LampDetails::operator=(const LampDetails& other)
{
    hardwareVersion = other.hardwareVersion;
    firmwareVersion = other.firmwareVersion;
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
    voltage = other.voltage;
    wattage = other.wattage;
    wattageEquivalent = other.wattageEquivalent;
    maxOutput = other.maxOutput;
    minTemperature = other.minTemperature;
    maxTemperature = other.maxTemperature;
    colorRenderingIndex = other.colorRenderingIndex;
    lifespan = other.lifespan;
    lampID = other.lampID;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

const char* LampDetails::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret.append("\nhardwareVersion=");
    ret.append(qcc::U32ToString(hardwareVersion));
    ret.append("\nfirmwareVersion=");
    ret.append(qcc::U32ToString(firmwareVersion));
    ret.append("\nmake=");
    ret.append(qcc::U32ToString(make));
    ret.append("\nmodel=");
    ret.append(qcc::U32ToString(model));
    ret.append("\ntype=");
    ret.append(qcc::U32ToString(type));
    ret.append("\nlampType=");
    ret.append(qcc::U32ToString(lampType));
    ret.append("\nlampBaseType=");
    ret.append(qcc::U32ToString(lampBaseType));
    ret.append("\nlampBeamAngle=");
    ret.append(qcc::U32ToString(lampBeamAngle));
    ret.append("\ndimmable=");
    ret.append(qcc::U32ToString(dimmable));
    ret.append("\ncolor=");
    ret.append(qcc::U32ToString(color));
    ret.append("\nvariableColorTemp=");
    ret.append(qcc::U32ToString(variableColorTemp));
    ret.append("\nhasEffects=");
    ret.append(qcc::U32ToString(hasEffects));
    ret.append("\nvoltage=");
    ret.append(qcc::U32ToString(voltage));
    ret.append("\nwattage=");
    ret.append(qcc::U32ToString(wattage));
    ret.append("\nwattageEquivalent=");
    ret.append(qcc::U32ToString(wattageEquivalent));
    ret.append("\nmaxOutput=");
    ret.append(qcc::U32ToString(maxOutput));
    ret.append("\nminTemperature=");
    ret.append(qcc::U32ToString(minTemperature));
    ret.append("\nmaxTemperature=");
    ret.append(qcc::U32ToString(maxTemperature));
    ret.append("\ncolorRenderingIndex=");
    ret.append(qcc::U32ToString(colorRenderingIndex));
    ret.append("\nlifespan=");
    ret.append(qcc::U32ToString(lifespan));
    ret.append("\nlampID=");
    ret.append(lampID.c_str());
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

        if (0 == strcmp(field, "HardwareVersion")) {
            value->Get("u", &hardwareVersion);
        } else if (0 == strcmp(field, "FirmwareVersion")) {
            value->Get("u", &firmwareVersion);
        } else if (0 == strcmp(field, "Make")) {
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
        } else if (0 == strcmp(field, "Voltage")) {
            value->Get("u", &voltage);
        } else if (0 == strcmp(field, "Wattage")) {
            value->Get("u", &wattage);
        } else if (0 == strcmp(field, "WattageEquivalent")) {
            value->Get("u", &wattageEquivalent);
        } else if (0 == strcmp(field, "MaxOutput")) {
            value->Get("u", &maxOutput);
        } else if (0 == strcmp(field, "MinTemperature")) {
            value->Get("u", &minTemperature);
        } else if (0 == strcmp(field, "MaxTemperature")) {
            value->Get("u", &maxTemperature);
        } else if (0 == strcmp(field, "ColorRenderingIndex")) {
            value->Get("u", &colorRenderingIndex);
        } else if (0 == strcmp(field, "Lifespan")) {
            value->Get("u", &lifespan);
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
        "HardwareVersion",
        "FirmwareVersion",
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
        "Voltage",
        "Wattage",
        "WattageEquivalent",
        "MaxOutput",
        "MinTemperature",
        "MaxTemperature",
        "ColorRenderingIndex",
        "Lifespan",
        "LampID"
    };

    MsgArg* dict = new MsgArg[22];

    MsgArg* var = new MsgArg("u", hardwareVersion);
    dict[0].Set("{sv}", str[0], var);
    dict[0].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", firmwareVersion);
    dict[1].Set("{sv}", str[1], var);
    dict[1].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", make);
    dict[2].Set("{sv}", str[3], var);
    dict[2].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", model);
    dict[3].Set("{sv}", str[4], var);
    dict[3].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", type);
    dict[4].Set("{sv}", str[5], var);
    dict[4].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", lampType);
    dict[5].Set("{sv}", str[6], var);
    dict[5].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", lampBaseType);
    dict[6].Set("{sv}", str[7], var);
    dict[6].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", lampBeamAngle);
    dict[7].Set("{sv}", str[8], var);
    dict[7].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("b", dimmable);
    dict[8].Set("{sv}", str[9], var);
    dict[8].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("b", color);
    dict[9].Set("{sv}", str[10], var);
    dict[9].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("b", variableColorTemp);
    dict[10].Set("{sv}", str[11], var);
    dict[10].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("b", hasEffects);
    dict[11].Set("{sv}", str[12], var);
    dict[11].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", voltage);
    dict[12].Set("{sv}", str[13], var);
    dict[12].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", wattage);
    dict[13].Set("{sv}", str[14], var);
    dict[13].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", wattageEquivalent);
    dict[14].Set("{sv}", str[15], var);
    dict[14].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", maxOutput);
    dict[15].Set("{sv}", str[16], var);
    dict[15].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", minTemperature);
    dict[16].Set("{sv}", str[17], var);
    dict[16].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", maxTemperature);
    dict[17].Set("{sv}", str[18], var);
    dict[17].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", colorRenderingIndex);
    dict[18].Set("{sv}", str[19], var);
    dict[18].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("u", lifespan);
    dict[19].Set("{sv}", str[20], var);
    dict[19].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("s", lampID.c_str());
    dict[20].Set("{sv}", str[21], var);
    dict[20].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    arg->Set("a{sv}", (size_t)21, dict);
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

    ret.append("Lamps: \n");
    LSFStringList::const_iterator it;
    for (it = lamps.begin(); it != lamps.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
    }
    ret.append("Lamp Groups: \n");
    for (it = lampGroups.begin(); it != lampGroups.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
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
    for (size_t i = 0; i < idsSize; i++) {
        char* lampID;
        idsArray[i].Get("s", &lampID);
        lamps.push_back(LSFString(lampID));
    }

    MsgArg* gidsArray;
    size_t gidsSize;
    lampGroupList.Get("as", &gidsSize, &gidsArray);
    for (size_t i = 0; i < gidsSize; i++) {
        char* gid;
        gidsArray[i].Get("s", &gid);
        lampGroups.push_back(LSFString(gid));
    }
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
    lamps(lampList), lampGroups(lampGroupList), state(lampState), transitionPeriod(transPeriod)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

TransitionLampsLampGroupsToState::TransitionLampsLampGroupsToState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& lampState) :
    lamps(lampList), lampGroups(lampGroupList), state(lampState), transitionPeriod(0)
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
    ret.append("Lamps: \n");
    LSFStringList::const_iterator it;
    for (it = lamps.begin(); it != lamps.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
    }
    ret.append("Lamp Groups: \n");
    for (it = lampGroups.begin(); it != lampGroups.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
    }
    ret.append("State: ");
    ret.append(state.c_str());
    ret.append("\nTransition Period: \n");
    ret.append(qcc::U32ToString(transitionPeriod));
    return ret.c_str();
}

TransitionLampsLampGroupsToState::TransitionLampsLampGroupsToState(const TransitionLampsLampGroupsToState& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), state(other.state), transitionPeriod(other.transitionPeriod)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

TransitionLampsLampGroupsToState& TransitionLampsLampGroupsToState::operator=(const TransitionLampsLampGroupsToState& other)
{
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    state = other.state;
    transitionPeriod = other.transitionPeriod;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void TransitionLampsLampGroupsToState::Set(const ajn::MsgArg& component)
{
    const char** lampList;
    size_t numLamps;
    const char** lampGroupList;
    size_t numLampGroups;
    MsgArg* stateArgs;
    size_t stateArgsSize;

    component.Get("(asasa{sv}u)", &numLamps, &lampList, &numLampGroups, &lampGroupList, &stateArgsSize, &stateArgs, &transitionPeriod);

    for (size_t j = 0; j < numLamps; j++) {
        LSFString id(lampList[j]);
        lamps.push_back(id);
    }

    for (size_t k = 0; k < numLampGroups; k++) {
        LSFString id(lampGroupList[k]);
        lampGroups.push_back(id);
    }

    MsgArg arg;
    arg.Set("a{sv}", stateArgsSize, stateArgs);
    state.Set(arg);
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
    lamps(lampList), lampGroups(lampGroupList), presetID(presetID), transitionPeriod(transPeriod)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

TransitionLampsLampGroupsToPreset::TransitionLampsLampGroupsToPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& presetID) :
    lamps(lampList), lampGroups(lampGroupList), presetID(presetID), transitionPeriod(0)
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
    ret.append("Lamps: \n");
    LSFStringList::const_iterator it;
    for (it = lamps.begin(); it != lamps.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
    }
    ret.append("Lamp Groups: \n");
    for (it = lampGroups.begin(); it != lampGroups.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
    }
    ret.append("Preset ID: \n");
    ret.append(presetID.c_str());
    ret.append("\nTransition Period: \n");
    ret.append(qcc::U32ToString(transitionPeriod));
    return ret.c_str();
}

TransitionLampsLampGroupsToPreset::TransitionLampsLampGroupsToPreset(const TransitionLampsLampGroupsToPreset& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), presetID(other.presetID), transitionPeriod(other.transitionPeriod)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

TransitionLampsLampGroupsToPreset& TransitionLampsLampGroupsToPreset::operator=(const TransitionLampsLampGroupsToPreset& other)
{
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    presetID = other.presetID;
    transitionPeriod = other.transitionPeriod;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void TransitionLampsLampGroupsToPreset::Set(const ajn::MsgArg& component)
{
    const char** lampList;
    size_t numLamps;
    const char** lampGroupList;
    size_t numLampGroups;
    const char* presetId;

    component.Get("(asassu)", &numLamps, &lampList, &numLampGroups, &lampGroupList, &presetId, &transitionPeriod);

    for (size_t j = 0; j < numLamps; j++) {
        LSFString id(lampList[j]);
        lamps.push_back(id);
    }

    for (size_t k = 0; k < numLampGroups; k++) {
        LSFString id(lampGroupList[k]);
        lampGroups.push_back(id);
    }

    presetID = LSFString(presetId);
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
    lamps(lampList), lampGroups(lampGroupList), fromState(fromLampState), toState(toLampState), pulsePeriod(period), pulseDuration(duration), numPulses(numOfPulses)
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
    ret.append("Lamps: \n");
    LSFStringList::const_iterator it;
    for (it = lamps.begin(); it != lamps.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
    }
    ret.append("Lamp Groups: \n");
    for (it = lampGroups.begin(); it != lampGroups.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
    }
    ret.append("From State: ");
    ret.append(fromState.c_str());
    ret.append("To State: ");
    ret.append(toState.c_str());
    ret.append("\nPulse Period: \n");
    ret.append(qcc::U32ToString(pulsePeriod));
    ret.append("\nPulse Duration: \n");
    ret.append(qcc::U32ToString(pulseDuration));
    ret.append("\nNumber Of Pulses: \n");
    ret.append(qcc::U32ToString(numPulses));
    return ret.c_str();
}

PulseLampsLampGroupsWithState::PulseLampsLampGroupsWithState(const PulseLampsLampGroupsWithState& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), fromState(other.fromState), toState(other.toState), pulsePeriod(other.pulsePeriod), pulseDuration(other.pulseDuration), numPulses(other.numPulses)
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
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void PulseLampsLampGroupsWithState::Set(const ajn::MsgArg& component)
{
    const char** lampList;
    size_t numLamps;
    const char** lampGroupList;
    size_t numLampGroups;
    MsgArg* fromStateArgs;
    size_t fromStateArgsSize;
    MsgArg* toStateArgs;
    size_t toStateArgsSize;

    component.Get("(asasa{sv}a{sv}uuu)", &numLamps, &lampList, &numLampGroups, &lampGroupList, &fromStateArgsSize, &fromStateArgs, &toStateArgsSize, &toStateArgs, &pulsePeriod, &pulseDuration, &numPulses);

    for (size_t j = 0; j < numLamps; j++) {
        LSFString id(lampList[j]);
        lamps.push_back(id);
    }

    for (size_t k = 0; k < numLampGroups; k++) {
        LSFString id(lampGroupList[k]);
        lampGroups.push_back(id);
    }

    MsgArg fromArg;
    fromArg.Set("a{sv}", fromStateArgsSize, fromStateArgs);
    fromState.Set(fromArg);

    MsgArg toArg;
    toArg.Set("a{sv}", toStateArgsSize, toStateArgs);
    toState.Set(toArg);

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
    lamps(lampList), lampGroups(lampGroupList), fromPreset(fromPreset), toPreset(toPreset), pulsePeriod(period), pulseDuration(duration), numPulses(numOfPulses)
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
    ret.append("Lamps: \n");
    LSFStringList::const_iterator it;
    for (it = lamps.begin(); it != lamps.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
    }
    ret.append("Lamp Groups: \n");
    for (it = lampGroups.begin(); it != lampGroups.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
    }
    ret.append("From Preset: ");
    ret.append(fromPreset.c_str());
    ret.append("To Preset: ");
    ret.append(toPreset.c_str());
    ret.append("\nPulse Period: \n");
    ret.append(qcc::U32ToString(pulsePeriod));
    ret.append("\nPulse Duration: \n");
    ret.append(qcc::U32ToString(pulseDuration));
    ret.append("\nNumber Of Pulses: \n");
    ret.append(qcc::U32ToString(numPulses));
    return ret.c_str();
}

PulseLampsLampGroupsWithPreset::PulseLampsLampGroupsWithPreset(const PulseLampsLampGroupsWithPreset& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), fromPreset(other.fromPreset), toPreset(other.toPreset), pulsePeriod(other.pulsePeriod), pulseDuration(other.pulseDuration), numPulses(other.numPulses)
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
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void PulseLampsLampGroupsWithPreset::Set(const ajn::MsgArg& component)
{
    const char** lampList;
    size_t numLamps;
    const char** lampGroupList;
    size_t numLampGroups;
    const char* fromPresetId;
    const char* toPresetId;

    component.Get("(asasssuuu)", &numLamps, &lampList, &numLampGroups, &lampGroupList, &fromPresetId, &toPresetId, &pulsePeriod, &pulseDuration, &numPulses);

    for (size_t j = 0; j < numLamps; j++) {
        LSFString id(lampList[j]);
        lamps.push_back(id);
    }

    for (size_t k = 0; k < numLampGroups; k++) {
        LSFString id(lampGroupList[k]);
        lampGroups.push_back(id);
    }

    fromPreset = LSFString(fromPresetId);
    toPreset = LSFString(toPresetId);

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

StrobeLampsLampGroupsWithState::StrobeLampsLampGroupsWithState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& fromLampState, LampState& toLampState, uint32_t& period, uint32_t& numOfStrobes) :
    lamps(lampList), lampGroups(lampGroupList), fromState(fromLampState), toState(toLampState), strobePeriod(period), numStrobes(numOfStrobes)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

StrobeLampsLampGroupsWithState::StrobeLampsLampGroupsWithState(const ajn::MsgArg& component)
{
    Set(component);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

const char* StrobeLampsLampGroupsWithState::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret.append("Lamps: \n");
    LSFStringList::const_iterator it;
    for (it = lamps.begin(); it != lamps.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
    }
    ret.append("Lamp Groups: \n");
    for (it = lampGroups.begin(); it != lampGroups.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
    }
    ret.append("From State: ");
    ret.append(fromState.c_str());
    ret.append("To State: ");
    ret.append(toState.c_str());
    ret.append("\nStrobe Period: \n");
    ret.append(qcc::U32ToString(strobePeriod));
    ret.append("\nNumber Of Strobes: \n");
    ret.append(qcc::U32ToString(numStrobes));
    return ret.c_str();
}

StrobeLampsLampGroupsWithState::StrobeLampsLampGroupsWithState(const StrobeLampsLampGroupsWithState& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), fromState(other.fromState), toState(other.toState), strobePeriod(other.strobePeriod), numStrobes(other.numStrobes)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

StrobeLampsLampGroupsWithState& StrobeLampsLampGroupsWithState::operator=(const StrobeLampsLampGroupsWithState& other)
{
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    fromState = other.fromState;
    toState = other.toState;
    strobePeriod = other.strobePeriod;
    numStrobes = other.numStrobes;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void StrobeLampsLampGroupsWithState::Set(const ajn::MsgArg& component)
{
    const char** lampList;
    size_t numLamps;
    const char** lampGroupList;
    size_t numLampGroups;
    MsgArg* fromStateArgs;
    size_t fromStateArgsSize;
    MsgArg* toStateArgs;
    size_t toStateArgsSize;

    component.Get("(asasa{sv}a{sv}uu)", &numLamps, &lampList, &numLampGroups, &lampGroupList, &fromStateArgsSize, &fromStateArgs, &toStateArgsSize, &toStateArgs, &strobePeriod, &numStrobes);

    for (size_t j = 0; j < numLamps; j++) {
        LSFString id(lampList[j]);
        lamps.push_back(id);
    }

    for (size_t k = 0; k < numLampGroups; k++) {
        LSFString id(lampGroupList[k]);
        lampGroups.push_back(id);
    }

    MsgArg fromArg;
    fromArg.Set("a{sv}", fromStateArgsSize, fromStateArgs);
    fromState.Set(fromArg);

    MsgArg toArg;
    toArg.Set("a{sv}", toStateArgsSize, toStateArgs);
    toState.Set(toArg);

    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void StrobeLampsLampGroupsWithState::Get(ajn::MsgArg* component) const
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

    component->Set("(asasa{sv}a{sv}uu)", numLamps, lampList, numLampGroups, lampGroupList, fromStateArgsSize, fromStateArgs, toStateArgsSize, toStateArgs, strobePeriod, numStrobes);
    component->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
}

StrobeLampsLampGroupsWithPreset::StrobeLampsLampGroupsWithPreset(LSFStringList& lampList, LSFStringList& lampGroupList,  LSFString& fromPreset, LSFString& toPreset, uint32_t& period, uint32_t& numOfStrobes) :
    lamps(lampList), lampGroups(lampGroupList), fromPreset(fromPreset), toPreset(toPreset), strobePeriod(period), numStrobes(numOfStrobes)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

StrobeLampsLampGroupsWithPreset::StrobeLampsLampGroupsWithPreset(const ajn::MsgArg& component)
{
    Set(component);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

const char* StrobeLampsLampGroupsWithPreset::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();
    ret.append("Lamps: \n");
    LSFStringList::const_iterator it;
    for (it = lamps.begin(); it != lamps.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
    }
    ret.append("Lamp Groups: \n");
    for (it = lampGroups.begin(); it != lampGroups.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
    }
    ret.append("From Preset: ");
    ret.append(fromPreset.c_str());
    ret.append("To Preset: ");
    ret.append(toPreset.c_str());
    ret.append("\nStrobe Period: \n");
    ret.append(qcc::U32ToString(strobePeriod));
    ret.append("\nNumber Of Strobes: \n");
    ret.append(qcc::U32ToString(numStrobes));
    return ret.c_str();
}

StrobeLampsLampGroupsWithPreset::StrobeLampsLampGroupsWithPreset(const StrobeLampsLampGroupsWithPreset& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), fromPreset(other.fromPreset), toPreset(other.toPreset), strobePeriod(other.strobePeriod), numStrobes(other.numStrobes)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

StrobeLampsLampGroupsWithPreset& StrobeLampsLampGroupsWithPreset::operator=(const StrobeLampsLampGroupsWithPreset& other)
{
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    fromPreset = other.fromPreset;
    toPreset = other.toPreset;
    strobePeriod = other.strobePeriod;
    numStrobes = other.numStrobes;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void StrobeLampsLampGroupsWithPreset::Set(const ajn::MsgArg& component)
{
    const char** lampList;
    size_t numLamps;
    const char** lampGroupList;
    size_t numLampGroups;
    const char* fromPresetId;
    const char* toPresetId;

    component.Get("(asasssuu)", &numLamps, &lampList, &numLampGroups, &lampGroupList, &fromPresetId, &toPresetId, &strobePeriod, &numStrobes);

    for (size_t j = 0; j < numLamps; j++) {
        LSFString id(lampList[j]);
        lamps.push_back(id);
    }

    for (size_t k = 0; k < numLampGroups; k++) {
        LSFString id(lampGroupList[k]);
        lampGroups.push_back(id);
    }

    fromPreset = LSFString(fromPresetId);
    toPreset = LSFString(toPresetId);

    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void StrobeLampsLampGroupsWithPreset::Get(ajn::MsgArg* component) const
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

    component->Set("(asasssuu)", numLamps, lampList, numLampGroups, lampGroupList, fromPreset.c_str(), toPreset.c_str(), strobePeriod, numStrobes);
    component->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
}

Scene::Scene()
{
    transitionToStateComponent.clear();
    transitionToPresetComponent.clear();
    pulseWithStateComponent.clear();
    pulseWithPresetComponent.clear();
    strobeWithStateComponent.clear();
    strobeWithPresetComponent.clear();
    cycleWithStateComponent.clear();
    cycleWithPresetComponent.clear();
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

Scene::Scene(const ajn::MsgArg& transitionToStateComponentList, const ajn::MsgArg& transitionToPresetComponentList,
             const ajn::MsgArg& pulseWithStateComponentList, const ajn::MsgArg& pulseWithPresetComponentList,
             const ajn::MsgArg& strobeWithStateComponentList, const ajn::MsgArg& strobeWithPresetComponentList,
             const ajn::MsgArg& cycleWithStateComponentList, const ajn::MsgArg& cycleWithPresetComponentList)
{
    Set(transitionToStateComponentList, transitionToPresetComponentList, pulseWithStateComponentList, pulseWithPresetComponentList,
        strobeWithStateComponentList, strobeWithPresetComponentList, cycleWithStateComponentList, cycleWithPresetComponentList);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

Scene::Scene(TransitionLampsLampGroupsToStateList& transitionToStateComponentList, TransitionLampsLampGroupsToPresetList& transitionToPresetComponentList,
             PulseLampsLampGroupsWithStateList& pulseWithStateComponentList, PulseLampsLampGroupsWithPresetList& pulseWithPresetComponentList,
             StrobeLampsLampGroupsWithStateList& strobeWithStateComponentList, StrobeLampsLampGroupsWithPresetList& strobeWithPresetComponentList,
             CycleLampsLampGroupsWithStateList& cycleWithStateComponentList, CycleLampsLampGroupsWithPresetList& cycleWithPresetComponentList)
{
    transitionToStateComponent.clear();
    transitionToPresetComponent.clear();
    pulseWithStateComponent.clear();
    pulseWithPresetComponent.clear();
    strobeWithStateComponent.clear();
    strobeWithPresetComponent.clear();
    cycleWithStateComponent.clear();
    cycleWithPresetComponent.clear();
    transitionToStateComponent = transitionToStateComponentList;
    transitionToPresetComponent = transitionToPresetComponentList;
    pulseWithStateComponent = pulseWithStateComponentList;
    pulseWithPresetComponent = pulseWithPresetComponentList;
    strobeWithStateComponent = strobeWithStateComponentList;
    strobeWithPresetComponent = strobeWithPresetComponentList;
    cycleWithStateComponent = cycleWithStateComponentList;
    cycleWithPresetComponent = cycleWithPresetComponentList;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

Scene::Scene(const Scene& other)
{
    transitionToStateComponent.clear();
    transitionToPresetComponent.clear();
    pulseWithStateComponent.clear();
    pulseWithPresetComponent.clear();
    strobeWithStateComponent.clear();
    strobeWithPresetComponent.clear();
    cycleWithStateComponent.clear();
    cycleWithPresetComponent.clear();
    transitionToStateComponent = other.transitionToStateComponent;
    transitionToPresetComponent = other.transitionToPresetComponent;
    pulseWithStateComponent = other.pulseWithStateComponent;
    pulseWithPresetComponent = other.pulseWithPresetComponent;
    strobeWithStateComponent = other.strobeWithStateComponent;
    strobeWithPresetComponent = other.strobeWithPresetComponent;
    cycleWithStateComponent = other.cycleWithStateComponent;
    cycleWithPresetComponent = other.cycleWithPresetComponent;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

Scene& Scene::operator=(const Scene& other)
{
    transitionToStateComponent.clear();
    transitionToPresetComponent.clear();
    pulseWithStateComponent.clear();
    pulseWithPresetComponent.clear();
    strobeWithStateComponent.clear();
    strobeWithPresetComponent.clear();
    cycleWithStateComponent.clear();
    cycleWithPresetComponent.clear();
    transitionToStateComponent = other.transitionToStateComponent;
    transitionToPresetComponent = other.transitionToPresetComponent;
    pulseWithStateComponent = other.pulseWithStateComponent;
    pulseWithPresetComponent = other.pulseWithPresetComponent;
    strobeWithStateComponent = other.strobeWithStateComponent;
    strobeWithPresetComponent = other.strobeWithPresetComponent;
    cycleWithStateComponent = other.cycleWithStateComponent;
    cycleWithPresetComponent = other.cycleWithPresetComponent;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

const char* Scene::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();

    ret.append("TransitionToState Components: \n");
    TransitionLampsLampGroupsToStateList::const_iterator it;
    for (it = transitionToStateComponent.begin(); it != transitionToStateComponent.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n***\n");
    }
    ret.append("\n------------------------------------------------------------\n");
    ret.append("TransitionToPreset Components: \n");
    TransitionLampsLampGroupsToPresetList::const_iterator ite;
    for (ite = transitionToPresetComponent.begin(); ite != transitionToPresetComponent.end(); ite++) {
        ret.append(ite->c_str());
        ret.append("\n***\n");
    }
    ret.append("\n------------------------------------------------------------\n");
    ret.append("PulseWithState Components: \n");
    PulseLampsLampGroupsWithStateList::const_iterator pte;
    for (pte = pulseWithStateComponent.begin(); pte != pulseWithStateComponent.end(); pte++) {
        ret.append(pte->c_str());
        ret.append("\n***\n");
    }
    ret.append("\n------------------------------------------------------------\n");
    ret.append("PulseWithPreset Components: \n");
    PulseLampsLampGroupsWithPresetList::const_iterator prte;
    for (prte = pulseWithPresetComponent.begin(); prte != pulseWithPresetComponent.end(); prte++) {
        ret.append(prte->c_str());
        ret.append("\n***\n");
    }
    ret.append("\n------------------------------------------------------------\n");
    ret.append("StrobeWithState Components: \n");
    StrobeLampsLampGroupsWithStateList::const_iterator ste;
    for (ste = strobeWithStateComponent.begin(); ste != strobeWithStateComponent.end(); ste++) {
        ret.append(ste->c_str());
        ret.append("\n***\n");
    }
    ret.append("\n------------------------------------------------------------\n");
    ret.append("StrobeWithPreset Components: \n");
    StrobeLampsLampGroupsWithPresetList::const_iterator srte;
    for (srte = strobeWithPresetComponent.begin(); srte != strobeWithPresetComponent.end(); srte++) {
        ret.append(srte->c_str());
        ret.append("\n***\n");
    }
    ret.append("\n------------------------------------------------------------\n");
    ret.append("CycleWithState Components: \n");
    CycleLampsLampGroupsWithStateList::const_iterator cte;
    for (cte = cycleWithStateComponent.begin(); cte != cycleWithStateComponent.end(); cte++) {
        ret.append(cte->c_str());
        ret.append("\n***\n");
    }
    ret.append("\n------------------------------------------------------------\n");
    ret.append("CycleWithPreset Components: \n");
    CycleLampsLampGroupsWithPresetList::const_iterator crte;
    for (crte = cycleWithPresetComponent.begin(); crte != cycleWithPresetComponent.end(); crte++) {
        ret.append(crte->c_str());
        ret.append("\n***\n");
    }

    return ret.c_str();
}

void Scene::Set(const ajn::MsgArg& transitionToStateComponentList, const ajn::MsgArg& transitionToPresetComponentList, const ajn::MsgArg& pulseWithStateComponentList,
                const ajn::MsgArg& pulseWithPresetComponentList, const ajn::MsgArg& strobeWithStateComponentList, const ajn::MsgArg& strobeWithPresetComponentList,
                const ajn::MsgArg& cycleWithStateComponentList, const ajn::MsgArg& cycleWithPresetComponentList)
{
    transitionToStateComponent.clear();
    transitionToPresetComponent.clear();
    pulseWithStateComponent.clear();
    pulseWithPresetComponent.clear();
    strobeWithStateComponent.clear();
    strobeWithPresetComponent.clear();
    cycleWithStateComponent.clear();
    cycleWithPresetComponent.clear();

    MsgArg* transitionToStateComponentArray;
    size_t transitionToStateComponentSize;
    transitionToStateComponentList.Get("a(asasa{sv}u)", &transitionToStateComponentSize, &transitionToStateComponentArray);

    for (size_t i = 0; i < transitionToStateComponentSize; i++) {
        TransitionLampsLampGroupsToState tempStateComponent(transitionToStateComponentArray[i]);
        transitionToStateComponent.push_back(tempStateComponent);
    }

    MsgArg* transitionToPresetComponentArray;
    size_t transitionToPresetComponentSize;
    transitionToPresetComponentList.Get("a(asassu)", &transitionToPresetComponentSize, &transitionToPresetComponentArray);
    for (size_t i = 0; i < transitionToPresetComponentSize; i++) {
        TransitionLampsLampGroupsToPreset tempPresetComponent(transitionToPresetComponentArray[i]);
        transitionToPresetComponent.push_back(tempPresetComponent);
    }

    MsgArg* pulseWithStateComponentArray;
    size_t pulseWithStateComponentSize;
    pulseWithStateComponentList.Get("a(asasa{sv}a{sv}uuu)", &pulseWithStateComponentSize, &pulseWithStateComponentArray);
    for (size_t i = 0; i < pulseWithStateComponentSize; i++) {
        PulseLampsLampGroupsWithState tempPulseComponent(pulseWithStateComponentArray[i]);
        pulseWithStateComponent.push_back(tempPulseComponent);
    }

    MsgArg* pulseWithPresetComponentArray;
    size_t pulseWithPresetComponentSize;
    pulseWithPresetComponentList.Get("a(asasssuuu)", &pulseWithPresetComponentSize, &pulseWithPresetComponentArray);
    for (size_t i = 0; i < pulseWithPresetComponentSize; i++) {
        PulseLampsLampGroupsWithPreset tempPulseComponent(pulseWithPresetComponentArray[i]);
        pulseWithPresetComponent.push_back(tempPulseComponent);
    }

    MsgArg* strobeWithStateComponentArray;
    size_t strobeWithStateComponentSize;
    strobeWithStateComponentList.Get("a(asasa{sv}a{sv}uu)", &strobeWithStateComponentSize, &strobeWithStateComponentArray);
    for (size_t i = 0; i < strobeWithStateComponentSize; i++) {
        StrobeLampsLampGroupsWithState tempStrobeComponent(strobeWithStateComponentArray[i]);
        strobeWithStateComponent.push_back(tempStrobeComponent);
    }

    MsgArg* strobeWithPresetComponentArray;
    size_t strobeWithPresetComponentSize;
    strobeWithPresetComponentList.Get("a(asasssuu)", &strobeWithPresetComponentSize, &strobeWithPresetComponentArray);
    for (size_t i = 0; i < strobeWithPresetComponentSize; i++) {
        StrobeLampsLampGroupsWithPreset tempStrobeComponent(strobeWithPresetComponentArray[i]);
        strobeWithPresetComponent.push_back(tempStrobeComponent);
    }

    MsgArg* cycleWithStateComponentArray;
    size_t cycleWithStateComponentSize;
    cycleWithStateComponentList.Get("a(asasa{sv}a{sv}uuu)", &cycleWithStateComponentSize, &cycleWithStateComponentArray);
    for (size_t i = 0; i < cycleWithStateComponentSize; i++) {
        CycleLampsLampGroupsWithState tempCycleComponent(cycleWithStateComponentArray[i]);
        cycleWithStateComponent.push_back(tempCycleComponent);
    }

    MsgArg* cycleWithPresetComponentArray;
    size_t cycleWithPresetComponentSize;
    cycleWithPresetComponentList.Get("a(asasssuuu)", &cycleWithPresetComponentSize, &cycleWithPresetComponentArray);
    for (size_t i = 0; i < cycleWithPresetComponentSize; i++) {
        CycleLampsLampGroupsWithPreset tempCycleComponent(cycleWithPresetComponentArray[i]);
        cycleWithPresetComponent.push_back(tempCycleComponent);
    }

    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void Scene::Get(ajn::MsgArg* transitionToStateComponentList, ajn::MsgArg* transitionToPresetComponentList, ajn::MsgArg* pulseWithStateComponentList,
                ajn::MsgArg* pulseWithPresetComponentList, ajn::MsgArg* strobeWithStateComponentList, ajn::MsgArg* strobeWithPresetComponentList,
                ajn::MsgArg* cycleWithStateComponentList, ajn::MsgArg* cycleWithPresetComponentList) const
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

    size_t strobeWithStateComponentArraySize = strobeWithStateComponent.size();
    if (strobeWithStateComponentArraySize) {
        MsgArg* strobeWithStateComponentArray = new MsgArg[strobeWithStateComponentArraySize];
        size_t i = 0;
        for (StrobeLampsLampGroupsWithStateList::const_iterator it = strobeWithStateComponent.begin(); it != strobeWithStateComponent.end(); it++, i++) {
            it->Get(&strobeWithStateComponentArray[i]);
        }
        strobeWithStateComponentList->Set("a(asasa{sv}a{sv}uu)", strobeWithStateComponentArraySize, strobeWithStateComponentArray);
        strobeWithStateComponentList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        strobeWithStateComponentList->Set("a(asasa{sv}a{sv}uu)", 0, NULL);
    }

    size_t strobeWithPresetComponentArraySize = strobeWithPresetComponent.size();
    if (strobeWithPresetComponentArraySize) {
        MsgArg* strobeWithPresetComponentArray = new MsgArg[strobeWithPresetComponentArraySize];
        size_t i = 0;
        for (StrobeLampsLampGroupsWithPresetList::const_iterator it = strobeWithPresetComponent.begin(); it != strobeWithPresetComponent.end(); it++, i++) {
            it->Get(&strobeWithPresetComponentArray[i]);
        }
        strobeWithPresetComponentList->Set("a(asasssuu)", strobeWithPresetComponentArraySize, strobeWithPresetComponentArray);
        strobeWithPresetComponentList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        strobeWithPresetComponentList->Set("a(asasssuu)", 0, NULL);
    }

    size_t cycleWithStateComponentArraySize = cycleWithStateComponent.size();
    if (cycleWithStateComponentArraySize) {
        MsgArg* cycleWithStateComponentArray = new MsgArg[cycleWithStateComponentArraySize];
        size_t i = 0;
        for (CycleLampsLampGroupsWithStateList::const_iterator it = cycleWithStateComponent.begin(); it != cycleWithStateComponent.end(); it++, i++) {
            it->Get(&cycleWithStateComponentArray[i]);
        }
        cycleWithStateComponentList->Set("a(asasa{sv}a{sv}uuu)", cycleWithStateComponentArraySize, cycleWithStateComponentArray);
        cycleWithStateComponentList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        cycleWithStateComponentList->Set("a(asasa{sv}a{sv}uuu)", 0, NULL);
    }

    size_t cycleWithPresetComponentArraySize = cycleWithPresetComponent.size();
    if (cycleWithPresetComponentArraySize) {
        MsgArg* cycleWithPresetComponentArray = new MsgArg[cycleWithPresetComponentArraySize];
        size_t i = 0;
        for (CycleLampsLampGroupsWithPresetList::const_iterator it = cycleWithPresetComponent.begin(); it != cycleWithPresetComponent.end(); it++, i++) {
            it->Get(&cycleWithPresetComponentArray[i]);
        }
        cycleWithPresetComponentList->Set("a(asasssuuu)", cycleWithPresetComponentArraySize, cycleWithPresetComponentArray);
        cycleWithPresetComponentList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        cycleWithPresetComponentList->Set("a(asasssuuu)", 0, NULL);
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

    if (LSF_OK == responseCode) {
        for (StrobeLampsLampGroupsWithPresetList::iterator it = strobeWithPresetComponent.begin(); it != strobeWithPresetComponent.end(); it++) {
            if (0 == strcmp(it->fromPreset.c_str(), presetID.c_str())) {
                responseCode = LSF_ERR_DEPENDENCY;
            } else if (0 == strcmp(it->toPreset.c_str(), presetID.c_str())) {
                responseCode = LSF_ERR_DEPENDENCY;
            }
        }
    }

    if (LSF_OK == responseCode) {
        for (CycleLampsLampGroupsWithPresetList::iterator it = cycleWithPresetComponent.begin(); it != cycleWithPresetComponent.end(); it++) {
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
        for (StrobeLampsLampGroupsWithStateList::iterator it = strobeWithStateComponent.begin(); it != strobeWithStateComponent.end(); it++) {
            LSFStringList::iterator findIt = std::find(it->lampGroups.begin(), it->lampGroups.end(), lampGroupID);
            if (findIt != it->lampGroups.end()) {
                responseCode = LSF_ERR_DEPENDENCY;
            }
        }
    }

    if (LSF_OK == responseCode) {
        for (CycleLampsLampGroupsWithStateList::iterator it = cycleWithStateComponent.begin(); it != cycleWithStateComponent.end(); it++) {
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

    if (LSF_OK == responseCode) {
        for (StrobeLampsLampGroupsWithPresetList::iterator it = strobeWithPresetComponent.begin(); it != strobeWithPresetComponent.end(); it++) {
            LSFStringList::iterator findIt = std::find(it->lampGroups.begin(), it->lampGroups.end(), lampGroupID);
            if (findIt != it->lampGroups.end()) {
                responseCode = LSF_ERR_DEPENDENCY;
            }
        }
    }

    if (LSF_OK == responseCode) {
        for (CycleLampsLampGroupsWithPresetList::iterator it = cycleWithPresetComponent.begin(); it != cycleWithPresetComponent.end(); it++) {
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

    ret.assign("Scenes: \n");
    LSFStringList::const_iterator it;
    for (it = scenes.begin(); it != scenes.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n");
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
    for (size_t i = 0; i < gidsSize; i++) {
        char* gid;
        gidsArray[i].Get("s", &gid);
        scenes.push_back(LSFString(gid));
    }
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
