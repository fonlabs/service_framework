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
#include <PropertyParser.h>
#include <OEM_CS_Config.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <algorithm>

#include <qcc/Debug.h>
#include <qcc/StringUtil.h>
#include <alljoyn/about/AboutServiceApi.h>

#define QCC_MODULE "LSF_PROPSTORE"

using namespace ajn;
using namespace services;
using namespace lsf;

qcc::String LSFPropertyStore::PropertyStoreName[NUMBER_OF_KEYS + 1] = {
    "DeviceId", "DeviceName", "AppId",
    "AppName", "DefaultLanguage", "SupportedLanguages", "Description", "Manufacturer",
    "DateOfManufacture", "ModelNumber", "SoftwareVersion", "AJSoftwareVersion", "HardwareVersion",
    "SupportUrl", "Rank", "IsLeader", ""
};



LSFPropertyStore::LSFPropertyStore(const std::string& obsConfigFile, const std::string& factoryConfigFile, const std::string& configFile)
    : isInitialized(false),
    obsConfigFile(obsConfigFile),
    configFileName(configFile),
    factoryConfigFileName(factoryConfigFile),
    factoryReset(false),
    needsWrite(false),
    running(true)
{
    QCC_DbgTrace(("%s", __func__));
    Start();
}


LSFPropertyStore::~LSFPropertyStore()
{
    QCC_DbgTrace(("%s", __func__));
}

void LSFPropertyStore::Initialize()
{
    QCC_DbgTrace(("%s", __func__));
    // nothing is locked yet but no threads are running

    ReadFactoryConfiguration();

    ReadConfiguration();

    isInitialized = true;
}


void LSFPropertyStore::ReadFactoryConfiguration()
{
    QCC_DbgTrace(("%s", __func__));
    // we need to get the same DeviceId as the onboarding manager
    if (obsSettings.empty()) {
        if (PropertyParser::ParseFile(obsConfigFile, obsSettings)) {
            StringMap::const_iterator iter = obsSettings.find(PropertyStoreName[DEVICE_ID]);
            if (iter != obsSettings.end()) {
                setProperty(DEVICE_ID, iter->second, true, false, true);
            }
        }

        // we need a device id
        if (getProperty(DEVICE_ID) == NULL) {
            qcc::String new_id = qcc::RandHexString(16);
            setProperty(DEVICE_ID, new_id, true, false, true);
        }
    }


    if (factorySettings.empty()) {
        if (!PropertyParser::ParseFile(factoryConfigFileName, factorySettings)) {
            // ini not found: fall back to hard-coded defaults
            PopulateDefaultProperties(*this);
            return;
        }
    }

    StringMap::const_iterator iter;
    iter = factorySettings.find("SupportedLanguages");
    if (iter != factorySettings.end()) {
        std::vector<qcc::String> supported_langs;
        PropertyParser::Tokenize(iter->second, supported_langs, ' ');

        // now add to properties
        setSupportedLangs(supported_langs);
    }

    iter = factorySettings.find("DefaultLanguage");
    if (iter != factorySettings.end()) {
        setProperty(DEFAULT_LANG, iter->second, true, true, true);
    }

    iter = factorySettings.find("AppName");
    if (iter != factorySettings.end()) {
        setProperty(APP_NAME, iter->second, true, false, true);
    }

    iter = factorySettings.find(PropertyStoreName[APP_ID]);
    if (iter != factorySettings.end()) {
        qcc::String device_id = iter->second;

        uint8_t* AppId = new uint8_t[16];
        qcc::HexStringToBytes(device_id, AppId, 16);
        setProperty(APP_ID, AppId, 16, true, false, true);
    } else {
        // use a random AppId since we don't have one
        qcc::String device_id = qcc::RandHexString(16);
        uint8_t* AppId = new uint8_t[16];
        qcc::HexStringToBytes(device_id, AppId, 16);
        setProperty(APP_ID, AppId, 16, true, false, true);
    }

    iter = factorySettings.find("Rank");
    if (iter != factorySettings.end()) {
        uint64_t value = qcc::StringToU64(iter->second);
        setProperty(RANK, value, true, false, true);
    }

    // isLeader==false by default
    setProperty(IS_LEADER, false, true, false, true);

    std::vector<qcc::String>::const_iterator it;
    for (it = supportedLanguages.begin(); it != supportedLanguages.end(); ++it) {
        iter = factorySettings.find("DeviceName." + *it);
        if (iter != factorySettings.end()) {
            setProperty(DEVICE_NAME, iter->second, *it, true, true, true);
        }

        iter = factorySettings.find("SupportUrl." + *it);
        if (iter != factorySettings.end()) {
            setProperty(SUPPORT_URL, iter->second, *it, true, false, true);
        }

        iter = factorySettings.find("Manufacturer." + *it);
        if (iter != factorySettings.end()) {
            setProperty(MANUFACTURER, iter->second, *it, true, false, true);
        }

        iter = factorySettings.find("Description." + *it);
        if (iter != factorySettings.end()) {
            setProperty(DESCRIPTION, iter->second, *it, true, false, false);
        }
    }
}

// now read user-defined values that have overridden the
void LSFPropertyStore::ReadConfiguration()
{
    QCC_DbgTrace(("%s", __func__));
    StringMap data;
    if (!PropertyParser::ParseFile(configFileName, data)) {
        return;
    }

    StringMap::const_iterator iter = data.find("DefaultLanguage");
    if (iter != data.end()) {
        setProperty(DEFAULT_LANG, iter->second, true, true, true);
    }

    std::vector<qcc::String>::const_iterator it;
    for (it = supportedLanguages.begin(); it != supportedLanguages.end(); ++it) {
        iter = data.find("DeviceName." + *it);
        if (iter != data.end()) {
            setProperty(DEVICE_NAME, iter->second, *it, true, true, true);
        }
    }
}

QStatus LSFPropertyStore::Reset()
{
    QCC_DbgTrace(("%s", __func__));
    factoryReset = true;
    return ER_OK;
}


QStatus LSFPropertyStore::ReadAll(const char* languageTag, Filter filter, ajn::MsgArg& all)
{
    QCC_DbgTrace(("%s", __func__));
    if (!isInitialized) {
        return ER_FAIL;
    }

    // make a local copy before reading.  this will minimize the size
    // of the critical section
    QStatus status = ER_OK;
    propsLock.Lock();
    PropertyMap mapCopy = properties;
    propsLock.Unlock();

    if (filter == ANNOUNCE) {
        PropertyMap::iterator defaultLang = mapCopy.find(DEFAULT_LANG);

        qcc::String defaultLanguage = "";
        if (defaultLang != mapCopy.end()) {
            char* tempdefLang;
            defaultLang->second.getPropertyValue().Get("s", &tempdefLang);
            defaultLanguage = tempdefLang;
        }

        MsgArg* argsAnnounceData = new MsgArg[mapCopy.size()];
        uint32_t announceArgCount = 0;

        for (PropertyMap::const_iterator it = mapCopy.begin(); it != mapCopy.end(); ++it) {
            const PropertyStoreProperty& property = it->second;

            if (property.getIsAnnouncable() && (property.getLanguage().empty() || property.getLanguage() == defaultLanguage)) {
                status = argsAnnounceData[announceArgCount].Set("{sv}", property.getPropertyName().c_str(), new MsgArg(property.getPropertyValue()));
                if (status != ER_OK) {
                    break;
                }
                argsAnnounceData[announceArgCount].SetOwnershipFlags(MsgArg::OwnsArgs, true);
                announceArgCount++;
            }
        }

        status = all.Set("a{sv}", announceArgCount, argsAnnounceData);
        if (status == ER_OK) {
            all.SetOwnershipFlags(MsgArg::OwnsArgs, true);
        } else {
            delete[] argsAnnounceData;
        }
    } else if (filter == READ) {
        if (languageTag != NULL && languageTag[0] != 0) {
            status = isLanguageSupported(languageTag);
        } else {
            PropertyMap::iterator it = mapCopy.find(DEFAULT_LANG);
            if (it == properties.end()) {
                return ER_LANGUAGE_NOT_SUPPORTED;
            }

            status = it->second.getPropertyValue().Get("s", &languageTag);
        }

        if (status != ER_OK) {
            return status;
        }

        MsgArg* argsReadData = new MsgArg[mapCopy.size()];
        uint32_t readArgCount = 0;
        for (PropertyMap::const_iterator it = mapCopy.begin(); it != mapCopy.end(); ++it) {
            const PropertyStoreProperty& property = it->second;

            // check that it is from the defaultLanguage or empty.
            if (property.getIsPublic() && (property.getLanguage().empty() || property.getLanguage() == languageTag)) {
                status = argsReadData[readArgCount].Set("{sv}", property.getPropertyName().c_str(), new MsgArg(property.getPropertyValue()));
                if (status != ER_OK) {
                    break;
                }
                argsReadData[readArgCount].SetOwnershipFlags(MsgArg::OwnsArgs, true);
                readArgCount++;
            }
        }

        status = all.Set("a{sv}", readArgCount, argsReadData);
        if (status == ER_OK) {
            all.SetOwnershipFlags(MsgArg::OwnsArgs, true);
        } else {
            delete[] argsReadData;
        }
    } else if (filter == WRITE) {
        QStatus status = ER_OK;
        if (languageTag != NULL && languageTag[0] != 0) { // check that the language is in the supported languages;
            status = isLanguageSupported(languageTag);
            if (status != ER_OK) {
                return status;
            }
        } else {
            PropertyMap::iterator it = mapCopy.find(DEFAULT_LANG);
            if (it == mapCopy.end()) {
                return ER_LANGUAGE_NOT_SUPPORTED;
            }

            status = it->second.getPropertyValue().Get("s", &languageTag);
            if (status != ER_OK) {
                return status;
            }
        }

        MsgArg* argsWriteData = new MsgArg[mapCopy.size()];
        uint32_t writeArgCount = 0;
        for (PropertyMap::const_iterator it = mapCopy.begin(); it != mapCopy.end(); ++it) {
            const PropertyStoreProperty& property = it->second;

            // check that it is from the defaultLanguage or empty.
            if (property.getIsWritable() && (property.getLanguage().empty() || property.getLanguage() == languageTag)) {
                status = argsWriteData[writeArgCount].Set("{sv}", property.getPropertyName().c_str(), new MsgArg(property.getPropertyValue()));
                if (status != ER_OK) {
                    break;
                }

                argsWriteData[writeArgCount].SetOwnershipFlags(MsgArg::OwnsArgs, true);
                writeArgCount++;
            }
        }

        status = all.Set("a{sv}", writeArgCount, argsWriteData);
        if (status == ER_OK) {
            all.SetOwnershipFlags(MsgArg::OwnsArgs, true);
        } else {
            delete[] argsWriteData;
        }
    }

    return status;
}

QStatus LSFPropertyStore::Update(const char* name, const char* languageTag, const ajn::MsgArg* value)
{
    QCC_DbgTrace(("%s", __func__));
    if (!isInitialized) {
        return ER_FAIL;
    }

    PropertyStoreKey propertyKey = getPropertyStoreKeyFromName(name);
    if (propertyKey >= NUMBER_OF_KEYS) {
        return ER_FEATURE_NOT_AVAILABLE;
    }

    // check the languageTag
    // case languageTag == NULL: is not a valid value for the languageTag
    // case languageTag == "": use the default language
    // case languageTag == string: check value, must be one of the supported languages
    QStatus status = ER_OK;
    if (propertyKey == DEFAULT_LANG) {
        // Special case DEFAULT_LANG is not associated with a language in the PropertyMap and
        // its only valid languageTag = NULL
        // By setting it here, we to let the user follow the same language rules as any other property
        languageTag = NULL;
    } else if (languageTag[0] == 0) {
        propsLock.Lock();
        PropertyMap::iterator it = properties.find(DEFAULT_LANG);
        if (it != properties.end()) {
            status = it->second.getPropertyValue().Get("s", &languageTag);
        } else {
            status = ER_LANGUAGE_NOT_SUPPORTED;
        }

        propsLock.Unlock();
    } else {
        status = isLanguageSupported(languageTag);
    }

    if (status != ER_OK) {
        return status;
    }

    //validate that the value is acceptable
    qcc::String languageString = languageTag ? languageTag : "";
    status = validateValue(propertyKey, *value, languageString);
    if (status != ER_OK) {
        return status;
    }

    bool updated = false;

    propsLock.Lock();
    std::pair<PropertyMap::iterator, PropertyMap::iterator> propertiesIter = properties.equal_range(propertyKey);
    for (PropertyMap::iterator it = propertiesIter.first; it != propertiesIter.second; ++it) {
        const PropertyStoreProperty& property = it->second;

        if (property.getIsWritable()) {
            if ((languageTag == NULL && property.getLanguage().empty()) || (languageTag != NULL && property.getLanguage() == languageTag)) {
                PropertyStoreProperty newProperty(property.getPropertyName(), *value, property.getIsPublic(), property.getIsWritable(), property.getIsAnnouncable());
                if (languageTag) {
                    newProperty.setLanguage(languageTag);
                }

                updated = true;
                it->second = newProperty;
                break;
            }
        }
    }

    if (!updated) {
        status = languageTag ? ER_LANGUAGE_NOT_SUPPORTED : ER_INVALID_VALUE;
    }

    if (status == ER_OK) {
        needsWrite = true;
        propsLock.Unlock();
        propsCond.Signal();
    } else {
        propsLock.Unlock();
    }

    return status;
}

QStatus LSFPropertyStore::Delete(const char* name, const char* languageTag)
{
    QCC_DbgTrace(("%s", __func__));
    if (!isInitialized) {
        return ER_FAIL;
    }

    PropertyStoreKey propertyKey = getPropertyStoreKeyFromName(name);
    if (propertyKey >= NUMBER_OF_KEYS) {
        return ER_FEATURE_NOT_AVAILABLE;
    }

    QStatus status = ER_OK;
    if (propertyKey == DEFAULT_LANG) {
        // DefaultLanguage has no language tag
        languageTag = NULL;
    } else if (languageTag[0] == 0) {
        propsLock.Lock();
        // use properties here becuase writeProperties has not yet
        // been committed to disk and is therefore not yet valid
        PropertyMap::iterator it = properties.find(DEFAULT_LANG);
        if (it != properties.end()) {
            status = it->second.getPropertyValue().Get("s", &languageTag);
        } else {
            status = ER_LANGUAGE_NOT_SUPPORTED;
        }

        propsLock.Unlock();
    } else {
        status = isLanguageSupported(languageTag);
    }

    if (status != ER_OK) {
        return status;
    }

    bool deleted = false;
    propsLock.Lock();
    std::pair<PropertyMap::iterator, PropertyMap::iterator> propertiesIter = properties.equal_range(propertyKey);
    for (PropertyMap::iterator it = propertiesIter.first; it != propertiesIter.second;) {
        const PropertyStoreProperty& property = it->second;
        if (property.getIsWritable()) {
            if ((languageTag == NULL && property.getLanguage().empty()) || (languageTag != NULL && property.getLanguage() == languageTag)) {
                // replace with the old value, or erase from the map
                // remember, the factory settings map keys follow the format: FieldName{.LanguageTag}
                qcc::String propName = name;
                if (languageTag) {
                    propName += ".";
                    propName += languageTag;
                }

                StringMap::iterator sit = factorySettings.find(propName);
                if (sit != factorySettings.end()) {
                    MsgArg value("s", sit->second.c_str());
                    PropertyStoreProperty newProperty(property.getPropertyName(), value, property.getIsPublic(), property.getIsWritable(), property.getIsAnnouncable());

                    if (languageTag) {
                        newProperty.setLanguage(languageTag);
                    }

                    it->second = newProperty;
                    ++it;
                } else {
                    it = properties.erase(it);
                }

                deleted = true;
                break;
            } else {
                ++it;
            }
        } else {
            ++it;
        }
    }

    if (!deleted) {
        status = languageTag ? ER_LANGUAGE_NOT_SUPPORTED : ER_INVALID_VALUE;
    }

    if (status == ER_OK) {
        needsWrite = true;
        propsLock.Unlock();
        propsCond.Signal();
    } else {
        propsLock.Unlock();
    }

    return status;
}

void LSFPropertyStore::Run()
{
    QCC_DbgTrace(("%s", __func__));
    while (running) {
        propsLock.Lock();
        while (!needsWrite && running) {
            propsCond.Wait(propsLock);
        }

        if (!running) {
            propsLock.Unlock();
            break;
        }

        // make a copy to minimize the critical section
        PropertyMap toWrite = properties;
        needsWrite = false;
        propsLock.Unlock();

        StringMap fileOutput;
        for (PropertyMap::const_iterator it = toWrite.begin(); it != toWrite.end(); ++it) {
            const PropertyStoreProperty& property = it->second;

            if (property.getIsWritable()) {
                qcc::String name = property.getPropertyName();
                const qcc::String& lang = property.getLanguage();


                if (!lang.empty()) {
                    name += "." + lang;
                }

                const MsgArg& val = property.getPropertyValue();
                if (val.typeId != ALLJOYN_BOOLEAN) {
                    const char* propVal;
                    val.Get("s", &propVal);
                    fileOutput[name] = propVal;
                } else {
                    bool propVal;
                    val.Get("b", &propVal);
                    fileOutput[name] = propVal ? "1" : "0";
                }
            }
        }

        // these are the new Properties if and only if the data
        // was successfully written to the user config file.
        if (PropertyParser::WriteFile(configFileName, fileOutput)) {
            // send the announcement!
            AboutService* aboutService = AboutServiceApi::getInstance();
            if (aboutService) {
                aboutService->Announce();
            }
        }
        // what to do if this fails?
    }
}

void LSFPropertyStore::Stop()
{
    QCC_DbgTrace(("%s", __func__));
    running = false;
    propsCond.Signal();
}

void LSFPropertyStore::Join()
{
    QCC_DbgTrace(("%s", __func__));
    Thread::Join();
}

QStatus LSFPropertyStore::isLanguageSupported(const char* language)
{
    QCC_DbgTrace(("%s", __func__));
    if (!language) {
        return ER_LANGUAGE_NOT_SUPPORTED;
    }

    if (std::find(supportedLanguages.begin(), supportedLanguages.end(), qcc::String(language)) == supportedLanguages.end()) {
        return ER_LANGUAGE_NOT_SUPPORTED;
    }

    return ER_OK;
}

LSFPropertyStore::PropertyStoreKey LSFPropertyStore::getPropertyStoreKeyFromName(const qcc::String& propertyStoreName)
{
    QCC_DbgTrace(("%s", __func__));
    for (int indx = 0; indx < NUMBER_OF_KEYS; indx++) {
        if (PropertyStoreName[indx] == propertyStoreName) {
            return (PropertyStoreKey) indx;
        }
    }
    return NUMBER_OF_KEYS;
}


QStatus LSFPropertyStore::setSupportedLangs(const std::vector<qcc::String>& supportedLangs)
{
    QCC_DbgTrace(("%s", __func__));
    QStatus status = ER_OK;
    PropertyStoreKey propertyKey = SUPPORTED_LANGS;
    std::vector<const char*> supportedLangsVec(supportedLangs.size());
    for (uint32_t indx = 0; indx < supportedLangs.size(); indx++) {
        supportedLangsVec[indx] = supportedLangs[indx].c_str();
    }

    MsgArg msgArg("as", supportedLangsVec.size(), (supportedLangsVec.empty()) ? NULL : &supportedLangsVec.front());

    status = validateValue(propertyKey, msgArg);
    if (status != ER_OK) {
        return status;
    }
    removeExisting(propertyKey);
    supportedLanguages = supportedLangs;

    PropertyStoreProperty property(PropertyStoreName[propertyKey], msgArg, true, false, false);
    properties.insert(PropertyPair(propertyKey, property));
    return status;
}


PropertyStoreProperty* LSFPropertyStore::getProperty(PropertyStoreKey propertyKey)
{
    QCC_DbgTrace(("%s", __func__));
    // assume we are already locked!
    PropertyStoreProperty* prop = NULL;
    PropertyMap::iterator iter = properties.find(propertyKey);
    if (iter != properties.end()) {
        prop = &iter->second;
    }

    return prop;
}

uint64_t LSFPropertyStore::GetRank()
{
    QCC_DbgTrace(("%s", __func__));
    uint64_t rank = 0UL;
    propsLock.Lock();
    PropertyMap::iterator iter = properties.find(RANK);
    if (iter != properties.end()) {
        iter->second.getPropertyValue().Get("t", &rank);
    }

    propsLock.Unlock();
    return rank;
}

bool LSFPropertyStore::IsLeader()
{
    QCC_DbgTrace(("%s", __func__));
    bool leader = 0;
    propsLock.Lock();
    PropertyMap::iterator iter = properties.find(IS_LEADER);
    if (iter != properties.end()) {
        iter->second.getPropertyValue().Get("b", &leader);
    }

    propsLock.Unlock();
    return leader;
}

void LSFPropertyStore::SetIsLeader(bool leader)
{
    propsLock.Lock();
    setProperty(IS_LEADER, leader, true, false, true);
    propsLock.Unlock();
    AboutServiceApi::getInstance()->Announce();
}

QStatus LSFPropertyStore::setProperty(PropertyStoreKey propertyKey, uint64_t value, bool isPublic, bool isWritable, bool isAnnouncable)
{
    QCC_DbgTrace(("%s", __func__));
    QStatus status = ER_OK;
    MsgArg msgArg("t", value);
    status = validateValue(propertyKey, msgArg);
    if (status != ER_OK) {
        return status;
    }

    removeExisting(propertyKey);

    PropertyStoreProperty property(PropertyStoreName[propertyKey], msgArg, isPublic, isWritable, isAnnouncable);
    properties.insert(PropertyPair(propertyKey, property));
    return status;
}

QStatus LSFPropertyStore::setProperty(PropertyStoreKey propertyKey, uint32_t value, bool isPublic, bool isWritable, bool isAnnouncable)
{
    QCC_DbgTrace(("%s", __func__));
    QStatus status = ER_OK;
    MsgArg msgArg("u", value);
    status = validateValue(propertyKey, msgArg);
    if (status != ER_OK) {
        return status;
    }

    removeExisting(propertyKey);

    PropertyStoreProperty property(PropertyStoreName[propertyKey], msgArg, isPublic, isWritable, isAnnouncable);
    properties.insert(PropertyPair(propertyKey, property));
    return status;
}

QStatus LSFPropertyStore::setProperty(PropertyStoreKey propertyKey, bool value, bool isPublic, bool isWritable, bool isAnnouncable)
{
    QCC_DbgTrace(("%s", __func__));
    QStatus status = ER_OK;
    MsgArg msgArg("b", value);
    status = validateValue(propertyKey, msgArg);
    if (status != ER_OK) {
        return status;
    }

    removeExisting(propertyKey);

    PropertyStoreProperty property(PropertyStoreName[propertyKey], msgArg, isPublic, isWritable, isAnnouncable);
    properties.insert(PropertyPair(propertyKey, property));
    return status;
}

QStatus LSFPropertyStore::setProperty(PropertyStoreKey propertyKey, const uint8_t* value, uint32_t len,
                                      bool isPublic, bool isWritable, bool isAnnouncable)
{
    QCC_DbgTrace(("%s", __func__));
    MsgArg msgArg("ay", len, value);
    msgArg.SetOwnershipFlags(MsgArg::OwnsData);
    QStatus status = validateValue(propertyKey, msgArg);

    if (status != ER_OK) {
        return status;
    }

    removeExisting(propertyKey);
    PropertyStoreProperty property(PropertyStoreName[propertyKey], msgArg, isPublic, isWritable, isAnnouncable);
    properties.insert(PropertyPair(propertyKey, property));
    return status;
}

QStatus LSFPropertyStore::setProperty(PropertyStoreKey propertyKey, const qcc::String& value,
                                      bool isPublic, bool isWritable, bool isAnnouncable)
{
    QCC_DbgTrace(("%s", __func__));
    QStatus status = ER_OK;
    MsgArg msgArg("s", value.c_str());
    status = validateValue(propertyKey, msgArg);
    if (status != ER_OK) {
        return status;
    }

    removeExisting(propertyKey);

    PropertyStoreProperty property(PropertyStoreName[propertyKey], msgArg, isPublic, isWritable, isAnnouncable);
    properties.insert(PropertyPair(propertyKey, property));
    return status;
}

QStatus LSFPropertyStore::setProperty(PropertyStoreKey propertyKey, const char* value, bool isPublic, bool isWritable, bool isAnnouncable)
{
    QCC_DbgTrace(("%s", __func__));
    QStatus status = ER_OK;
    MsgArg msgArg("s", value);
    status = validateValue(propertyKey, msgArg);
    if (status != ER_OK) {
        return status;
    }

    removeExisting(propertyKey);

    PropertyStoreProperty property(PropertyStoreName[propertyKey], msgArg, isPublic, isWritable, isAnnouncable);
    properties.insert(PropertyPair(propertyKey, property));
    return status;
}

QStatus LSFPropertyStore::setProperty(PropertyStoreKey propertyKey, const char* value, const qcc::String& language,
                                      bool isPublic, bool isWritable, bool isAnnouncable)
{
    QCC_DbgTrace(("%s", __func__));
    QStatus status = ER_OK;
    MsgArg msgArg("s", value);
    status = validateValue(propertyKey, msgArg, language);
    if (status != ER_OK) {
        return status;
    }

    removeExisting(propertyKey, language);

    PropertyStoreProperty property(PropertyStoreName[propertyKey], msgArg, language, isPublic, isWritable, isAnnouncable);
    properties.insert(PropertyPair(propertyKey, property));
    return status;
}

QStatus LSFPropertyStore::setProperty(PropertyStoreKey propertyKey, const qcc::String& value, const qcc::String& language,
                                      bool isPublic, bool isWritable, bool isAnnouncable)
{
    QCC_DbgTrace(("%s", __func__));
    QStatus status = ER_OK;
    MsgArg msgArg("s", value.c_str());
    status = validateValue(propertyKey, msgArg, language);
    if (status != ER_OK) {
        return status;
    }

    removeExisting(propertyKey, language);

    PropertyStoreProperty property(PropertyStoreName[propertyKey], msgArg, language, isPublic, isWritable, isAnnouncable);
    properties.insert(PropertyPair(propertyKey, property));
    return status;
}

void LSFPropertyStore::removeExisting(PropertyStoreKey propertyKey)
{
    QCC_DbgTrace(("%s", __func__));
    PropertyMap::iterator iter = properties.find(propertyKey);
    if (iter != properties.end()) {
        properties.erase(iter);
    }
}

void LSFPropertyStore::removeExisting(PropertyStoreKey propertyKey, const qcc::String& language)
{
    QCC_DbgTrace(("%s", __func__));
    std::pair<PropertyMap::iterator, PropertyMap::iterator> iter = properties.equal_range(propertyKey);
    for (PropertyMap::iterator it = iter.first; it != iter.second; ++it) {
        ajn::services::PropertyStoreProperty& prop = it->second;

        if (prop.getLanguage() == language) {
            properties.erase(it);
            break;
        }
    }
}

QStatus LSFPropertyStore::validateValue(PropertyStoreKey propertyKey, const ajn::MsgArg& value, const qcc::String& languageTag)
{
    QCC_DbgTrace(("%s", __func__));
    QStatus status = ER_OK;

    switch (propertyKey) {
    case APP_ID:
        if (value.typeId != ALLJOYN_BYTE_ARRAY) {
            status = ER_INVALID_VALUE;
        }
        break;

    case DEVICE_ID:
    case DEVICE_NAME:
    case APP_NAME:
        if (value.typeId != ALLJOYN_STRING) {
            status = ER_INVALID_VALUE;
        } else if (value.v_string.len == 0) {
            status = ER_INVALID_VALUE;
        }
        break;

    case DESCRIPTION:
    case MANUFACTURER:
    case DATE_OF_MANUFACTURE:
    case MODEL_NUMBER:
    case SOFTWARE_VERSION:
    case AJ_SOFTWARE_VERSION:
    case HARDWARE_VERSION:
    case SUPPORT_URL:
        if (value.typeId != ALLJOYN_STRING) {
            status = ER_INVALID_VALUE;
        }
        break;

    case DEFAULT_LANG:
        if (value.typeId != ALLJOYN_STRING) {
            status = ER_INVALID_VALUE;
        } else if (value.v_string.len == 0) {
            status = ER_INVALID_VALUE;
        } else {
            status = isLanguageSupported(value.v_string.str);
        }

        break;

    case SUPPORTED_LANGS:
        if (value.typeId != ALLJOYN_ARRAY) {
            status = ER_INVALID_VALUE;
        } else if (value.v_array.GetNumElements() == 0) {
            status = ER_INVALID_VALUE;
        } else if (strcmp(value.v_array.GetElemSig(), "s") != 0) {
            status = ER_INVALID_VALUE;
        }

        break;

    case RANK:
        if (value.typeId != ALLJOYN_UINT64) {
            status = ER_INVALID_VALUE;
        }
        break;

    case IS_LEADER:
        if (value.typeId != ALLJOYN_BOOLEAN) {
            status = ER_INVALID_VALUE;
        }
        break;

    case NUMBER_OF_KEYS:
    default:
        status = ER_INVALID_VALUE;
        break;
    }

    if (status != ER_OK) {
        QCC_LogError(status, ("Validation of PropertyStore value %s failed", PropertyStoreName[propertyKey].c_str()));
    }
    return status;
}
