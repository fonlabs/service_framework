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

#include <LampClients.h>
#include <alljoyn/Status.h>
#include <qcc/Debug.h>
#include <algorithm>
#include <ControllerService.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "LAMP_CLIENTS"

static const char* const LampServicePath = "/org/allseen/LSF/Lamp";
static const char* const LampInterface = "org.allseen.LSF.LampService";
static const char* CONFIG_OBJECT_PATH = "/Config";
static const char* CONFIG_INTERFACE_NAME = "org.alljoyn.Config";

class LampClients::QueueHandler : public WorkerQueue<QueuedMethodCall>::Handler {
  public:

    QueueHandler(LampClients& mgr) : manager(mgr) { }

    virtual ~QueueHandler() { }

    virtual int HandleItem(QueuedMethodCall* item) {
        LampClients::QueuedMethodCall::MethodHandler handler = item->handler;
        (manager.*handler)(item);
        return 0;
    }

    LampClients& manager;
};

class LampClients::ServiceHandler : public services::AnnounceHandler {
  public:
    ServiceHandler(LampClients& mgr) : manager(mgr) { }

    virtual void Announce(uint16_t version, uint16_t port, const char* busName, const ObjectDescriptions& objectDescs, const AboutData& aboutData);

    LampClients& manager;
};

void LampClients::ServiceHandler::Announce(
    uint16_t version,
    uint16_t port,
    const char* busName,
    const ObjectDescriptions& objectDescs,
    const AboutData& aboutData)
{
    LSFString lampID;
    LSFString lampName;

    ObjectDescriptions::const_iterator oit = objectDescs.find(LampServicePath);
    if (oit != objectDescs.end()) {
        if (std::find(oit->second.begin(), oit->second.end(), LampInterface) != oit->second.end()) {
            AboutData::const_iterator ait = aboutData.find("DeviceId");
            if (ait != aboutData.end()) {
                const char* id;
                ait->second.Get("s", &id);
                lampID = id;
            }

            ait = aboutData.find("DeviceName");
            if (ait != aboutData.end()) {
                const char* name;
                ait->second.Get("s", &name);
                lampName = name;
            }
        }
    }

    if (!lampID.empty()) {
        manager.HandleAboutAnnounce(lampID, lampName, port, busName);
    }
}

void LampClients::GetAllLampIDs(LSFStringList& lamps)
{
    lamps.clear();
    lampLock.Lock();
    for (LampMap::const_iterator it = activeLamps.begin(); it != activeLamps.end(); ++it) {
        lamps.push_back(it->first);
    }
    lampLock.Unlock();
}

LampClients::LampClients(ControllerService& controllerSvc, LampClientsCallback& callback)
    : Manager(controllerSvc),
    queueHandler(new QueueHandler(*this)),
    methodQueue(*queueHandler),
    serviceHandler(new ServiceHandler(*this)),
    callback(callback)
{
    keyListener.SetPassCode(INITIAL_PASSCODE);
}

LampClients::~LampClients()
{
    delete serviceHandler;
    delete queueHandler;
}

QStatus LampClients::Start()
{
    // to receive About data
    QStatus status = services::AnnouncementRegistrar::RegisterAnnounceHandler(controllerService.GetBusAttachment(), *serviceHandler);
    QCC_DbgPrintf(("RegisterAnnounceHandler(): %s\n", QCC_StatusText(status)));
    status = controllerService.GetBusAttachment().AddMatch("sessionless='t',type='error'");
    QCC_DbgPrintf(("AddMatch(): %s\n", QCC_StatusText(status)));

    status = controllerService.GetBusAttachment().EnablePeerSecurity("ALLJOYN_PIN_KEYX", &keyListener);
    QCC_DbgPrintf(("EnablePeerSecurity(): %s\n", QCC_StatusText(status)));

    // spin up a message queue thread
    status = methodQueue.Start();
    QCC_DbgPrintf(("Start(): %s\n", QCC_StatusText(status)));

    if (status == ER_OK) {
        return ER_OK;
    } else {
        return ER_FAIL;
    }
}

QStatus LampClients::Stop(void)
{
    services::AnnouncementRegistrar::UnRegisterAnnounceHandler(controllerService.GetBusAttachment(), *serviceHandler);
    controllerService.GetBusAttachment().RemoveMatch("sessionless='t',type='error'");

    lampLock.Lock();
    sessionLampMap.clear();
    for (LampMap::iterator it = activeLamps.begin(); it != activeLamps.end(); ++it) {
        LampConnection* conn = it->second;
        controllerService.GetBusAttachment().LeaveSession(conn->object.GetSessionId());
        delete conn;
    }
    activeLamps.clear();
    lampLock.Unlock();

    // join the worker queue thread
    methodQueue.Join();
    return ER_OK;
}

void LampClients::HandleAboutAnnounce(const LSFString lampID, const LSFString& lampName, uint16_t port, const char* busName)
{
    QCC_DbgPrintf(("LampClients::HandleAboutAnnounce(%s,%s,%u,%s)\n", lampID.c_str(), lampName.c_str(), port, busName));
    LampMap::const_iterator lit = activeLamps.find(lampID);
    if (lit != activeLamps.end()) {
        // this is not a new lamp!
        const LampConnection* conn = lit->second;
        if (conn->name != lampName) {
            //TODO: Fix this code to send LampsNameChanged
/*            const char* lamp_id;
            args[0].Get("s", &lamp_id);
            LSFStringList idList;
            idList.push_back(lamp_id);
            controllerService.SendSignal(interfaceName, "LampsNameChanged", idList);*/
        }
    } else {
        LampConnection* connection = new LampConnection();
        connection->id = lampID;
        connection->wkn = busName;
        connection->name = lampName;

        SessionOpts opts;
        opts.isMultipoint = true;
        QStatus status = controllerService.GetBusAttachment().JoinSessionAsync(busName, port, this, opts, this, connection);
        QCC_DbgPrintf(("JoinSessionAsync(%s,%u): %s\n", busName, port, QCC_StatusText(status)));

        if (status != ER_OK) {
            delete connection;
        }
    }
}

void LampClients::JoinSessionCB(QStatus status, SessionId sessionId, const SessionOpts& opts, void* context)
{
    LampConnection* connection = static_cast<LampConnection*>(context);

    QCC_DbgPrintf(("Controller::JoinSessionCB(%s, %u)\n", QCC_StatusText(status), sessionId));

    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    if (status != ER_OK) {
        // what to do?  oh no!
        delete connection;
        return;
        // emit error!
    }

    QCC_DbgPrintf(("New connection to lamp ID [%s]\n", connection->id.c_str()));

    lampLock.Lock();
    activeLamps[connection->id] = connection;
    QCC_DbgPrintf(("Mapping [%u] to [%s]\n", sessionId, connection->id.c_str()));
    sessionLampMap[sessionId] = connection->id;
    connection->object = ProxyBusObject(controllerService.GetBusAttachment(), connection->wkn.c_str(), LampServicePath, sessionId);


    connection->configObject = ProxyBusObject(controllerService.GetBusAttachment(), connection->wkn.c_str(), CONFIG_OBJECT_PATH, sessionId);
    lampLock.Unlock();

    QStatus status2 = connection->object.IntrospectRemoteObject();
    QCC_DbgPrintf(("IntrospectRemoteObject returns %s\n", QCC_StatusText(status2)));

    // do not introspect the remote Config object!

    const InterfaceDescription* intf = controllerService.GetBusAttachment().GetInterface(CONFIG_INTERFACE_NAME);
    connection->configObject.AddInterface(*intf);


    const InterfaceDescription* id = controllerService.GetBusAttachment().GetInterface("org.allseen.LSF.LampState");

    const InterfaceDescription::Member* sig = id->GetMember("LampStateChanged");
    controllerService.GetBusAttachment().RegisterSignalHandler(this, static_cast<MessageReceiver::SignalHandler>(&LampClients::LampStateChangedSignalHandler), sig, LampServicePath);

    QCC_DbgPrintf(("Handlers registered!\n"));
}

// SessionListener
void LampClients::SessionLost(SessionId sessionId, SessionLostReason reason)
{
    LSFString id;
    lampLock.Lock();
    SessionIDMap::iterator sit = sessionLampMap.find(sessionId);
    if (sit != sessionLampMap.end()) {
        id = sit->second;
        sessionLampMap.erase(sit);
        LampMap::iterator nit = activeLamps.find(id);
        if (nit != activeLamps.end()) {
            QCC_DbgPrintf(("Lost session %u with id [%s]\n", sessionId, id.c_str()));
            id = nit->second->id;
            delete nit->second;
            activeLamps.erase(nit);
        }
    }
    lampLock.Unlock();
}

LSFResponseCode LampClients::QueueLampMethod(const LSFString& lampId, QueuedMethodCall* queuedCall)
{
    LSFResponseCode response = LSF_OK;
    QStatus status = lampLock.Lock();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Error acquiring mutex", __FUNCTION__));
        response = LSF_ERR_BUSY;
        delete queuedCall;
        return response;
    }

    LampMap::iterator lit = activeLamps.find(lampId);
    if (lit != activeLamps.end()) {
        LampConnection* conn = lit->second;
        queuedCall->objects.push_back(conn->object);
        queuedCall->configObjects.push_back(conn->configObject);
    } else {
        response = LSF_ERR_NOT_FOUND;
        lampLock.Unlock();
        delete queuedCall;
        return response;
    }


    lampLock.Unlock();
    status = methodQueue.AddItem(queuedCall);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to add to queue", __FUNCTION__));
        response = LSF_ERR_FAILURE;
    }

    if (status != ER_OK) {
        delete queuedCall;
    }

    return response;
}

LSFResponseCode LampClients::GetLampState(const LSFString& lampID, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, &LampClients::DoGetProperties, &LampClientsCallback::GetLampStateReplyCB);
    queuedCall->interface = "org.allseen.LSF.LampState";
    queuedCall->errorOutput.Set("a{sv}", 0, NULL);
    return QueueLampMethod(lampID, queuedCall);
}

LSFResponseCode LampClients::GetLampStateField(const LSFString& lampID, const std::string& field, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, &LampClients::DoGetProperties, &LampClientsCallback::GetLampStateFieldReplyCB);
    queuedCall->interface = "org.allseen.LSF.LampState";
    queuedCall->property = field;
    queuedCall->errorOutput.Set("v", "u", 0);
    return QueueLampMethod(lampID, queuedCall);
}

LSFResponseCode LampClients::GetLampDetails(const LSFString& lampID, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, &LampClients::DoGetProperties, &LampClientsCallback::GetLampDetailsReplyCB);
    queuedCall->interface = "org.allseen.LSF.LampDetails";
    queuedCall->errorOutput.Set("a{sv}", 0, NULL);
    return QueueLampMethod(lampID, queuedCall);
}

LSFResponseCode LampClients::GetLampParameters(const LSFString& lampID, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, &LampClients::DoGetProperties, &LampClientsCallback::GetLampParametersReplyCB);
    queuedCall->interface = "org.allseen.LSF.LampParameters";
    queuedCall->errorOutput.Set("a{sv}", 0, NULL);
    return QueueLampMethod(lampID, queuedCall);
}

LSFResponseCode LampClients::GetLampParametersField(const LSFString& lampID, const std::string& field, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, &LampClients::DoGetProperties, &LampClientsCallback::GetLampParametersFieldReplyCB);
    queuedCall->interface = "org.allseen.LSF.LampParameters";
    queuedCall->property = field;
    queuedCall->errorOutput.Set("v", "u", 0);
    return QueueLampMethod(lampID, queuedCall);
}

void LampClients::DoGetProperties(QueuedMethodCall* call)
{
    ProxyBusObject pobj = *(call->objects.begin());

    QStatus status = ER_NONE;
    if (pobj.IsValid()) {

        if (call->property.empty()) {
            MsgArg arg("s", call->interface.c_str());

            status = pobj.MethodCallAsync(
                org::freedesktop::DBus::Properties::InterfaceName,
                "GetAll",
                this,
                static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoGetPropertiesReply),
                &arg,
                1,
                call
                );
        } else {
            MsgArg args[2];
            args[0].Set("s", call->interface.c_str());
            args[1].Set("s", call->property.c_str());

            status = pobj.MethodCallAsync(
                org::freedesktop::DBus::Properties::InterfaceName,
                "Get",
                this,
                static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoGetPropertiesReply),
                args,
                2,
                call
                );
        }
    }

    if (status != ER_OK) {
        QCC_LogError(status, ("%s: GetAllPropertiesAsync/GetPropertyAsync failed", __FUNCTION__));
        if (call->callbackMethod) {
            LampClients::QueuedMethodCall::CallbackMethod cb = call->callbackMethod;
            (callback.*cb)(call->inMsg, call->errorOutput, LSF_ERR_FAILURE);
        }
        delete call;
    }
}

//void LampClients::DoGetPropertiesReply(QStatus status, ProxyBusObject* obj, const MsgArg& value, void* context)
void LampClients::DoGetPropertiesReply(ajn::Message& message, void* context)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    QueuedMethodCall* call = static_cast<QueuedMethodCall*>(context);

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (call->callbackMethod) {
        LampClients::QueuedMethodCall::CallbackMethod cb = call->callbackMethod;
        (callback.*cb)(call->inMsg, args[0], LSF_OK);
    }
    delete call;
}

void LampClients::DoTransitionLampState(QueuedMethodCall* call)
{
    ResponseCounter* counter = new ResponseCounter();
    //counter->total = call->objects.size();
    counter->total = 1;
    counter->numWaiting = call->objects.size();
    counter->call = call;

    //for (ObjectMap::const_iterator nit = call->objects.begin(); nit != call->objects.end(); ++nit) {
    ProxyBusObject pobj = call->objects.front();
    if (pobj.IsValid()) {
        QStatus status = pobj.MethodCallAsync(
            "org.allseen.LSF.LampState",
            "TransitionLampState",
            this,
            static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoSetLampMultipleReply),
            &call->args[0],
            call->args.size(),
            counter
            );

        if (status != ER_OK) {
            QCC_LogError(status, ("%s: MethodCallAsync failed", __FUNCTION__));
            DecrementWaitingAndSendResponse(counter);
        }
    }
    //}
}

LSFResponseCode LampClients::TransitionLampFieldState(
    const ajn::Message& inMsg,
    const LSFStringList& lamps,
    uint64_t timestamp,
    const char* field,
    const ajn::MsgArg& value,
    uint32_t period)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, &LampClients::DoTransitionLampState);
    LSFResponseCode response = LSF_OK;
    queuedCall->property = field;

    // get the ProxyBusObjects now so we don't need to lock lampLock on the worker thread
    QStatus status = lampLock.Lock();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Error acquiring mutex", __FUNCTION__));
        response = LSF_ERR_BUSY;
        delete queuedCall;
        return response;
    }

    for (LSFStringList::const_iterator it = lamps.begin(); it != lamps.end(); ++it) {
        LampMap::iterator lit = activeLamps.find(*it);
        if (lit != activeLamps.end()) {
            LampConnection* conn = lit->second;
            QCC_DbgPrintf(("Adding %s\n", it->c_str()));
            queuedCall->objects.push_back(conn->object);
        } else {
            QCC_LogError(status, ("%s: Lamp Not found", __FUNCTION__));
            response = LSF_ERR_NOT_FOUND;
        }
    }
    lampLock.Unlock();

    queuedCall->args.resize(3);
    queuedCall->args[0] = MsgArg("t", timestamp);

    MsgArg* arrayVals = new MsgArg[1];
    arrayVals[0].Set("{sv}", strdup(field), new MsgArg(value));
    arrayVals[0].SetOwnershipFlags(MsgArg::OwnsArgs | MsgArg::OwnsData);

    MsgArg keyValue("a{sv}", 1, arrayVals);
    keyValue.SetOwnershipFlags(MsgArg::OwnsArgs | MsgArg::OwnsData);

    queuedCall->args[1] = keyValue;
    queuedCall->args[2] = MsgArg("u", period);

    status = methodQueue.AddItem(queuedCall);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to add to queue", __FUNCTION__));
        response = LSF_ERR_FAILURE;
    }

    if (status != ER_OK) {
        delete queuedCall;
    }

    return response;
}

LSFResponseCode LampClients::TransitionLampState(const Message& inMsg, const LSFStringList& lamps, uint64_t timestamp, const MsgArg& state, uint32_t period)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, &LampClients::DoTransitionLampState);
    LSFResponseCode response = LSF_OK;

    // get the ProxyBusObjects now so we don't need to lock lampLock on the worker thread
    QStatus status = lampLock.Lock();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Error acquiring mutex", __FUNCTION__));
        response = LSF_ERR_BUSY;
        delete queuedCall;
        return response;
    }

    for (LSFStringList::const_iterator it = lamps.begin(); it != lamps.end(); ++it) {
        LampMap::iterator lit = activeLamps.find(*it);
        if (lit != activeLamps.end()) {
            LampConnection* conn = lit->second;
            QCC_DbgPrintf(("Adding %s\n", it->c_str()));
            queuedCall->objects.push_back(conn->object);
        } else {
            QCC_LogError(status, ("%s: Lamp Not found", __FUNCTION__));
            response = LSF_ERR_NOT_FOUND;
        }
    }
    lampLock.Unlock();

    queuedCall->args.resize(3);
    queuedCall->args[0] = MsgArg("t", timestamp);
    queuedCall->args[1] = state;
    queuedCall->args[2] = MsgArg("u", period);

    status = methodQueue.AddItem(queuedCall);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to add to queue", __FUNCTION__));
        response = LSF_ERR_FAILURE;
    }

    if (status != ER_OK) {
        delete queuedCall;
    }

    return response;
}

void LampClients::DecrementWaitingAndSendResponse(ResponseCounter* counter)
{
    if (qcc::DecrementAndFetch(&counter->numWaiting) == 0) {
        LSFResponseCode rc;
        if (counter->successCount == 0) {
            rc = LSF_ERR_FAILURE;
        } else if (counter->failCount != 0) {
            rc = LSF_ERR_PARTIAL;
        } else {
            rc = LSF_OK;
        }

        if (counter->call->property.empty()) {
            callback.TransitionLampStateReplyCB(counter->call->inMsg, rc);
        } else {
            callback.TransitionLampStateFieldReplyCB(counter->call->inMsg, counter->call->property.c_str(), rc);
        }

        delete counter->call;
        delete counter;
    }
}

void LampClients::DoSetLampMultipleReply(Message& message, void* context)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    QCC_DbgPrintf(("MethodReplyMultiplePassthrough with message: %s\n", message->GetSignature()));

    ResponseCounter* counter = static_cast<ResponseCounter*>(context);

    // only one!  do a direct passthrough
    if (counter->total == 1) {
        QCC_DbgPrintf(("Found only one!\n"));
        LSFResponseCode rc;
        args[0].Get("u", &rc);

        if (counter->call->property.empty()) {
            callback.TransitionLampStateReplyCB(counter->call->inMsg, rc);
        } else {
            callback.TransitionLampStateFieldReplyCB(counter->call->inMsg, counter->call->property.c_str(), rc);
        }

        delete counter->call;
        delete counter;
    } else {
        LSFResponseCode rc;
        // rc is always the LAST arg
        args[0].Get("u", &rc);

        switch (rc) {
        case LSF_OK:
            qcc::IncrementAndFetch(&counter->successCount);
            break;

        default:
            qcc::IncrementAndFetch(&counter->failCount);
            break;
        }

        // waiting on one less; possibly send reply!
        DecrementWaitingAndSendResponse(counter);
    }
}

LSFResponseCode LampClients::GetLampFaults(const LSFString& lampID, ajn::Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, &LampClients::DoGetLampFaults);
    queuedCall->interface = "org.allseen.LSF.LampService";
    queuedCall->property = "LampFaults";
    queuedCall->errorOutput.Set("au", 0, NULL);
    return QueueLampMethod(lampID, queuedCall);
}

void LampClients::DoGetLampFaults(QueuedMethodCall* call)
{
    ProxyBusObject pobj = *(call->objects.begin());

    MsgArg args[2];
    args[0].Set("s", call->interface.c_str());
    args[1].Set("s", call->property.c_str());

    QStatus status = ER_NONE;
    if (pobj.IsValid()) {
        status = pobj.MethodCallAsync(
            org::freedesktop::DBus::Properties::InterfaceName,
            "Get",
            this,
            static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoGetLampFaultsReply),
            args,
            2,
            call
            );
    }

    if (status != ER_OK) {
        callback.GetLampFaultsReplyCB(call->inMsg, call->errorOutput, LSF_ERR_FAILURE);
        delete call;
    }
}

void LampClients::DoGetLampFaultsReply(ajn::Message& message, void* context)
{
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    QueuedMethodCall* call = static_cast<QueuedMethodCall*>(context);

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    MsgArg* arrayArg;
    args[0].Get("v", &arrayArg);

    callback.GetLampFaultsReplyCB(call->inMsg, *arrayArg, LSF_OK);
    delete call;
}

LSFResponseCode LampClients::ClearLampFault(const LSFString& lampID, ajn::Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, &LampClients::DoClearLampFault);
    queuedCall->interface = "org.allseen.LSF.LampService";
    const MsgArg* args;
    size_t numArgs;
    inMsg->GetArgs(numArgs, args);
    queuedCall->args.reserve(1);
    queuedCall->args[0] = args[1];

    return QueueLampMethod(lampID, queuedCall);
}

void LampClients::DoClearLampFault(QueuedMethodCall* call)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    ProxyBusObject pobj = *(call->objects.begin());

    if (pobj.IsValid()) {
        QStatus status = pobj.MethodCallAsync(
            call->interface.c_str(),
            "ClearLampFault",
            this,
            static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoClearLampFaultReply),
            &call->args[0],
            1,
            call);

        if (status != ER_OK) {
            callback.ClearLampFaultReplyCB(call->inMsg, LSF_ERR_FAILURE, call->args[1].v_uint32);
            delete call;
        }
    }
}

void LampClients::DoClearLampFaultReply(ajn::Message& message, void* context)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    QueuedMethodCall* call = static_cast<QueuedMethodCall*>(context);

    const MsgArg* args;
    size_t numArgs;
    message->GetArgs(numArgs, args);

    LSFResponseCode rc;
    args[0].Get("u", &rc);

    LampFaultCode code;
    args[1].Get("u", &code);

    callback.ClearLampFaultReplyCB(call->inMsg, rc, code);
    delete call;
}

void LampClients::LampStateChangedSignalHandler(const InterfaceDescription::Member* member, const char* sourcePath, Message& message)
{
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);
    LSFStringList ids;
    ids.push_back(id);

    controllerService.SendSignal("org.allseen.LSF.ControllerService.Lamp", "LampsStateChanged", ids);
}

LSFResponseCode LampClients::GetLampName(const LSFString& lampID, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, &LampClients::DoGetLampConfigString);
    queuedCall->errorOutput.Set("s", "<ERROR>");
    queuedCall->property = "DeviceName";
    return QueueLampMethod(lampID, queuedCall);
}

LSFResponseCode LampClients::GetLampManufacturer(const LSFString& lampID, ajn::Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, &LampClients::DoGetLampConfigString);
    queuedCall->errorOutput.Set("s", "<ERROR>");
    queuedCall->property = "Manufacturer";
    return QueueLampMethod(lampID, queuedCall);
}

void LampClients::DoGetLampConfigString(QueuedMethodCall* call)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    ProxyBusObject pobj = *(call->configObjects.begin());

    if (pobj.IsValid()) {
        size_t numArgs;
        const MsgArg* args;
        call->inMsg->GetArgs(numArgs, args);

        // need to pass the language through
        const char* language;
        args[1].Get("s", &language);

        MsgArg arg("s", language);
        QStatus status = pobj.MethodCallAsync(
            CONFIG_INTERFACE_NAME,
            "GetConfigurations",
            this,
            static_cast<MessageReceiver::ReplyHandler>(&LampClients::GetConfigurationsReply),
            &arg,
            1,
            call);

        if (status != ER_OK) {
            callback.GetLampNameReplyCB(call->inMsg, "", LSF_ERR_FAILURE);
            delete call;
        }
    }
}

void LampClients::GetConfigurationsReply(Message& message, void* context)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    QueuedMethodCall* call = static_cast<QueuedMethodCall*>(context);

    const MsgArg* args;
    size_t numArgs;
    message->GetArgs(numArgs, args);

    size_t numEntries;
    MsgArg* entries;

    args[0].Get("a{sv}", &numEntries, &entries);
    for (size_t i = 0; i < numEntries; ++i) {
        char* key;
        MsgArg* value;
        entries[i].Get("{sv}", &key, &value);

        if (call->property == key) {
            char* name;
            value->Get("s", &name);

            if (call->property == "DeviceName") {
                callback.GetLampNameReplyCB(call->inMsg, name, LSF_OK);
            } else if (call->property == "Manufacturer") {
                callback.GetLampManufacturerReplyCB(call->inMsg, name, LSF_OK);
            }
        }
    }

    delete call;
}

LSFResponseCode LampClients::SetLampName(const LSFString& lampID, const char* name, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, &LampClients::DoSetLampName);
    queuedCall->property = name;
    return QueueLampMethod(lampID, queuedCall);
}

void LampClients::DoSetLampName(QueuedMethodCall* call)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    ProxyBusObject pobj = *(call->configObjects.begin());

    if (pobj.IsValid()) {
        MsgArg args[2];
        args[0].Set("s", "en");

        MsgArg name_arg("s", call->property.c_str());
        MsgArg arg("{sv}", "DeviceName", &name_arg);
        args[1].Set("a{sv}", 1, &arg);
        Message* message = new Message(call->inMsg);
        QStatus status = pobj.MethodCallAsync(
            CONFIG_INTERFACE_NAME,
            "UpdateConfigurations",
            this,
            static_cast<MessageReceiver::ReplyHandler>(&LampClients::UpdateConfigurationsReply),
            args,
            2,
            message);

        if (status != ER_OK) {
            callback.SetLampNameReplyCB(call->inMsg, LSF_ERR_FAILURE);
            delete message;
        }
    }

    delete call;
}

void LampClients::UpdateConfigurationsReply(Message& message, void* context)
{
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    Message* origMsg = static_cast<Message*>(context);
    callback.SetLampNameReplyCB(*origMsg, LSF_OK);
    delete origMsg;
}
