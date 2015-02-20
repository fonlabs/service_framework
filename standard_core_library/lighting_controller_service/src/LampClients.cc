/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
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

#include <alljoyn/lighting/LampClients.h>
#include <alljoyn/Status.h>
#include <alljoyn/AllJoynStd.h>
#include <qcc/Debug.h>
#include <algorithm>
#include <alljoyn/lighting/ControllerService.h>
#include <alljoyn/lighting/OEM_CS_Config.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "LAMP_CLIENTS"

lsf::Mutex l_methodCallCountMutex;
uint32_t l_methodCallCount = 0;

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
    QCC_DbgPrintf(("%s:version=%u, port=%u, busName=%s", __func__, version, port, busName));
    LSFString lampID;
    LSFString lampName;
    LSFString busname = LSFString(busName);

    ObjectDescriptions::const_iterator oit = objectDescs.find(LampServiceObjectPath);
    if (oit != objectDescs.end()) {
        AboutData::const_iterator ait = aboutData.find("DeviceId");
        if (ait != aboutData.end()) {
            const char* uniqueId;
            ait->second.Get("s", &uniqueId);
            lampID = uniqueId;
        }

        ait = aboutData.find("DeviceName");
        if (ait != aboutData.end()) {
            const char* name;
            ait->second.Get("s", &name);
            lampName = name;
        }

        QCC_DbgPrintf(("%s: About Data Dump", __func__));
        for (ait = aboutData.begin(); ait != aboutData.end(); ait++) {
            QCC_DbgPrintf(("%s: %s", ait->first.c_str(), ait->second.ToString().c_str()));
        }
    }

    if (!lampID.empty()) {
        manager.HandleAboutAnnounce(lampID, lampName, port, busname);
    }
}

void LampClients::RequestAllLampIDs(Message& message)
{
    QCC_DbgTrace(("%s", __func__));

    LSFResponseCode responseCode = LSF_OK;

    if (!connectToLamps) {
        responseCode = LSF_ERR_REJECTED;
    } else {
        QStatus status = getAllLampIDsLock.Lock();
        if (ER_OK != status) {
            QCC_LogError(ER_FAIL, ("%s: Failed to lock mutex", __func__));
            responseCode = LSF_ERR_FAILURE;
        } else {
            getAllLampIDsRequests.push_back(message);

            status = getAllLampIDsLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(ER_FAIL, ("%s: Failed to unlock mutex", __func__));
            }
            wakeUp.Post();
        }
    }

    if (LSF_OK != responseCode) {
        LSFStringList idList;
        controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(message, responseCode, idList);
    }
}

void LampClients::GetAllLamps(LampNameMap& lamps)
{
    QCC_LogError(ER_OK, ("%s: PLEASE NOTE THIS FUNCTION IS NOT THREAD SAFE", __func__));
    lamps.clear();
    LampMap tempMap = activeLamps;
    for (LampMap::const_iterator it = tempMap.begin(); it != tempMap.end(); ++it) {
        if (it->second->IsConnected()) {
            lamps.insert(std::make_pair(it->first, it->second->name));
        }
    }
}

LampClients::LampClients(ControllerService& controllerSvc)
    : Manager(controllerSvc),
    serviceHandler(new ServiceHandler(*this)),
    isRunning(false),
    lampStateChangedSignalHandlerRegistered(false),
    connectToLamps(false),
    disconnectFromLampsTimestamp(0),
    alarmTriggered(false),
    retryAlarm(this)
{
    QCC_DbgTrace(("%s", __func__));
    keyListener.SetPassCode(INITIAL_PASSCODE);
    methodQueue.clear();
    aboutsList.clear();
    getLampStateList.clear();
    activeLamps.clear();
    joinSessionCBList.clear();
    lostSessionList.clear();
    getAllLampIDsRequests.clear();
}

LampClients::~LampClients()
{
    QCC_DbgTrace(("%s", __func__));

    if (serviceHandler) {
        delete serviceHandler;
        serviceHandler = NULL;
    }

    while (methodQueue.size()) {
        QueuedMethodCall* queuedCall = methodQueue.front();
        delete queuedCall;
        methodQueue.pop_front();
    }

    aboutsListLock.Lock();
    aboutsList.clear();
    aboutsListLock.Unlock();

    getLampStateListLock.Lock();
    getLampStateList.clear();
    getLampStateListLock.Unlock();

    joinSessionCBListLock.Lock();
    joinSessionCBList.clear();
    joinSessionCBListLock.Unlock();

    lostSessionListLock.Lock();
    lostSessionList.clear();
    lostSessionListLock.Unlock();
}

static const char* interfaces[] =
{
    LampServiceInterfaceName,
    LampServiceStateInterfaceName,
    LampServiceParametersInterfaceName,
    LampServiceDetailsInterfaceName,
    ConfigServiceInterfaceName,
    AboutInterfaceName
};

QStatus LampClients::Start(const char* keyStoreFileLocation)
{
    QCC_DbgTrace(("%s", __func__));

    QCC_DbgPrintf(("%s: KeyStore location = %s", __func__, keyStoreFileLocation));
    QStatus status = controllerService.GetBusAttachment().EnablePeerSecurity("ALLJOYN_PIN_KEYX ALLJOYN_ECDHE_PSK", &keyListener, keyStoreFileLocation);
    QCC_DbgPrintf(("EnablePeerSecurity(): %s\n", QCC_StatusText(status)));

    if (ER_OK == status) {
        status = RegisterAnnounceHandler();
    }

    if (ER_OK == status) {
        isRunning = true;
        status = Thread::Start();
        QCC_DbgPrintf(("%s: Thread::Start(): %s\n", __func__, QCC_StatusText(status)));
    }

    return status;
}

void LampClients::Join(void)
{
    QCC_DbgTrace(("%s", __func__));

    retryAlarm.Join();

    Thread::Join();

    for (LampMap::iterator it = activeLamps.begin(); it != activeLamps.end(); ++it) {
        LampConnection* conn = it->second;
        if (conn->sessionID) {
            controllerService.DoLeaveSessionAsync(conn->object.GetSessionId());
        }
        conn->Clear();
    }
    activeLamps.clear();
}

void LampClients::Stop(void)
{
    QCC_DbgTrace(("%s", __func__));
    retryAlarm.Stop();
    DisconnectFromLamps();
    isRunning = false;
    wakeUp.Post();
}

void LampClients::HandleAboutAnnounce(LSFString& lampID, LSFString& lampName, uint16_t& port, LSFString& busName)
{
    QCC_DbgPrintf(("LampClients::HandleAboutAnnounce(%s,%s,%u,%s)\n", lampID.c_str(), lampName.c_str(), port, busName.c_str()));
    LampConnection* connection = new LampConnection();
    if (connection) {
        connection->Set(lampID, busName, lampName, port);

        QStatus status = aboutsListLock.Lock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: aboutsListLock.Lock() failed", __func__));
            connection->Clear();
            return;
        }

        LampMap::iterator it = aboutsList.find(lampID);
        if (it != aboutsList.end()) {
            QCC_DbgPrintf(("%s: Got another announcement for a lamp that we already know about", __func__));
            it->second->Clear();
            it->second = connection;
            wakeUp.Post();
        } else {
            if (aboutsList.size() < (OEM_CS_MAX_SUPPORTED_LAMPS)) {
                aboutsList.insert(std::make_pair(lampID, connection));
                wakeUp.Post();
            } else {
                QCC_LogError(status, ("%s: Controller Service can cache only a maximum of %d announcements. Max'ed out on the capacity. Ignoring the announcement", __func__, (OEM_CS_MAX_SUPPORTED_LAMPS)));
                connection->Clear();
            }
        }
        status = aboutsListLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: aboutsListLock.Unlock() failed", __func__));
        }
    } else {
        QCC_LogError(ER_FAIL, ("%s: Could not allocate memory for new LampConnection", __func__));
    }
}

void LampClients::ConnectToLamps(void)
{
    QCC_DbgTrace(("%s", __func__));
    connectToLamps = true;
    wakeUp.Post();
}

void LampClients::DisconnectFromLamps(void)
{
    QCC_DbgTrace(("%s", __func__));
    disconnectFromLampsTimestamp = GetTimestampInSeconds();
    connectToLamps = false;
    wakeUp.Post();
}

void LampClients::JoinSessionCB(QStatus status, SessionId sessionId, const SessionOpts& opts, void* context)
{
    QCC_DbgTrace(("%s:status=%s sessionId=%u", __func__, QCC_StatusText(status), sessionId));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();

    if (!context) {
        QCC_LogError(ER_FAIL, ("%s: No context", __func__));
        return;
    }

    LampConnection* connection = static_cast<LampConnection*>(context);

    QStatus tempStatus = ER_OK;

    if (!connectToLamps) {
        QCC_DbgPrintf(("%s: connectToLamps is false", __func__));
        tempStatus = ER_FAIL;
    } else {
        if (status != ER_OK) {
            QCC_LogError(status, ("%s: Join Session failed for lamp with ID = %s", __func__, connection->lampId.c_str()));
            tempStatus = joinSessionCBListLock.Lock();
            if (ER_OK != tempStatus) {
                QCC_LogError(tempStatus, ("%s: joinSessionCBListLock.Lock() failed", __func__));
            } else {
                joinSessionCBList.insert(std::make_pair(connection->lampId, status));
                QCC_DbgPrintf(("%s: Add %s to joinSessionCBList", __func__, connection->lampId.c_str()));
                if (ER_OK != joinSessionCBListLock.Unlock()) {
                    QCC_LogError(tempStatus, ("%s: joinSessionCBListLock.Unlock() failed", __func__));
                }
                wakeUp.Post();
            }
            return;
        }

        QCC_DbgPrintf(("%s: status = %s, sessionId = %u\n", __func__, QCC_StatusText(status), sessionId));
        QCC_DbgPrintf(("New connection to lamp ID [%s]\n", connection->lampId.c_str()));

        connection->InitializeSessionAndObjects(controllerService.GetBusAttachment(), sessionId);

        QCC_DbgPrintf(("%s: Invoking IntrospectRemoteObjectAsync\n", __func__));
        tempStatus = connection->object.IntrospectRemoteObjectAsync(this, static_cast<ProxyBusObject::Listener::IntrospectCB>(&LampClients::IntrospectCB), context);
        QCC_DbgPrintf(("%s: IntrospectRemoteObjectAsync returns %s\n", __func__, QCC_StatusText(tempStatus)));
    }

    if (ER_OK != tempStatus) {
        /*
         * Tear down the session if the object setup was unsuccessful
         */
        controllerService.DoLeaveSessionAsync(sessionId);
        tempStatus = joinSessionCBListLock.Lock();
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: joinSessionCBListLock.Lock() failed", __func__));
        } else {
            joinSessionCBList.insert(std::make_pair(connection->lampId, tempStatus));
            QCC_DbgPrintf(("%s: Add %s to joinSessionCBList", __func__, connection->lampId.c_str()));
            if (ER_OK != joinSessionCBListLock.Unlock()) {
                QCC_LogError(tempStatus, ("%s: joinSessionCBListLock.Unlock() failed", __func__));
            }
            wakeUp.Post();
        }
    }
}

void LampClients::SessionLost(SessionId sessionId, SessionLostReason reason)
{
    QCC_DbgPrintf(("%s: sessionId=0x%x reason=0x%x\n", __func__, sessionId, reason));

    if (!connectToLamps) {
        QCC_DbgPrintf(("%s: connectToLamps is false", __func__));
        return;
    }

    QStatus status = lostSessionListLock.Lock();
    if (ER_OK != status) {
        QCC_LogError(status, ("%s: lostSessionListLock.Lock() failed", __func__));
        return;
    }
    lostSessionList.insert((uint32_t)sessionId);
    status = lostSessionListLock.Unlock();
    if (ER_OK != status) {
        QCC_LogError(status, ("%s: lostSessionListLock.Unlock() failed", __func__));
    }
    wakeUp.Post();
}

void LampClients::SendMethodReply(LSFResponseCode responseCode, ajn::Message msg, std::list<ajn::MsgArg>& stdArgs, std::list<ajn::MsgArg>& custArgs)
{
    QCC_DbgPrintf(("%s\n", __func__));
    size_t numArgs = stdArgs.size() + custArgs.size() + 1;
    QCC_DbgPrintf(("%s: NumArgs = %d", __func__, numArgs));
    MsgArg* args = new MsgArg[numArgs];
    if (args) {
        args[0] = MsgArg("u", responseCode);
        args[0].SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs, true);
        uint8_t index = 1;
        while (stdArgs.size()) {
            args[index] = stdArgs.front();
            args[index].SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs, true);
            stdArgs.pop_front();
            index++;
        }
        while (custArgs.size()) {
            args[index] = custArgs.front();
            args[index].SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs, true);
            custArgs.pop_front();
            index++;
        }
        controllerService.SendMethodReply(msg, args, numArgs);
        delete [] args;
    } else {
        QCC_LogError(ER_OUT_OF_MEMORY, ("%s: Failed to allocate enough memory", __func__));
    }
}

void LampClients::QueueLampMethod(QueuedMethodCall* queuedCall)
{
    QCC_DbgPrintf(("%s", __func__));
    LSFResponseCode responseCode = LSF_OK;

    if (!connectToLamps) {
        QCC_DbgPrintf(("%s: connectToLamps is false", __func__));
        responseCode = LSF_ERR_REJECTED;
    } else {
        QStatus status = queueLock.Lock();
        if (status != ER_OK) {
            QCC_LogError(status, ("%s: queueLock.Lock() failed or doNotQueue set", __func__));
            responseCode = LSF_ERR_BUSY;
        } else {
            if (methodQueue.size() < OEM_CS_MAX_LAMP_CLIENTS_METHOD_QUEUE_SIZE) {
                queuedCall->methodCallCount = 0;

                l_methodCallCountMutex.Lock();
                l_methodCallCount++;
                queuedCall->methodCallCount = l_methodCallCount;
                l_methodCallCountMutex.Unlock();


                QCC_DbgPrintf(("%s: Queuing Method call %s with method call count %u", __func__, queuedCall->inMsg->GetMemberName(), queuedCall->methodCallCount));

                methodQueue.push_back(queuedCall);
            } else {
                responseCode = LSF_ERR_NO_SLOT;
                QCC_LogError(ER_OUT_OF_MEMORY, ("%s: No slot for new method call", __func__));
            }

            status = queueLock.Unlock();
            if (status != ER_OK) {
                QCC_LogError(status, ("%s: queueLock.Unlock() failed", __func__));
            }
        }
    }

    if (LSF_OK == responseCode) {
        QCC_DbgPrintf(("%s: Adding response counter with ID=%s to response map", __func__, queuedCall->responseID.c_str()));
        responseLock.Lock();
        responseMap.insert(std::make_pair(queuedCall->responseID, queuedCall->responseCounter));
        responseLock.Unlock();
        wakeUp.Post();
    } else {
        if (strstr(queuedCall->inMsg->GetInterface(), ApplySceneEventActionInterfaceName)) {
            QCC_DbgPrintf(("%s: Skipping the sending of reply because interface = %s", __func__, queuedCall->inMsg->GetInterface()));
        } else {
            SendMethodReply(responseCode, queuedCall->inMsg, queuedCall->responseCounter.standardReplyArgs, queuedCall->responseCounter.customReplyArgs);
        }
        delete queuedCall;
    }
}

LSFResponseCode LampClients::DoMethodCallAsync(QueuedMethodCall* queuedCall)
{
    QCC_DbgPrintf(("%s", __func__));
    LSFResponseCode responseCode = LSF_OK;
    QStatus status = ER_OK;
    uint32_t notFound = 0;
    uint32_t failures = 0;
    QueuedMethodCallElementList elementList = queuedCall->methodCallElements;
    QueuedMethodCallContext* ctx = NULL;

    while (elementList.size()) {
        QueuedMethodCallElement element = elementList.front();
        LSFStringList lamps = element.lamps;

        for (LSFStringList::const_iterator it = lamps.begin(); it != lamps.end(); it++) {
            QCC_DbgPrintf(("%s: Processing for LampID=%s", __func__, (*it).c_str()));
            LampMap::iterator lit = activeLamps.find(*it);
            if (lit != activeLamps.end()) {
                QCC_DbgPrintf(("%s: Found Lamp", __func__));
                ctx = NULL;
                if (lit->second->IsConnected()) {
                    ctx = new QueuedMethodCallContext(*it, queuedCall, element.method);
                    if (!ctx) {
                        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for context", __func__));
                        status = ER_FAIL;
                    } else {
                        ctx->timeSent = GetTimestampInMs();
                        ctx->method = element.method;
                        if (0 == strcmp(element.interface.c_str(), ConfigServiceInterfaceName)) {
                            QCC_DbgPrintf(("%s: Config Call", __func__));
                            QCC_DbgPrintf(("%s: Calling %s on lamp %s for method call %s and count %u", __func__,
                                           element.method.c_str(), (*it).c_str(), queuedCall->inMsg->GetMemberName(), queuedCall->methodCallCount));
                            status = lit->second->configObject.MethodCallAsync(
                                element.interface.c_str(),
                                element.method.c_str(),
                                this,
                                queuedCall->replyFunc,
                                &element.args[0],
                                element.args.size(),
                                ctx,
                                OEM_CS_LAMP_METHOD_CALL_TIMEOUT
                                );
                        } else if (0 == strcmp(element.interface.c_str(), AboutInterfaceName)) {
                            QCC_DbgPrintf(("%s: About Call", __func__));
                            QCC_DbgPrintf(("%s: Calling %s on lamp %s for method call %s and count %u", __func__,
                                           element.method.c_str(), (*it).c_str(), queuedCall->inMsg->GetMemberName(), queuedCall->methodCallCount));
                            status = lit->second->aboutObject.MethodCallAsync(
                                element.interface.c_str(),
                                element.method.c_str(),
                                this,
                                queuedCall->replyFunc,
                                &element.args[0],
                                element.args.size(),
                                ctx,
                                OEM_CS_LAMP_METHOD_CALL_TIMEOUT
                                );
                        } else {
                            QCC_DbgPrintf(("%s: LampService Call", __func__));
                            QCC_DbgPrintf(("%s: Calling %s on lamp %s for method call %s and count %u", __func__,
                                           element.method.c_str(), (*it).c_str(), queuedCall->inMsg->GetMemberName(), queuedCall->methodCallCount));
                            status = lit->second->object.MethodCallAsync(
                                element.interface.c_str(),
                                element.method.c_str(),
                                this,
                                queuedCall->replyFunc,
                                &element.args[0],
                                element.args.size(),
                                ctx,
                                OEM_CS_LAMP_METHOD_CALL_TIMEOUT
                                );
                        }
                    }
                } else {
                    QCC_DbgPrintf(("%s:Not connected to lamp", __func__));
                    status = ER_FAIL;
                }

                if (status != ER_OK) {
                    QCC_LogError(status, ("%s: MethodCallAsync failed", __func__));
                    if (ctx) {
                        delete ctx;
                    }
                    failures++;
                } else {
                    lit->second->pendingMethodCallCount++;
                    QCC_DbgPrintf(("%s: Increased pendingMethodCallCount for lamp %s to %u", __func__, lit->first.c_str(), lit->second->pendingMethodCallCount));
                }
            } else {
                notFound++;
            }
        }

        elementList.pop_front();
    }

    if (notFound || failures) {
        DecrementWaitingAndSendResponse(queuedCall, 0, failures, notFound);
    }

    return responseCode;
}

LSFResponseCode LampClients::DoGetLampState(QueuedMethodCallContext* ctx)
{
    QCC_DbgPrintf(("%s", __func__));
    LSFResponseCode responseCode = LSF_OK;
    QStatus status = ER_OK;

    MsgArg arg("s", LampServiceStateInterfaceName);

    QCC_DbgPrintf(("%s: Processing for LampID=%s", __func__, ctx->lampID.c_str()));
    LampMap::iterator lit = activeLamps.find(ctx->lampID);
    if (lit != activeLamps.end()) {
        QCC_DbgPrintf(("%s: Found Lamp", __func__));
        if (lit->second->IsConnected()) {
            ctx->timeSent = GetTimestampInMs();
            QCC_DbgPrintf(("%s: LampService Call", __func__));
            status = lit->second->object.MethodCallAsync(
                org::freedesktop::DBus::Properties::InterfaceName,
                "GetAll",
                this,
                static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleGetLampStateReply),
                &arg,
                1,
                ctx,
                OEM_CS_LAMP_METHOD_CALL_TIMEOUT
                );
        } else {
            QCC_DbgPrintf(("%s:Not connected to lamp", __func__));
            status = ER_FAIL;
        }

        if (status != ER_OK) {
            QCC_LogError(status, ("%s: MethodCallAsync failed", __func__));
            if (ctx) {
                delete ctx;
            }
        } else {
            lit->second->pendingMethodCallCount++;
            QCC_DbgPrintf(("%s: Increased pendingMethodCallCount for lamp %s to %u", __func__, lit->first.c_str(), lit->second->pendingMethodCallCount));
        }
    }

    return responseCode;
}

void LampClients::HandleGetLampStateReply(ajn::Message& message, void* context)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    QueuedMethodCallContext* ctx = static_cast<QueuedMethodCallContext*>(context);

    if (ctx == NULL) {
        QCC_LogError(ER_FAIL, ("%s: Received NULL context", __func__));
        return;
    }

    QCC_DbgTrace(("%s: Received reply to call %s on lamp %s in %lu msec", __func__,
                  ctx->method.c_str(), ctx->lampID.c_str(), (GetTimestampInMs() - ctx->timeSent)));

    if (MESSAGE_METHOD_RET == message->GetType()) {
        size_t numArgs;
        const MsgArg* args;
        message->GetArgs(numArgs, args);

        if (numArgs == 1) {
            LampState state(args[0]);
            controllerService.SendStateChangedSignal(ControllerServiceLampInterfaceName, "LampStateChanged", ctx->lampID, state);
        } else {
            QCC_LogError(ER_BAD_ARG_COUNT, ("%s: Did not receive the expected number of arguments in the method reply", __func__));
        }
    }

    delete ctx;
}

void LampClients::GetLampState(const LSFString& lampID, Message& inMsg)
{
    QCC_DbgTrace(("%s", __func__));
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleGetReply));
    if (!queuedCall) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return;
    }
    QueuedMethodCallElement element = QueuedMethodCallElement(lampID, org::freedesktop::DBus::Properties::InterfaceName, "GetAll");
    element.args.push_back(MsgArg("s", LampServiceStateInterfaceName));
    queuedCall->AddMethodCallElement(element);
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->responseCounter.customReplyArgs.push_back(MsgArg("a{sv}", 0, NULL));
    QueueLampMethod(queuedCall);
}

void LampClients::GetLampStateField(const LSFString& lampID, const LSFString& field, Message& inMsg)
{
    QCC_DbgTrace(("%s", __func__));
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleGetReply));
    if (!queuedCall) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return;
    }
    QueuedMethodCallElement element = QueuedMethodCallElement(lampID, org::freedesktop::DBus::Properties::InterfaceName, "Get");
    element.args.push_back(MsgArg("s", LampServiceStateInterfaceName));
    element.args.push_back(MsgArg("s", field.c_str()));
    queuedCall->AddMethodCallElement(element);
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", field.c_str()));
    MsgArg arg("u", 0);
    queuedCall->responseCounter.customReplyArgs.push_back(MsgArg("v", &arg));
    QueueLampMethod(queuedCall);
}

void LampClients::GetLampDetails(const LSFString& lampID, Message& inMsg)
{
    QCC_DbgTrace(("%s", __func__));
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleGetReply));
    if (!queuedCall) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return;
    }
    QueuedMethodCallElement element = QueuedMethodCallElement(lampID, org::freedesktop::DBus::Properties::InterfaceName, "GetAll");
    element.args.push_back(MsgArg("s", LampServiceDetailsInterfaceName));
    queuedCall->AddMethodCallElement(element);
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->responseCounter.customReplyArgs.push_back(MsgArg("a{sv}", 0, NULL));
    QueueLampMethod(queuedCall);
}

void LampClients::GetLampParameters(const LSFString& lampID, Message& inMsg)
{
    QCC_DbgTrace(("%s", __func__));
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleGetReply));
    if (!queuedCall) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return;
    }
    QueuedMethodCallElement element = QueuedMethodCallElement(lampID, org::freedesktop::DBus::Properties::InterfaceName, "GetAll");
    element.args.push_back(MsgArg("s", LampServiceParametersInterfaceName));
    queuedCall->AddMethodCallElement(element);
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->responseCounter.customReplyArgs.push_back(MsgArg("a{sv}", 0, NULL));
    QueueLampMethod(queuedCall);
}

void LampClients::GetLampParametersField(const LSFString& lampID, const LSFString& field, Message& inMsg)
{
    QCC_DbgTrace(("%s", __func__));
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleGetReply));
    if (!queuedCall) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return;
    }
    QueuedMethodCallElement element = QueuedMethodCallElement(lampID, org::freedesktop::DBus::Properties::InterfaceName, "Get");
    element.args.push_back(MsgArg("s", LampServiceParametersInterfaceName));
    element.args.push_back(MsgArg("s", field.c_str()));
    queuedCall->AddMethodCallElement(element);
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", field.c_str()));
    MsgArg arg("u", 0);
    queuedCall->responseCounter.customReplyArgs.push_back(MsgArg("v", &arg));
    QueueLampMethod(queuedCall);
}

void LampClients::HandleGetReply(ajn::Message& message, void* context)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    QueuedMethodCallContext* ctx = static_cast<QueuedMethodCallContext*>(context);

    if (ctx == NULL) {
        QCC_LogError(ER_FAIL, ("%s: Received NULL context", __func__));
        return;
    }

    QueuedMethodCall* queuedCall = ctx->queuedCallPtr;

    QCC_DbgTrace(("%s: Received reply to call %s on lamp %s in %lu msec", __func__,
                  ctx->method.c_str(), ctx->lampID.c_str(), (GetTimestampInMs() - ctx->timeSent)));

    if (MESSAGE_METHOD_RET == message->GetType()) {
        size_t numArgs;
        const MsgArg* args;
        message->GetArgs(numArgs, args);

        if (numArgs == 0) {
            DecrementWaitingAndSendResponse(queuedCall, 1, 0, 0);
        } else if (numArgs == 1) {
            DecrementWaitingAndSendResponse(queuedCall, 1, 0, 0, args);
        } else {
            QCC_LogError(ER_BAD_ARG_COUNT, ("%s: Did not receive the expected number of arguments in the method reply", __func__));
            DecrementWaitingAndSendResponse(queuedCall, 0, 1, 0);
        }
    } else {
        DecrementWaitingAndSendResponse(queuedCall, 0, 1, 0);
    }

    delete ctx;
}

void LampClients::ChangeLampState(const ajn::Message& inMsg, bool groupOperation, bool sceneOperation, TransitionStateParamsList& transitionStateParams,
                                  TransitionStateFieldParamsList& transitionStateFieldparams, PulseStateParamsList& pulseParams, LSFString sceneOrMasterSceneID)
{
    QCC_DbgTrace(("%s", __func__));
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleReplyWithLampResponseCode));

    if (!queuedCall) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return;
    }

    if (groupOperation || (sceneOperation && ((0 == strcmp(ControllerServiceSceneInterfaceName, inMsg->GetInterface())) || (0 == strcmp(ControllerServiceMasterSceneInterfaceName, inMsg->GetInterface()))))) {
        size_t numArgs;
        const MsgArg* args;
        Message tempMsg = inMsg;
        tempMsg->GetArgs(numArgs, args);
        queuedCall->responseCounter.standardReplyArgs.push_back(args[0]);
    }

    while (transitionStateFieldparams.size()) {
        bool firstIteration = true;
        TransitionStateFieldParams transitionStateFieldParam = transitionStateFieldparams.front();

        if (firstIteration) {
            if (!groupOperation && !sceneOperation) {
                queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", transitionStateFieldParam.lamps.front().c_str()));
            }

            if (!sceneOperation) {
                queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", transitionStateFieldParam.field));
            }
            firstIteration = false;
        }

        QueuedMethodCallElement element = QueuedMethodCallElement(transitionStateFieldParam.lamps, LampServiceStateInterfaceName, "TransitionLampState");
        MsgArg* arrayVals = new MsgArg[1];
        arrayVals[0].Set("{sv}", strdupnew(transitionStateFieldParam.field), new MsgArg(transitionStateFieldParam.value));
        arrayVals[0].SetOwnershipFlags(MsgArg::OwnsArgs | MsgArg::OwnsData);

        element.args.push_back(MsgArg("t", transitionStateFieldParam.timestamp));
        element.args.push_back(MsgArg("a{sv}", 1, arrayVals));
        element.args.push_back(MsgArg("u", transitionStateFieldParam.period));
        queuedCall->AddMethodCallElement(element);

        transitionStateFieldparams.pop_front();
    }

    while (transitionStateParams.size()) {
        bool firstIteration = true;
        TransitionStateParams transitionStateParam = transitionStateParams.front();

        if (firstIteration) {
            if (!groupOperation && !sceneOperation) {
                queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", transitionStateParam.lamps.front().c_str()));
            }
            firstIteration = false;
        }

        QueuedMethodCallElement element = QueuedMethodCallElement(transitionStateParam.lamps, LampServiceStateInterfaceName, "TransitionLampState");

        element.args.push_back(MsgArg("t", transitionStateParam.timestamp));
        element.args.push_back(transitionStateParam.state);
        element.args.push_back(MsgArg("u", transitionStateParam.period));
        queuedCall->AddMethodCallElement(element);

        transitionStateParams.pop_front();
    }

    while (pulseParams.size()) {
        bool firstIteration = true;
        PulseStateParams pulseParam = pulseParams.front();

        if (firstIteration) {
            if (!groupOperation && !sceneOperation) {
                queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", pulseParam.lamps.front().c_str()));
            }
            firstIteration = false;
        }

        QueuedMethodCallElement element = QueuedMethodCallElement(pulseParam.lamps, LampServiceStateInterfaceName, "ApplyPulseEffect");

        element.args.push_back(pulseParam.oldState);
        element.args.push_back(pulseParam.newState);
        element.args.push_back(MsgArg("u", pulseParam.period));
        element.args.push_back(MsgArg("u", pulseParam.duration));
        element.args.push_back(MsgArg("u", pulseParam.numPulses));
        element.args.push_back(MsgArg("t", pulseParam.timestamp));
        queuedCall->AddMethodCallElement(element);

        pulseParams.pop_front();
    }

    if (!sceneOrMasterSceneID.empty()) {
        QCC_DbgPrintf(("%s: Recording sceneOrMasterSceneID %s", __func__, sceneOrMasterSceneID.c_str()));
        queuedCall->responseCounter.sceneOrMasterSceneID = sceneOrMasterSceneID;
    }

    QueueLampMethod(queuedCall);
}

void LampClients::DecrementWaitingAndSendResponse(QueuedMethodCall* queuedCall, uint32_t success, uint32_t failure, uint32_t notFound, const ajn::MsgArg* arg)
{
    QCC_DbgPrintf(("%s: responseID=%s ", __func__, queuedCall->responseID.c_str()));
    LSFResponseCode responseCode = LSF_ERR_UNEXPECTED;
    ResponseCounter responseCounter;
    bool sendResponse = false;

    responseLock.Lock();

    ResponseMap::iterator it = responseMap.find(queuedCall->responseID);

    if (it != responseMap.end()) {
        it->second.notFoundCount += notFound;
        it->second.numWaiting -= notFound;
        it->second.successCount += success;
        it->second.numWaiting -= success;
        it->second.failCount += failure;
        it->second.numWaiting -= failure;

        if (arg) {
            it->second.customReplyArgs.clear();
            it->second.customReplyArgs.push_back(arg[0]);
        }

        if (it->second.numWaiting == 0) {
            if (it->second.notFoundCount == it->second.total) {
                responseCode = LSF_ERR_NOT_FOUND;
                QCC_DbgPrintf(("%s: Response is LSF_ERR_NOT_FOUND for method %s", __func__, queuedCall->inMsg->GetMemberName()));
            } else if (it->second.successCount == it->second.total) {
                responseCode = LSF_OK;
                QCC_DbgPrintf(("%s: Response is LSF_OK for method %s", __func__, queuedCall->inMsg->GetMemberName()));
            } else if (it->second.failCount == it->second.total) {
                responseCode = LSF_ERR_FAILURE;
                QCC_DbgPrintf(("%s: Response is LSF_ERR_FAILURE for method %s", __func__, queuedCall->inMsg->GetMemberName()));
            } else if ((it->second.notFoundCount + it->second.successCount + it->second.failCount) == it->second.total) {
                responseCode = LSF_ERR_PARTIAL;
                QCC_DbgPrintf(("%s: Response is LSF_ERR_PARTIAL for method %s", __func__, queuedCall->inMsg->GetMemberName()));
            }

            responseCounter = it->second;
            responseMap.erase(it);
            sendResponse = true;
        }
    } else {
        QCC_LogError(ER_FAIL, ("%s: Response map entry not found for response ID = %s", __func__, queuedCall->responseID.c_str()));
    }
    responseLock.Unlock();

    if (sendResponse) {
        if (strstr(queuedCall->inMsg->GetInterface(), ApplySceneEventActionInterfaceName)) {
            QCC_DbgPrintf(("%s: Skipping the sending of reply because interface = %s", __func__, queuedCall->inMsg->GetInterface()));
        } else {
            QCC_DbgPrintf(("%s: Sending reply %s for method call %s and count %u", __func__,
                           LSFResponseCodeText(responseCode), queuedCall->inMsg->GetMemberName(), queuedCall->methodCallCount));
            SendMethodReply(responseCode, queuedCall->inMsg, responseCounter.standardReplyArgs, responseCounter.customReplyArgs);
        }

        if ((!responseCounter.sceneOrMasterSceneID.empty()) && ((responseCode == LSF_OK) || (responseCode == LSF_ERR_PARTIAL))) {
            controllerService.SendSceneOrMasterSceneAppliedSignal(responseCounter.sceneOrMasterSceneID);
        }

        delete queuedCall;
    }
}

void LampClients::HandleReplyWithLampResponseCode(Message& message, void* context)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    QueuedMethodCallContext* ctx = static_cast<QueuedMethodCallContext*>(context);

    if (ctx == NULL) {
        QCC_LogError(ER_FAIL, ("%s: Received NULL context", __func__));
        return;
    }

    QCC_DbgTrace(("%s: Received reply to call %s on lamp %s in %lu msec", __func__,
                  ctx->method.c_str(), ctx->lampID.c_str(), (GetTimestampInMs() - ctx->timeSent)));

    QueuedMethodCall* queuedCall = ctx->queuedCallPtr;

    QCC_DbgPrintf(("%s: Got %s for method call %s from lamp %s for lamp method call %s and count %u", __func__,
                   ((MESSAGE_METHOD_RET == message->GetType()) ? "REPLY" : "ERROR"), queuedCall->inMsg->GetMemberName(), ctx->lampID.c_str(), ctx->method.c_str(),
                   queuedCall->methodCallCount));

    if (MESSAGE_METHOD_RET == message->GetType()) {
        size_t numArgs;
        const MsgArg* args;
        message->GetArgs(numArgs, args);

        uint32_t success = 0;
        uint32_t failure = 0;

        if (((numArgs == 1) && (ctx->method != "ClearLampFault")) || ((numArgs == 2) && (ctx->method == "ClearLampFault"))) {
            LampResponseCode responseCode;
            args[0].Get("u", &responseCode);

            if (responseCode == LAMP_OK) {
                success++;
            } else {
                failure++;
            }
        } else {
            QCC_LogError(ER_BAD_ARG_COUNT, ("%s: Did not receive the expected number of arguments in the method reply", __func__));
            failure++;
        }

        DecrementWaitingAndSendResponse(queuedCall, success, failure, 0);
    } else {
        DecrementWaitingAndSendResponse(queuedCall, 0, 1, 0);
    }

    delete ctx;
}

void LampClients::GetLampFaults(const LSFString& lampID, ajn::Message& inMsg)
{
    QCC_DbgTrace(("%s", __func__));
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleReplyWithVariant));
    if (!queuedCall) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return;
    }
    QueuedMethodCallElement element = QueuedMethodCallElement(lampID, org::freedesktop::DBus::Properties::InterfaceName, "Get");
    element.args.push_back(MsgArg("s", LampServiceInterfaceName));
    element.args.push_back(MsgArg("s", "LampFaults"));
    queuedCall->AddMethodCallElement(element);
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->responseCounter.customReplyArgs.push_back(MsgArg("au", 0, NULL));
    QueueLampMethod(queuedCall);
}

void LampClients::GetLampVersion(const LSFString& lampID, ajn::Message& inMsg)
{
    QCC_DbgTrace(("%s", __func__));
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleReplyWithVariant));
    if (!queuedCall) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return;
    }
    QueuedMethodCallElement element = QueuedMethodCallElement(lampID, org::freedesktop::DBus::Properties::InterfaceName, "Get");
    element.args.push_back(MsgArg("s", LampServiceInterfaceName));
    element.args.push_back(MsgArg("s", "LampServiceVersion"));
    queuedCall->AddMethodCallElement(element);
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->responseCounter.customReplyArgs.push_back(MsgArg("u", 0, NULL));
    QueueLampMethod(queuedCall);
}

void LampClients::HandleReplyWithVariant(ajn::Message& message, void* context)
{
    QCC_DbgTrace(("%s", __func__));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    QueuedMethodCallContext* ctx = static_cast<QueuedMethodCallContext*>(context);

    if (ctx == NULL) {
        QCC_LogError(ER_FAIL, ("%s: Received NULL context", __func__));
        return;
    }

    QueuedMethodCall* queuedCall = ctx->queuedCallPtr;

    QCC_DbgTrace(("%s: Received reply to call %s on lamp %s in %lu msec", __func__,
                  ctx->method.c_str(), ctx->lampID.c_str(), (GetTimestampInMs() - ctx->timeSent)));

    if (MESSAGE_METHOD_RET == message->GetType()) {
        size_t numArgs;
        const MsgArg* args;
        message->GetArgs(numArgs, args);

        if (numArgs == 1) {
            MsgArg* verArg;
            args[0].Get("v", &verArg);
            DecrementWaitingAndSendResponse(queuedCall, 1, 0, 0, verArg);
        } else {
            QCC_LogError(ER_BAD_ARG_COUNT, ("%s: Did not receive the expected number of arguments in the method reply", __func__));
            DecrementWaitingAndSendResponse(queuedCall, 0, 1, 0);
        }
    } else {
        DecrementWaitingAndSendResponse(queuedCall, 0, 1, 0);
    }

    delete ctx;
}

void LampClients::ClearLampFault(const LSFString& lampID, LampFaultCode faultCode, ajn::Message& inMsg)
{
    QCC_DbgTrace(("%s", __func__));
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleReplyWithLampResponseCode));
    if (!queuedCall) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return;
    }
    QueuedMethodCallElement element = QueuedMethodCallElement(lampID, LampServiceInterfaceName, "ClearLampFault");
    element.args.push_back(MsgArg("u", faultCode));
    queuedCall->AddMethodCallElement(element);

    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("u", faultCode));

    QueueLampMethod(queuedCall);
}

void LampClients::LampStateChangedSignalHandler(const InterfaceDescription::Member* member, const char* sourcePath, Message& message)
{
    QCC_DbgTrace(("%s", __func__));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();

    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    const char* uniqueId;
    args[0].Get("s", &uniqueId);

    QueuedMethodCallContext* ctx = new QueuedMethodCallContext(uniqueId, "GetAll");
    if (!ctx) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
    } else {
        getLampStateListLock.Lock();
        getLampStateList.push_back(ctx);
        getLampStateListLock.Unlock();
        QCC_DbgPrintf(("%s: Queued a GetLampState call to lamp %s in response to a LampStateChangedSignal", __func__, uniqueId));
        wakeUp.Post();
    }
}

void LampClients::GetLampSupportedLanguages(const LSFString& lampID, ajn::Message& inMsg)
{
    QCC_DbgTrace(("%s", __func__));
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleReplyWithKeyValuePairs));
    if (!queuedCall) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return;
    }
    QueuedMethodCallElement element = QueuedMethodCallElement(lampID, AboutInterfaceName, "GetAboutData");
    element.args.push_back(MsgArg("s", "en"));
    queuedCall->AddMethodCallElement(element);

    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->responseCounter.customReplyArgs.push_back(MsgArg("as", 0, NULL));

    QueueLampMethod(queuedCall);
}

void LampClients::HandleReplyWithKeyValuePairs(Message& message, void* context)
{
    QCC_DbgPrintf(("%s: Method Reply %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    QueuedMethodCallContext* ctx = static_cast<QueuedMethodCallContext*>(context);

    if (ctx == NULL) {
        QCC_LogError(ER_FAIL, ("%s: Received NULL context", __func__));
        return;
    }

    QueuedMethodCall* queuedCall = ctx->queuedCallPtr;

    QCC_DbgTrace(("%s: Received reply to call %s on lamp %s in %lu msec", __func__,
                  ctx->method.c_str(), ctx->lampID.c_str(), (GetTimestampInMs() - ctx->timeSent)));

    if (MESSAGE_METHOD_RET == message->GetType()) {
        const MsgArg* args;
        size_t numArgs;
        message->GetArgs(numArgs, args);

        if (numArgs == 1) {
            size_t numEntries;
            MsgArg* entries;

            args[0].Get("a{sv}", &numEntries, &entries);
            for (size_t i = 0; i < numEntries; ++i) {
                char* key;
                MsgArg* value;
                entries[i].Get("{sv}", &key, &value);
                QCC_DbgPrintf(("%s: %s", __func__, key));
                if (((0 == strcmp(key, "SupportedLanguages")) && (0 == strcmp(queuedCall->inMsg->GetMemberName(), "GetLampSupportedLanguages"))) ||
                    ((0 == strcmp(key, "DeviceName")) && (0 == strcmp(queuedCall->inMsg->GetMemberName(), "GetLampName"))) ||
                    ((0 == strcmp(key, "Manufacturer")) && (0 == strcmp(queuedCall->inMsg->GetMemberName(), "GetLampManufacturer")))) {
                    DecrementWaitingAndSendResponse(queuedCall, 1, 0, 0, value);
                    break;
                }
            }
        } else {
            QCC_LogError(ER_BAD_ARG_COUNT, ("%s: Did not receive the expected number of arguments in the method reply", __func__));
            DecrementWaitingAndSendResponse(queuedCall, 0, 1, 0);
        }
    } else {
        DecrementWaitingAndSendResponse(queuedCall, 0, 1, 0);
    }

    delete ctx;
}

void LampClients::GetLampName(const LSFString& lampID, const LSFString& language, Message& inMsg)
{
    QCC_DbgTrace(("%s", __func__));
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleReplyWithKeyValuePairs));
    if (!queuedCall) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return;
    }
    QueuedMethodCallElement element = QueuedMethodCallElement(lampID, ConfigServiceInterfaceName, "GetConfigurations");
    element.args.push_back(MsgArg("s", language.c_str()));
    queuedCall->AddMethodCallElement(element);
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", language.c_str()));
    queuedCall->responseCounter.customReplyArgs.push_back(MsgArg("s", "<ERROR>"));
    QueueLampMethod(queuedCall);
}

void LampClients::GetLampManufacturer(const LSFString& lampID, const LSFString& language, ajn::Message& inMsg)
{
    QCC_DbgTrace(("%s", __func__));
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleReplyWithKeyValuePairs));
    if (!queuedCall) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return;
    }
    QueuedMethodCallElement element = QueuedMethodCallElement(lampID, ConfigServiceInterfaceName, "GetConfigurations");
    element.args.push_back(MsgArg("s", language.c_str()));
    queuedCall->AddMethodCallElement(element);
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", language.c_str()));
    queuedCall->responseCounter.customReplyArgs.push_back(MsgArg("s", "<ERROR>"));
    QueueLampMethod(queuedCall);
}

void LampClients::SetLampName(const LSFString& lampID, const LSFString& name, const LSFString& language, Message& inMsg)
{
    QCC_DbgTrace(("%s", __func__));
    QueuedMethodCall* queuedCall = new QueuedMethodCall(inMsg, static_cast<MessageReceiver::ReplyHandler>(&LampClients::HandleGetReply));
    if (!queuedCall) {
        QCC_LogError(ER_FAIL, ("%s: Unable to allocate memory for call", __func__));
        return;
    }
    QueuedMethodCallElement element = QueuedMethodCallElement(lampID, ConfigServiceInterfaceName, "UpdateConfigurations");

    MsgArg name_arg("s", name.c_str());
    MsgArg arg("{sv}", "DeviceName", &name_arg);

    element.args.push_back(MsgArg("s", language.c_str()));
    element.args.push_back(MsgArg("a{sv}", 1, &arg));
    queuedCall->AddMethodCallElement(element);

    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", lampID.c_str()));
    queuedCall->responseCounter.standardReplyArgs.push_back(MsgArg("s", language.c_str()));

    QueueLampMethod(queuedCall);
}

void LampClients::IntrospectCB(QStatus status, ajn::ProxyBusObject* obj, void* context)
{
    QCC_DbgPrintf(("%s: status = %s", __func__, QCC_StatusText(status)));

    if (!context) {
        QCC_LogError(ER_FAIL, ("%s: No context", __func__));
        return;
    }

    LampConnection* connection = static_cast<LampConnection*>(context);

    if (!connectToLamps) {
        QCC_DbgPrintf(("%s: connectToLamps is false", __func__));
        return;
    }

    if (ER_OK != status) {
        QCC_DbgPrintf(("%s: status != ER_OK", __func__));
        QStatus tempStatus = joinSessionCBListLock.Lock();
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: joinSessionCBListLock.Lock() failed", __func__));
        } else {
            joinSessionCBList.insert(std::make_pair(connection->lampId, status));
            QCC_DbgPrintf(("%s: Add %s to joinSessionCBList", __func__, connection->lampId.c_str()));
            if (ER_OK != joinSessionCBListLock.Unlock()) {
                QCC_LogError(tempStatus, ("%s: joinSessionCBListLock.Unlock() failed", __func__));
            }
            wakeUp.Post();
        }
        return;
    }

    /*
     * Do not introspect the remote Config and About object!
     */
    QStatus tempStatus = ER_OK;
    const InterfaceDescription* intf = controllerService.GetBusAttachment().GetInterface(ConfigServiceInterfaceName);
    if (intf == NULL) {
        QCC_DbgPrintf(("%s: Interface %s not present", __func__, ConfigServiceInterfaceName));
        tempStatus = ER_FAIL;
    } else {
        tempStatus = connection->configObject.AddInterface(*intf);
        QCC_DbgPrintf(("%s: connection->configObject.AddInterface returns %s\n", __func__, QCC_StatusText(tempStatus)));
    }

    if (ER_OK == tempStatus) {
        intf = controllerService.GetBusAttachment().GetInterface(AboutInterfaceName);
        tempStatus = connection->aboutObject.AddInterface(*intf);
        QCC_DbgPrintf(("%s: connection->aboutObject.AddInterface returns %s\n", __func__, QCC_StatusText(tempStatus)));

        if (ER_OK == tempStatus) {
            if (!lampStateChangedSignalHandlerRegistered) {
                intf = connection->object.GetInterface(LampServiceStateInterfaceName);
                if (intf) {
                    const InterfaceDescription::Member* sig = intf->GetMember("LampStateChanged");
                    if (sig) {
                        tempStatus = controllerService.GetBusAttachment().RegisterSignalHandler(this, static_cast<MessageReceiver::SignalHandler>(&LampClients::LampStateChangedSignalHandler), sig, LampServiceObjectPath);
                        QCC_DbgPrintf(("%s: RegisterSignalHandler returns %s\n", __func__, QCC_StatusText(tempStatus)));

                        if (ER_OK == tempStatus) {
                            lampStateChangedSignalHandlerRegistered = true;
                        }
                    }
                }
            }

            if (ER_OK == tempStatus) {
                tempStatus = joinSessionCBListLock.Lock();
                if (ER_OK != tempStatus) {
                    QCC_LogError(tempStatus, ("%s: joinSessionCBListLock.Lock() failed", __func__));
                } else {
                    joinSessionCBList.insert(std::make_pair(connection->lampId, ER_OK));
                    QCC_DbgPrintf(("%s: Add %s to joinSessionCBList", __func__, connection->lampId.c_str()));
                    if (ER_OK != joinSessionCBListLock.Unlock()) {
                        QCC_LogError(tempStatus, ("%s: joinSessionCBListLock.Unlock() failed", __func__));
                    }
                    wakeUp.Post();
                }
            }
        }
    }

    if (ER_OK != tempStatus) {
        QCC_LogError(status, ("%s: IntrospectCB failed for lamp with ID = %s", __func__, connection->lampId.c_str()));
        controllerService.DoLeaveSessionAsync(connection->sessionID);
        tempStatus = joinSessionCBListLock.Lock();
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: joinSessionCBListLock.Lock() failed", __func__));
        } else {
            joinSessionCBList.insert(std::make_pair(connection->lampId, tempStatus));
            QCC_DbgPrintf(("%s: Add %s to joinSessionCBList", __func__, connection->lampId.c_str()));
            if (ER_OK != joinSessionCBListLock.Unlock()) {
                QCC_LogError(tempStatus, ("%s: joinSessionCBListLock.Unlock() failed", __func__));
            }
            wakeUp.Post();
        }
    }
}

QStatus LampClients::RegisterAnnounceHandler(void)
{
    QCC_DbgPrintf(("%s", __func__));
    QStatus status = services::AnnouncementRegistrar::RegisterAnnounceHandler(controllerService.GetBusAttachment(), *serviceHandler, interfaces, sizeof(interfaces) / sizeof(interfaces[0]));
    if (ER_OK == status) {
        QCC_DbgPrintf(("%s: RegisterAnnounceHandler successful", __func__));
    } else {
        QCC_LogError(status, ("%s: RegisterAnnounceHandler unsuccessful", __func__));
    }
    return status;
}

void LampClients::AlarmTriggered(void)
{
    QCC_DbgPrintf(("%s", __func__));
    alarmTriggered = true;
    wakeUp.Post();
}

void LampClients::Run(void)
{
    QCC_DbgTrace(("%s", __func__));

    bool oneTimeCleanupDone = false;

    bool alarmSet = false;

    while (isRunning) {
        /*
         * Wait for something to happen
         */
        QCC_DbgPrintf(("%s: Waiting on wakeUp", __func__));
        wakeUp.Wait();
        QStatus status = ER_OK;

        if (connectToLamps) {
            QCC_DbgPrintf(("%s: In the ConnectToLamps loop", __func__));

            if (oneTimeCleanupDone) {
                oneTimeCleanupDone = false;
            }

            /*
             * Handle all the lost sessions
             */
            std::set<uint32_t> tempLostSessionList;
            tempLostSessionList.clear();
            status = lostSessionListLock.Lock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: lostSessionListLock.Lock() failed", __func__));
                return;
            }
            /*
             * Make a local copy and release lostSessionList for use by Session Losts
             */
            tempLostSessionList = lostSessionList;
            lostSessionList.clear();
            status = lostSessionListLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: lostSessionListLock.Unlock() failed", __func__));
            }

            LSFStringList lostLamps;
            lostLamps.clear();

            if (tempLostSessionList.size()) {
                for (LampMap::iterator it = activeLamps.begin(); it != activeLamps.end(); it++) {
                    if (tempLostSessionList.find((uint32_t)it->second->sessionID) != tempLostSessionList.end()) {
                        QCC_DbgPrintf(("%s: Removing %s from activeLamps", __func__, it->second->lampId.c_str()));
                        it->second->ClearSessionAndObjects();
                        it->second->connectionState = BLACKLISTED;
                        lostLamps.push_back(it->second->lampId);
                    }
                }
            }

            /*
             * Send the lost lamps signal if required
             */
            if (lostLamps.size()) {
                controllerService.SendSignal(ControllerServiceLampInterfaceName, "LampsLost", lostLamps);
            }

            /*
             * Send all GetAllLampIDs responses
             */
            std::list<Message> tempGetAllLampIDsRequests;
            tempGetAllLampIDsRequests.clear();
            status = getAllLampIDsLock.Lock();
            if (ER_OK != status) {
                QCC_LogError(ER_FAIL, ("%s: getAllLampIDsLock.Lock() failed", __func__));
            } else {
                tempGetAllLampIDsRequests = getAllLampIDsRequests;
                getAllLampIDsRequests.clear();
                status = getAllLampIDsLock.Unlock();
                if (ER_OK != status) {
                    QCC_LogError(ER_FAIL, ("%s: getAllLampIDsLock.Unlock() failed", __func__));
                }
            }

            if (tempGetAllLampIDsRequests.size()) {
                LSFStringList idList;
                idList.clear();
                LSFResponseCode responseCode = LSF_OK;
                /*
                 * Get all the Lamp IDs
                 */
                for (LampMap::const_iterator it = activeLamps.begin(); it != activeLamps.end(); ++it) {
                    if (it->second->IsConnected()) {
                        idList.push_back(it->first);
                    }
                }

                while (tempGetAllLampIDsRequests.size()) {
                    QCC_DbgPrintf(("%s: Sending GetAllLampIDs reply with with tempGetAllLampIDsRequests.size() %d", __func__, tempGetAllLampIDsRequests.size()));
                    controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(tempGetAllLampIDsRequests.front(), responseCode, idList);
                    tempGetAllLampIDsRequests.pop_front();
                }
            }

            /*
             * Handle all received About Announcements as appropriate
             */
            LampMap tempAboutList;
            tempAboutList.clear();

            /*
             * Handle announcements
             */
            status = aboutsListLock.Lock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: aboutsListLock.Lock() failed", __func__));
            } else {
                /*
                 * Make a local copy of aboutsList and release the list for use by the About Handler
                 */
                tempAboutList = aboutsList;
                aboutsList.clear();
                status = aboutsListLock.Unlock();
                if (ER_OK != status) {
                    QCC_LogError(status, ("%s: aboutsListLock.Unlock() failed", __func__));
                }
            }

            typedef std::map<LSFString, LSFString> NameChangedMap;
            NameChangedMap nameChangedList;
            for (LampMap::iterator it = tempAboutList.begin(); it != tempAboutList.end(); it++) {
                LampConnection* newConn = it->second;
                LampMap::const_iterator lit = activeLamps.find(newConn->lampId);
                if (lit != activeLamps.end()) {
                    QCC_DbgPrintf(("%s: Got another announcement for a lamp that we already know about", __func__));
                    /*
                     * We already know about this Lamp
                     */
                    LampConnection* conn = lit->second;
                    if (conn->busName == newConn->busName) {
                        if (conn->name != newConn->name) {
                            conn->name = newConn->name;
                        }
                        if (conn->IsConnected()) {
                            nameChangedList.insert(std::make_pair(conn->lampId, conn->name));
                        }
                        QCC_DbgPrintf(("%s: Name Changed for %s", __func__, newConn->lampId.c_str()));
                    } else {
                        /*
                         * We got a new announcement from a lamp but the busName has changed. Clean up the
                         * old session and setup a new session with the lamp
                         */
                        LampConnectionState backup = conn->connectionState;
                        if (conn->sessionID) {
                            controllerService.DoLeaveSessionAsync(conn->sessionID);
                        }
                        *conn = *newConn;
                        if (backup == JOIN_SESSION_IN_PROGRESS) {
                            conn->connectionState = JOIN_SESSION_IN_PROGRESS;
                            conn->replaced = true;
                        }
                    }
                    newConn->Clear();
                } else {
                    if (activeLamps.size() < OEM_CS_MAX_SUPPORTED_LAMPS) {
                        activeLamps.insert(std::make_pair(it->first, newConn));
                    } else {
                        QCC_DbgPrintf(("%s: No slot for connection with a new lamp", __func__));
                        newConn->Clear();
                    }
                }
            }

            /*
             * Send out the LampNameChanged signal if required
             */
            for (NameChangedMap::iterator it = nameChangedList.begin(); it != nameChangedList.end(); it++) {
                controllerService.SendNameChangedSignal(ControllerServiceLampInterfaceName, "LampNameChanged", it->first, it->second);
            }

            /*
             * Send out Join Session requests
             */
            SessionOpts opts;
            opts.isMultipoint = true;
            for (LampMap::iterator it = activeLamps.begin(); it != activeLamps.end(); it++) {
                LampConnection* newConn =  it->second;
                if (newConn->IsDisconnected() || (alarmTriggered && (newConn->connectionState == RETRY_JOIN_SESSION))) {
                    status = controllerService.GetBusAttachment().JoinSessionAsync(newConn->busName.c_str(), newConn->port, this, opts, this, newConn);
                    QCC_DbgPrintf(("JoinSessionAsync(%s,%u): %s\n", newConn->busName.c_str(), newConn->port, QCC_StatusText(status)));
                    if (status != ER_OK) {
                        QCC_DbgPrintf(("%s: JoinSessionAsync failed for lamp %s", __func__, it->first.c_str()));
                        newConn->connectionState = BLACKLISTED;
                    } else {
                        newConn->connectionState = JOIN_SESSION_IN_PROGRESS;
                    }
                    newConn->replaced = false;
                }
            }

            if (alarmTriggered) {
                alarmSet = false;
                alarmTriggered = false;
            }

            /*
             * Handle all the successful Join Sessions
             */
            JoinSessionReplyMap tempJoinList;
            tempJoinList.clear();

            /*
             * Update the active lamps list
             */
            status = joinSessionCBListLock.Lock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: joinSessionCBListLock.Lock() failed", __func__));
            } else {
                /*
                 * Make a local copy of joinSessionCBList and release the list for use by the Join Session callbacks
                 */
                tempJoinList = joinSessionCBList;
                joinSessionCBList.clear();
                status = joinSessionCBListLock.Unlock();
                if (ER_OK != status) {
                    QCC_LogError(status, ("%s: joinSessionCBListLock.Unlock() failed", __func__));
                }
            }

            LSFStringList foundLamps;
            foundLamps.clear();

            bool retryJoinSession = false;

            for (JoinSessionReplyMap::iterator it = tempJoinList.begin(); it != tempJoinList.end(); it++) {
                LampMap::iterator lit = activeLamps.find(it->first);

                if (lit != activeLamps.end()) {
                    LampConnection* newConn = lit->second;

                    if (newConn->replaced) {
                        newConn->connectionState = DISCONNECTED;
                        wakeUp.Post();
                    } else {
                        if (it->second == ER_OK) {
                            newConn->connectionState = CONNECTED;
                            QCC_DbgPrintf(("%s: Connected to %s", __func__, newConn->lampId.c_str()));
                            foundLamps.push_back(newConn->lampId);
                        } else if (it->second == ER_ALLJOYN_JOINSESSION_REPLY_ALREADY_JOINED) {
                            newConn->connectionState = RETRY_JOIN_SESSION;
                            QCC_DbgPrintf(("%s: Will retry JoinSession to %s", __func__, newConn->lampId.c_str()));
                            retryJoinSession = true;
                        } else {
                            if (newConn->sessionID) {
                                controllerService.DoLeaveSessionAsync(newConn->sessionID);
                            }
                            newConn->ClearSessionAndObjects();
                            newConn->connectionState = BLACKLISTED;
                        }
                    }
                }
            }

            /*
             * Send the found lamps signal if required
             */
            if (foundLamps.size()) {
                controllerService.SendSignal(ControllerServiceLampInterfaceName, "LampsFound", foundLamps);
            }

            if (retryJoinSession) {
                uint32_t currentTime = GetTimestampInSeconds();
                if ((currentTime - disconnectFromLampsTimestamp) < LSF_MIN_LINK_TIMEOUT_IN_SECONDS) {
                    /*
                     * Set an alarm if LSF_MIN_LINK_TIMEOUT_IN_SECONDS have not elapsed since the Controller
                     * Service disconnected from the lamps
                     */
                    if (!alarmSet) {
                        alarmSet = true;
                        /*
                         * Add a 2s buffer to ensure that the session is cleaned up by the daemon on the accepting side
                         */
                        uint32_t timeToSet = 2 + LSF_MIN_LINK_TIMEOUT_IN_SECONDS - (currentTime - disconnectFromLampsTimestamp);
                        retryAlarm.SetAlarm(timeToSet);
                    }
                } else {
                    /*
                     * Post the wakeUp semaphore so that we can immediately retry if LSF_MIN_LINK_TIMEOUT_IN_SECONDS have already
                     * elapsed since the disconnect
                     */
                    alarmTriggered = true;
                    wakeUp.Post();
                }
            }

            /*
             * Handle all LampStateChangedSignals
             */
            GetLampStateList getLampStateListCopy;
            getLampStateListLock.Lock();
            getLampStateListCopy = getLampStateList;
            getLampStateList.clear();
            getLampStateListLock.Unlock();

            while (getLampStateListCopy.size()) {
                QueuedMethodCallContext* ctx = getLampStateListCopy.front();
                QCC_DbgPrintf(("%s: Calling GetLampState with tempMethodQueue.size() %d", __func__, getLampStateListCopy.size()));
                DoGetLampState(ctx);
                getLampStateListCopy.pop_front();
            }

            /*
             * Handle all the incoming method requests
             */
            std::list<QueuedMethodCall*> tempMethodQueue;
            tempMethodQueue.clear();
            status = queueLock.Lock();
            if (status != ER_OK) {
                QCC_LogError(status, ("%s: queueLock.Lock() failed", __func__));
            } else {
                /*
                 * Make a local copy and release methodQueue
                 */
                tempMethodQueue = methodQueue;
                methodQueue.clear();
                status = queueLock.Unlock();
                if (status != ER_OK) {
                    QCC_LogError(status, ("%s: queueLock.Unlock() failed", __func__));
                }
            }

            while (tempMethodQueue.size()) {
                QueuedMethodCall* queuedCall = tempMethodQueue.front();
                QCC_DbgPrintf(("%s: Calling DoMethodCallAsync with tempMethodQueue.size() %d", __func__, tempMethodQueue.size()));
                DoMethodCallAsync(queuedCall);
                tempMethodQueue.pop_front();
            }
        } else {
            QCC_DbgPrintf(("%s: In the DisconnectFromLamps loop", __func__));
            if (!oneTimeCleanupDone) {
                status = queueLock.Lock();
                if (status != ER_OK) {
                    QCC_LogError(status, ("%s: queueLock.Lock() failed", __func__));
                } else {
                    while (methodQueue.size()) {
                        QueuedMethodCall* queuedCall = methodQueue.front();
                        delete queuedCall;
                        methodQueue.pop_front();
                    }
                    QCC_DbgPrintf(("%s: Cleared methodQueue", __func__));
                    status = queueLock.Unlock();
                    if (status != ER_OK) {
                        QCC_LogError(status, ("%s: queueLock.Unlock() failed", __func__));
                    }
                }

                for (LampMap::iterator it = activeLamps.begin(); it != activeLamps.end(); ++it) {
                    LampConnection* conn = it->second;
                    if (it->second->sessionID) {
                        controllerService.DoLeaveSessionAsync(conn->object.GetSessionId());
                    }
                    conn->ClearSessionAndObjects();
                    conn->replaced = false;
                }

                status = joinSessionCBListLock.Lock();
                if (ER_OK != status) {
                    QCC_LogError(status, ("%s: joinSessionCBListLock.Lock() failed", __func__));
                } else {
                    joinSessionCBList.clear();
                    QCC_DbgPrintf(("%s: Cleared joinSessionCBList", __func__));
                    status = joinSessionCBListLock.Unlock();
                    if (ER_OK != status) {
                        QCC_LogError(status, ("%s: joinSessionCBListLock.Unlock() failed", __func__));
                    }
                }

                status = lostSessionListLock.Lock();
                if (ER_OK != status) {
                    QCC_LogError(status, ("%s: lostSessionListLock.Lock() failed", __func__));
                    return;
                }
                lostSessionList.clear();
                QCC_DbgPrintf(("%s: Cleared lostSessionList", __func__));
                status = lostSessionListLock.Unlock();
                if (ER_OK != status) {
                    QCC_LogError(status, ("%s: lostSessionListLock.Unlock() failed", __func__));
                }

                status = getAllLampIDsLock.Lock();
                if (ER_OK != status) {
                    QCC_LogError(ER_FAIL, ("%s: getAllLampIDsLock.Lock() failed", __func__));
                } else {
                    getAllLampIDsRequests.clear();
                    QCC_DbgPrintf(("%s: Cleared getAllLampIDsRequests", __func__));
                    status = getAllLampIDsLock.Unlock();
                    if (ER_OK != status) {
                        QCC_LogError(ER_FAIL, ("%s: getAllLampIDsLock.Unlock() failed", __func__));
                    }
                }
                oneTimeCleanupDone = true;
            } else {
                /*
                 * Handle announcements
                 */
                LampMap tempAboutList;
                tempAboutList.clear();

                status = aboutsListLock.Lock();
                if (ER_OK != status) {
                    QCC_LogError(status, ("%s: aboutsListLock.Lock() failed", __func__));
                } else {
                    /*
                     * Make a local copy of aboutsList and release the list for use by the About Handler
                     */
                    tempAboutList = aboutsList;
                    aboutsList.clear();
                    status = aboutsListLock.Unlock();
                    if (ER_OK != status) {
                        QCC_LogError(status, ("%s: aboutsListLock.Unlock() failed", __func__));
                    }
                }

                for (LampMap::iterator it = tempAboutList.begin(); it != tempAboutList.end(); it++) {
                    LampConnection* newConn = it->second;
                    LampMap::const_iterator lit = activeLamps.find(newConn->lampId);
                    if (lit != activeLamps.end()) {
                        QCC_DbgPrintf(("%s: Got another announcement for a lamp that we already know about", __func__));
                        /*
                         * We already know about this Lamp
                         */
                        LampConnection* conn = lit->second;
                        if (conn->name != newConn->name) {
                            conn->name = newConn->name;
                            QCC_DbgPrintf(("%s: Name Changed for %s", __func__, newConn->lampId.c_str()));
                        }
                        newConn->Clear();
                    } else {
                        if (activeLamps.size() < OEM_CS_MAX_SUPPORTED_LAMPS) {
                            activeLamps.insert(std::make_pair(it->first, newConn));
                        } else {
                            QCC_DbgPrintf(("%s: No slot for connection with a new lamp", __func__));
                            newConn->Clear();
                        }
                    }
                }
            }
        }

        QCC_DbgPrintf(("%s: Exited", __func__));
    }
}
