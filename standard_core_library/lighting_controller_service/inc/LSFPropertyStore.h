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
#ifndef LSF_PROPERTY_STORE
#define LSF_PROPERTY_STORE

#include <stdio.h>
#include <iostream>
#include <string>

#include <alljoyn/about/AboutPropertyStoreImpl.h>

namespace lsf {

/**
 * class PropertyStoreImpl
 * Property store implementation
 */
class LSFPropertyStore : public ajn::services::AboutPropertyStoreImpl {

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
    void FactoryReset();

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

    bool isInitialized;

    std::string configFileName;

    std::string factoryConfigFileName;

    void ReadConfiguration();
    void ReadFactoryConfiguration();

    ajn::services::PropertyStoreKey getPropertyStoreKeyFromName(qcc::String const& propertyStoreName);

    typedef std::map<std::string, std::string> StringMap;

    std::vector<std::string> supportedLanguages;
};

}

#endif /* LSF_PROPERTY_STORE */
