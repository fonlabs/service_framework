#ifndef _CONTROLLER_SERVICE_H_
#define _CONTROLLER_SERVICE_H_

/**
 * @file
 * This file provides definitions for the Lighting ControllerService Service Dispatcher
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

#include <map>
#include <string>

#include <alljoyn/BusAttachment.h>
#include <alljoyn/ProxyBusObject.h>
#include <alljoyn/notification/NotificationSender.h>
#include <alljoyn/about/AboutService.h>
#include <alljoyn/config/ConfigService.h>


#include <LSFTypes.h>
#include <Mutex.h>

#include <LSFPropertyStore.h>
#include <LampManager.h>
#include <LampGroupManager.h>
#include <PresetManager.h>
#include <SceneManager.h>
#include <MasterSceneManager.h>

namespace lsf {

#define CONTROLLER_SERVICE_VERSION 1

/**
 * This class functions as the message dispatcher. It receives the messages from AllJoyn
 * and forwards it to the appropriate manager and receives a reply from a manager and
 * passes it on to AllJoyn
 */
class ControllerService : public ajn::BusObject, public ajn::services::ConfigService::Listener {
    friend class ControllerServiceManager;
  public:

    /**
     * Constructor
     */
    ControllerService(const std::string& factoryConfigFile, const std::string& configFile);

    /**
     * Destructor
     */
    ~ControllerService();

    /**
     * Starts the ControllerService
     *
     * @param  None
     * @return ER_OK if successful, error otherwise
     */
    QStatus Start(void);

    /**
     * Stops the ControllerService
     *
     * @param  None
     * @return ER_OK if successful, error otherwise
     */
    QStatus Stop(void);

    /**
     * Returns a reference to the AllJoyn BusAttachment
     *
     * @param  None
     * @return reference to the AllJoyn BusAttachment
     */
    ajn::BusAttachment& GetBusAttachment(void) { return bus; }

    LampManager& GetLampManager(void) { return lampManager; };
    LampGroupManager& GetLampGroupManager(void) { return lampGroupManager; };
    PresetManager& GetPresetManager(void) { return presetManager; };
    SceneManager& GetSceneManager(void) { return sceneManager; };
    MasterSceneManager& GetMasterSceneManager(void) { return masterSceneManager; };

    void SendMethodReply(const ajn::Message& msg, const ajn::MsgArg* args = NULL, size_t numArgs = 0);

    void SendMethodReplyWithResponseCodeAndListOfIDs(const ajn::Message& msg, LSFResponseCode& responseCode, LSFStringList& idList);

    void SendMethodReplyWithResponseCodeIDAndName(const ajn::Message& msg, LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName);

    void SendMethodReplyWithResponseCodeAndID(const ajn::Message& msg, LSFResponseCode& responseCode, LSFString& lsfId);

    void SendMethodReplyWithUint32Value(const ajn::Message& msg, uint32_t& value);

    void SendMethodReplyWithResponseCodeIDLanguageAndName(const ajn::Message& msg, LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language, LSFString& name);

    QStatus SendSignal(const char* ifaceName, const char* methodName, LSFStringList& idList);

    QStatus SendSignalWithoutArg(const char* ifaceName, const char* signalName);

  private:

    virtual void ObjectRegistered(void);

    typedef struct {
        std::string interfaceDescription;  /**< Interface Description */
        const char* interfaceName;         /**< Interface Name */
    } InterfaceEntry;

    QStatus CreateAndAddInterface(std::string interfaceDescription, const char* interfaceName);

    QStatus CreateAndAddInterfaces(const InterfaceEntry* entries, size_t numEntries);

    ajn::BusAttachment bus;
    ajn::SessionId serviceSession;

    class ControllerListener;
    ControllerListener* listener;

    LampManager lampManager;
    LampGroupManager lampGroupManager;
    PresetManager presetManager;
    SceneManager sceneManager;
    MasterSceneManager masterSceneManager;

    void SessionLost(ajn::SessionId sessionId);
    void LeaveSession(ajn::SessionId sessionId);
    void SessionJoined(ajn::SessionId sessionId);

    LSFPropertyStore propertyStore;
    ajn::services::AboutServiceApi* aboutService;
    ajn::services::ConfigService configService;
    ajn::services::NotificationSender* notificationSender;

    // Interface for ajn::services::ConfigService::Listener
    QStatus Restart();
    QStatus FactoryReset();
    QStatus SetPassphrase(const char* daemonRealm, size_t passcodeSize, const char* passcode, ajn::SessionId sessionId);

    void MethodCallDispatcher(const ajn::InterfaceDescription::Member* member, ajn::Message& msg);

    /*
     * Lighting Reset the Controller Service
     */
    void LightingResetControllerService(ajn::Message& msg);

    void GetControllerServiceVersion(ajn::Message& msg);

    template <typename OBJ>
    void AddMethodHandler(const std::string& methodName, OBJ* obj, void (OBJ::* methodCall)(ajn::Message &))
    {
        MethodHandlerBase* handler = new MethodHandler<OBJ>(obj, methodCall);
        std::pair<DispatcherMap::iterator, bool> ins = messageHandlers.insert(std::make_pair(methodName, handler));
        if (ins.second == false) {
            // if this was already there, overwrite and delete the old handler
            delete ins.first->second;
        }
    }

    class MethodHandlerBase {
      public:
        virtual ~MethodHandlerBase() { }
        virtual void Handle(ajn::Message& msg) = 0;
    };

    template <typename OBJ>
    class MethodHandler : public MethodHandlerBase {
        typedef void (OBJ::* HandlerFunction)(ajn::Message&);

      public:
        MethodHandler(OBJ* obj, HandlerFunction handleFunc) :
            object(obj), handler(handleFunc) { }

        virtual ~MethodHandler() { }

        virtual void Handle(ajn::Message& msg) {
            (object->*(handler))(msg);
        }

        OBJ* object;
        HandlerFunction handler;
    };

    typedef std::map<std::string, MethodHandlerBase*> DispatcherMap;
    DispatcherMap messageHandlers;
};

class ControllerServiceManager {
  public:
    ControllerServiceManager(const std::string& factoryConfigFile, const std::string& configFile) :
        controllerService(factoryConfigFile, configFile) {

    }

    ~ControllerServiceManager() {

    }
    /**
     * Starts the ControllerService
     *
     * @param  None
     * @return ER_OK if successful, error otherwise
     */
    QStatus Start(void) {
        return controllerService.Start();
    }

    /**
     * Stops the ControllerService
     *
     * @param  None
     * @return ER_OK if successful, error otherwise
     */
    QStatus Stop(void) {
        return controllerService.Stop();
    }

    ControllerService& GetControllerService(void) { return controllerService; };

  private:
    ControllerService controllerService;
};


}

#endif
