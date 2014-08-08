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
#include <alljoyn/about/AboutIconService.h>
#include <alljoyn/config/ConfigService.h>


#include <LSFTypes.h>
#include <Mutex.h>

#include <PersistenceThread.h>
#include <LSFPropertyStore.h>
#include <LampManager.h>
#include <LampGroupManager.h>
#include <PresetManager.h>
#include <SceneManager.h>
#include <MasterSceneManager.h>
#include <LeaderElectionObject.h>
#include <LampClients.h>

namespace lsf {

#define CONTROLLER_SERVICE_VERSION 1

/**
 * This class functions as the message dispatcher. It receives the messages from AllJoyn
 * and forwards it to the appropriate manager and receives a reply from a manager and
 * passes it on to AllJoyn
 */
class ControllerService : public ajn::BusObject, public ajn::services::ConfigService::Listener {
    friend class ControllerServiceManager;
    friend class LampClients;
    friend class LeaderElectionObject;
  public:

    /**
     * Constructor
     */
    ControllerService(
        const std::string& factoryConfigFile,
        const std::string& configFile,
        const std::string& lampGroupFile,
        const std::string& presetFile,
        const std::string& sceneFile,
        const std::string& masterSceneFile);

    ControllerService(
        ajn::services::PropertyStore& propStore,
        const std::string& factoryConfigFile,
        const std::string& configFile,
        const std::string& lampGroupFile,
        const std::string& presetFile,
        const std::string& sceneFile,
        const std::string& masterSceneFile);

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
    QStatus Start(const char* keyStoreFileLocation);

    QStatus RegisterMethodHandlers(void);

    /**
     * Stops the ControllerService
     *
     * @param  None
     * @return ER_OK if successful, error otherwise
     */
    QStatus Stop(void);

    QStatus Join(void);

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

    void SendMethodReplyWithResponseCodeAndListOfIDs(const ajn::Message& msg, LSFResponseCode responseCode, const LSFStringList& idList);

    void SendMethodReplyWithResponseCodeIDAndName(const ajn::Message& msg, LSFResponseCode responseCode, const LSFString& lsfId, const LSFString& lsfName);

    void SendMethodReplyWithResponseCodeAndID(const ajn::Message& msg, LSFResponseCode responseCode, const LSFString& lsfId);

    void SendMethodReplyWithUint32Value(const ajn::Message& msg, uint32_t value);

    void SendMethodReplyWithResponseCodeIDLanguageAndName(const ajn::Message& msg, LSFResponseCode responseCode, const LSFString& lsfId, const LSFString& language, const LSFString& name);

    QStatus SendSignal(const char* ifaceName, const char* methodName, const LSFStringList& idList);

    QStatus SendSignalWithoutArg(const char* ifaceName, const char* signalName);

    void SendSceneOrMasterSceneAppliedSignal(LSFString& sceneorMasterSceneId) {
        sceneManager.SendSceneOrMasterSceneAppliedSignal(sceneorMasterSceneId);
    }

    void ScheduleFileReadWrite(Manager* manager);

    QStatus SendBlobUpdate(LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp);

    void SendGetBlobReply(ajn::Message& message, LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp);

    bool IsRunning();

    uint64_t GetRank();

    bool IsLeader();

    void SetIsLeader(bool val);

    void AddObjDescriptionToAnnouncement(qcc::String path, qcc::String interface);

    void RemoveObjDescriptionFromAnnouncement(qcc::String path, qcc::String interface);

    void SetAllowUpdates(bool allow);

    bool UpdatesAllowed();

    void DoLeaveSessionAsync(ajn::SessionId sessionId);

    void LeaveSessionAsyncReplyHandler(ajn::Message& message, void* context);

    void OverrideRank(uint64_t rank) {
        internalPropertyStore.SetRank(rank);
    }

    LeaderElectionObject& GetLeaderElectionObj(void) {
        return elector;
    }

  private:

    void Initialize();

    uint32_t GetControllerServiceInterfaceVersion(void);

    /**
     * Handles the GetPropery request
     * @param ifcName  interface name
     * @param propName the name of the propery
     * @param val reference of MsgArg out parameter.
     * @return status - success/failure
     */
    QStatus Get(const char* ifcName, const char* propName, ajn::MsgArg& val);

    virtual void ObjectRegistered(void);

    typedef struct {
        std::string interfaceDescription;  /**< Interface Description */
        const char* interfaceName;         /**< Interface Name */
    } InterfaceEntry;

    QStatus CreateAndAddInterface(std::string interfaceDescription, const char* interfaceName);

    QStatus CreateAndAddInterfaces(const InterfaceEntry* entries, size_t numEntries);

    ajn::BusAttachment bus;
    LeaderElectionObject elector;
    lsf::Mutex serviceSessionMutex;
    ajn::SessionId serviceSession;

    class ControllerListener;
    ControllerListener* listener;

    class OBSJoiner;

    LampManager lampManager;
    LampGroupManager lampGroupManager;
    PresetManager presetManager;
    SceneManager sceneManager;
    MasterSceneManager masterSceneManager;

    void SessionLost(ajn::SessionId sessionId);
    void SessionJoined(ajn::SessionId sessionId, const char* joiner);
    void LeaveSession(void);

    void FoundLocalOnboardingService(const char* busName, ajn::SessionPort port);

    LSFPropertyStore internalPropertyStore;
    ajn::services::PropertyStore& propertyStore;
    ajn::services::AboutServiceApi* aboutService;
    ajn::services::AboutIconService aboutIconService;
    ajn::services::ConfigService configService;
    ajn::services::NotificationSender* notificationSender;

    ajn::ProxyBusObject* obsObject;
    bool isObsObjectReady;
    Mutex obsObjectLock;

    bool isRunning;
    Mutex isRunningLock;

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

    /*
     * This function is not thread safe. it should not be called without locking messageHandlersLock
     */
    template <typename OBJ>
    void AddMethodHandler(const std::string& methodName, OBJ* obj, void (OBJ::* methodCall)(ajn::Message &))
    {
        MethodHandlerBase* handler = new MethodHandler<OBJ>(obj, methodCall);
        std::pair<DispatcherMap::iterator, bool> ins = messageHandlers.insert(std::make_pair(methodName, handler));
        if (ins.second == false) {
            // if this was already there, overwrite and delete the old handler
            delete ins.first->second;
            ins.first->second = handler;
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
    Mutex messageHandlersLock;


    PersistenceThread fileWriterThread;
    bool firstAnnouncementSent;

    bool updatesAllowed;
    Mutex updatesAllowedLock;
};

class ControllerServiceManager {
  public:
    ControllerServiceManager(
        const std::string& factoryConfigFile,
        const std::string& configFile,
        const std::string& lampGroupFile,
        const std::string& presetFile,
        const std::string& sceneFile,
        const std::string& masterSceneFile) :
        controllerService(factoryConfigFile, configFile, lampGroupFile, presetFile, sceneFile, masterSceneFile) {

    }

    ControllerServiceManager(
        ajn::services::PropertyStore& propStore,
        const std::string& factoryConfigFile,
        const std::string& configFile,
        const std::string& lampGroupFile,
        const std::string& presetFile,
        const std::string& sceneFile,
        const std::string& masterSceneFile) :
        controllerService(propStore, factoryConfigFile, configFile, lampGroupFile, presetFile, sceneFile, masterSceneFile) {

    }

    ~ControllerServiceManager() {
    }
    /**
     * Starts the ControllerService
     *
     * @param  keyStoreFileLocation Absolute path of the location to put the AllJoyn keystore file in. If this is not specified, the
     *                              default location will be used. Android applications running the Controller Service should pass in the
     *                              location returned by Context.getFileStreamPath("alljoyn_keystore").getAbsolutePath()
     * @return ER_OK if successful, error otherwise
     */
    QStatus Start(const char* keyStoreFileLocation) {
        return controllerService.Start(keyStoreFileLocation);
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

    QStatus Join(void) {
        return controllerService.Join();
    }

    bool IsRunning() {
        return controllerService.IsRunning();
    }

    ControllerService& GetControllerService(void) { return controllerService; };

    void OverrideRank(uint64_t rank) {
        controllerService.OverrideRank(rank);
    }

  private:
    ControllerService controllerService;
};


}

#endif
