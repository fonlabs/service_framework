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

static const char* ABOUT_OBJECT_PATH = "/About";
static const char* ABOUT_INTERFACE_NAME = "org.alljoyn.About";

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

LSFResponseCode LampClients::GetAllLampIDs(LSFStringList& lamps)
{
    lamps.clear();
    QStatus status = lampLock.Lock();
    if (ER_OK != status) {
        QCC_LogError(ER_FAIL, ("%s: Failed to lock mutex", __FUNCTION__));
        return LSF_ERR_BUSY;
    }

    for (LampMap::const_iterator it = activeLamps.begin(); it != activeLamps.end(); ++it) {
        lamps.push_back(it->first);
    }

    status = lampLock.Unlock();
    if (ER_OK != status) {
        QCC_LogError(ER_FAIL, ("%s: Failed to unlock mutex", __FUNCTION__));
    }

    return LSF_OK;
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
            LSFStringList idList;
            idList.push_back(lampID);
            controllerService.SendSignal("org.allseen.LSF.ControllerService.Lamp", "LampsNameChanged", idList);
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
    connection->sessionID = sessionId;
    activeLamps[connection->id] = connection;
    connection->object = ProxyBusObject(controllerService.GetBusAttachment(), connection->wkn.c_str(), LampServicePath, sessionId);


    connection->configObject = ProxyBusObject(controllerService.GetBusAttachment(), connection->wkn.c_str(), CONFIG_OBJECT_PATH, sessionId);
    connection->aboutObject = ProxyBusObject(controllerService.GetBusAttachment(), connection->wkn.c_str(), ABOUT_OBJECT_PATH, sessionId);
    lampLock.Unlock();

    QStatus status2 = connection->object.IntrospectRemoteObject();
    QCC_DbgPrintf(("IntrospectRemoteObject returns %s\n", QCC_StatusText(status2)));

    // do not introspect the remote Config object!

    const InterfaceDescription* intf = controllerService.GetBusAttachment().GetInterface(CONFIG_INTERFACE_NAME);
    connection->configObject.AddInterface(*intf);

    intf = controllerService.GetBusAttachment().GetInterface(ABOUT_INTERFACE_NAME);
    connection->aboutObject.AddInterface(*intf);

    const InterfaceDescription* id = controllerService.GetBusAttachment().GetInterface("org.allseen.LSF.LampState");

    const InterfaceDescription::Member* sig = id->GetMember("LampStateChanged");
    controllerService.GetBusAttachment().RegisterSignalHandler(this, static_cast<MessageReceiver::SignalHandler>(&LampClients::LampStateChangedSignalHandler), sig, LampServicePath);

    QCC_DbgPrintf(("Handlers registered!\n"));
}

// SessionListener
void LampClients::SessionLost(SessionId sessionId, SessionLostReason reason)
{
// TODO: Add processing
}

void LampClients::SendMethodReply(LSFResponseCode responseCode, QueuedMethodCall* queuedMethodCall)
{
    size_t numArgs = queuedMethodCall->standardReplyArgs.size() + queuedMethodCall->customReplyArgs.size() + 1;
    QCC_DbgPrintf(("%s: NumArgs = %d", __FUNCTION__, numArgs));
    MsgArg* args = new MsgArg[numArgs];
    if (args) {
        args[0] = MsgArg("u", responseCode);
        uint8_t index = 1;
        while (queuedMethodCall->standardReplyArgs.size()) {
            args[index] = queuedMethodCall->standardReplyArgs.front();
            queuedMethodCall->standardReplyArgs.pop_front();
            index++;
        }
        while (queuedMethodCall->customReplyArgs.size()) {
            args[index] = queuedMethodCall->customReplyArgs.front();
            queuedMethodCall->customReplyArgs.pop_front();
            index++;
        }
        controllerService.SendMethodReply(queuedMethodCall->inMsg, args, numArgs);
    } else {
        QCC_LogError(ER_OUT_OF_MEMORY, ("%s: Failed to allocate enough memory", __FUNCTION__));
    }
}

LSFResponseCode LampClients::QueueLampMethod(QueuedMethodCall* queuedCall)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = methodQueue.AddItem(queuedCall);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to add to queue", __FUNCTION__));
        responseCode = LSF_ERR_BUSY;
        SendMethodReply(responseCode, queuedCall);
        delete queuedCall;
    }

    return responseCode;
}

LSFResponseCode LampClients::DoMethodCallAsync(QueuedMethodCall* queuedCall)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    LSFResponseCode responseCode = LSF_OK;

    LSFString responseID = GenerateUniqueID("RESPONSE");
    LSFString* context = new LSFString(responseID);
    ResponseCounter* response = new ResponseCounter(queuedCall->inMsg, context, queuedCall->callbackMethod, queuedCall->lamps.size());

    responseLock.Lock();
    responseMap.insert(std::make_pair(responseID, response));
    responseLock.Unlock();

    QStatus status = lampLock.Lock();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Error acquiring mutex", __FUNCTION__));
        responseCode = LSF_ERR_BUSY;
        delete queuedCall;
        return responseCode;
    }

    QCC_DbgPrintf(("%s: Acquired lock. Args size=%d Num Lamps=%d", __FUNCTION__, queuedCall->args.size(), queuedCall->lamps.size()));
    for (LSFStringList::const_iterator it = queuedCall->lamps.begin(); it != queuedCall->lamps.end(); it++) {
        QCC_DbgPrintf(("%s: Processing for LampID=%s", __FUNCTION__, (*it).c_str()));
        LampMap::iterator lit = activeLamps.find(queuedCall->lamps.front());
        if (lit != activeLamps.end()) {
            QCC_DbgPrintf(("%s: Found Lamp", __FUNCTION__));
            if (0 == strcmp(queuedCall->interface.c_str(), CONFIG_INTERFACE_NAME)) {
                QCC_DbgPrintf(("%s: Config Call", __FUNCTION__));
                status = lit->second->configObject.MethodCallAsync(
                    queuedCall->interface.c_str(),
                    queuedCall->method.c_str(),
                    this,
                    queuedCall->replyFunc,
                    &queuedCall->args[0],
                    queuedCall->args.size(),
                    context
                    );
            } else if (0 == strcmp(queuedCall->interface.c_str(), ABOUT_INTERFACE_NAME)) {
                QCC_DbgPrintf(("%s: About Call", __FUNCTION__));
                status = lit->second->aboutObject.MethodCallAsync(
                    queuedCall->interface.c_str(),
                    queuedCall->method.c_str(),
                    this,
                    queuedCall->replyFunc,
                    &queuedCall->args[0],
                    queuedCall->args.size(),
                    context
                    );
            } else {
                QCC_DbgPrintf(("%s: LampService Call", __FUNCTION__));
                status = lit->second->object.MethodCallAsync(
                    queuedCall->interface.c_str(),
                    queuedCall->method.c_str(),
                    this,
                    queuedCall->replyFunc,
                    &queuedCall->args[0],
                    queuedCall->args.size(),
                    context
                    );
            }

            if (status != ER_OK) {
                QCC_LogError(status, ("%s: MethodCallAsync failed", __FUNCTION__));
            }
        } else {
            //TODO
        }
    }

    lampLock.Unlock();

    delete queuedCall;

    return responseCode;
}

LSFResponseCode LampClients::GetLampState(const LSFString& lampID, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lampID, org::freedesktop::DBus::Properties::InterfaceName, "GetAll", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoGetPropertiesReply), &LampClientsCallback::GetLampStateReplyCB);
    queuedCall->args.push_back(MsgArg("s", "org.allseen.LSF.LampState"));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->customReplyArgs.push_back(MsgArg("a{sv}", 0, NULL));
    return QueueLampMethod(queuedCall);
}

LSFResponseCode LampClients::GetLampStateField(const LSFString& lampID, const LSFString& field, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lampID, org::freedesktop::DBus::Properties::InterfaceName, "Get", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoGetPropertiesReply), &LampClientsCallback::GetLampStateFieldReplyCB);
    queuedCall->args.push_back(MsgArg("s", "org.allseen.LSF.LampState"));
    queuedCall->args.push_back(MsgArg("s", field.c_str()));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", field.c_str()));
    MsgArg arg("u", 0);
    queuedCall->customReplyArgs.push_back(MsgArg("v", &arg));
    return QueueLampMethod(queuedCall);
}

LSFResponseCode LampClients::GetLampDetails(const LSFString& lampID, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lampID, org::freedesktop::DBus::Properties::InterfaceName, "GetAll", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoGetPropertiesReply), &LampClientsCallback::GetLampDetailsReplyCB);
    queuedCall->args.push_back(MsgArg("s", "org.allseen.LSF.LampDetails"));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->customReplyArgs.push_back(MsgArg("a{sv}", 0, NULL));
    return QueueLampMethod(queuedCall);
}

LSFResponseCode LampClients::GetLampParameters(const LSFString& lampID, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lampID, org::freedesktop::DBus::Properties::InterfaceName, "GetAll", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoGetPropertiesReply), &LampClientsCallback::GetLampParametersReplyCB);
    queuedCall->args.push_back(MsgArg("s", "org.allseen.LSF.LampParameters"));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->customReplyArgs.push_back(MsgArg("a{sv}", 0, NULL));
    return QueueLampMethod(queuedCall);
}

LSFResponseCode LampClients::GetLampParametersField(const LSFString& lampID, const LSFString& field, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lampID, org::freedesktop::DBus::Properties::InterfaceName, "Get", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoGetPropertiesReply), &LampClientsCallback::GetLampParametersFieldReplyCB);
    queuedCall->args.push_back(MsgArg("s", "org.allseen.LSF.LampParameters"));
    queuedCall->args.push_back(MsgArg("s", field.c_str()));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", field.c_str()));
    MsgArg arg("u", 0);
    queuedCall->customReplyArgs.push_back(MsgArg("v", &arg));
    return QueueLampMethod(queuedCall);
}

void LampClients::DoGetPropertiesReply(ajn::Message& message, void* context)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    LSFString* responseID = static_cast<LSFString*>(context);

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    responseLock.Lock();
    if (responseMap[*responseID]->callbackMethod) {
        LampClients::CallbackMethod cb = responseMap[*responseID]->callbackMethod;
        (callback.*cb)(responseMap[*responseID]->inMsg, args[0], LSF_OK);
    }
    delete responseMap[*responseID]->context;
    responseLock.Unlock();
}

LSFResponseCode LampClients::TransitionLampStateField(
    const ajn::Message& inMsg,
    const LSFStringList& lamps,
    uint64_t stateTimestamp,
    const char* field,
    const ajn::MsgArg& value,
    uint32_t period)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lamps, "org.allseen.LSF.LampState", "TransitionLampState", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoSetLampMultipleReply));

    MsgArg* arrayVals = new MsgArg[1];
    arrayVals[0].Set("{sv}", strdup(field), new MsgArg(value));
    arrayVals[0].SetOwnershipFlags(MsgArg::OwnsArgs | MsgArg::OwnsData);

    queuedCall->args.push_back(MsgArg("t", stateTimestamp));
    queuedCall->args.push_back(MsgArg("a{sv}", 1, arrayVals));
    queuedCall->args.push_back(MsgArg("u", period));

    //TODO: Fix for Lamp Groups
    queuedCall->standardReplyArgs.push_back(MsgArg("s", lamps.front().c_str()));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", field));

    return QueueLampMethod(queuedCall);
}

LSFResponseCode LampClients::PulseLampWithState(
    const ajn::Message& inMsg,
    const LSFStringList& lamps,
    const ajn::MsgArg& oldState,
    const ajn::MsgArg& newState,
    uint32_t period,
    uint32_t duration,
    uint32_t numPulses,
    uint64_t stateTimestamp)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lamps, "org.allseen.LSF.LampState", "ApplyPulseEffect", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoSetLampMultipleReply));

    queuedCall->args.resize(6);
    queuedCall->args[0] = oldState;
    queuedCall->args[1] = newState;
    queuedCall->args[2] = MsgArg("u", period);
    queuedCall->args[3] = MsgArg("u", duration);
    queuedCall->args[4] = MsgArg("u", numPulses);
    queuedCall->args[5] = MsgArg("t", stateTimestamp);

    //TODO: Fix for Lamp Groups
    queuedCall->standardReplyArgs.push_back(MsgArg("s", lamps.front().c_str()));

    return QueueLampMethod(queuedCall);
}

LSFResponseCode LampClients::TransitionLampState(const Message& inMsg, const LSFStringList& lamps, uint64_t stateTimestamp, const MsgArg& state, uint32_t period)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lamps, "org.allseen.LSF.LampState", "TransitionLampState", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoSetLampMultipleReply));

    queuedCall->args.resize(3);
    queuedCall->args[0] = MsgArg("t", stateTimestamp);
    queuedCall->args[1] = state;
    queuedCall->args[2] = MsgArg("u", period);

    //TODO: Fix for Lamp Groups
    queuedCall->standardReplyArgs.push_back(MsgArg("s", lamps.front().c_str()));

    return QueueLampMethod(queuedCall);
}

void LampClients::DecrementWaitingAndSendResponse(ResponseCounter* counter)
{
    if (qcc::DecrementAndFetch(&counter->numWaiting) == 0) {
        LSFResponseCode responseCode;
        if (counter->successCount == 0) {
            responseCode = LSF_ERR_FAILURE;
        } else if (counter->failCount != 0) {
            responseCode = LSF_ERR_PARTIAL;
        } else {
            responseCode = LSF_OK;
        }

        if ((0 == strcmp(counter->inMsg->GetMemberName(), "TransitionLampStateField")) ||
            (0 == strcmp(counter->inMsg->GetMemberName(), "TransitionLampGroupStateField"))) {
            callback.TransitionLampStateFieldReplyCB(counter->inMsg, responseCode);
        } else {
            callback.ChangeLampStateReplyCB(counter->inMsg, responseCode);
        }

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

    LSFString* responseID = static_cast<LSFString*>(context);

    QCC_DbgPrintf(("Found only one!\n"));
    LSFResponseCode responseCode;
    args[0].Get("u", &responseCode);

    responseLock.Lock();
    // TODO: Chnage this
    if ((0 == strcmp(responseMap[*responseID]->inMsg->GetMemberName(), "TransitionLampState")) ||
        (0 == strcmp(responseMap[*responseID]->inMsg->GetMemberName(), "ResetLampState")) ||
        (0 == strcmp(responseMap[*responseID]->inMsg->GetMemberName(), "TransitionLampStateToPreset")) ||
        (0 == strcmp(responseMap[*responseID]->inMsg->GetMemberName(), "TransitionLampGroupState")) ||
        (0 == strcmp(responseMap[*responseID]->inMsg->GetMemberName(), "PulseLampWithState")) ||
        (0 == strcmp(responseMap[*responseID]->inMsg->GetMemberName(), "PulseLampWithPreset"))) {
        QCC_DbgPrintf(("%s: Method reply for state API", __FUNCTION__));
        callback.ChangeLampStateReplyCB(responseMap[*responseID]->inMsg, responseCode);
    } else {
        QCC_DbgPrintf(("%s: Method reply for Field API", __FUNCTION__));
        callback.TransitionLampStateFieldReplyCB(responseMap[*responseID]->inMsg, responseCode);
    }

    delete responseMap[*responseID]->context;
    responseLock.Unlock();

#if 0
} else {
    LSFResponseCode responseCode;
    // responseCode is always the LAST arg
    args[0].Get("u", &responseCode);

    switch (responseCode) {
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
#endif
}

LSFResponseCode LampClients::GetLampFaults(const LSFString& lampID, ajn::Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lampID, org::freedesktop::DBus::Properties::InterfaceName, "Get", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoGetLampFaultsReply));
    queuedCall->args.resize(2);
    queuedCall->args[0] = MsgArg("s", "org.allseen.LSF.LampService");
    queuedCall->args[1] = MsgArg("s", "LampFaults");
    queuedCall->standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->customReplyArgs.push_back(MsgArg("au", 0, NULL));
    return QueueLampMethod(queuedCall);
}

LSFResponseCode LampClients::GetLampVersion(const LSFString& lampID, ajn::Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lampID, org::freedesktop::DBus::Properties::InterfaceName, "Get", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoGetLampVersionReply));
    queuedCall->args.push_back(MsgArg("s", "org.allseen.LSF.LampService"));
    queuedCall->args.push_back(MsgArg("s", "LampServiceVersion"));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->customReplyArgs.push_back(MsgArg("u", 0, NULL));
    return QueueLampMethod(queuedCall);
}

void LampClients::DoGetLampVersionReply(ajn::Message& message, void* context)
{
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    LSFString* responseID = static_cast<LSFString*>(context);

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    MsgArg* verArg;
    args[0].Get("v", &verArg);

    responseLock.Lock();
    callback.GetLampVersionReplyCB(responseMap[*responseID]->inMsg, LSF_OK, verArg->v_uint32);
    delete responseMap[*responseID]->context;
    responseLock.Unlock();
}

void LampClients::DoGetLampFaultsReply(ajn::Message& message, void* context)
{
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    LSFString* responseID = static_cast<LSFString*>(context);

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    MsgArg* arrayArg;
    args[0].Get("v", &arrayArg);

    responseLock.Lock();
    callback.GetLampFaultsReplyCB(responseMap[*responseID]->inMsg, *arrayArg, LSF_OK);
    delete responseMap[*responseID]->context;
    responseLock.Unlock();
}

LSFResponseCode LampClients::ClearLampFault(const LSFString& lampID, LampFaultCode faultCode, ajn::Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lampID, "org.allseen.LSF.LampService", "ClearLampFault", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::DoClearLampFaultReply));
    queuedCall->args.push_back(MsgArg("u", faultCode));

    queuedCall->standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->standardReplyArgs.push_back(MsgArg("u", faultCode));

    return QueueLampMethod(queuedCall);
}

void LampClients::DoClearLampFaultReply(ajn::Message& message, void* context)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    LSFString* responseID = static_cast<LSFString*>(context);

    const MsgArg* args;
    size_t numArgs;
    message->GetArgs(numArgs, args);

    LSFResponseCode responseCode;
    args[0].Get("u", &responseCode);

    LampFaultCode code;
    args[1].Get("u", &code);

    responseLock.Lock();


    callback.ClearLampFaultReplyCB(responseMap[*responseID]->inMsg, responseCode, code);

    delete responseMap[*responseID]->context;
    responseLock.Unlock();
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

LSFResponseCode LampClients::GetLampSupportedLanguages(const LSFString& lampID, ajn::Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lampID, ABOUT_INTERFACE_NAME, "GetAboutData", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::GetAboutReply));
    queuedCall->args.push_back(MsgArg("s", "en"));

    queuedCall->standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->customReplyArgs.push_back(MsgArg("as", 0, NULL));

    return QueueLampMethod(queuedCall);
}

void LampClients::GetAboutReply(Message& message, void* context)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    LSFString* responseID = static_cast<LSFString*>(context);

    const MsgArg* args;
    size_t numArgs;
    message->GetArgs(numArgs, args);

    size_t numEntries;
    MsgArg* entries;

    args[0].Get("a{sv}", &numEntries, &entries);
    responseLock.Lock();
    for (size_t i = 0; i < numEntries; ++i) {
        char* key;
        MsgArg* value;
        entries[i].Get("{sv}", &key, &value);
        QCC_DbgPrintf(("%s: %s", __FUNCTION__, key));
        if ((0 == strcmp(key, "SupportedLanguages")) && (0 == strcmp(responseMap[*responseID]->inMsg->GetMemberName(), "GetLampSupportedLanguages"))) {
            callback.GetLampSupportedLanguagesReplyCB(responseMap[*responseID]->inMsg, *value, LSF_OK);
        }
    }
    delete responseMap[*responseID]->context;
    responseLock.Unlock();
}

LSFResponseCode LampClients::GetLampName(const LSFString& lampID, const LSFString& language, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lampID, CONFIG_INTERFACE_NAME, "GetConfigurations", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::GetConfigurationsReply));
    queuedCall->args.push_back(MsgArg("s", language.c_str()));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", language.c_str()));
    queuedCall->customReplyArgs.push_back(MsgArg("s", "<ERROR>"));
    return QueueLampMethod(queuedCall);
}

LSFResponseCode LampClients::GetLampManufacturer(const LSFString& lampID, const LSFString& language, ajn::Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lampID, CONFIG_INTERFACE_NAME, "GetConfigurations", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::GetConfigurationsReply));
    queuedCall->args.push_back(MsgArg("s", language.c_str()));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", language.c_str()));
    queuedCall->customReplyArgs.push_back(MsgArg("s", "<ERROR>"));
    return QueueLampMethod(queuedCall);
}

void LampClients::GetConfigurationsReply(Message& message, void* context)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __FUNCTION__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    LSFString* responseID = static_cast<LSFString*>(context);

    const MsgArg* args;
    size_t numArgs;
    message->GetArgs(numArgs, args);

    size_t numEntries;
    MsgArg* entries;

    args[0].Get("a{sv}", &numEntries, &entries);
    responseLock.Lock();
    for (size_t i = 0; i < numEntries; ++i) {
        char* key;
        MsgArg* value;
        entries[i].Get("{sv}", &key, &value);

        char* name;
        value->Get("s", &name);

        QCC_DbgPrintf(("%s: %s(%s)", __FUNCTION__, key, name));
        if ((0 == strcmp(key, "DeviceName")) && (0 == strcmp(responseMap[*responseID]->inMsg->GetMemberName(), "GetLampName"))) {
            callback.GetLampNameReplyCB(responseMap[*responseID]->inMsg, name, LSF_OK);
        } else if (0 == strcmp(key, "Manufacturer") && (0 == strcmp(responseMap[*responseID]->inMsg->GetMemberName(), "GetLampManufacturer"))) {
            callback.GetLampManufacturerReplyCB(responseMap[*responseID]->inMsg, name, LSF_OK);
        }
    }
    delete responseMap[*responseID]->context;
    responseLock.Unlock();
}

LSFResponseCode LampClients::SetLampName(const LSFString& lampID, const LSFString& name, const LSFString& language, Message& inMsg)
{
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, lampID, CONFIG_INTERFACE_NAME, "UpdateConfigurations", &LampClients::DoMethodCallAsync, static_cast<MessageReceiver::ReplyHandler>(&LampClients::UpdateConfigurationsReply));

    MsgArg name_arg("s", name.c_str());
    MsgArg arg("{sv}", "DeviceName", &name_arg);

    queuedCall->args.push_back(MsgArg("s", language.c_str()));
    queuedCall->args.push_back(MsgArg("a{sv}", 1, &arg));

    queuedCall->standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->standardReplyArgs.push_back(MsgArg("s", language.c_str()));

    return QueueLampMethod(queuedCall);
}

void LampClients::UpdateConfigurationsReply(Message& message, void* context)
{
    QCC_DbgPrintf(("UpdateConfigurationsReply: %s\n", message->ToString().c_str()));
    LSFString* responseID = static_cast<LSFString*>(context);
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();

    responseLock.Lock();

    callback.SetLampNameReplyCB(responseMap[*responseID]->inMsg, LSF_OK);

    delete responseMap[*responseID]->context;
    responseLock.Unlock();
}
