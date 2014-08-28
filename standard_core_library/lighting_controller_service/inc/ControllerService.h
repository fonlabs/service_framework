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
/**
 * controller service version
 */
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
    /**
     * Constructor
     */
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
     * @param  keyStoreFileLocation
     * @return ER_OK if successful, error otherwise
     */
    QStatus Start(const char* keyStoreFileLocation);
    /**
     * Register Method Handlers
     */
    QStatus RegisterMethodHandlers(void);

    /**
     * Stops the ControllerService
     *
     * @return ER_OK if successful, error otherwise
     */
    QStatus Stop(void);
    /**
     * join thread
     */
    QStatus Join(void);

    /**
     * Returns a reference to the AllJoyn BusAttachment
     *
     * @return reference to the AllJoyn BusAttachment
     */
    ajn::BusAttachment& GetBusAttachment(void) { return bus; }
    /**
     * Get Lamp Manager
     */
    LampManager& GetLampManager(void) { return lampManager; };
    /**
     * Get Lamp Group Manager
     */
    LampGroupManager& GetLampGroupManager(void) { return lampGroupManager; };
    /**
     * Get Preset Manager
     */
    PresetManager& GetPresetManager(void) { return presetManager; };
    /**
     * Get Scene Manager
     */
    SceneManager& GetSceneManager(void) { return sceneManager; };
    /**
     * Get Master Scene Manager
     */
    MasterSceneManager& GetMasterSceneManager(void) { return masterSceneManager; };
    /**
     * Send Method Reply
     */
    void SendMethodReply(const ajn::Message& msg, const ajn::MsgArg* args = NULL, size_t numArgs = 0);
    /**
     * Send Method Reply With Response Code And List Of IDs
     */
    void SendMethodReplyWithResponseCodeAndListOfIDs(const ajn::Message& msg, LSFResponseCode responseCode, const LSFStringList& idList);
    /**
     * Send Method Reply With Response Code ID And Name
     */
    void SendMethodReplyWithResponseCodeIDAndName(const ajn::Message& msg, LSFResponseCode responseCode, const LSFString& lsfId, const LSFString& lsfName);
    /**
     * Send Method Reply With Response Code And ID
     */
    void SendMethodReplyWithResponseCodeAndID(const ajn::Message& msg, LSFResponseCode responseCode, const LSFString& lsfId);
    /**
     * Send Method Reply With Uint32 Value
     */
    void SendMethodReplyWithUint32Value(const ajn::Message& msg, uint32_t value);
    /**
     * Send Method Reply With Response Code ID Language And Name
     */
    void SendMethodReplyWithResponseCodeIDLanguageAndName(const ajn::Message& msg, LSFResponseCode responseCode, const LSFString& lsfId, const LSFString& language, const LSFString& name);
    /**
     * Send Signal
     */
    QStatus SendSignal(const char* ifaceName, const char* methodName, const LSFStringList& idList);
    /**
     * Send Signal Without Arg
     */
    QStatus SendSignalWithoutArg(const char* ifaceName, const char* signalName);
    /**
     * Send Scene Or Master Scene Applied Signal
     */
    void SendSceneOrMasterSceneAppliedSignal(LSFString& sceneorMasterSceneId) {
        sceneManager.SendSceneOrMasterSceneAppliedSignal(sceneorMasterSceneId);
    }
    /**
     * Schedule File Read Write
     */
    void ScheduleFileReadWrite(Manager* manager);
    /**
     * Send Blob Update
     */
    QStatus SendBlobUpdate(LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp);
    /**
     * Send Get Blob Reply
     */
    void SendGetBlobReply(ajn::Message& message, LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp);
    /**
     * Is Running
     */
    bool IsRunning();
    /**
     * Get Rank
     */
    uint64_t GetRank();
    /**
     * Is Leader
     */
    bool IsLeader();
    /**
     * Set Is Leader
     */
    void SetIsLeader(bool val);
    /**
     * Add Obj Description To Announcement
     */
    void AddObjDescriptionToAnnouncement(qcc::String path, qcc::String interface);
    /**
     * Remove Obj Description From Announcement
     */
    void RemoveObjDescriptionFromAnnouncement(qcc::String path, qcc::String interface);
    /**
     * Set Allow Updates
     */
    void SetAllowUpdates(bool allow);
    /**
     * Updates Allowed
     */
    bool UpdatesAllowed(void);
    /**
     * Do Leave Session Async
     */
    void DoLeaveSessionAsync(ajn::SessionId sessionId);
    /**
     * Leave Session Async Reply Handler
     */
    void LeaveSessionAsyncReplyHandler(ajn::Message& message, void* context);
    /**
     * Override Rank
     */
    void OverrideRank(uint64_t rank) {
        internalPropertyStore.SetRank(rank);
    }
    /**
     * Get Leader Election Obj
     */
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

    volatile sig_atomic_t updatesAllowed;
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

    volatile sig_atomic_t isRunning;

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
};
/**
 * controller service management class
 */
class ControllerServiceManager {
  public:
    /**
     * ControllerServiceManager constructor
     */
    ControllerServiceManager(
        const std::string& factoryConfigFile,
        const std::string& configFile,
        const std::string& lampGroupFile,
        const std::string& presetFile,
        const std::string& sceneFile,
        const std::string& masterSceneFile) :
        controllerService(factoryConfigFile, configFile, lampGroupFile, presetFile, sceneFile, masterSceneFile) {

    }
    /**
     * ControllerServiceManager constructor
     */
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
    /**
     * ControllerServiceManager destructor
     */
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
     * @return ER_OK if successful, error otherwise
     */
    QStatus Stop(void) {
        return controllerService.Stop();
    }
    /**
     * join thread
     */
    QStatus Join(void) {
        return controllerService.Join();
    }
    /**
     * is running
     */
    bool IsRunning() {
        return controllerService.IsRunning();
    }
    /**
     * Get Controller Service
     */
    ControllerService& GetControllerService(void) { return controllerService; };
    /**
     * Override Rank
     */
    void OverrideRank(uint64_t rank) {
        controllerService.OverrideRank(rank);
    }

  private:
    ControllerService controllerService;
};


}

#endif
