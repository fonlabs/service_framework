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

    virtual void SessionMemberRemoved(SessionId sessionId, const char* uniqueName);

  private:

    /**
     * Reference to the Controller Client instance
     */
    ControllerClient& controllerClient;
};


void ControllerClient::ControllerClientBusHandler::JoinSessionCB(QStatus status, SessionId sessionId, const SessionOpts& opts, void* context)
{
    controllerClient.bus.EnableConcurrentCallbacks();
    QCC_DbgPrintf(("%s", __func__));
    controllerClient.OnSessionJoined(status, sessionId, context);
}

void ControllerClient::ControllerClientBusHandler::SessionLost(SessionId sessionId, SessionLostReason reason)
{
    controllerClient.bus.EnableConcurrentCallbacks();
    QCC_DbgPrintf(("SessionLost(%u)", sessionId));
    controllerClient.OnSessionLost(sessionId);
}

void ControllerClient::ControllerClientBusHandler::SessionMemberRemoved(SessionId sessionId, const char* uniqueName)
{
    controllerClient.bus.EnableConcurrentCallbacks();
    QCC_DbgPrintf(("SessionMemberRemoved(%u,%s)", sessionId, uniqueName));
    controllerClient.OnSessionMemberRemoved(sessionId, uniqueName);
}

void ControllerClient::ControllerClientBusHandler::Announce(
    uint16_t version,
    SessionPort port,
    const char* busName,
    const ObjectDescriptions& objectDescs,
    const AboutData& aboutData)
{
    QCC_DbgPrintf(("%s", __func__));

    AboutData::const_iterator ait;
    const char* deviceID = NULL;
    const char* deviceName = NULL;

    controllerClient.bus.EnableConcurrentCallbacks();

    ObjectDescriptions::const_iterator oit = objectDescs.find(ControllerServiceObjectPath);
    if (oit != objectDescs.end()) {
        QCC_DbgPrintf(("%s: About Data Dump", __func__));
        for (ait = aboutData.begin(); ait != aboutData.end(); ait++) {
            QCC_DbgPrintf(("%s: %s", ait->first.c_str(), ait->second.ToString().c_str()));
        }

        ait = aboutData.find("DeviceId");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: DeviceId missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("s", &deviceID);

        ait = aboutData.find("DeviceName");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: DeviceName missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("s", &deviceName);

        uint64_t rank;
        ait = aboutData.find("Rank");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: Rank missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("t", &rank);

        uint32_t isLeader = 0;
        ait = aboutData.find("IsLeader");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: IsLeader missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("u", &isLeader);

        QCC_DbgPrintf(("%s: Received Announce: busName=%s port=%u deviceID=%s deviceName=%s rank=%d isLeader=%d", __func__,
                       busName, port, deviceID, deviceName, rank, isLeader));
        controllerClient.OnAnnounced(port, busName, deviceID, deviceName, rank, isLeader);
    }
}

static const char* interfaces[] =
{
    ControllerServiceInterfaceName,
    ControllerServiceLampInterfaceName,
    ControllerServiceLampGroupInterfaceName,
    ControllerServicePresetInterfaceName,
    ControllerServiceSceneInterfaceName,
    ControllerServiceMasterSceneInterfaceName,
    ConfigServiceInterfaceName,
    AboutInterfaceName
};

ControllerClient::ControllerClient(
    ajn::BusAttachment& bus,
    ControllerClientCallback& clientCB) :
    bus(bus),
    busHandler(new ControllerClientBusHandler(*this)),
    callback(clientCB),
    proxyObject(NULL),
    controllerServiceManagerPtr(NULL),
    lampManagerPtr(NULL),
    lampGroupManagerPtr(NULL),
    presetManagerPtr(NULL),
    sceneManagerPtr(NULL),
    masterSceneManagerPtr(NULL),
    alreadyInSession(false)
{
    ClearCurrentLeader();
    QStatus status = services::AnnouncementRegistrar::RegisterAnnounceHandler(bus, *busHandler, interfaces, sizeof(interfaces) / sizeof(interfaces[0]));
    QCC_DbgPrintf(("services::AnnouncementRegistrar::RegisterAnnounceHandler: %s\n", QCC_StatusText(status)));
}

ControllerClient::~ControllerClient()
{
    services::AnnouncementRegistrar::UnRegisterAnnounceHandler(bus, *busHandler, interfaces, sizeof(interfaces) / sizeof(interfaces[0]));
    delete busHandler;
}

uint32_t ControllerClient::GetVersion(void)
{
    QCC_DbgPrintf(("%s", __func__));
    return CONTROLLER_CLIENT_VERSION;
}

void ControllerClient::ClearCurrentLeader()
{
    currentLeader.busName = "";
    currentLeader.rank = 0;
    currentLeader.isLeader = 0;
    currentLeader.port = 0;
    currentLeader.joining = false;
}

ControllerClient::ControllerEntry* ControllerClient::GetMaxRankedEntry()
{
    ControllerEntry* max_rank = NULL;

    QCC_DbgPrintf(("%s", __func__));

    // now decide what to do!
    ControllerEntryMap::iterator it = controllers.begin();
    for (; it != controllers.end(); ++it) {
        ControllerEntry& entry = it->second;
        if (max_rank) {
            if ((max_rank->isLeader && entry.isLeader) && (max_rank->rank < entry.rank)) {
                max_rank = &entry;
            }
        } else {
            max_rank = &entry;
        }
    }

    return max_rank;
}

void ControllerClient::JoinLeaderSession()
{
    QCC_DbgPrintf(("%s", __func__));

    if (alreadyInSession) {
        QCC_DbgPrintf(("%s: Already in a session with a Controller Service", __func__));
        return;
    }

    ErrorCodeList errorList;
    controllersLock.Lock();
    ControllerEntry* max_rank = GetMaxRankedEntry();

    //QCC_DbgPrintf(("Max rank: %lu; currentleader=%lu", (max_rank ? max_rank->rank : 0UL), currentLeader.rank));

    bool change_session = false;
    if (max_rank != NULL && currentLeader.rank < max_rank->rank) {
        change_session = true;
    }

    SessionId serviceSession = proxyObject ? proxyObject->GetSessionId() : 0;
    // need to clean this up
    delete proxyObject;
    proxyObject = NULL;

    controllersLock.Unlock();

    // best to do this after releasing the lock
    if (change_session) {
        if (serviceSession) {
            bus.LeaveSession(serviceSession);
            ClearCurrentLeader();
        }

        // don't try to join the same session twice!
        if (!max_rank->joining) {
            SessionOpts options;
            options.isMultipoint = true;
            QStatus status = bus.JoinSessionAsync(max_rank->busName.c_str(), max_rank->port, busHandler, options, busHandler, max_rank);
            if (status != ER_OK) {
                errorList.push_back(ERROR_NO_ACTIVE_CONTROLLER_SERVICE_FOUND);
                QCC_LogError(status, ("%s: JoinSessionAsync\n", __func__));

                // unable to join
                controllersLock.Lock();
                RemoveUniqueName(max_rank->busName);
                controllersLock.Unlock();
            } else {
                max_rank->joining = true;
            }
        }
        // else nothing to join
    }

    if (!errorList.empty()) {
        callback.ControllerClientErrorCB(errorList);
    }

    alreadyInSession = true;
}

// call while locked!
void ControllerClient::RemoveUniqueName(const qcc::String& uniqueName)
{
    QCC_DbgPrintf(("%s", __func__));
    BusNameToDeviceId::iterator bit = nameToId.find(uniqueName);
    if (bit != nameToId.end()) {
        ControllerEntryMap::iterator it = controllers.find(bit->second);
        if (it != controllers.end()) {
            QCC_DbgPrintf(("Removing mapping: %s -> %s", uniqueName.c_str(), bit->second.c_str()));
            controllers.erase(it);
        }

        nameToId.erase(bit);
    }
}

void ControllerClient::OnSessionMemberRemoved(ajn::SessionId sessionId, const char* uniqueName)
{
    controllersLock.Lock();

    if (currentLeader.busName == uniqueName) {
        controllersLock.Unlock();
        // the leader has left its own session so treat this as a SessionLost
        OnSessionLost(sessionId);
    } else {
        RemoveUniqueName(uniqueName);
        controllersLock.Unlock();
    }
}


void ControllerClient::OnSessionLost(ajn::SessionId sessionID)
{
    QCC_DbgPrintf(("OnSessionLost(%u)\n", sessionID));

    QCC_DbgPrintf(("alreadyInSession set to false\n"));
    alreadyInSession = false;

    controllersLock.Lock();
    if (proxyObject != NULL && proxyObject->GetSessionId() == sessionID) {
        qcc::String busName = proxyObject->GetServiceName();
        delete proxyObject;
        proxyObject = NULL;
        RemoveUniqueName(busName);

        ClearCurrentLeader();
        controllersLock.Unlock();
        callback.DisconnectedFromControllerServiceCB(currentLeader.deviceID, currentLeader.deviceName);
        JoinLeaderSession();
    } else {
        controllersLock.Unlock();
    }
}

void ControllerClient::SignalWithArgDispatcher(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& message)
{
    bus.EnableConcurrentCallbacks();

    QCC_DbgPrintf(("%s: Received Signal %s", __func__, message->GetMemberName()));

    SignalDispatcherMap::iterator it = signalHandlers.find(message->GetMemberName());
    if (it != signalHandlers.end()) {
        SignalHandlerBase* handler = it->second;

        size_t numInputArgs;
        const MsgArg* inputArgs;
        message->GetArgs(numInputArgs, inputArgs);

        LSFStringList idList;

        MsgArg* idArgs;
        size_t numIds;
        inputArgs[0].Get("as", &numIds, &idArgs);

        for (size_t i = 0; i < numIds; ++i) {
            char* tempId;
            idArgs[i].Get("s", &tempId);
            idList.push_back(LSFString(tempId));
        }

        handler->Handle(idList);
    }
}

void ControllerClient::SignalWithoutArgDispatcher(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& message)
{
    bus.EnableConcurrentCallbacks();

    QCC_DbgPrintf(("%s: Received Signal %s", __func__, message->GetMemberName()));

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

    ControllerEntry* joined = static_cast<ControllerEntry*>(context);
    joined->joining = false;

    QCC_DbgPrintf(("%s: sessionId= %u status=%s\n", __func__, sessionId, QCC_StatusText(status)));

    if (status == ER_OK) {
        controllersLock.Lock();
        SessionId oldSession = proxyObject ? proxyObject->GetSessionId() : 0;

        LSFString device_id = joined->deviceID;
        LSFString device_name = joined->deviceName;
        bool make_callback = false;

        if (currentLeader.rank <= joined->rank) {
            currentLeader = *joined;
            make_callback = true;

            // we are connected to the new leader!
            delete proxyObject;
            proxyObject = new ProxyBusObject(bus, currentLeader.busName.c_str(), ControllerServiceObjectPath, sessionId);
            // add the synchronization interface
            //const InterfaceDescription* stateSyncInterface = bus.GetInterface(LeaderElectionAndStateSyncInterfaceName);
            //leaderObj->AddInterface(*stateSyncInterface);
            proxyObject->IntrospectRemoteObject();

            // call these with the lock
            AddMethodHandlers();
            AddSignalHandlers();
        } else {
            // we don't care about this session anymore!
            // leave it!
            oldSession = sessionId;
        }

        controllersLock.Unlock();

        // there is an old session to leave
        if (oldSession) {
            bus.LeaveSession(oldSession);
        }

        if (make_callback) {
            callback.ConnectedToControllerServiceCB(device_id, device_name);
        }

        /*
         * Get and print the versions of all the Controller Service interfaces
         */
        /*
           QCC_DbgPrintf(("%s: Trying to read the versions of all the Controller Service interfaces", __func__));
           MsgArg val;
           status = proxyObject->GetProperty(ControllerServiceInterfaceName, "Version", val);
           if (ER_OK == status) {
            uint32_t iVal = 0;
            val.Get("u", &iVal);
            QCC_DbgPrintf(("%s: ControllerServiceInterfaceVersion = %d", __func__, iVal));
           } else {
            QCC_LogError(status, ("GetProperty on %s failed", ControllerServiceInterfaceName));
           }
           val.Clear();
           status = proxyObject->GetProperty(ControllerServiceLampInterfaceName, "Version", val);
           if (ER_OK == status) {
            uint32_t iVal = 0;
            val.Get("u", &iVal);
            QCC_DbgPrintf(("%s: ControllerServiceLampInterfaceVersion = %d", __func__, iVal));
           } else {
            QCC_LogError(status, ("GetProperty on %s failed", ControllerServiceLampInterfaceName));
           }
           val.Clear();
           status = proxyObject->GetProperty(ControllerServiceLampGroupInterfaceName, "Version", val);
           if (ER_OK == status) {
            uint32_t iVal = 0;
            val.Get("u", &iVal);
            QCC_DbgPrintf(("%s: ControllerServiceLampGroupInterfaceVersion = %d", __func__, iVal));
           } else {
            QCC_LogError(status, ("GetProperty on %s failed", ControllerServiceLampGroupInterfaceName));
           }
           val.Clear();
           status = proxyObject->GetProperty(ControllerServicePresetInterfaceName, "Version", val);
           if (ER_OK == status) {
            uint32_t iVal = 0;
            val.Get("u", &iVal);
            QCC_DbgPrintf(("%s: ControllerServicePresetInterfaceVersion = %d", __func__, iVal));
           } else {
            QCC_LogError(status, ("GetProperty on %s failed", ControllerServicePresetInterfaceName));
           }
           val.Clear();
           status = proxyObject->GetProperty(ControllerServiceSceneInterfaceName, "Version", val);
           if (ER_OK == status) {
            uint32_t iVal = 0;
            val.Get("u", &iVal);
            QCC_DbgPrintf(("%s: ControllerServiceSceneInterfaceVersion = %d", __func__, iVal));
           } else {
            QCC_LogError(status, ("GetProperty on %s failed", ControllerServiceSceneInterfaceName));
           }
           val.Clear();
           status = proxyObject->GetProperty(ControllerServiceMasterSceneInterfaceName, "Version", val);
           if (ER_OK == status) {
            uint32_t iVal = 0;
            val.Get("u", &iVal);
            QCC_DbgPrintf(("%s: ControllerServiceMasterSceneInterfaceVersion = %d", __func__, iVal));
           } else {
            QCC_LogError(status, ("GetProperty on %s failed", ControllerServiceMasterSceneInterfaceName));
           }
         */
    } else {
        callback.ConnectToControllerServiceFailedCB(joined->deviceID, joined->deviceName);
        alreadyInSession = false;

        // if the connection failed, assume the CS is no longer there until we see another announcement
        controllersLock.Lock();
        RemoveUniqueName(joined->busName);
        controllersLock.Unlock();
        JoinLeaderSession();
    }

    if (!errorList.empty()) {
        callback.ControllerClientErrorCB(errorList);
    }
}

void ControllerClient::OnAnnounced(SessionPort port, const char* busName, const char* deviceID, const char* deviceName, uint64_t rank, uint32_t isLeader)
{
    QCC_DbgPrintf(("ControllerClient::OnAnnounced(port=%u, busName=%s, deviceID=%s, deviceName=%s, rank=%lu, isLeader=%d)",
                   port, busName, deviceID, deviceName, rank, isLeader));

    controllersLock.Lock();
    // find or insert
    nameToId[busName] = deviceID;
    ControllerEntry& entry = controllers[deviceID];

    // if the name of the current CS has changed...
    if (deviceID == currentLeader.deviceID) {
        // TODO: can the rank change?
        bool nameChanged = false;
        if (deviceName != currentLeader.deviceName) {
            entry.deviceName = deviceName;
            currentLeader.deviceName = deviceName;
            nameChanged = true;
        }
        controllersLock.Unlock();
        if (nameChanged) {
            controllerServiceManagerPtr->callback.ControllerServiceNameChangedCB();
        }
        return;
    }

    entry.port = port;
    entry.busName = busName;
    entry.rank = rank;
    entry.isLeader = isLeader;
    entry.joining = false;
    entry.deviceID = deviceID;
    entry.deviceName = deviceName;
    controllersLock.Unlock();

    JoinLeaderSession();
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
    controllersLock.Lock();

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

    controllersLock.Unlock();
    return status;
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithResponseCodeAndListOfIDs(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Call=%s", __func__, methodName));
    LSFString* methodNameContext = new LSFString(methodName);
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
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __func__, ((LSFString*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
        bus.EnableConcurrentCallbacks();

        if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
            MethodReplyWithResponseCodeAndListOfIDsDispatcherMap::iterator it = methodReplyWithResponseCodeAndListOfIDsHandlers.find(*((LSFString*)context));
            if (it != methodReplyWithResponseCodeAndListOfIDsHandlers.end()) {
                MethodReplyWithResponseCodeAndListOfIDsHandlerBase* handler = it->second;

                size_t numInputArgs;
                const MsgArg* inputArgs;
                message->GetArgs(numInputArgs, inputArgs);

                LSFStringList idList;
                LSFResponseCode responseCode;

                inputArgs[0].Get("u", &responseCode);

                MsgArg* idArgs;
                size_t numIds;
                inputArgs[1].Get("as", &numIds, &idArgs);

                for (size_t i = 0; i < numIds; ++i) {
                    char* tempId;
                    idArgs[i].Get("s", &tempId);
                    idList.push_back(LSFString(tempId));
                }

                handler->Handle(responseCode, idList);
            }
        } else {
            ErrorCodeList errorList;
            errorList.push_back(ERROR_ALLJOYN_METHOD_CALL_TIMEOUT);
            callback.ControllerClientErrorCB(errorList);
        }

        delete ((LSFString*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __func__));
    }
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithResponseCodeIDAndName(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Call=%s", __func__, methodName));
    LSFString* methodNameContext = new LSFString(methodName);
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
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __func__, ((LSFString*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
        bus.EnableConcurrentCallbacks();

        if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
            MethodReplyWithResponseCodeIDAndNameDispatcherMap::iterator it = methodReplyWithResponseCodeIDAndNameHandlers.find(*((LSFString*)context));
            if (it != methodReplyWithResponseCodeIDAndNameHandlers.end()) {
                MethodReplyWithResponseCodeIDAndNameHandlerBase* handler = it->second;

                size_t numInputArgs;
                const MsgArg* inputArgs;
                message->GetArgs(numInputArgs, inputArgs);

                char* uniqueId;
                char* name;
                LSFResponseCode responseCode;

                inputArgs[0].Get("u", &responseCode);
                inputArgs[1].Get("s", &uniqueId);
                inputArgs[2].Get("s", &name);

                LSFString lsfId = LSFString(uniqueId);
                LSFString lsfName = LSFString(name);

                handler->Handle(responseCode, lsfId, lsfName);
            }
        } else {
            ErrorCodeList errorList;
            errorList.push_back(ERROR_ALLJOYN_METHOD_CALL_TIMEOUT);
            callback.ControllerClientErrorCB(errorList);
        }

        delete ((LSFString*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __func__));
    }
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithResponseCodeAndID(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Call=%s", __func__, methodName));
    LSFString* methodNameContext = new LSFString(methodName);
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
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __func__, ((LSFString*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
        bus.EnableConcurrentCallbacks();

        if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
            MethodReplyWithResponseCodeAndIDDispatcherMap::iterator it = methodReplyWithResponseCodeAndIDHandlers.find(*((LSFString*)context));
            if (it != methodReplyWithResponseCodeAndIDHandlers.end()) {
                MethodReplyWithResponseCodeAndIDHandlerBase* handler = it->second;

                size_t numInputArgs;
                const MsgArg* inputArgs;
                message->GetArgs(numInputArgs, inputArgs);

                char* id;
                LSFResponseCode responseCode;

                inputArgs[0].Get("u", &responseCode);
                inputArgs[1].Get("s", &id);

                LSFString lsfId = LSFString(id);

                handler->Handle(responseCode, lsfId);
            }
        } else {
            ErrorCodeList errorList;
            errorList.push_back(ERROR_ALLJOYN_METHOD_CALL_TIMEOUT);
            callback.ControllerClientErrorCB(errorList);
        }

        delete ((LSFString*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __func__));
    }
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithUint32Value(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Call=%s", __func__, methodName));
    LSFString* methodNameContext = new LSFString(methodName);
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
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __func__, ((LSFString*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
        bus.EnableConcurrentCallbacks();

        if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
            MethodReplyWithUint32ValueDispatcherMap::iterator it = methodReplyWithUint32ValueHandlers.find(*((LSFString*)context));
            if (it != methodReplyWithUint32ValueHandlers.end()) {
                MethodReplyWithUint32ValueHandlerBase* handler = it->second;

                size_t numInputArgs;
                const MsgArg* inputArgs;
                message->GetArgs(numInputArgs, inputArgs);

                uint32_t value;
                inputArgs[0].Get("u", &value);
                handler->Handle(value);
            } else {
                QCC_LogError(ER_FAIL, ("%s: Did not find handler", __func__));
            }
        } else {
            ErrorCodeList errorList;
            errorList.push_back(ERROR_ALLJOYN_METHOD_CALL_TIMEOUT);
            callback.ControllerClientErrorCB(errorList);
        }

        delete ((LSFString*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __func__));
    }
}

ControllerClientStatus ControllerClient::MethodCallAsyncForReplyWithResponseCodeIDLanguageAndName(
    const char* ifaceName,
    const char* methodName,
    const ajn::MsgArg* args,
    size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Call=%s", __func__, methodName));
    LSFString* methodNameContext = new LSFString(methodName);
    ControllerClientStatus status = MethodCallAsyncHelper(
        ifaceName,
        methodName,
        this,
        static_cast<ajn::MessageReceiver::ReplyHandler>(&ControllerClient::HandlerForMethodReplyWithResponseCodeIDLanguageAndName),
        args,
        numArgs,
        (void*)methodNameContext);
    if (status != CONTROLLER_CLIENT_OK) {
        delete methodNameContext;
    }
    return status;
}

void ControllerClient::HandlerForMethodReplyWithResponseCodeIDLanguageAndName(Message& message, void* context)
{
    if (context) {
        QCC_DbgPrintf(("%s: Method Reply for %s:%s", __func__, ((LSFString*)context)->c_str(), (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
        bus.EnableConcurrentCallbacks();

        if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
            MethodReplyWithResponseCodeIDLanguageAndNameDispatcherMap::iterator it = methodReplyWithResponseCodeIDLanguageAndNameHandlers.find(*((LSFString*)context));
            if (it != methodReplyWithResponseCodeIDLanguageAndNameHandlers.end()) {
                MethodReplyWithResponseCodeIDLanguageAndNameHandlerBase* handler = it->second;

                size_t numInputArgs;
                const MsgArg* inputArgs;
                message->GetArgs(numInputArgs, inputArgs);

                LSFResponseCode responseCode;
                char* id;
                char* lang;
                char* name;

                inputArgs[0].Get("u", &responseCode);
                inputArgs[1].Get("s", &id);
                inputArgs[2].Get("s", &lang);
                inputArgs[3].Get("s", &name);

                LSFString lsfId = LSFString(id);
                LSFString language = LSFString(lang);
                LSFString lsfName = LSFString(name);

                handler->Handle(responseCode, lsfId, language, lsfName);
            } else {
                QCC_LogError(ER_FAIL, ("%s: Did not find handler", __func__));
            }
        } else {
            ErrorCodeList errorList;
            errorList.push_back(ERROR_ALLJOYN_METHOD_CALL_TIMEOUT);
            callback.ControllerClientErrorCB(errorList);
        }

        delete ((LSFString*)context);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Received a NULL context in method reply", __func__));
    }
}

void ControllerClient::Reset(void) {

}

void ControllerClient::AddMethodHandlers()
{
    //methodReplyWithResponseCodeAndListOfIDsHandlers.clear();

    if (controllerServiceManagerPtr) {
        AddNoArgSignalHandler("ControllerServiceLightingReset", controllerServiceManagerPtr, &ControllerServiceManager::ControllerServiceLightingReset);

        AddMethodReplyWithUint32ValueHandler("GetControllerServiceVersion", controllerServiceManagerPtr, &ControllerServiceManager::GetControllerServiceVersionReply);
        AddMethodReplyWithUint32ValueHandler("LightingResetControllerService", controllerServiceManagerPtr, &ControllerServiceManager::LightingResetControllerServiceReply);
    }

    if (lampManagerPtr) {
        AddSignalHandler("LampsNameChanged", lampManagerPtr, &LampManager::LampsNameChanged);
        AddSignalHandler("LampsStateChanged", lampManagerPtr, &LampManager::LampsStateChanged);

        AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllLampIDs", lampManagerPtr, &LampManager::GetAllLampIDsReply);

        AddMethodReplyWithResponseCodeIDAndNameHandler("TransitionLampStateField", lampManagerPtr, &LampManager::TransitionLampStateFieldReply);
        AddMethodReplyWithResponseCodeIDAndNameHandler("ResetLampStateField", lampManagerPtr, &LampManager::ResetLampStateFieldReply);
        AddMethodReplyWithResponseCodeIDAndNameHandler("SetLampName", lampManagerPtr, &LampManager::SetLampNameReply);

        AddMethodReplyWithResponseCodeIDLanguageAndNameHandler("GetLampName", lampManagerPtr, &LampManager::GetLampNameReply);
        AddMethodReplyWithResponseCodeIDLanguageAndNameHandler("GetLampManufacturer", lampManagerPtr, &LampManager::GetLampManufacturerReply);

        AddMethodReplyWithResponseCodeAndIDHandler("ResetLampState", lampManagerPtr, &LampManager::ResetLampStateReply);
        AddMethodReplyWithResponseCodeAndIDHandler("TransitionLampState", lampManagerPtr, &LampManager::TransitionLampStateReply);
        AddMethodReplyWithResponseCodeAndIDHandler("TransitionLampStateToPreset", lampManagerPtr, &LampManager::TransitionLampStateToPresetReply);
        AddMethodReplyWithResponseCodeAndIDHandler("PulseLampWithState", lampManagerPtr, &LampManager::PulseLampWithStateReply);
        AddMethodReplyWithResponseCodeAndIDHandler("PulseLampWithPreset", lampManagerPtr, &LampManager::PulseLampWithPresetReply);
    }

    if (lampGroupManagerPtr) {
        AddSignalHandler("LampGroupsNameChanged", lampGroupManagerPtr, &LampGroupManager::LampGroupsNameChanged);
        AddSignalHandler("LampGroupsCreated", lampGroupManagerPtr, &LampGroupManager::LampGroupsCreated);
        AddSignalHandler("LampGroupsUpdated", lampGroupManagerPtr, &LampGroupManager::LampGroupsUpdated);
        AddSignalHandler("LampGroupsDeleted", lampGroupManagerPtr, &LampGroupManager::LampGroupsDeleted);

        AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllLampGroupIDs", lampGroupManagerPtr, &LampGroupManager::GetAllLampGroupIDsReply);

        AddMethodReplyWithResponseCodeIDLanguageAndNameHandler("GetLampGroupName", lampGroupManagerPtr, &LampGroupManager::GetLampGroupNameReply);
        AddMethodReplyWithResponseCodeIDAndNameHandler("TransitionLampGroupStateField", lampGroupManagerPtr, &LampGroupManager::TransitionLampGroupStateFieldReply);
        AddMethodReplyWithResponseCodeIDAndNameHandler("ResetLampGroupStateField", lampGroupManagerPtr, &LampGroupManager::ResetLampGroupStateFieldReply);

        AddMethodReplyWithResponseCodeIDAndNameHandler("SetLampGroupName", lampGroupManagerPtr, &LampGroupManager::SetLampGroupNameReply);
        AddMethodReplyWithResponseCodeAndIDHandler("ResetLampGroupState", lampGroupManagerPtr, &LampGroupManager::ResetLampGroupStateReply);
        AddMethodReplyWithResponseCodeAndIDHandler("TransitionLampGroupState", lampGroupManagerPtr, &LampGroupManager::TransitionLampGroupStateReply);
        AddMethodReplyWithResponseCodeAndIDHandler("TransitionLampGroupStateToPreset", lampGroupManagerPtr, &LampGroupManager::TransitionLampGroupStateToPresetReply);
        AddMethodReplyWithResponseCodeAndIDHandler("CreateLampGroup", lampGroupManagerPtr, &LampGroupManager::CreateLampGroupReply);
        AddMethodReplyWithResponseCodeAndIDHandler("UpdateLampGroup", lampGroupManagerPtr, &LampGroupManager::UpdateLampGroupReply);
        AddMethodReplyWithResponseCodeAndIDHandler("DeleteLampGroup", lampGroupManagerPtr, &LampGroupManager::DeleteLampGroupReply);

        AddMethodReplyWithResponseCodeAndIDHandler("PulseLampGroupWithState", lampGroupManagerPtr, &LampGroupManager::PulseLampGroupWithStateReply);
        AddMethodReplyWithResponseCodeAndIDHandler("PulseLampGroupWithPreset", lampGroupManagerPtr, &LampGroupManager::PulseLampGroupWithPresetReply);
    }

    if (presetManagerPtr) {
        AddNoArgSignalHandler("DefaultLampStateChanged", presetManagerPtr, &PresetManager::DefaultLampStateChanged);
        AddSignalHandler("PresetsNameChanged", presetManagerPtr, &PresetManager::PresetsNameChanged);
        AddSignalHandler("PresetsCreated", presetManagerPtr, &PresetManager::PresetsCreated);
        AddSignalHandler("PresetsUpdated", presetManagerPtr, &PresetManager::PresetsUpdated);
        AddSignalHandler("PresetsDeleted", presetManagerPtr, &PresetManager::PresetsDeleted);

        AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllPresetIDs", presetManagerPtr, &PresetManager::GetAllPresetIDsReply);

        AddMethodReplyWithResponseCodeIDLanguageAndNameHandler("GetPresetName", presetManagerPtr, &PresetManager::GetPresetNameReply);

        AddMethodReplyWithResponseCodeIDAndNameHandler("SetPresetName", presetManagerPtr, &PresetManager::SetPresetNameReply);
        AddMethodReplyWithResponseCodeAndIDHandler("CreatePreset", presetManagerPtr, &PresetManager::CreatePresetReply);
        AddMethodReplyWithResponseCodeAndIDHandler("UpdatePreset", presetManagerPtr, &PresetManager::UpdatePresetReply);
        AddMethodReplyWithResponseCodeAndIDHandler("DeletePreset", presetManagerPtr, &PresetManager::DeletePresetReply);

        AddMethodReplyWithUint32ValueHandler("SetDefaultLampState", presetManagerPtr, &PresetManager::SetDefaultLampStateReply);
    }

    if (sceneManagerPtr) {
        AddSignalHandler("ScenesNameChanged", sceneManagerPtr, &SceneManager::ScenesNameChanged);
        AddSignalHandler("ScenesCreated", sceneManagerPtr, &SceneManager::ScenesCreated);
        AddSignalHandler("ScenesUpdated", sceneManagerPtr, &SceneManager::ScenesUpdated);
        AddSignalHandler("ScenesDeleted", sceneManagerPtr, &SceneManager::ScenesDeleted);
        AddSignalHandler("ScenesApplied", sceneManagerPtr, &SceneManager::ScenesApplied);

        AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllSceneIDs", sceneManagerPtr, &SceneManager::GetAllSceneIDsReply);

        AddMethodReplyWithResponseCodeIDLanguageAndNameHandler("GetSceneName", sceneManagerPtr, &SceneManager::GetSceneNameReply);

        AddMethodReplyWithResponseCodeIDAndNameHandler("SetSceneName", sceneManagerPtr, &SceneManager::SetSceneNameReply);
        AddMethodReplyWithResponseCodeAndIDHandler("CreateScene", sceneManagerPtr, &SceneManager::CreateSceneReply);
        AddMethodReplyWithResponseCodeAndIDHandler("UpdateScene", sceneManagerPtr, &SceneManager::UpdateSceneReply);
        AddMethodReplyWithResponseCodeAndIDHandler("DeleteScene", sceneManagerPtr, &SceneManager::DeleteSceneReply);
        AddMethodReplyWithResponseCodeAndIDHandler("ApplyScene", sceneManagerPtr, &SceneManager::ApplySceneReply);
    }

    if (masterSceneManagerPtr) {
        AddSignalHandler("MasterScenesNameChanged", masterSceneManagerPtr, &MasterSceneManager::MasterScenesNameChanged);
        AddSignalHandler("MasterScenesCreated", masterSceneManagerPtr, &MasterSceneManager::MasterScenesCreated);
        AddSignalHandler("MasterScenesUpdated", masterSceneManagerPtr, &MasterSceneManager::MasterScenesUpdated);
        AddSignalHandler("MasterScenesDeleted", masterSceneManagerPtr, &MasterSceneManager::MasterScenesDeleted);
        AddSignalHandler("MasterScenesApplied", masterSceneManagerPtr, &MasterSceneManager::MasterScenesApplied);

        AddMethodReplyWithResponseCodeAndListOfIDsHandler("GetAllMasterSceneIDs", masterSceneManagerPtr, &MasterSceneManager::GetAllMasterSceneIDsReply);

        AddMethodReplyWithResponseCodeIDLanguageAndNameHandler("GetMasterSceneName", masterSceneManagerPtr, &MasterSceneManager::GetMasterSceneNameReply);

        AddMethodReplyWithResponseCodeIDAndNameHandler("SetMasterSceneName", masterSceneManagerPtr, &MasterSceneManager::SetMasterSceneNameReply);
        AddMethodReplyWithResponseCodeAndIDHandler("CreateMasterScene", masterSceneManagerPtr, &MasterSceneManager::CreateMasterSceneReply);
        AddMethodReplyWithResponseCodeAndIDHandler("UpdateMasterScene", masterSceneManagerPtr, &MasterSceneManager::UpdateMasterSceneReply);
        AddMethodReplyWithResponseCodeAndIDHandler("DeleteMasterScene", masterSceneManagerPtr, &MasterSceneManager::DeleteMasterSceneReply);
        AddMethodReplyWithResponseCodeAndIDHandler("ApplyMasterScene", masterSceneManagerPtr, &MasterSceneManager::ApplyMasterSceneReply);
    }
}

void ControllerClient::AddSignalHandlers()
{
    const InterfaceDescription* controllerServiceInterface = proxyObject->GetInterface(ControllerServiceInterfaceName);
    const InterfaceDescription* controllerServiceLampInterface = proxyObject->GetInterface(ControllerServiceLampInterfaceName);
    const InterfaceDescription* controllerServiceLampGroupInterface = proxyObject->GetInterface(ControllerServiceLampGroupInterfaceName);
    const InterfaceDescription* controllerServicePresetInterface = proxyObject->GetInterface(ControllerServicePresetInterfaceName);
    const InterfaceDescription* controllerServiceSceneInterface = proxyObject->GetInterface(ControllerServiceSceneInterfaceName);
    const InterfaceDescription* controllerServiceMasterSceneInterface = proxyObject->GetInterface(ControllerServiceMasterSceneInterfaceName);

    const SignalEntry signalEntries[] = {
        { controllerServiceInterface->GetMember("ControllerServiceLightingReset"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithoutArgDispatcher) },
        { controllerServiceLampInterface->GetMember("LampsNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceLampInterface->GetMember("LampsStateChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("LampGroupsNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("LampGroupsCreated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("LampGroupsUpdated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("LampGroupsDeleted"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServicePresetInterface->GetMember("DefaultLampStateChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithoutArgDispatcher) },
        { controllerServicePresetInterface->GetMember("PresetsNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServicePresetInterface->GetMember("PresetsCreated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServicePresetInterface->GetMember("PresetsUpdated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServicePresetInterface->GetMember("PresetsDeleted"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceSceneInterface->GetMember("ScenesNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceSceneInterface->GetMember("ScenesCreated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceSceneInterface->GetMember("ScenesUpdated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceSceneInterface->GetMember("ScenesDeleted"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceSceneInterface->GetMember("ScenesApplied"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("MasterScenesNameChanged"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("MasterScenesCreated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("MasterScenesUpdated"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("MasterScenesDeleted"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("MasterScenesApplied"), static_cast<MessageReceiver::SignalHandler>(&ControllerClient::SignalWithArgDispatcher) }
    };

    for (size_t i = 0; i < (sizeof(signalEntries) / sizeof(SignalEntry)); ++i) {
        bus.RegisterSignalHandler(
            this,
            signalEntries[i].handler,
            signalEntries[i].member,
            ControllerServiceObjectPath);
    }
}

}
