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

}

LampState::LampState(bool onOff, uint32_t hue, uint32_t saturation, uint32_t colorTemp, uint32_t brightness) :
    onOff(onOff),
    hue(hue),
    saturation(saturation),
    colorTemp(colorTemp),
    brightness(brightness)
{

}

LampState::LampState(const ajn::MsgArg& arg)
{
    Set(arg);
}

LampState::LampState(const LampState& other) :
    onOff(other.onOff),
    hue(other.hue),
    saturation(other.saturation),
    colorTemp(other.colorTemp),
    brightness(other.brightness)
{

}

LampState& LampState::operator=(const LampState& other)
{
    onOff = other.onOff;
    hue = other.hue;
    saturation = other.saturation;
    colorTemp = other.colorTemp;
    brightness = other.brightness;
    return *this;
}

const char* LampState::c_str(void) const
{
    qcc::String ret;
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
}

void LampState::Get(ajn::MsgArg* arg) const
{
    const char* str[] = { "OnOff", "Hue", "Saturation", "Brightness", "ColorTemp" };
    MsgArg* dict = new MsgArg[5];

    MsgArg* var = new MsgArg("v", new MsgArg("b", onOff));
    dict[0].Set("{sv}", str[0], var);
    dict[0].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", hue));
    dict[1].Set("{sv}", str[1], var);
    dict[1].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", saturation));
    dict[2].Set("{sv}", str[2], var);
    dict[2].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", brightness));
    dict[3].Set("{sv}", str[3], var);
    dict[3].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", colorTemp));
    dict[4].Set("{sv}", str[4], var);
    dict[4].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    arg->Set("a{sv}", (size_t)5, dict);
}

LampParameters::LampParameters() :
    energy_usage_milliwatts(0),
    brightness_lumens(0)
{
}

LampParameters::LampParameters(const ajn::MsgArg& arg)
{
    Set(arg);
}

LampParameters::LampParameters(const LampParameters& other) :
    energy_usage_milliwatts(other.energy_usage_milliwatts),
    brightness_lumens(other.brightness_lumens)
{
}

LampParameters& LampParameters::operator=(const LampParameters& other)
{
    energy_usage_milliwatts = other.energy_usage_milliwatts;
    brightness_lumens = other.brightness_lumens;
    return *this;
}

const char* LampParameters::c_str(void) const
{
    qcc::String ret;
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
}

void LampParameters::Get(ajn::MsgArg* arg) const
{
    const char* str[] = { "Energy_Usage_Milliwatts", "Brightness_Lumens" };
    MsgArg* dict = new MsgArg[2];

    MsgArg* var = new MsgArg("v", new MsgArg("u", energy_usage_milliwatts));
    dict[0].Set("{sv}", str[0], var);
    dict[0].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", brightness_lumens));
    dict[1].Set("{sv}", str[1], var);
    dict[1].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    arg->Set("a{sv}", (size_t)2, dict);
}

LampDetails::LampDetails()
{
}

LampDetails::LampDetails(const ajn::MsgArg& arg)
{
    Set(arg);
}

LampDetails::LampDetails(const LampDetails& other) :
    hardwareVersion(other.hardwareVersion),
    firmwareVersion(other.firmwareVersion),
    manufacturer(other.manufacturer),
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

}

LampDetails& LampDetails::operator=(const LampDetails& other)
{
    hardwareVersion = other.hardwareVersion;
    firmwareVersion = other.firmwareVersion;
    manufacturer = other.manufacturer;
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
    return *this;
}

const char* LampDetails::c_str(void) const
{
    qcc::String ret;
    ret.append("\nhardwareVersion=");
    ret.append(qcc::U32ToString(hardwareVersion));
    ret.append("\nfirmwareVersion=");
    ret.append(qcc::U32ToString(firmwareVersion));
    ret.append("\nmanufacturer=");
    ret.append(manufacturer.c_str());
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
        } else if (0 == strcmp(field, "Manufacturer")) {
            char* temp;
            value->Get("s", &temp);
            manufacturer = LSF_Name(temp);
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
            lampID = LSF_ID(temp);
        }
    }
}

void LampDetails::Get(ajn::MsgArg* arg) const
{
    const char* str[] = {
        "HardwareVersion",
        "FirmwareVersion",
        "Manufacturer",
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

    MsgArg* var = new MsgArg("v", new MsgArg("u", hardwareVersion));
    dict[0].Set("{sv}", str[0], var);
    dict[0].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", firmwareVersion));
    dict[1].Set("{sv}", str[1], var);
    dict[1].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("s", manufacturer.c_str()));
    dict[2].Set("{sv}", str[2], var);
    dict[2].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", make));
    dict[3].Set("{sv}", str[3], var);
    dict[3].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", model));
    dict[4].Set("{sv}", str[4], var);
    dict[4].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", type));
    dict[5].Set("{sv}", str[5], var);
    dict[5].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", lampType));
    dict[6].Set("{sv}", str[6], var);
    dict[6].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", lampBaseType));
    dict[7].Set("{sv}", str[7], var);
    dict[7].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", lampBeamAngle));
    dict[8].Set("{sv}", str[8], var);
    dict[8].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("b", dimmable));
    dict[9].Set("{sv}", str[9], var);
    dict[9].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("b", color));
    dict[10].Set("{sv}", str[10], var);
    dict[10].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("b", variableColorTemp));
    dict[11].Set("{sv}", str[11], var);
    dict[11].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("b", hasEffects));
    dict[12].Set("{sv}", str[12], var);
    dict[12].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", voltage));
    dict[13].Set("{sv}", str[13], var);
    dict[13].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", wattage));
    dict[14].Set("{sv}", str[14], var);
    dict[14].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", wattageEquivalent));
    dict[15].Set("{sv}", str[15], var);
    dict[15].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", maxOutput));
    dict[16].Set("{sv}", str[16], var);
    dict[16].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", minTemperature));
    dict[17].Set("{sv}", str[17], var);
    dict[17].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", maxTemperature));
    dict[18].Set("{sv}", str[18], var);
    dict[18].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", colorRenderingIndex));
    dict[19].Set("{sv}", str[19], var);
    dict[19].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("u", lifespan));
    dict[20].Set("{sv}", str[20], var);
    dict[20].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    var = new MsgArg("v", new MsgArg("s", lampID.c_str()));
    dict[21].Set("{sv}", str[21], var);
    dict[21].SetOwnershipFlags(MsgArg::OwnsArgs, true);

    arg->Set("a{sv}", (size_t)22, dict);
}

LampGroup::LampGroup()
{
    lamps.clear();
    lampGroups.clear();
}

LampGroup::LampGroup(const ajn::MsgArg& lampList, const ajn::MsgArg& lampGroupList)
{
    Set(lampList, lampGroupList);
}

LampGroup::LampGroup(LSF_ID_List lampList, LSF_ID_List lampGroupList) :
    lamps(lampList), lampGroups(lampGroupList)
{
}

LampGroup::LampGroup(const LampGroup& other)
{
    lamps.clear();
    lampGroups.clear();
    lamps = other.lamps;
    lampGroups = other.lampGroups;
}

const char* LampGroup::c_str(void) const
{
    qcc::String ret;

    ret.append("Lamps: \n");
    LSF_ID_List::const_iterator it;
    for (it = lamps.begin(); it != lamps.end(); ++it) {
        ret.append(qcc::String(it->c_str()));
        ret.append(qcc::String("\n"));
    }
    ret.append("Lamp Groups: \n");
    for (it = lampGroups.begin(); it != lampGroups.end(); ++it) {
        ret.append(qcc::String(it->c_str()));
        ret.append(qcc::String("\n"));
    }

    return ret.c_str();
}

LampGroup& LampGroup::operator=(const LampGroup& other)
{
    lamps.clear();
    lampGroups.clear();
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    return *this;
}

void LampGroup::Set(const ajn::MsgArg& lampList, const ajn::MsgArg& lampGroupList)
{
    lamps.clear();
    lampGroups.clear();

    MsgArg* idsArray;
    size_t idsSize;
    lampList.Get("as", &idsSize, &idsArray);
    for (size_t i = 0; i < idsSize; ++i) {
        char* lampID;
        idsArray[i].Get("s", &lampID);
        lamps.push_back(LSF_ID(lampID));
    }

    MsgArg* gidsArray;
    size_t gidsSize;
    lampGroupList.Get("as", &gidsSize, &gidsArray);
    for (size_t i = 0; i < gidsSize; ++i) {
        char* gid;
        gidsArray[i].Get("s", &gid);
        lampGroups.push_back(LSF_ID(gid));
    }
}

void LampGroup::Get(ajn::MsgArg* lampList, ajn::MsgArg* lampGroupList) const
{
    size_t idsVecSize = lamps.size();

    if (idsVecSize) {
        const char** idsVec = new const char*[idsVecSize];
        size_t i = 0;
        for (LSF_ID_List::const_iterator it = lamps.begin(); it != lamps.end(); ++it) {
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
        for (LSF_ID_List::const_iterator it = lampGroups.begin(); it != lampGroups.end(); ++it) {
            gidsVec[i++] = it->c_str();
        }
        lampGroupList->Set("as", gidsVecSize, gidsVec);
        lampGroupList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        lampGroupList->Set("as", 0, NULL);
    }
}

LampsLampGroupsAndState::LampsLampGroupsAndState(LSF_ID_List& lampList, LSF_ID_List& lampGroupList, LampState& lampState, uint32_t& transPeriod) :
    lamps(lampList), lampGroups(lampGroupList), state(lampState), transitionPeriod(transPeriod)
{
}

LampsLampGroupsAndState::LampsLampGroupsAndState(LSF_ID_List& lampList, LSF_ID_List& lampGroupList, LampState& lampState) :
    lamps(lampList), lampGroups(lampGroupList), state(lampState), transitionPeriod(0)
{
}

LampsLampGroupsAndState::LampsLampGroupsAndState(const ajn::MsgArg& component)
{
    Set(component);
}

const char* LampsLampGroupsAndState::c_str(void) const
{
    qcc::String ret;
    ret.append("----------\n");
    ret.append("Lamps: \n");
    LSF_ID_List::const_iterator it;
    for (it = lamps.begin(); it != lamps.end(); ++it) {
        ret.append(qcc::String(it->c_str()));
        ret.append(qcc::String("\n"));
    }
    ret.append("Lamp Groups: \n");
    for (it = lampGroups.begin(); it != lampGroups.end(); ++it) {
        ret.append(qcc::String(it->c_str()));
        ret.append(qcc::String("\n"));
    }
    ret.append("State: \n");
    ret.append(state.c_str());
    ret.append("Transition Period: \n");
    ret.append(qcc::U32ToString(transitionPeriod));
    ret.append("----------\n");
    return ret.c_str();
}

LampsLampGroupsAndState::LampsLampGroupsAndState(const LampsLampGroupsAndState& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), state(other.state), transitionPeriod(other.transitionPeriod)
{
}

LampsLampGroupsAndState& LampsLampGroupsAndState::operator=(const LampsLampGroupsAndState& other)
{
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    state = other.state;
    transitionPeriod = other.transitionPeriod;
    return *this;
}

void LampsLampGroupsAndState::Set(const ajn::MsgArg& component)
{
    const char** lampList;
    size_t numLamps;
    const char** lampGroupList;
    size_t numLampGroups;
    const MsgArg* lampState;

    component.Get("asasa{sv}u", &numLamps, &lampList, &numLampGroups, &lampGroupList, &lampState, &transitionPeriod);

    for (size_t j = 0; j < numLamps; ++j) {
        LSF_ID id(lampList[j]);
        lamps.push_back(id);
    }

    for (size_t k = 0; k < numLampGroups; ++k) {
        LSF_ID id(lampGroupList[k]);
        lampGroups.push_back(id);
    }

    state.Set(*lampState);
}

void LampsLampGroupsAndState::Get(ajn::MsgArg* component) const
{
    size_t numLamps = lamps.size();
    const char** lampList = new const char*[numLamps];
    size_t j = 0;
    for (LSF_ID_List::const_iterator nit = lamps.begin(); nit != lamps.end(); ++nit) {
        lampList[j++] = nit->c_str();
    }

    size_t numLampGroups = lampGroups.size();
    const char** lampGroupList = new const char*[numLampGroups];
    j = 0;
    for (LSF_ID_List::const_iterator nit = lampGroups.begin(); nit != lampGroups.end(); ++nit) {
        lampGroupList[j++] = nit->c_str();
    }

    MsgArg stateArg;
    state.Get(&stateArg);

    component->Set("asasa{sv}u", numLamps, lampList, numLampGroups, lampGroupList, &stateArg, transitionPeriod);
    component->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
}

LampsLampGroupsAndSavedState::LampsLampGroupsAndSavedState(LSF_ID_List& lampList, LSF_ID_List& lampGroupList, LSF_ID& stateID, uint32_t& transPeriod) :
    lamps(lampList), lampGroups(lampGroupList), savedStateID(stateID), transitionPeriod(transPeriod)
{
}

LampsLampGroupsAndSavedState::LampsLampGroupsAndSavedState(LSF_ID_List& lampList, LSF_ID_List& lampGroupList, LSF_ID& stateID) :
    lamps(lampList), lampGroups(lampGroupList), savedStateID(stateID), transitionPeriod(0)
{
}

LampsLampGroupsAndSavedState::LampsLampGroupsAndSavedState(const ajn::MsgArg& component)
{
    Set(component);
}

const char* LampsLampGroupsAndSavedState::c_str(void) const
{
    qcc::String ret;
    ret.append("----------\n");
    ret.append("Lamps: \n");
    LSF_ID_List::const_iterator it;
    for (it = lamps.begin(); it != lamps.end(); ++it) {
        ret.append(qcc::String(it->c_str()));
        ret.append(qcc::String("\n"));
    }
    ret.append("Lamp Groups: \n");
    for (it = lampGroups.begin(); it != lampGroups.end(); ++it) {
        ret.append(qcc::String(it->c_str()));
        ret.append(qcc::String("\n"));
    }
    ret.append("Saved State ID: \n");
    ret.append(savedStateID.c_str());
    ret.append("Transition Period: \n");
    ret.append(qcc::U32ToString(transitionPeriod));
    ret.append("----------\n");
    return ret.c_str();
}

LampsLampGroupsAndSavedState::LampsLampGroupsAndSavedState(const LampsLampGroupsAndSavedState& other) :
    lamps(other.lamps), lampGroups(other.lampGroups), savedStateID(other.savedStateID), transitionPeriod(other.transitionPeriod)
{

}

LampsLampGroupsAndSavedState& LampsLampGroupsAndSavedState::operator=(const LampsLampGroupsAndSavedState& other)
{
    lamps = other.lamps;
    lampGroups = other.lampGroups;
    savedStateID = other.savedStateID;
    transitionPeriod = other.transitionPeriod;
    return *this;
}

void LampsLampGroupsAndSavedState::Set(const ajn::MsgArg& component)
{
    const char** lampList;
    size_t numLamps;
    const char** lampGroupList;
    size_t numLampGroups;
    const char* stateID;

    component.Get("asassu", &numLamps, &lampList, &numLampGroups, &lampGroupList, &stateID, &transitionPeriod);

    for (size_t j = 0; j < numLamps; ++j) {
        LSF_ID id(lampList[j]);
        lamps.push_back(id);
    }

    for (size_t k = 0; k < numLampGroups; ++k) {
        LSF_ID id(lampGroupList[k]);
        lampGroups.push_back(id);
    }

    savedStateID = LSF_ID(stateID);
}

void LampsLampGroupsAndSavedState::Get(ajn::MsgArg* component) const
{
    size_t numLamps = lamps.size();
    const char** lampList = new const char*[numLamps];
    size_t j = 0;
    for (LSF_ID_List::const_iterator nit = lamps.begin(); nit != lamps.end(); ++nit) {
        lampList[j++] = nit->c_str();
    }

    size_t numLampGroups = lampGroups.size();
    const char** lampGroupList = new const char*[numLampGroups];
    j = 0;
    for (LSF_ID_List::const_iterator nit = lampGroups.begin(); nit != lampGroups.end(); ++nit) {
        lampGroupList[j++] = nit->c_str();
    }

    component->Set("asassu", numLamps, lampList, numLampGroups, lampGroupList, savedStateID.c_str(), transitionPeriod);
    component->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
}

Scene::Scene()
{
    stateComponent.clear();
    savedStateComponent.clear();
}

Scene::Scene(const ajn::MsgArg& stateList, const ajn::MsgArg& savedStateList)
{
    Set(stateList, savedStateList);
}

Scene::Scene(LampsLampGroupsAndStateList& stateList, LampsLampGroupsAndSavedStateList& savedStateList)
{
    stateComponent.clear();
    savedStateComponent.clear();
    stateComponent = stateList;
    savedStateComponent = savedStateList;
}

Scene::Scene(const Scene& other)
{
    stateComponent.clear();
    savedStateComponent.clear();
    stateComponent = other.stateComponent;
    savedStateComponent = other.savedStateComponent;
}

Scene& Scene::operator=(const Scene& other)
{
    stateComponent.clear();
    savedStateComponent.clear();
    stateComponent = other.stateComponent;
    savedStateComponent = other.savedStateComponent;
    return *this;
}

const char* Scene::c_str(void) const
{
    qcc::String ret;

    ret.append("State Components: \n");
    LampsLampGroupsAndStateList::const_iterator it;
    for (it = stateComponent.begin(); it != stateComponent.end(); ++it) {
        ret.append(qcc::String(it->c_str()));
        ret.append(qcc::String("\n"));
    }
    ret.append("Saved State Components: \n");
    LampsLampGroupsAndSavedStateList::const_iterator ite;
    for (ite = savedStateComponent.begin(); ite != savedStateComponent.end(); ++ite) {
        ret.append(qcc::String(ite->c_str()));
        ret.append(qcc::String("\n"));
    }

    return ret.c_str();
}

void Scene::Set(const ajn::MsgArg& stateList, const ajn::MsgArg& savedStateList)
{
    stateComponent.clear();
    savedStateComponent.clear();

    MsgArg* stateComponentArray;
    size_t stateComponentSize;
    stateList.Get("a(asasa{sv}u)", &stateComponentSize, &stateComponentArray);

    for (size_t i = 0; i < stateComponentSize; ++i) {
        LampsLampGroupsAndState tempStateComponent(stateComponentArray[i]);
        stateComponent.push_back(tempStateComponent);
    }

    MsgArg* savedStateComponentArray;
    size_t savedStateComponentSize;
    stateList.Get("a(asassu)", &savedStateComponentSize, &savedStateComponentArray);
    for (size_t i = 0; i < savedStateComponentSize; ++i) {
        LampsLampGroupsAndSavedState tempSavedStateComponent(savedStateComponentArray[i]);
        savedStateComponent.push_back(tempSavedStateComponent);
    }
}

void Scene::Get(ajn::MsgArg* stateList, ajn::MsgArg* savedStateList) const
{
    size_t stateComponentArraySize = stateComponent.size();
    if (stateComponentArraySize) {
        MsgArg* stateComponentArray = new MsgArg[stateComponentArraySize];
        size_t i = 0;
        for (LampsLampGroupsAndStateList::const_iterator it = stateComponent.begin(); it != stateComponent.end(); ++it, ++i) {
            it->Get(&stateComponentArray[i]);
        }
        stateList->Set("a(asasa{sv}u)", stateComponentArraySize, stateComponentArray);
        stateList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        stateList->Set("a(asasa{sv}u)", 0, NULL);
    }

    size_t savedStateComponentArraySize = savedStateComponent.size();
    if (savedStateComponentArraySize) {
        MsgArg* savedStateComponentArray = new MsgArg[savedStateComponentArraySize];
        size_t i = 0;
        for (LampsLampGroupsAndSavedStateList::const_iterator it = savedStateComponent.begin(); it != savedStateComponent.end(); ++it, ++i) {
            it->Get(&savedStateComponentArray[i]);
        }
        savedStateList->Set("a(asassu)", savedStateComponentArraySize, savedStateComponentArray);
        savedStateList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        savedStateList->Set("a(asassu)", 0, NULL);
    }
}

SceneGroup::SceneGroup()
{
    scenes.clear();
}

SceneGroup::SceneGroup(const ajn::MsgArg& sceneList)
{
    Set(sceneList);
}

SceneGroup::SceneGroup(LSF_ID_List sceneList) :
    scenes(sceneList)
{
}

SceneGroup::SceneGroup(const SceneGroup& other)
{
    scenes.clear();
    scenes = other.scenes;
}

const char* SceneGroup::c_str(void) const
{
    qcc::String ret;

    ret.assign("Scenes: \n");
    LSF_ID_List::const_iterator it;
    for (it = scenes.begin(); it != scenes.end(); ++it) {
        ret.append(qcc::String(it->c_str()));
        ret.append(qcc::String("\n"));
    }

    return ret.c_str();
}

SceneGroup& SceneGroup::operator=(const SceneGroup& other)
{
    scenes.clear();
    scenes = other.scenes;
    return *this;
}

void SceneGroup::Set(const ajn::MsgArg& sceneList)
{
    scenes.clear();

    MsgArg* gidsArray;
    size_t gidsSize;
    sceneList.Get("as", &gidsSize, &gidsArray);
    for (size_t i = 0; i < gidsSize; ++i) {
        char* gid;
        gidsArray[i].Get("s", &gid);
        scenes.push_back(LSF_ID(gid));
    }
}

void SceneGroup::Get(ajn::MsgArg* sceneList) const
{
    size_t gidsVecSize = scenes.size();
    if (gidsVecSize) {
        const char** gidsVec = new const char*[gidsVecSize];
        size_t i = 0;
        for (LSF_ID_List::const_iterator it = scenes.begin(); it != scenes.end(); ++it) {
            gidsVec[i++] = it->c_str();
        }
        sceneList->Set("as", gidsVecSize, gidsVec);
        sceneList->SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    } else {
        sceneList->Set("as", 0, NULL);
    }
}
