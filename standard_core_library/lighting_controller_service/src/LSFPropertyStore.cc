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
#include "LSFPropertyStore.h"
#include "PropertyParser.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>

#include <alljoyn/about/AboutServiceApi.h>

using namespace ajn;
using namespace services;
using namespace lsf;

LSFPropertyStore::LSFPropertyStore(const std::string& factoryConfigFile, const std::string& configFile)
    : isInitialized(false),
    configFileName(configFile),
    factoryConfigFileName(factoryConfigFile)
{
}


LSFPropertyStore::~LSFPropertyStore()
{
}

void LSFPropertyStore::Initialize()
{
    isInitialized = true;

    ReadFactoryConfiguration();

    ReadConfiguration();
}


void LSFPropertyStore::ReadFactoryConfiguration()
{
    StringMap data;
    if (!PropertyParser::ParseFile(factoryConfigFileName.c_str(), data)) {
        return;
    }


    StringMap::const_iterator iter;

    iter = data.find("DeviceId");
    if (iter != data.end()) {
        setDeviceId(iter->second.c_str());
        setAppId(iter->second.c_str());
    }

    iter = data.find("SupportedLanguages");
    if (iter != data.end()) {
        PropertyParser::Tokenize(iter->second, supportedLanguages, ' ');

        std::vector<qcc::String> qcc_langs;
        std::vector<std::string>::const_iterator it;
        for (it = supportedLanguages.begin(); it != supportedLanguages.end(); ++it) {
            qcc_langs.push_back(it->c_str());
        }

        setSupportedLangs(qcc_langs);
    }

    iter = data.find("DefaultLanguage");
    if (iter != data.end()) {
        setDefaultLang(iter->second.c_str());
    }

    iter = data.find("AppName");
    if (iter != data.end()) {
        setAppName(iter->second.c_str());
    }

    std::vector<std::string>::const_iterator it;
    for (it = supportedLanguages.begin(); it != supportedLanguages.end(); ++it) {

        iter = data.find("DeviceName." + *it);
        if (iter != data.end()) {
            setProperty(DEVICE_NAME, iter->second.c_str(), it->c_str(), true, true, true);
        }

        iter = data.find("SupportUrl." + *it);
        if (iter != data.end()) {
            setProperty(SUPPORT_URL, iter->second.c_str(), it->c_str(), true, false, true);
        }

        iter = data.find("Manufacturer." + *it);
        if (iter != data.end()) {
            setProperty(MANUFACTURER, iter->second.c_str(), it->c_str(), true, false, true);
        }

        iter = data.find("Description." + *it);
        if (iter != data.end()) {
            setProperty(DESCRIPTION, iter->second.c_str(), it->c_str(), true, false, false);
        }
    }
}

// now read user-defined values that have overridden the
void LSFPropertyStore::ReadConfiguration()
{
    std::vector<std::string> languages;

    StringMap data;
    if (!PropertyParser::ParseFile(factoryConfigFileName.c_str(), data)) {
        return;
    }

    StringMap::const_iterator iter;

    iter = data.find("DefaultLanguage");
    if (iter != data.end()) {
        setDefaultLang(iter->second.c_str());
    }

    std::vector<std::string>::const_iterator it;
    for (it = supportedLanguages.begin(); it != supportedLanguages.end(); ++it) {

        iter = data.find("DeviceName." + *it);
        if (iter != data.end()) {
            setProperty(DEVICE_NAME, iter->second.c_str(), it->c_str(), true, true, true);
        }
    }
}

void LSFPropertyStore::FactoryReset()
{
    // delete the user config file
    unlink(configFileName.c_str());

    m_Properties.clear();
    supportedLanguages.clear();

    // now reparse the factory config file
    ReadFactoryConfiguration();
}


QStatus LSFPropertyStore::ReadAll(const char* languageTag, Filter filter, ajn::MsgArg& all)
{
    if (!isInitialized) {
        return ER_FAIL;
    }

    if (filter == ANNOUNCE || filter == READ) {
        return AboutPropertyStoreImpl::ReadAll(languageTag, filter, all);
    }

    if (filter != WRITE) {
        return ER_FAIL;
    }

    QStatus status = ER_OK;
    if (languageTag != NULL && languageTag[0] != 0) { // check that the language is in the supported languages;
        status = isLanguageSupported(languageTag);
        if (status != ER_OK) {
            return status;
        }
    } else {
        PropertyMap::iterator it = m_Properties.find(DEFAULT_LANG);
        if (it == m_Properties.end()) {
            return ER_LANGUAGE_NOT_SUPPORTED;
        }

        status = it->second.getPropertyValue().Get("s", &languageTag);
        if (status != ER_OK) {
            return status;
        }
    }

    MsgArg* argsWriteData = new MsgArg[m_Properties.size()];
    uint32_t writeArgCount = 0;
    for (PropertyMap::const_iterator it = m_Properties.begin(); it != m_Properties.end(); ++it) {
        const PropertyStoreProperty& property = it->second;

        if (!property.getIsWritable()) {
            continue;
        }

        // check that it is from the defaultLanguage or empty.
        if (!(property.getLanguage().empty() || property.getLanguage() == languageTag)) {
            continue;
        }

        status = argsWriteData[writeArgCount].Set("{sv}", property.getPropertyName().c_str(), new MsgArg(property.getPropertyValue()));
        if (status != ER_OK) {
            break;
        }

        argsWriteData[writeArgCount].SetOwnershipFlags(MsgArg::OwnsArgs, true);
        writeArgCount++;
    }

    status = all.Set("a{sv}", writeArgCount, argsWriteData);
    if (status == ER_OK) {
        all.SetOwnershipFlags(MsgArg::OwnsArgs, true);
    }

    if (status != ER_OK) {
        delete[] argsWriteData;
    }

    return status;
}

QStatus LSFPropertyStore::Update(const char* name, const char* languageTag, const ajn::MsgArg* value)
{
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
    if (languageTag == NULL) {
        return ER_INVALID_VALUE;
    } else if (languageTag[0] == 0) {
        PropertyMap::iterator it = m_Properties.find(DEFAULT_LANG);
        if (it == m_Properties.end()) {
            return ER_LANGUAGE_NOT_SUPPORTED;
        }
        status = it->second.getPropertyValue().Get("s", &languageTag);
    } else {
        status = isLanguageSupported(languageTag);
        if  (status != ER_OK) {
            return status;
        }
    }

    // Special case DEFAULT_LANG is not associated with a language in the PropertyMap and
    // its only valid languageTag = NULL
    // By setting it here, we to let the user follow the same language rules as any other property
    if (propertyKey == DEFAULT_LANG) {
        languageTag = NULL;
    }

    //validate that the value is acceptable
    qcc::String languageString = languageTag ? languageTag : "";
    status = validateValue(propertyKey, *value, languageString);
    if (status != ER_OK) {
        std::cout << "New Value failed validation. Will not update" << std::endl;
        return status;
    }

    bool updated = false;
    std::pair<PropertyMap::iterator, PropertyMap::iterator> propertiesIter = m_Properties.equal_range(propertyKey);
    std::string propName = name;

    for (PropertyMap::iterator it = propertiesIter.first; it != propertiesIter.second; ++it) {
        const PropertyStoreProperty& property = it->second;
        if (property.getIsWritable()) {
            if ((languageTag == NULL && property.getLanguage().empty()) || (languageTag != NULL && property.getLanguage() == languageTag)) {

                PropertyStoreProperty newProperty(property.getPropertyName(), *value, property.getIsPublic(), property.getIsWritable(), property.getIsAnnouncable());
                if (languageTag) {
                    propName += ".";
                    propName += languageTag;
                    newProperty.setLanguage(languageTag);
                }

                updated = true;
                it->second = newProperty;
                break;
            }
        }
    }

    if (!updated) {
        return ER_INVALID_VALUE;
    }

    // read the existing user file and update with the new value
    StringMap data;
    if (!PropertyParser::ParseFile(configFileName, data)) {
        return ER_FAIL;
    }

    // update!
    data[propName] = value->v_string.str;

    if (PropertyParser::WriteFile(configFileName, data)) {
        AboutService* aboutService = AboutServiceApi::getInstance();
        if (aboutService) {
            aboutService->Announce();
        }
        return ER_OK;
    } else {
        return ER_INVALID_VALUE;
    }
}

QStatus LSFPropertyStore::Delete(const char* name, const char* languageTag)
{
    if (!isInitialized) {
        return ER_FAIL;
    }

    PropertyStoreKey propertyKey = getPropertyStoreKeyFromName(name);
    if (propertyKey >= NUMBER_OF_KEYS) {
        return ER_FEATURE_NOT_AVAILABLE;
    }

    QStatus status = ER_OK;
    if (languageTag == NULL) {
        return ER_INVALID_VALUE;
    } else if (languageTag[0] == 0) {
        PropertyMap::iterator it = m_Properties.find(DEFAULT_LANG);
        if (it == m_Properties.end()) {
            return ER_LANGUAGE_NOT_SUPPORTED;
        }
        status = it->second.getPropertyValue().Get("s", &languageTag);
    } else {
        status = isLanguageSupported(languageTag);
        if  (status != ER_OK) {
            return status;
        }
    }

    if (propertyKey == DEFAULT_LANG) {
        languageTag = NULL;
    }

    std::string propName = name;
    if (languageTag) {
        propName += ".";
        propName += languageTag;
    }

    bool isAnnouncable;

    bool deleted = false;
    std::pair<PropertyMap::iterator, PropertyMap::iterator> propertiesIter = m_Properties.equal_range(propertyKey);

    for (PropertyMap::iterator it = propertiesIter.first; it != propertiesIter.second; it++) {
        const PropertyStoreProperty& property = it->second;
        if (property.getIsWritable()) {
            if ((languageTag == NULL && property.getLanguage().empty()) ||
                (languageTag != NULL && property.getLanguage().compare(languageTag) == 0)) {
                PropertyStoreProperty& prop = it->second;
                isAnnouncable = prop.getIsAnnouncable();

                m_Properties.erase(it);
                // insert from backup.
                deleted = true;
                break;
            }
        }
    }

    if (!deleted) {
        if (languageTag != NULL) {
            return ER_LANGUAGE_NOT_SUPPORTED;
        } else {
            return ER_INVALID_VALUE;
        }
    }

    // now write the file, first removing the deleted value
    StringMap data;
    if (!PropertyParser::ParseFile(configFileName, data)) {
        return ER_FAIL;
    }

    StringMap::iterator sit = data.find(propName);
    if (sit != data.end()) {
        data.erase(sit);
    }


    // now we have to revert to the factory value, if there is one
    if (!PropertyParser::ParseFile(factoryConfigFileName, data)) {
        return ER_FAIL;
    }

    sit = data.find(propName);
    if (sit != data.end()) {
        // we know isPublic and isWriteable must be true or we wouldn't be here
        if (languageTag) {
            setProperty(propertyKey, sit->second.c_str(), languageTag, true, true, isAnnouncable);
        } else {
            setProperty(propertyKey, sit->second.c_str(), true, true, isAnnouncable);
        }
    }


    if (PropertyParser::WriteFile(configFileName, data)) {
        AboutService* aboutService = AboutServiceApi::getInstance();
        if (aboutService) {
            aboutService->Announce();
        }
        return ER_OK;
    } else {
        return ER_INVALID_VALUE;
    }
}

PropertyStoreKey LSFPropertyStore::getPropertyStoreKeyFromName(qcc::String const& propertyStoreName)
{
    for (int indx = 0; indx < NUMBER_OF_KEYS; indx++) {
        if (PropertyStoreName[indx] == propertyStoreName) {
            return (PropertyStoreKey) indx;
        }
    }
    return NUMBER_OF_KEYS;
}
