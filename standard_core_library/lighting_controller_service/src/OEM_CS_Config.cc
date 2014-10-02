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

#include <LSFPropertyStore.h>
#include <OEM_CS_Config.h>
#include <qcc/Debug.h>
#include <qcc/Util.h>

namespace lsf {

#define QCC_MODULE "OEM_CS_CONFIG"

static const LampState OEM_CS_DefaultLampState = LampState(true, 0, 0, 0, 0);

uint64_t OEM_Rank = 0;

void OEM_CS_GetFactorySetDefaultLampState(LampState& defaultState)
{
    QCC_DbgPrintf(("%s", __func__));
    defaultState = OEM_CS_DefaultLampState;
}

void OEM_CS_GetSyncTimeStamp(uint64_t& timeStamp)
{
    /* This is just a sample implementation and so it passes back a
     * hard coded value. OEMs are supposed to integrate this
     * with their Time Sync module*/
    qcc::String timeString = qcc::RandHexString(16);
    timeStamp = StringToU64(timeString, 16);
    QCC_DbgPrintf(("%s: timeString = %s timestamp = %llu", __func__, timeString.c_str(), timeStamp));
}

uint64_t OEM_CS_GetRank(void)
{
    while (OEM_Rank == 0) {
        OEM_Rank = qcc::Rand64();
        QCC_DbgPrintf(("%s: new rank = %llu", __func__, OEM_Rank));
    }
    QCC_DbgPrintf(("%s: rank = %llu", __func__, OEM_Rank));
    return OEM_Rank;
}

bool OEM_CS_IsLeader(void)
{
    return false;
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
