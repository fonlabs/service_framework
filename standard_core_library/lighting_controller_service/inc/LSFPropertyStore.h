#ifndef LSF_PROPERTY_STORE
#define LSF_PROPERTY_STORE
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

#include <stdio.h>
#include <iostream>
#include <string>
#include <Thread.h>
#include <Mutex.h>
#include <Condition.h>

#include <alljoyn/about/AboutServiceApi.h>
#include <alljoyn/about/PropertyStore.h>
#include <alljoyn/about/PropertyStoreProperty.h>

namespace lsf {

/**
 * class PropertyStoreImpl
 * Property store implementation
 */
class LSFPropertyStore : public ajn::services::PropertyStore, public Thread {

  public:

    typedef enum {
        DEVICE_ID = 0,
        DEVICE_NAME = 1,
        APP_ID = 2,
        APP_NAME = 3,
        DEFAULT_LANG = 4,
        SUPPORTED_LANGS = 5,
        DESCRIPTION = 6,
        MANUFACTURER = 7,
        DATE_OF_MANUFACTURE = 8,
        MODEL_NUMBER = 9,
        SOFTWARE_VERSION = 10,
        AJ_SOFTWARE_VERSION = 11,
        HARDWARE_VERSION = 12,
        SUPPORT_URL = 13,
        NUMBER_OF_KEYS = 14
    } PropertyStoreKey;

  private:
    friend void PopulateDefaultProperties(LSFPropertyStore& propStore);

    //typedef std::pair<PropertyStoreKey, qcc::String> PropertyMapKey;

    typedef std::multimap<PropertyStoreKey, ajn::services::PropertyStoreProperty> PropertyMap;

    /**
     * Relate PropertyStoreKey with its PropertyStorePoperty
     * Used to hold properties that are not localizable
     */
    typedef PropertyMap::value_type PropertyPair;

  public:
    /**
     * PropertyStoreImpl - constructor
     * @param factoryConfigFile
     * @param configFile
     */
    LSFPropertyStore(const std::string& factoryConfigFile, const std::string& configFile);

    /**
     * virtual Destructor
     */
    virtual ~LSFPropertyStore();

    /**
     * FactoryReset
     */
    virtual QStatus Reset();

    /**
     * virtual method ReadAll
     * @param languageTag
     * @param filter
     * @param all
     * @return QStatus
     */
    virtual QStatus ReadAll(const char* languageTag, Filter filter, ajn::MsgArg& all);

    /**
     * virtual method Update
     * @param name
     * @param languageTag
     * @param value
     * @return QStatus
     */
    virtual QStatus Update(const char* name, const char* languageTag, const ajn::MsgArg* value);

    /**
     * virtual method Delete
     * @param name
     * @param languageTag
     * @return QStatus
     */
    virtual QStatus Delete(const char* name, const char* languageTag);

    /**
     * method Initialize
     */
    void Initialize();

  private:

    QStatus setSupportedLangs(const std::vector<qcc::String>& supportedLangs);

    /**
     * getProperty
     * @param[in] propertyKey
     * @return PropertyStoreProperty.
     */
    ajn::services::PropertyStoreProperty* getProperty(PropertyStoreKey propertyKey);

    bool isInitialized;

    std::string configFileName;

    std::string factoryConfigFileName;

    bool factoryReset;

    void ReadConfiguration();
    void ReadFactoryConfiguration();

    PropertyStoreKey getPropertyStoreKeyFromName(const qcc::String& propertyStoreName);

    typedef std::map<qcc::String, qcc::String> StringMap;


    /**
     * removeExisting
     * @param[in] propertyKey the value to be removed from the PropertyStore
     */
    void removeExisting(PropertyStoreKey propertyKey);
    /**
     * removeExisting
     * @param[in] propertyKey
     * @param[in] language
     */
    void removeExisting(PropertyStoreKey propertyKey, const qcc::String& language);

    /**
     * validateValue
     * @param[in] propertyKey
     * @param[in] value
     * @param[in] languageTag
     * @return QStatus
     */
    QStatus validateValue(PropertyStoreKey propertyKey, const ajn::MsgArg& value, const qcc::String& languageTag = "");
    /**
     * setProperty
     * @param[in] propertyKey
     * @param[in] value
     * @param[in] isPublic
     * @param[in] isWritable
     * @param[in] isAnnouncable
     * @return QStatus
     */
    QStatus setProperty(PropertyStoreKey propertyKey, const qcc::String& value, bool isPublic, bool isWritable, bool isAnnouncable);

    QStatus setProperty(PropertyStoreKey propertyKey, const uint8_t* value, uint32_t len, bool isPublic, bool isWritable, bool isAnnouncable);

    /**
     * setProperty
     * @param[in] propertyKey
     * @param[in] value
     * @param[in] language
     * @param[in] isPublic
     * @param[in] isWritable
     * @param[in] isAnnouncable
     * @return QStatus
     */
    QStatus setProperty(PropertyStoreKey propertyKey, const qcc::String& value, const qcc::String& language, bool isPublic, bool isWritable, bool isAnnouncable);

    /**
     * m_Properties
     */
    PropertyMap properties;
    Mutex propsLock;
    Condition propsCond;

    bool needsWrite;
    bool running;

    /**
     * Keep a backup of the factory settings ini file
     */
    StringMap factorySettings;

    virtual void Run();
    virtual void Stop();

    /**
     * m_PropertyStoreName
     */
    static qcc::String PropertyStoreName[NUMBER_OF_KEYS + 1];

    /**
     * isLanguageSupported
     * @param[in] language
     * @return
     *   - ER_OK if language is supported
     *   - ER_LANGUAGE_NOT_SUPPORTED if language is not supported
     */
    QStatus isLanguageSupported(const char* language);

    /**
     * supportedLanguages Stores the supported languages
     */
    std::vector<qcc::String> supportedLanguages;
};

}

#endif /* LSF_PROPERTY_STORE */
