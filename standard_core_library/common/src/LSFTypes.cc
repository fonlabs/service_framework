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

LampsLampGroupsAndState::LampsLampGroupsAndState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& lampState, uint32_t& transPeriod) :
    lamps(lampList), lampGroups(lampGroupList), state(lampState), transitionPeriod(transPeriod)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampsLampGroupsAndState::LampsLampGroupsAndState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& lampState) :
    lamps(lampList), lampGroups(lampGroupList), state(lampState), transitionPeriod(0)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampsLampGroupsAndState::LampsLampGroupsAndState(const ajn::MsgArg& component)
{
    Set(component);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

const char* LampsLampGroupsAndState::c_str(void) const
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

LampsLampGroupsAndState::LampsLampGroupsAndState(const LampsLampGroupsAndState& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), state(other.state), transitionPeriod(other.transitionPeriod)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampsLampGroupsAndState& LampsLampGroupsAndState::operator=(const LampsLampGroupsAndState& other)
{
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    state = other.state;
    transitionPeriod = other.transitionPeriod;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void LampsLampGroupsAndState::Set(const ajn::MsgArg& component)
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

void LampsLampGroupsAndState::Get(ajn::MsgArg* component) const
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

LampsLampGroupsAndPreset::LampsLampGroupsAndPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& presetID, uint32_t& transPeriod) :
    lamps(lampList), lampGroups(lampGroupList), presetID(presetID), transitionPeriod(transPeriod)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampsLampGroupsAndPreset::LampsLampGroupsAndPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& presetID) :
    lamps(lampList), lampGroups(lampGroupList), presetID(presetID), transitionPeriod(0)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampsLampGroupsAndPreset::LampsLampGroupsAndPreset(const ajn::MsgArg& component)
{
    Set(component);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

const char* LampsLampGroupsAndPreset::c_str(void) const
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

LampsLampGroupsAndPreset::LampsLampGroupsAndPreset(const LampsLampGroupsAndPreset& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), presetID(other.presetID), transitionPeriod(other.transitionPeriod)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

LampsLampGroupsAndPreset& LampsLampGroupsAndPreset::operator=(const LampsLampGroupsAndPreset& other)
{
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    presetID = other.presetID;
    transitionPeriod = other.transitionPeriod;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

void LampsLampGroupsAndPreset::Set(const ajn::MsgArg& component)
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

void LampsLampGroupsAndPreset::Get(ajn::MsgArg* component) const
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

Scene::Scene()
{
    stateComponent.clear();
    presetComponent.clear();
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

Scene::Scene(const ajn::MsgArg& stateList, const ajn::MsgArg& presetList)
{
    Set(stateList, presetList);
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

Scene::Scene(LampsLampGroupsAndStateList& stateList, LampsLampGroupsAndPresetList& presetList)
{
    stateComponent.clear();
    presetComponent.clear();
    stateComponent = stateList;
    presetComponent = presetList;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

Scene::Scene(const Scene& other)
{
    stateComponent.clear();
    presetComponent.clear();
    stateComponent = other.stateComponent;
    presetComponent = other.presetComponent;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

Scene& Scene::operator=(const Scene& other)
{
    stateComponent.clear();
    presetComponent.clear();
    stateComponent = other.stateComponent;
    presetComponent = other.presetComponent;
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
    return *this;
}

const char* Scene::c_str(void) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    qcc::String ret;
    ret.clear();

    ret.append("State Components: \n");
    LampsLampGroupsAndStateList::const_iterator it;
    for (it = stateComponent.begin(); it != stateComponent.end(); it++) {
        ret.append(it->c_str());
        ret.append("\n***\n");
    }
    ret.append("\n------------------------------------------------------------\n");
    ret.append("Preset Components: \n");
    LampsLampGroupsAndPresetList::const_iterator ite;
    for (ite = presetComponent.begin(); ite != presetComponent.end(); ite++) {
        ret.append(ite->c_str());
        ret.append("\n***\n");
    }

    return ret.c_str();
}

void Scene::Set(const ajn::MsgArg& stateList, const ajn::MsgArg& presetList)
{
    stateComponent.clear();
    presetComponent.clear();

    MsgArg* stateComponentArray;
    size_t stateComponentSize;
    stateList.Get("a(asasa{sv}u)", &stateComponentSize, &stateComponentArray);

    for (size_t i = 0; i < stateComponentSize; i++) {
        LampsLampGroupsAndState tempStateComponent(stateComponentArray[i]);
        stateComponent.push_back(tempStateComponent);
    }

    MsgArg* presetComponentArray;
    size_t presetComponentSize;
    presetList.Get("a(asassu)", &presetComponentSize, &presetComponentArray);
    for (size_t i = 0; i < presetComponentSize; i++) {
        LampsLampGroupsAndPreset tempPresetComponent(presetComponentArray[i]);
        presetComponent.push_back(tempPresetComponent);
    }
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, this->c_str()));
}

void Scene::Get(ajn::MsgArg* stateList, ajn::MsgArg* presetList) const
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    size_t stateComponentArraySize = stateComponent.size();
    if (stateComponentArraySize) {
        MsgArg* stateComponentArray = new MsgArg[stateComponentArraySize];
        size_t i = 0;
        for (LampsLampGroupsAndStateList::const_iterator it = stateComponent.begin(); it != stateComponent.end(); it++, i++) {
            it->Get(&stateComponentArray[i]);
        }
        stateList->Set("a(asasa{sv}u)", stateComponentArraySize, stateComponentArray);
        stateList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        stateList->Set("a(asasa{sv}u)", 0, NULL);
    }

    size_t presetComponentArraySize = presetComponent.size();
    if (presetComponentArraySize) {
        MsgArg* presetComponentArray = new MsgArg[presetComponentArraySize];
        size_t i = 0;
        for (LampsLampGroupsAndPresetList::const_iterator it = presetComponent.begin(); it != presetComponent.end(); it++, i++) {
            it->Get(&presetComponentArray[i]);
        }
        presetList->Set("a(asassu)", presetComponentArraySize, presetComponentArray);
        presetList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        presetList->Set("a(asassu)", 0, NULL);
    }
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
