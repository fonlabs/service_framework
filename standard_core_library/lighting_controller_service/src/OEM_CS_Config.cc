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

#include <alljoyn/lighting/LSFPropertyStore.h>
#include <alljoyn/lighting/OEM_CS_Config.h>
#include <qcc/Debug.h>
#include <qcc/Util.h>

namespace lsf {

#define QCC_MODULE "OEM_CS_CONFIG"

// OEMs should modify this default lamp state value as required
static const LampState OEM_CS_DefaultLampState = LampState(true, 0, 0, 0, 0);

uint64_t OEM_MacAddr = 0;

void OEM_CS_GetFactorySetDefaultLampState(LampState& defaultState)
{
    QCC_DbgPrintf(("%s", __func__));
    defaultState = OEM_CS_DefaultLampState;
}

void OEM_CS_GetSyncTimeStamp(uint64_t& timeStamp)
{
    // This is just a sample implementation and so it passes back a
    // random value. OEMs are supposed to integrate this
    // with their Time Sync module to return a valid time stamp
    qcc::String timeString = qcc::RandHexString(16);
    timeStamp = StringToU64(timeString, 16);
    QCC_DbgPrintf(("%s: timeString = %s timestamp = %llu", __func__, timeString.c_str(), timeStamp));
}

bool OEM_CS_FirmwareStart(OEM_CS_NetworkCallback& networkCallback)
{
    // OEMs should add code here to initialize and start the firmware and
    // return true/false accordingly. The firmware should also save off the
    // reference to the OEM_CS_NetworkCallback object and invoke the
    // Connected() and Disconnected() functions defined in this callback
    // whenever the device connects to and disconnects from the network
    // accordingly
    return true;
}

bool OEM_CS_FirmwareStop(void)
{
    // OEMs should add code here to stop and cleanup the firmware and
    // return true/false accordingly
    return true;
}

uint64_t OEM_CS_GetMACAddress(void)
{
    // This is just a sample implementation and so it passes back a
    // random value. OEMs should add code here to return the MAC address
    // of the device as a 48-bit value
    while (OEM_MacAddr == 0) {
        OEM_MacAddr = qcc::Rand64();
        QCC_DbgPrintf(("%s: MAC Address = %llu", __func__, OEM_MacAddr));
    }
    QCC_DbgPrintf(("%s: MAC Address = %llu", __func__, OEM_MacAddr));
    return OEM_MacAddr;
}

bool OEM_CS_IsNetworkConnected(void)
{
    // OEMs should add code here to find out if the device is connected to a network and
    // return true/false accordingly
    return true;
}

OEM_CS_RankParam_Power OEM_CS_GetRankParam_Power(void)
{
    // OEMs should add code here to return the appropriate enum value from
    // OEM_CS_RankParam_Power depending on the type of the device on which
    // the Controller Service is being run
    return ALWAYS_AC_POWERED;
}

OEM_CS_RankParam_Mobility OEM_CS_GetRankParam_Mobility(void)
{
    // OEMs should add code here to return the appropriate enum value from
    // OEM_CS_RankParam_Mobility depending on the type of the device on which
    // the Controller Service is being run
    return ALWAYS_STATIONARY;
}

OEM_CS_RankParam_Availability OEM_CS_GetRankParam_Availability(void)
{
    // OEMs should add code here to return the appropriate enum value from
    // OEM_CS_RankParam_Availability depending on the type of the device on which
    // the Controller Service is being run
    return TWENTY_ONE_TO_TWENTY_FOUR_HOURS;
}

OEM_CS_RankParam_NodeType OEM_CS_GetRankParam_NodeType(void)
{
    // OEMs should add code here to return the appropriate enum value from
    // OEM_CS_RankParam_NodeType depending on network configuration of the device on which
    // the Controller Service is being run
    return WIRED;
}

// NOTE: this function will only be called if no Factory Configuration ini file is found.
// This file is specified on the command line and defaults to OEMConfig.ini in the current
// working directory.
void OEM_CS_PopulateDefaultProperties(LSFPropertyStore& propStore)
{
    QCC_DbgTrace(("%s", __func__));
    std::vector<qcc::String> languages;
    languages.push_back("en");
    languages.push_back("de-AT");
    propStore.setSupportedLangs(languages);

    // use a random AppId since we don't have one
    qcc::String app_id = qcc::RandHexString(16);
    uint8_t* AppId = new uint8_t[16];
    qcc::HexStringToBytes(app_id, AppId, 16);
    propStore.setProperty(LSFPropertyStore::APP_ID, AppId, 16, true, false, true);

    propStore.setProperty(LSFPropertyStore::DEFAULT_LANG, "en", true, true, true);
    propStore.setProperty(LSFPropertyStore::APP_NAME, "LightingControllerService", true, false, true);
    propStore.setProperty(LSFPropertyStore::MODEL_NUMBER, "100", true, false, true);
    propStore.setProperty(LSFPropertyStore::SOFTWARE_VERSION, "1", true, false, false);

    propStore.setProperty(LSFPropertyStore::DEVICE_NAME, "English Name", "en", true, true, true);
    propStore.setProperty(LSFPropertyStore::DEVICE_NAME, "German Name", "de-AT", true, true, true);

    propStore.setProperty(LSFPropertyStore::SUPPORT_URL, "http://www.example.com", "en", true, false, true);
    propStore.setProperty(LSFPropertyStore::SUPPORT_URL, "http://www.example.com", "de-AT", true, false, true);

    propStore.setProperty(LSFPropertyStore::MANUFACTURER, "Company A (EN)", "en", true, false, true);
    propStore.setProperty(LSFPropertyStore::MANUFACTURER, "Firma A (DE-AT)", "de-AT", true, false, true);

    propStore.setProperty(LSFPropertyStore::DESCRIPTION, "Controller Service", "en", true, false, false);
    propStore.setProperty(LSFPropertyStore::DESCRIPTION, "Controller Service", "de-AT", true, false, false);
}

}
