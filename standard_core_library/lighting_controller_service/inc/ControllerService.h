#ifndef _CONTROLLER_SERVICE_H_
#define _CONTROLLER_SERVICE_H_
/**
 * \ingroup ControllerService
 */
/**
 * @file
 * This file provides definitions for ControllerService
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
#include <ControllerServiceRank.h>

namespace lsf {

/**
 * This class functions as the message dispatcher. It receives the messages from AllJoyn \n
 * and forwards it to the appropriate manager and receives a reply from a manager and \n
 * passes it on to AllJoyn
 */
class ControllerService : public ajn::BusObject, public ajn::services::ConfigService::Listener {
    friend class ControllerServiceManager;
    friend class LampClients;
    friend class LeaderElectionObject;
  public:

    /**
     * Constructor
     * @param factoryConfigFile - path of factory config file
     * @param configFile - path of config file
     * @param lampGroupFile - path of lamp group file
     * @param presetFile - path of pre-set file
     * @param sceneFile - path of scene file
     * @param masterSceneFile - path of master scene file
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
     * @param propStore - path of property store
     * @param factoryConfigFile - path of factory config file
     * @param configFile - path of config file
     * @param lampGroupFile - path of lamp group file
     * @param presetFile - path of pre-set file
     * @param sceneFile - path of scene file
     * @param masterSceneFile - path of master scene file
     */
    ControllerService(
        LSFPropertyStore& propStore,
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
     * @param  keyStoreFileLocation Absolute path of the location to put the AllJoyn keystore file in. If this is not specified, the
     *                              default location will be used. Android applications running the Controller Service should pass in the
     *                              location returned by Context.getFileStreamPath("alljoyn_keystore").getAbsolutePath()
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
     * Get reference to Lamp Manager object
     * @return LampManager
     */
    LampManager& GetLampManager(void) { return lampManager; };
    /**
     * Get reference to Lamp Group Manager object
     * @return LampGroupManager
     */
    LampGroupManager& GetLampGroupManager(void) { return lampGroupManager; };
    /**
     * Get reference to Preset Manager object
     * @return PresetManager
     */
    PresetManager& GetPresetManager(void) { return presetManager; };
    /**
     * Get reference to Scene Manager object
     * @return SceneManager
     */
    SceneManager& GetSceneManager(void) { return sceneManager; };
    /**
     * Get reference to Master Scene Manager
     * @return MasterSceneManager
     */
    MasterSceneManager& GetMasterSceneManager(void) { return masterSceneManager; };
    /**
     * Send Method Reply \n
     * Reply for asynchronous method call \n
     * @param msg      The method call message
     * @param args     The reply arguments (can be NULL)
     * @param numArgs  The number of arguments
     * @return
     *      - ER_OK if successful
     *      - ER_BUS_OBJECT_NOT_REGISTERED if bus object has not yet been registered
     *      - An error status otherwise
     */
    void SendMethodReply(const ajn::Message& msg, const ajn::MsgArg* args = NULL, size_t numArgs = 0);
    /**
     * Send Method Reply With Response Code And List Of IDs \n
     * Reply for asynchronous method call that needs LSFResponseCode and string of IDs of some list \n
     * @param msg      The method call message
     * @param responseCode type LSFResponseCode
     * @param idList - string of IDs
     */
    void SendMethodReplyWithResponseCodeAndListOfIDs(const ajn::Message& msg, LSFResponseCode responseCode, const LSFStringList& idList);
    /**
     * Send Method Reply With Response Code ID And Name \n
     * Reply for asynchronous method call that needs LSFResponseCode and ID and name \n
     * @param msg      The method call message
     * @param responseCode type LSFResponseCode
     * @param lsfId - id as a string
     * @param lsfName - name
     */
    void SendMethodReplyWithResponseCodeIDAndName(const ajn::Message& msg, LSFResponseCode responseCode, const LSFString& lsfId, const LSFString& lsfName);
    /**
     * Send Method Reply With Response Code And ID \n
     * Reply for asynchronous method call that needs LSFResponseCode and ID \n
     * @param msg      The method call message
     * @param responseCode type LSFResponseCode
     * @param lsfId - id as a string
     */
    void SendMethodReplyWithResponseCodeAndID(const ajn::Message& msg, LSFResponseCode responseCode, const LSFString& lsfId);
    /**
     * Send Method Reply With Uint32 Value \n
     * Reply for asynchronous method call that needs uint32_t \n
     * @param msg      The method call message
     * @param value    The uint32_t value need to be sent
     */
    void SendMethodReplyWithUint32Value(const ajn::Message& msg, uint32_t value);
    /**
     * Send Method Reply With Response Code ID Language And Name \n
     * Reply for asynchronous method call that needs LSFResponseCode, ID, language and name \n
     * @param msg      The method call message
     * @param responseCode type LSFResponseCode
     * @param lsfId - id as a string
     * @param language of the name to be sent
     * @param name to send
     */
    void SendMethodReplyWithResponseCodeIDLanguageAndName(const ajn::Message& msg, LSFResponseCode responseCode, const LSFString& lsfId, const LSFString& language, const LSFString& name);
    /**
     * Send Signal with list of IDs
     * @param ifaceName - interface that the signal is located
     * @param methodName - signal method name
     * @param idList - The list of IDs needed to be sent
     * @return QStatus
     */
    QStatus SendSignal(const char* ifaceName, const char* signalName, const LSFStringList& idList);

    /**
     * Send Name Changed signal
     * @param ifaceName  - interface that the signal is located
     * @param methodName - signal method name
     * @param lampId     - The Lamp ID
     * @param lampName   - The Lamp Name
     * @return QStatus
     */
    QStatus SendNameChangedSignal(const char* ifaceName, const char* signalName, const LSFString& lampID, const LSFString& lampName);

    /**
     * Send State Changed signal
     * @param ifaceName  - interface that the signal is located
     * @param methodName - signal method name
     * @param lampId     - The Lamp ID
     * @param lampState  - The Lamp State
     * @return QStatus
     */
    QStatus SendStateChangedSignal(const char* ifaceName, const char* signalName, const LSFString& lampID, const LampState& lampState);

    /**
     * Send Signal Without Arg - just an empty signal
     * @param ifaceName - interface that the signal is located
     * @param signalName - signal method name
     * @return QStatus
     */
    QStatus SendSignalWithoutArg(const char* ifaceName, const char* signalName);
    /**
     * Send Scene Or Master Scene Applied Signal \n
     * Sends signal for event - ScenesApplied signal or MasterScenesApplied signal \n
     * @param sceneorMasterSceneId - Scene, MasterScene
     */
    void SendSceneOrMasterSceneAppliedSignal(LSFString& sceneorMasterSceneId) {
        sceneManager.SendSceneOrMasterSceneAppliedSignal(sceneorMasterSceneId);
    }
    /**
     * Schedule File Read Write \n
     * a trigger to synchronize all lighting service meta data with persistent storage.\n
     * Meta data includes lamp groups, scenes, master scenes and pre-sets.
     * @param manager - parameter not in use
     */
    void ScheduleFileReadWrite(Manager* manager);
    /**
     * Send Blob Update \n
     * Updating the leader controller service about the current controller service meta data \n
     * @param type - which kind of meta data is this
     * @param blob - the information to update
     * @param checksum
     * @param timestamp
     */
    QStatus SendBlobUpdate(LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp);
    /**
     * Send Get Blob Reply \n
     * Replay to Get blob request \n
     * @param message - the request message
     * @param type - the type of the requested blob
     * @param blob - the requested meta data information
     * @param checksum
     * @param timestamp
     */
    void SendGetBlobReply(ajn::Message& message, LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp);
    /**
     * Is Running
     */
    bool IsRunning();

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
     * Get Leader Election Obj
     */
    LeaderElectionObject& GetLeaderElectionObj(void) {
        return elector;
    }

    /**
     * Check if the receivedNumArgs is same as expectedNumArgs and return a response
     * code accordingly
     */
    LSFResponseCode CheckNumArgsInMessage(uint32_t receivedNumArgs, uint32_t expectedNumArgs);

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

    Mutex updatesAllowedLock;
    bool updatesAllowed;
    ajn::BusAttachment bus;
    LeaderElectionObject elector;
    lsf::Mutex serviceSessionMutex;
    ajn::SessionId serviceSession;
    qcc::String multipointjoiner;

    class ControllerListener;
    ControllerListener* listener;

    class OBSJoiner;

    LampManager lampManager;
    LampGroupManager lampGroupManager;
    PresetManager presetManager;
    SceneManager sceneManager;
    MasterSceneManager masterSceneManager;

    void OnAccepMultipointSessionJoiner(const char* joiner);
    void SessionLost(ajn::SessionId sessionId);
    void SessionJoined(ajn::SessionId sessionId, const char* joiner);
    void LeaveSession(void);

    void FoundLocalOnboardingService(const char* busName, ajn::SessionPort port);

    LSFPropertyStore internalPropertyStore;
    LSFPropertyStore& propertyStore;
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

    ControllerServiceRank rank;
};
/**
 * controller service management class. \n
 * This is the class to create from the outside application that run the controller service.
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
     * Constructor
     * @param propStore - path of property store
     * @param factoryConfigFile - path of factory config file
     * @param configFile - path of config file
     * @param lampGroupFile - path of lamp group file
     * @param presetFile - path of pre-set file
     * @param sceneFile - path of scene file
     * @param masterSceneFile - path of master scene file
     */
    ControllerServiceManager(
        LSFPropertyStore& propStore,
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

  private:
    ControllerService controllerService;
};


}

#endif
