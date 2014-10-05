#ifndef LSF_PROPERTY_STORE
#define LSF_PROPERTY_STORE
/**
 * \ingroup ControllerService
 */
/**
 * @file
 * This file provides definitions for LSF property store
 */
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
 * class PropertyStoreImpl. \n
 * Property store implementation
 */
class LSFPropertyStore : public ajn::services::PropertyStore, public Thread {

  public:
    /**
     * Property store keys
     */
    typedef enum {
        DEVICE_ID = 0,
        DEVICE_NAME,
        APP_ID,
        APP_NAME,
        DEFAULT_LANG,
        SUPPORTED_LANGS,
        DESCRIPTION,
        MANUFACTURER,
        DATE_OF_MANUFACTURE,
        MODEL_NUMBER,
        SOFTWARE_VERSION,
        AJ_SOFTWARE_VERSION,
        HARDWARE_VERSION,
        SUPPORT_URL,
        RANK,
        IS_LEADER,
        NUMBER_OF_KEYS
    } PropertyStoreKey;

  private:
    typedef enum AsyncTasks {
        doNothing,
        writetoOEMConfig,
        writetoConfig,
        removeConfigFile,
    }AsyncTasks;

    friend void OEM_CS_PopulateDefaultProperties(LSFPropertyStore& propStore);

    //typedef std::pair<PropertyStoreKey, qcc::String> PropertyMapKey;

    typedef std::multimap<PropertyStoreKey, ajn::services::PropertyStoreProperty> PropertyMap;

    /**
     * Relate PropertyStoreKey with its PropertyStorePoperty. \n
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
     * PropertyStoreImpl - constructor
     * Use this constructor when not saving the properties to disk
     */
    LSFPropertyStore();

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
    /**
     * Get the controller rank
     * @return 64 bit rank value
     */
    uint64_t GetRank();
    /**
     * Is this controller is a leader?
     * @return boolean
     */
    bool IsLeader();
    /**
     * Stop all threads
     */
    virtual void Stop();
    /**
     * join the stopping threads
     */
    virtual void Join();
    /**
     * set true if that controller is a leader
     * @param leader - true or false
     */
    void SetIsLeader(bool leader);
    /**
     * Set the rank of the current controller
     * @param rank - 64 bit
     */
    void SetRank(uint64_t rank);

  protected:

    // NOTE: the methods below are not all thread-safe

    QStatus setSupportedLangs(const std::vector<qcc::String>& supportedLangs);

    /**
     * getProperty
     * @param[in] propertyKey
     * @return PropertyStoreProperty.
     */
    ajn::services::PropertyStoreProperty* getProperty(PropertyStoreKey propertyKey);

    bool isInitialized;

    const std::string configFileName;

    const std::string factoryConfigFileName;

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
    /**
     * setProperty
     * @param[in] propertyKey
     * @param[in] value
     * @param[in] isPublic
     * @param[in] isWritable
     * @param[in] isAnnouncable
     * @return QStatus
     */
    QStatus setProperty(PropertyStoreKey propertyKey, const char* value, bool isPublic, bool isWritable, bool isAnnouncable);

    QStatus setProperty(PropertyStoreKey propertyKey, const uint8_t* value, uint32_t len, bool isPublic, bool isWritable, bool isAnnouncable);

    QStatus setProperty(PropertyStoreKey propertyKey, uint64_t value, bool isPublic, bool isWritable, bool isAnnouncable);

    QStatus setProperty(PropertyStoreKey propertyKey, uint32_t value, bool isPublic, bool isWritable, bool isAnnouncable);

    QStatus setProperty(PropertyStoreKey propertyKey, bool value, bool isPublic, bool isWritable, bool isAnnouncable);
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
     * setProperty
     * @param[in] propertyKey
     * @param[in] value
     * @param[in] language
     * @param[in] isPublic
     * @param[in] isWritable
     * @param[in] isAnnouncable
     * @return QStatus
     */
    QStatus setProperty(PropertyStoreKey propertyKey, const char* value, const qcc::String& language, bool isPublic, bool isWritable, bool isAnnouncable);

    QStatus FillStringMapWithProperties(StringMap& fileOutput, PropertyMap& propertiestoWrite, bool justWritable);
    /**
     * m_Properties
     */
    PropertyMap properties;
    Mutex propsLock;
    Condition propsCond;
    AsyncTasks asyncTasks;
    bool running;

    /**
     * Are we using a disk writer thread?
     */
    const bool usingThread;

    /**
     * Keep a backup of the factory settings ini file
     */
    StringMap factorySettings;

    StringMap obsSettings;

    virtual void Run();

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
