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

#include <algorithm>

#include <alljoyn/about/AnnounceHandler.h>
#include <alljoyn/about/AnnouncementRegistrar.h>
#include <alljoyn/Status.h>
#include <qcc/Debug.h>

#include <ControllerClient.h>

using namespace qcc;
using namespace ajn;

#define QCC_MODULE "CONTROLLER_CLIENT"

namespace lsf {

/**
 * Controller Service Object Path
 */
const std::string ControllerClient::ControllerServiceObjectPath = "/org/allseen/LSF/ControllerService";

/**
 * Controller Service Interface Name
 */
const std::string ControllerClient::ControllerServiceInterfaceName = "org.allseen.LSF.ControllerService";

/**
 * Controller Service Lamp Interface Name
 */
const std::string ControllerClient::ControllerServiceLampInterfaceName = "org.allseen.LSF.ControllerService.Lamp";

/**
 * Controller Service Lamp Group Interface Name
 */
const std::string ControllerClient::ControllerServiceLampGroupInterfaceName = "org.allseen.LSF.ControllerService.LampGroup";

/**
 * Controller Service Saved State Interface Name
 */
const std::string ControllerClient::ControllerServiceSavedStateInterfaceName = "org.allseen.LSF.ControllerService.SavedState";

/**
 * Controller Service Scene Interface Name
 */
const std::string ControllerClient::ControllerServiceSceneInterfaceName = "org.allseen.LSF.ControllerService.Scene";

/**
 * Controller Service Scene Group Interface Name
 */
const std::string ControllerClient::ControllerServiceSceneGroupInterfaceName = "org.allseen.LSF.ControllerService.SceneGroup";

/**
 * Handler class for some standard AllJoyn signals and callbacks
 */
class ControllerClient::ControllerClientBusHandler :
    public SessionListener, public BusAttachment::JoinSessionAsyncCB,
    public services::AnnounceHandler {

  public:
    /**
     * Constructor
     */
    ControllerClientBusHandler(ControllerClient& client) : controllerClient(client) { }

    /**
     * Destructor
     */
    virtual ~ControllerClientBusHandler() { }

    /**
     * Session Lost signal handler
     */
    virtual void SessionLost(SessionId sessionId, SessionLostReason reason);

    /**
     * JoinSession callback handler
     */
    virtual void JoinSessionCB(QStatus status, SessionId sessionId, const SessionOpts& opts, void* context);

    /**
     * Announce signal handler
     */
    virtual void Announce(uint16_t version, uint16_t port, const char* busName, const ObjectDescriptions& objectDescs, const AboutData& aboutData);

  private:

    /**
     * Reference to the Controller Client instance
     */
    ControllerClient& controllerClient;
};


void ControllerClient::ControllerClientBusHandler::JoinSessionCB(QStatus status, SessionId sessionId, const SessionOpts& opts, void* context)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    controllerClient.OnSessionJoined(status, sessionId, context);
}

void ControllerClient::ControllerClientBusHandler::SessionLost(SessionId sessionId, SessionLostReason reason)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    controllerClient.OnSessionLost(sessionId);
}

void ControllerClient::ControllerClientBusHandler::Announce(
    uint16_t version,
    SessionPort port,
    const char* busName,
    const ObjectDescriptions& objectDescs,
    const AboutData& aboutData)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));

    AboutData::const_iterator ait;
    const char* deviceID = NULL;
    const char* deviceName = NULL;

    controllerClient.bus.EnableConcurrentCallbacks();

    ObjectDescriptions::const_iterator oit = objectDescs.find(ControllerServiceObjectPath.c_str());
    if (oit != objectDescs.end()) {
        if (std::find(oit->second.begin(), oit->second.end(), ControllerServiceInterfaceName.c_str()) != oit->second.end()) {
            ait = aboutData.find("DeviceId");
            if (ait == aboutData.end()) {
                QCC_LogError(ER_FAIL, ("%s: DeviceId missing in About Announcement", __FUNCTION__));
                return;
            }
            ait->second.Get("s", &deviceID);

            ait = aboutData.find("DeviceName");
            if (ait == aboutData.end()) {
                QCC_LogError(ER_FAIL, ("%s: DeviceName missing in About Announcement", __FUNCTION__));
                return;
            }
            ait->second.Get("s", &deviceName);

            QCC_DbgPrintf(("%s: Received Announce: busName=%s port=%u deviceID=%s deviceName=%s", __FUNCTION__, busName, port, deviceID, deviceName));
            controllerClient.OnAnnounced(port, busName, deviceID, deviceName);
        }
    }
}

ControllerClient::ControllerClient(
    ajn::BusAttachment& bus,
    ControllerClientCallback& clientCB) :
    bus(bus),
    busHandler(new ControllerClientBusHandler(*this)),
    callback(clientCB),
    isJoining(false),
    proxyObject(NULL),
    controllerServiceManagerPtr(NULL),
    lampManagerPtr(NULL),
    lampGroupManagerPtr(NULL),
    savedStateManagerPtr(NULL),
    sceneManagerPtr(NULL),
    sceneGroupManagerPtr(NULL)
{
    QStatus status = services::AnnouncementRegistrar::RegisterAnnounceHandler(bus, *busHandler);
    QCC_DbgPrintf(("services::AnnouncementRegistrar::RegisterAnnounceHandler: %s\n", QCC_StatusText(status)));
    status = bus.AddMatch("sessionless='t',type='error'");
    QCC_DbgPrintf(("AddMatch: %s\n", QCC_StatusText(status)));
}

ControllerClient::~ControllerClient()
{
    services::AnnouncementRegistrar::UnRegisterAnnounceHandler(bus, *busHandler);
    bus.RemoveMatch("sessionless='t',type='error'");
    delete busHandler;
}

uint32_t ControllerClient::GetVersion(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    return CONTROLLER_CLIENT_VERSION;
}

/*
   void ControllerClient::JoinNextAvailableController()
   {
    // NOTE: the mutex must be locked when this is called!
    if (!activeServices.empty()) {
        ActiveServiceMap::reverse_iterator rit = activeServices.rbegin();
        //QCC_DbgPrintf(("Connecting to %s\n", controllerServiceID.c_str()));

        SessionOpts opts;
        opts.isMultipoint = true;
        ActiveService& svc = rit->second;

        ActiveService* service = new ActiveService(svc);
        isJoining = true;
        bus.JoinSessionAsync(svc.busName.c_str(), svc.port, busHandler, opts, busHandler, service);
    }
   }
 */

void ControllerClient::OnSessionLost(ajn::SessionId sessionID)
{
    QCC_DbgPrintf(("OnSessionLost(%u)\n", sessionID));

    lock.Lock();

    // we don't care about this session any more
    SessionMap::iterator sit = activeSessions.find(sessionID);
    assert(sit != activeSessions.end());
    std::string id = sit->second;
    activeSessions.erase(sit);

    // this is no longer active unless it is Announced again
    ActiveServiceMap::iterator ait = activeServices.find(id);
    assert(ait != activeServices.end());
    activeServices.erase(ait);

    delete proxyObject;
    lock.Unlock();

    callback.DisconnectedFromControllerServiceCB(deviceID, deviceName);
/*
    // check if there is another controller ID to try
    if (!activeServices.empty()) {
        JoinNextAvailableController();
        lock.Unlock();
    } else {
        controllerServiceID = "";
        lock.Unlock();
        callback.DisconnectedFromControllerServiceCB(deviceID, deviceName);
    }*/
}

void ControllerClient::SignalWithArgDispatcher(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& message)
{
    bus.EnableConcurrentCallbacks();

    QCC_DbgPrintf(("%s: Received Signal %s", __FUNCTION__, message->GetMemberName()));

    SignalDispatcherMap::iterator it = signalHandlers.find(message->GetMemberName());
    if (it != signalHandlers.end()) {
        SignalHandlerBase* handler = it->second;

        size_t numInputArgs;
        const MsgArg* inputArgs;
        message->GetArgs(numInputArgs, inputArgs);

        LSF_ID_List idList;

        MsgArg* idArgs;
        size_t numIds;
        inputArgs[0].Get("as", &numIds, &idArgs);

        for (size_t i = 0; i < numIds; ++i) {
            char* tempId;
            idArgs[i].Get("s", &tempId);
            idList.push_back(LSF_ID(tempId));
        }

        handler->Handle(idList);
    }
}

void ControllerClient::SignalWithoutArgDispatcher(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& message)
{
    bus.EnableConcurrentCallbacks();

    QCC_DbgPrintf(("%s: Received Signal %s", __FUNCTION__, message->GetMemberName()));

    NoArgSignalDispatcherMap::iterator it = noArgSignalHandlers.find(message->GetMemberName());
    if (it != noArgSignalHandlers.end()) {
        NoArgSignalHandlerBase* handler = it->second;
        handler->Handle();
    }
}

void ControllerClient::OnSessionJoined(QStatus status, ajn::SessionId sessionId, void* context)
{
    ErrorCodeList errorList;

    bus.EnableConcurrentCallbacks();

    ActiveService* service = static_cast<ActiveService*>(context);

    QCC_DbgPrintf(("%s: sessionId= %u status=%s\n", __FUNCTION__, sessionId, QCC_StatusText(status)));

    if (status == ER_OK) {
        lock.Lock();
        isJoining = false;

        //ActiveService& svc = activeServices[service];
        proxyObject = new ProxyBusObject(bus, service->busName.c_str(), ControllerServiceObjectPath.c_str(), sessionId);
        // map the session id back to the service id
        activeSessions[sessionId] = service->deviceID;

        deviceID = service->deviceID;
        deviceName = service->deviceName;
        delete service;

        proxyObject->IntrospectRemoteObject();

        if (controllerServiceManagerPtr) {
            AddNoArgSignalHandler("ControllerServiceLightingReset", controllerServiceManagerPtr, &ControllerServiceManager::ControllerServiceLightingReset);

            AddMethodReplyWithUint32ValueHandler("GetControllerServiceVersion", controllerServiceManagerPtr, &ControllerServiceManager::GetControllerServiceVersionReply);
            AddMethodReplyWithUint32ValueHandler("LightingResetControllerService", controllerServiceManagerPtr, &ControllerServiceManager::LightingResetControllerServiceReply);
        }

        if (lampManagerPtr) {
            AddSignalHandler("LampsNameChanged", lampManagerPtr, &LampManager::LampsNameChanged);
            AddSignalHandler("LampsStateChanged", lampManagerPtr, &LampManager::LampsStateChanged);

            AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllLampIDs", lampManagerPtr, &LampManager::GetAllLampIDsReply);

            AddMethodReplyWithResponseCodeIDAndNameHandler("GetLampName", lampManagerPtr, &LampManager::GetLampNameReply);
            AddMethodReplyWithResponseCodeIDAndNameHandler("TransitionLampStateField", lampManagerPtr, &LampManager::TransitionLampStateFieldReply);
            AddMethodReplyWithResponseCodeIDAndNameHandler("ResetLampStateField", lampManagerPtr, &LampManager::ResetLampStateFieldReply);

            AddMethodReplyWithResponseCodeAndIDHandler("SetLampName", lampManagerPtr, &LampManager::SetLampNameReply);
            AddMethodReplyWithResponseCodeAndIDHandler("ResetLampState", lampManagerPtr, &LampManager::ResetLampStateReply);
            AddMethodReplyWithResponseCodeAndIDHandler("TransitionLampState", lampManagerPtr, &LampManager::TransitionLampStateReply);
            AddMethodReplyWithResponseCodeAndIDHandler("TransitionLampStateToSavedState", lampManagerPtr, &LampManager::TransitionLampStateToSavedStateReply);
        }

        if (lampGroupManagerPtr) {
            AddSignalHandler("LampGroupsNameChanged", lampGroupManagerPtr, &LampGroupManager::LampGroupsNameChanged);
            AddSignalHandler("LampGroupsCreated", lampGroupManagerPtr, &LampGroupManager::LampGroupsCreated);
            AddSignalHandler("LampGroupsUpdated", lampGroupManagerPtr, &LampGroupManager::LampGroupsUpdated);
            AddSignalHandler("LampGroupsDeleted", lampGroupManagerPtr, &LampGroupManager::LampGroupsDeleted);

            AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllLampGroupIDs", lampGroupManagerPtr, &LampGroupManager::GetAllLampGroupIDsReply);

            AddMethodReplyWithResponseCodeIDAndNameHandler("GetLampGroupName", lampGroupManagerPtr, &LampGroupManager::GetLampGroupNameReply);
            AddMethodReplyWithResponseCodeIDAndNameHandler("TransitionLampGroupStateField", lampGroupManagerPtr, &LampGroupManager::TransitionLampGroupStateFieldReply);
            AddMethodReplyWithResponseCodeIDAndNameHandler("ResetLampGroupFieldState", lampGroupManagerPtr, &LampGroupManager::ResetLampGroupFieldStateReply);

            AddMethodReplyWithResponseCodeAndIDHandler("SetLampGroupName", lampGroupManagerPtr, &LampGroupManager::SetLampGroupNameReply);
            AddMethodReplyWithResponseCodeAndIDHandler("ResetLampGroupState", lampGroupManagerPtr, &LampGroupManager::ResetLampGroupStateReply);
            AddMethodReplyWithResponseCodeAndIDHandler("TransitionLampGroupState", lampGroupManagerPtr, &LampGroupManager::TransitionLampGroupStateReply);
            AddMethodReplyWithResponseCodeAndIDHandler("TransitionLampGroupStateToSavedState", lampGroupManagerPtr, &LampGroupManager::TransitionLampGroupStateToSavedStateReply);
            AddMethodReplyWithResponseCodeAndIDHandler("CreateLampGroup", lampGroupManagerPtr, &LampGroupManager::CreateLampGroupReply);
            AddMethodReplyWithResponseCodeAndIDHandler("UpdateLampGroup", lampGroupManagerPtr, &LampGroupManager::UpdateLampGroupReply);
            AddMethodReplyWithResponseCodeAndIDHandler("DeleteLampGroup", lampGroupManagerPtr, &LampGroupManager::DeleteLampGroupReply);
        }

        if (savedStateManagerPtr) {
            AddNoArgSignalHandler("DefaultLampStateChanged", savedStateManagerPtr, &SavedStateManager::DefaultLampStateChanged);
            AddSignalHandler("SavedStatesNameChanged", savedStateManagerPtr, &SavedStateManager::SavedStatesNameChanged);
            AddSignalHandler("SavedStatesCreated", savedStateManagerPtr, &SavedStateManager::SavedStatesCreated);
            AddSignalHandler("SavedStatesUpdated", savedStateManagerPtr, &SavedStateManager::SavedStatesUpdated);
            AddSignalHandler("SavedStatesDeleted", savedStateManagerPtr, &SavedStateManager::SavedStatesDeleted);

            AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllSavedStateIDs", savedStateManagerPtr, &SavedStateManager::GetAllSavedStateIDsReply);

            AddMethodReplyWithResponseCodeIDAndNameHandler("GetSavedStateName", savedStateManagerPtr, &SavedStateManager::GetSavedStateNameReply);

            AddMethodReplyWithResponseCodeAndIDHandler("SetSavedStateName", savedStateManagerPtr, &SavedStateManager::SetSavedStateNameReply);
            AddMethodReplyWithResponseCodeAndIDHandler("CreateSavedState", savedStateManagerPtr, &SavedStateManager::CreateSavedStateReply);
            AddMethodReplyWithResponseCodeAndIDHandler("UpdateSavedState", savedStateManagerPtr, &SavedStateManager::UpdateSavedStateReply);
            AddMethodReplyWithResponseCodeAndIDHandler("DeleteSavedState", savedStateManagerPtr, &SavedStateManager::DeleteSavedStateReply);

            AddMethodReplyWithUint32ValueHandler("SetDefaultLampState", savedStateManagerPtr, &SavedStateManager::SetDefaultLampStateReply);
        }

        if (sceneManagerPtr) {
            AddSignalHandler("ScenesNameChanged", sceneManagerPtr, &SceneManager::ScenesNameChanged);
            AddSignalHandler("ScenesCreated", sceneManagerPtr, &SceneManager::ScenesCreated);
            AddSignalHandler("ScenesUpdated", sceneManagerPtr, &SceneManager::ScenesUpdated);
            AddSignalHandler("ScenesDeleted", sceneManagerPtr, &SceneManager::ScenesDeleted);
            AddSignalHandler("ScenesApplied", sceneManagerPtr, &SceneManager::ScenesApplied);

            AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllSceneIDs", sceneManagerPtr, &SceneManager::GetAllSceneIDsReply);

            AddMethodReplyWithResponseCodeIDAndNameHandler("GetSceneName", sceneManagerPtr, &SceneManager::GetSceneNameReply);

            AddMethodReplyWithResponseCodeAndIDHandler("SetSceneName", sceneManagerPtr, &SceneManager::SetSceneNameReply);
            AddMethodReplyWithResponseCodeAndIDHandler("CreateScene", sceneManagerPtr, &SceneManager::CreateSceneReply);
            AddMethodReplyWithResponseCodeAndIDHandler("UpdateScene", sceneManagerPtr, &SceneManager::UpdateSceneReply);
            AddMethodReplyWithResponseCodeAndIDHandler("DeleteScene", sceneManagerPtr, &SceneManager::DeleteSceneReply);
            AddMethodReplyWithResponseCodeAndIDHandler("ApplyScene", sceneManagerPtr, &SceneManager::ApplySceneReply);
        }

        if (sceneGroupManagerPtr) {
            AddSignalHandler("SceneGroupsNameChanged", sceneGroupManagerPtr, &SceneGroupManager::SceneGroupsNameChanged);
            AddSignalHandler("SceneGroupsCreated", sceneGroupManagerPtr, &SceneGroupManager::SceneGroupsCreated);
            AddSignalHandler("SceneGroupsUpdated", sceneGroupManagerPtr, &SceneGroupManager::SceneGroupsUpdated);
            AddSignalHandler("SceneGroupsDeleted", sceneGroupManagerPtr, &SceneGroupManager::SceneGroupsDeleted);
            AddSignalHandler("SceneGroupsApplied", sceneGroupManagerPtr, &SceneGroupManager::SceneGroupsApplied);

            AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllSceneGroupIDs", sceneGroupManagerPtr, &SceneGroupManager::GetAllSceneGroupIDsReply);

            AddMethodReplyWithResponseCodeIDAndNameHandler("GetSceneGroupName", sceneGroupManagerPtr, &SceneGroupManager::GetSceneGroupNameReply);

            AddMethodReplyWithResponseCodeAndIDHandler("SetSceneGroupName", sceneGroupManagerPtr, &SceneGroupManager::SetSceneGroupNameReply);
            AddMethodReplyWithResponseCodeAndIDHandler("CreateSceneGroup", sceneGroupManagerPtr, &SceneGroupManager::CreateSceneGroupReply);
            AddMethodReplyWithResponseCodeAndIDHandler("UpdateSceneGroup", sceneGroupManagerPtr, &SceneGroupManager::UpdateSceneGroupReply);
            AddMethodReplyWithResponseCodeAndIDHandler("DeleteSceneGroup", sceneGroupManagerPtr, &SceneGroupManager::DeleteSceneGroupReply);
            AddMethodReplyWithResponseCodeAndIDHandler("ApplySceneGroup", sceneGroupManagerPtr, &SceneGroupManager::ApplySceneGroupReply);
        }

        const InterfaceDescription* controllerServiceInterface = proxyObject->GetInterface(ControllerServiceInterfaceName.c_str());
        const InterfaceDescription* controllerServiceLampInterface = proxyObject->GetInterface(ControllerServiceLampInterfaceName.c_str());
        const InterfaceDescription* controllerServiceLampGroupInterface = proxyObject->GetInterface(ControllerServiceLampGroupInterfaceName.c_str());
        const InterfaceDescription* controllerServiceSavedStateInterface = proxyObject->GetInterface(ControllerServiceSavedStateInterfaceName.c_str());
        const InterfaceDescription* controllerServiceSceneInterface = proxyObject->GetInterface(ControllerServiceSceneInterfaceName.c_str());
        const InterfaceDescription* controllerServiceSceneGroupInterface = proxyObject->GetInterface(ControllerServiceSceneGroupInterfaceName.c_str());

        const SignalEntry signalEntries[] = {
            { controllerServiceInterface->GetMember("ControllerServiceLightingReset"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithoutArgDispatcher) },
            { controllerServiceLampInterface->GetMember("LampsNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceLampInterface->GetMember("LampsStateChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceLampGroupInterface->GetMember("LampGroupsNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceLampGroupInterface->GetMember("LampGroupsCreated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceLampGroupInterface->GetMember("LampGroupsUpdated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceLampGroupInterface->GetMember("LampGroupsDeleted"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSavedStateInterface->GetMember("DefaultLampStateChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithoutArgDispatcher) },
            { controllerServiceSavedStateInterface->GetMember("SavedStatesNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSavedStateInterface->GetMember("SavedStatesCreated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSavedStateInterface->GetMember("SavedStatesUpdated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSavedStateInterface->GetMember("SavedStatesDeleted"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSceneInterface->GetMember("ScenesNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSceneInterface->GetMember("ScenesCreated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSceneInterface->GetMember("ScenesUpdated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSceneInterface->GetMember("ScenesDeleted"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSceneInterface->GetMember("ScenesApplied"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSceneGroupInterface->GetMember("SceneGroupsNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSceneGroupInterface->GetMember("SceneGroupsCreated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSceneGroupInterface->GetMember("SceneGroupsUpdated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSceneGroupInterface->GetMember("SceneGroupsDeleted"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
            { controllerServiceSceneGroupInterface->GetMember("SceneGroupsApplied"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) }
        };

        ErrorCode errorCode = ERROR_NONE;

        for (size_t i = 0; i < (sizeof(signalEntries) / sizeof(SignalEntry)); ++i) {
            status = bus.RegisterSignalHandler(
                this,
                signalEntries[i].handler,
                signalEntries[i].member,
                ControllerClient::ControllerServiceObjectPath.c_str());

            if (status != ER_OK) {
                QCC_LogError(status, ("RegisterSignalHandler failed for %s", signalEntries[i].member->name.c_str()));

                if (errorCode != ERROR_REGISTERING_SIGNAL_HANDLERS) {
                    errorCode = ERROR_REGISTERING_SIGNAL_HANDLERS;
                    errorList.push_back(errorCode);
                }
            }
        }

        lock.Unlock();
        callback.ConnectedToControllerServiceCB(deviceID, deviceName);
/*
        ActiveServiceMap::reverse_iterator rit = activeServices.rbegin();
        if (rit->first > controllerServiceID) {
            lock.Unlock();
            bus.LeaveSession(sessionId);
            lock.Lock();

            // now join the session!
            JoinNextAvailableController();
        }
 */
        lock.Unlock();
    } else {
        isJoining = false;
        lock.Lock();

        callback.ConnectToControllerServiceFailedCB(service->deviceID, service->deviceName);
        delete service;

        // failed to join controllerServiceID!
        //ActiveServiceMap::iterator ait = activeServices.find(controllerServiceID);
        //assert(ait != activeServices.end());
        //activeServices.erase(ait);
/*
        if (!activeServices.empty()) {
            JoinNextAvailableController();
            lock.Unlock();
        } else {
            lock.Unlock();
            errorList.push_back(ERROR_NO_ACTIVE_CONTROLLER_SERVICE_FOUND);
        }*/
    }

    if (!errorList.empty()) {
        callback.ControllerClientErrorCB(errorList);
    }
}

void ControllerClient::OnAnnounced(SessionPort port, const char* busName, const char* deviceID, const char* deviceName)
{
    QCC_DbgPrintf(("%s: port=%u, busName=%s, deviceID=%s, deviceName=%s\n", __FUNCTION__, port, busName, deviceID, deviceName));

    ActiveService* svc = new ActiveService();
    svc->busName = busName;
    svc->port = port;
    svc->deviceID = deviceID;
    svc->deviceName = deviceName;

    lock.Lock();

    // we already know about this one
    if (activeServices.find(deviceID) != activeServices.end()) {
        lock.Unlock();
        return;
    }


    activeServices[deviceID] = *svc;

    ErrorCodeList errorList;

    if (!isJoining) {
        SessionOpts opts;
        opts.isMultipoint = true;
        isJoining = true;
        QStatus status = bus.JoinSessionAsync(busName, port, busHandler, opts, busHandler, svc);
        if (status != ER_OK) {
            isJoining = false;
            activeServices.erase(deviceID);
            errorList.push_back(ERROR_NO_ACTIVE_CONTROLLER_SERVICE_FOUND);
            delete svc;
        }
    }

    /*
     * activeSessions.empty() && !controllerServiceID.empty()
     * implies that a session is currently being established
     */
/*
    if (activeSessions.empty()) {
        // no session currently active; we must establish one!


        // no session currently being established; change that!
        if (!isJoining) {
            // make a copy in case the activeService is removed from
            // the map before the join succeeds
            ActiveService* service = new ActiveService(svc);

            SessionOpts opts;
            opts.isMultipoint = true;
            isJoining = true;
            bus.JoinSessionAsync(busName, port, busHandler, opts, busHandler, service);
        }
    } else {
        // we are currently in an active session

        // activeServices can't be empty here!
        ActiveServiceMap::reverse_iterator it = activeServices.rbegin();

        if (deviceID > it->first) {
            // need to leave *all* active sessions (there should only be one!
            for (SessionMap::const_iterator it = activeSessions.begin(); it != activeSessions.end(); ++it) {
                lock.Unlock();
                bus.LeaveSession(it->first);
                lock.Lock();

                JoinNextAvailableController();
            }
        }

    }
 */
    lock.Unlock();

    if (!errorList.empty()) {
        callback.ControllerClientErrorCB(errorList);
    }
}

ControllerClientStatus ControllerClient::MethodCallAsyncHelper(
    const char* ifaceName,
    const char* methodName,
    ajn::MessageReceiver* handler,
    ajn::MessageReceiver::ReplyHandler callback,
    const ajn::MsgArg* args,
    size_t numArgs,
    void* context)
{
    ControllerClientStatus status = CONTROLLER_CLIENT_OK;

    lock.Lock();
    //ActiveServiceMap::iterator it = activeServices.find(controllerServiceID);

    if (proxyObject != NULL) {
        QStatus ajStatus = proxyObject->MethodCallAsync(
            ifaceName,
            methodName,
            handler,
            callback,
            args,
            numArgs,
            context);
        if (ajStatus != ER_OK) {
            status = CONTROLLER_CLIENT_ERR_FAILURE;
            QCC_LogError(ajStatus, ("%s method call to Controller Service failed", methodName));
        }
    } else {
        // this is no longer available
        status = CONTROLLER_CLIENT_ERR_NOT_CONNECTED;
    }

    lock.Unlock();
    return status;
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithResponseCodeAndListOfIDs(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Call=%s", __FUNCTION__, methodName));
    LSF_Name* methodNameContext = new LSF_Name(methodName);
    ControllerClientStatus status = MethodCallAsyncHelper(
        ifaceName,
        methodName,
        this,
        static_cast<ajn::MessageReceiver::ReplyHandler>(&ControllerClient::HandlerForMethodReplyWithResponseCodeAndListOfIDs),
        args,
        numArgs,
        (void*)methodNameContext);
    if (status != CONTROLLER_CLIENT_OK) {
        delete methodNameContext;
    }
    return status;
}

void ControllerClient::HandlerForMethodReplyWithResponseCodeAndListOfIDs(Message& message, void* context)
{
    if (context) {
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __FUNCTION__, ((LSF_Name*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

        bus.EnableConcurrentCallbacks();

        MethodReplyWithResponseCodeAndListOfIDsDispatcherMap::iterator it = methodReplyWithResponseCodeAndListOfIDsHandlers.find(*((LSF_Name*)context));
        if (it != methodReplyWithResponseCodeAndListOfIDsHandlers.end()) {
            MethodReplyWithResponseCodeAndListOfIDsHandlerBase* handler = it->second;

            size_t numInputArgs;
            const MsgArg* inputArgs;
            message->GetArgs(numInputArgs, inputArgs);

            LSF_ID_List idList;
            LSFResponseCode responseCode;

            inputArgs[0].Get("u", &responseCode);

            MsgArg* idArgs;
            size_t numIds;
            inputArgs[1].Get("as", &numIds, &idArgs);

            for (size_t i = 0; i < numIds; ++i) {
                char* tempId;
                idArgs[i].Get("s", &tempId);
                idList.push_back(LSF_ID(tempId));
            }

            handler->Handle(responseCode, idList);
        }
        delete ((LSF_Name*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __FUNCTION__));
    }
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithResponseCodeIDAndName(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Call=%s", __FUNCTION__, methodName));
    LSF_Name* methodNameContext = new LSF_Name(methodName);
    ControllerClientStatus status = MethodCallAsyncHelper(
        ifaceName,
        methodName,
        this,
        static_cast<ajn::MessageReceiver::ReplyHandler>(&ControllerClient::HandlerForMethodReplyWithResponseCodeIDAndName),
        args,
        numArgs,
        (void*)methodNameContext);
    if (status != CONTROLLER_CLIENT_OK) {
        delete methodNameContext;
    }
    return status;
}

void ControllerClient::HandlerForMethodReplyWithResponseCodeIDAndName(Message& message, void* context)
{
    if (context) {
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __FUNCTION__, ((LSF_Name*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

        bus.EnableConcurrentCallbacks();

        MethodReplyWithResponseCodeIDAndNameDispatcherMap::iterator it = methodReplyWithResponseCodeIDAndNameHandlers.find(*((LSF_Name*)context));
        if (it != methodReplyWithResponseCodeIDAndNameHandlers.end()) {
            MethodReplyWithResponseCodeIDAndNameHandlerBase* handler = it->second;

            size_t numInputArgs;
            const MsgArg* inputArgs;
            message->GetArgs(numInputArgs, inputArgs);

            char* id;
            char* name;
            LSFResponseCode responseCode;

            inputArgs[0].Get("u", &responseCode);
            inputArgs[1].Get("s", &id);
            inputArgs[2].Get("s", &name);

            LSF_ID lsfId = LSF_ID(id);
            LSF_Name lsfName = LSF_Name(name);

            handler->Handle(responseCode, lsfId, lsfName);
        }
        delete ((LSF_Name*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __FUNCTION__));
    }
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithResponseCodeAndID(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Call=%s", __FUNCTION__, methodName));
    LSF_Name* methodNameContext = new LSF_Name(methodName);
    ControllerClientStatus status = MethodCallAsyncHelper(
        ifaceName,
        methodName,
        this,
        static_cast<ajn::MessageReceiver::ReplyHandler>(&ControllerClient::HandlerForMethodReplyWithResponseCodeAndID),
        args,
        numArgs,
        (void*)methodNameContext);
    if (status != CONTROLLER_CLIENT_OK) {
        delete methodNameContext;
    }
    return status;
}

void ControllerClient::HandlerForMethodReplyWithResponseCodeAndID(Message& message, void* context)
{
    if (context) {
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __FUNCTION__, ((LSF_Name*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

        bus.EnableConcurrentCallbacks();

        MethodReplyWithResponseCodeAndIDDispatcherMap::iterator it = methodReplyWithResponseCodeAndIDHandlers.find(*((LSF_Name*)context));
        if (it != methodReplyWithResponseCodeAndIDHandlers.end()) {
            MethodReplyWithResponseCodeAndIDHandlerBase* handler = it->second;

            size_t numInputArgs;
            const MsgArg* inputArgs;
            message->GetArgs(numInputArgs, inputArgs);

            char* id;
            LSFResponseCode responseCode;

            inputArgs[0].Get("u", &responseCode);
            inputArgs[1].Get("s", &id);

            LSF_ID lsfId = LSF_ID(id);

            handler->Handle(responseCode, lsfId);
        }
        delete ((LSF_Name*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __FUNCTION__));
    }
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithUint32Value(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Call=%s", __FUNCTION__, methodName));
    LSF_Name* methodNameContext = new LSF_Name(methodName);
    ControllerClientStatus status = MethodCallAsyncHelper(
        ifaceName,
        methodName,
        this,
        static_cast<ajn::MessageReceiver::ReplyHandler>(&ControllerClient::HandlerForMethodReplyWithUint32Value),
        args,
        numArgs,
        (void*)methodNameContext);
    if (status != CONTROLLER_CLIENT_OK) {
        delete methodNameContext;
    }
    return status;
}

void ControllerClient::HandlerForMethodReplyWithUint32Value(Message& message, void* context)
{
    if (context) {
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __FUNCTION__, ((LSF_Name*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

        bus.EnableConcurrentCallbacks();

        MethodReplyWithUint32ValueDispatcherMap::iterator it = methodReplyWithUint32ValueHandlers.find(*((LSF_Name*)context));
        if (it != methodReplyWithUint32ValueHandlers.end()) {
            MethodReplyWithUint32ValueHandlerBase* handler = it->second;

            size_t numInputArgs;
            const MsgArg* inputArgs;
            message->GetArgs(numInputArgs, inputArgs);

            uint32_t value;
            inputArgs[0].Get("u", &value);
            handler->Handle(value);
        } else {
            QCC_LogError(ER_FAIL, ("%s: Did not find handler", __FUNCTION__));
        }
        delete ((LSF_Name*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __FUNCTION__));
    }
}

void ControllerClient::Reset(void) {

}

}
