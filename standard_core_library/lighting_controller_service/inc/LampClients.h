#ifndef _LAMP_CLIENTS_H_
#define _LAMP_CLIENTS_H_
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

#include <alljoyn/BusAttachment.h>
#include <alljoyn/SessionListener.h>
#include <alljoyn/about/AnnounceHandler.h>
#include <alljoyn/about/AnnouncementRegistrar.h>
#include <LSFKeyListener.h>
#include <Mutex.h>
#include <WorkerQueue.h>
#include <Manager.h>

#include <string>
#include <map>
#include <vector>

#define INITIAL_PASSCODE "000000"

namespace lsf {

class LampClients : public Manager, public ajn::BusAttachment::JoinSessionAsyncCB, public ajn::SessionListener, public ajn::ProxyBusObject::Listener {
  public:

    LampClients(ControllerService& controllerSvc);

    ~LampClients();

    LSFResponseCode GetAllLampIDs(LSFStringList& lamps);

    /**
     * Start the Lamp Clients
     *
     * @param   None
     * @return  ER_OK if successful, error otherwise
     */
    QStatus Start(void);

    /**
     * Stop the Lamp Clients
     *
     * @param   None
     * @return  ER_OK if successful, error otherwise
     */
    QStatus Stop(void);

    /**
     * Called when JoinSessionAsync() completes.
     *
     * @param status       ER_OK if successful
     * @param sessionId    Unique identifier for session.
     * @param opts         Session options.
     * @param context      User defined context which will be passed as-is to callback.
     */
    virtual void JoinSessionCB(QStatus status, ajn::SessionId sessionId, const ajn::SessionOpts& opts, void* context);

    /**
     * Called by the bus when an existing session becomes disconnected.
     *
     * @param sessionId     Id of session that was lost.
     * @param reason        The reason for the session being lost
     */
    virtual void SessionLost(ajn::SessionId sessionId, SessionLostReason reason);

    /**
     * Get the Lamp name
     *
     * @param lampID    The lamp id
     * @param msg   The original message
     */
    LSFResponseCode GetLampName(const LSFString& lampID, const LSFString& language, ajn::Message& msg);

    /**
     * Set the Lamp name
     *
     * @param lampID    The lamp id
     * @param name      The new name
     * @param inMsg     The original message
     */
    LSFResponseCode SetLampName(const LSFString& lampID, const LSFString& name, const LSFString& language, ajn::Message& inMsg);

    /**
     * Get the Lamp manufacturer
     *
     * @param lampID    The lamp id
     * @param msg   The original message
     */
    LSFResponseCode GetLampManufacturer(const LSFString& lampID, const LSFString& language, ajn::Message& msg);

    /**
     * Get the Lamp manufacturer
     *
     * @param lampID    The lamp id
     * @param msg   The original message
     */
    LSFResponseCode GetLampSupportedLanguages(const LSFString& lampID, ajn::Message& msg);

    /**
     * Get the Lamp's entire state
     *
     * @param lampID    The lamp id
     * @param inMsg     The original message that led to this call
     *
     * @return          LSF_OK if the request was sent
     *
     * The reply will trigger a call to LampClientsCallback::GetLampStateReplyCB.
     * The callback will only happen if LSF_OK is returned
     */
    LSFResponseCode GetLampState(const LSFString& lampID, ajn::Message& inMsg);

    /**
     * Get the Lamp's details
     *
     * @param lampID    The lamp id
     * @param inMsg     The original message that led to this call
     *
     * @return          LSF_OK if the request was sent
     *
     * The reply will trigger a call to LampClientsCallback::GetLampDetailsReplyCB.
     * The callback will only happen if LSF_OK is returned
     */
    LSFResponseCode GetLampDetails(const LSFString& lampID, ajn::Message& inMsg);

    /**
     * Get the Lamp's parameters
     *
     * @param lampID    The lamp id
     * @param inMsg     The original message that led to this call
     *
     * @return          LSF_OK if the request was sent
     *
     * The reply will trigger a call to LampClientsCallback::GetLampParametersReplyCB.
     * The callback will only happen if LSF_OK is returned
     */
    LSFResponseCode GetLampParameters(const LSFString& lampID, ajn::Message& inMsg);

    /**
     * Get the Lamp's state field
     *
     * @param lampID    The lamp id
     * @param field     The field to get
     * @param inMsg     The original message that led to this call
     *
     * @return          LSF_OK if the request was sent
     *
     * The reply will trigger a call to LampClientsCallback::GetLampStateFieldReplyCB.
     * The callback will only happen if LSF_OK is returned
     */
    LSFResponseCode GetLampStateField(const LSFString& lampID, const LSFString& field, ajn::Message& inMsg);

    /**
     * Get the Lamp's parameters field
     *
     * @param lampID    The lamp id
     * @param field     The field to get
     * @param inMsg     The original message that led to this call
     *
     * @return          LSF_OK if the request was sent
     *
     * The reply will trigger a call to LampClientsCallback::GetLampParametersFieldReplyCB.
     * The callback will only happen if LSF_OK is returned
     */
    LSFResponseCode GetLampParametersField(const LSFString& lampID, const LSFString& field, ajn::Message& inMsg);

    /**
     * Apply a LampState to one or more lamps
     *
     * @param inMsg     The original message that led to this call
     * @param lamps     A list of lamps to apply the state to
     * @param timestamp The transition timestamp
     * @param arg       A MsgArg with the LampState encoded
     * @param period    The transition period
     *
     * @return      LSF_OK if all messages were successfully sent.
     *
     * The method LampClientsCallback::GetLampDetailsReplyCB will be called when all lamps have responded
     */
    LSFResponseCode TransitionLampState(const ajn::Message& inMsg, const LSFStringList& lamps, uint64_t stateTimestamp, const ajn::MsgArg& state, uint32_t period);


    LSFResponseCode PulseLampWithState(const ajn::Message& inMsg, const LSFStringList& lamps, const ajn::MsgArg& oldState, const ajn::MsgArg& newState, uint32_t period, uint32_t duration, uint32_t numPulses, uint64_t stateTimestamp);
    /**
     * Apply a Lamp State Field to one or more lamps
     *
     * @param inMsg     The original message that led to this call
     * @param lamps     A list of lamps to apply the state to
     * @param timestamp The transition timestamp
     * @param field     The State field to change
     * @param value     The state field's new value
     * @param period    The transition period
     *
     * @return      LSF_OK if all messages were successfully sent.
     *
     * The method LampClientsCallback::GetLampDetailsReplyCB will be called when all lamps have responded
     */
    LSFResponseCode TransitionLampStateField(const ajn::Message& inMsg, const LSFStringList& lamps, uint64_t stateTimestamp, const char* field, const ajn::MsgArg& value, uint32_t period);

    /**
     * Get the lamp faults
     *
     * @param lampID    The lamp id
     * @param inMsg     The original message that led to this call
     *
     * @return          LSF_OK if the request was sent
     *
     * The reply will trigger a call to LampClientsCallback::GetLampFaultsReplyCB.
     * The callback will only happen if LSF_OK is returned
     */
    LSFResponseCode GetLampFaults(const LSFString& lampID, ajn::Message& inMsg);

    LSFResponseCode GetLampVersion(const LSFString& lampID, ajn::Message& inMsg);

    /**
     * Clear the given lamp fault specified in inMsg
     *
     * @param lampID    The lamp id
     * @param inMsg     The original message that led to this call
     *
     * @return          LSF_OK if the request was sent
     *
     * The reply will trigger a call to LampClientsCallback::ClearLampFaultReplyCB.
     * The callback will only happen if LSF_OK is returned
     */
    LSFResponseCode ClearLampFault(const LSFString& lampID, LampFaultCode faultCode, ajn::Message& inMsg);

    LSFResponseCode GetAllLamps(LampNameMap& lamps);

  private:


    void HandleAboutAnnounce(const LSFString lampID, const LSFString& lampName, uint16_t port, const char* busName);

    void LampStateChangedSignalHandler(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& msg);

    typedef std::list<ajn::ProxyBusObject> ObjectMap;

    struct ResponseCounter {
        ResponseCounter() { }
        ResponseCounter(uint32_t numLamps) :
            total(numLamps)
        {
            numWaiting = numLamps;
            successCount = 0;
            failCount = 0;
            notFoundCount = 0;
        }

        volatile int32_t numWaiting;
        volatile uint32_t successCount;
        volatile uint32_t failCount;
        volatile uint32_t notFoundCount;
        uint32_t total;

        std::list<ajn::MsgArg> standardReplyArgs;
        std::list<ajn::MsgArg> customReplyArgs;
    };

    struct QueuedMethodCall {
        typedef LSFResponseCode (LampClients::* MethodHandler)(QueuedMethodCall*);

        QueuedMethodCall(const ajn::Message& msg, LSFString lampId, std::string intf, std::string methodName, MethodHandler handler, ajn::MessageReceiver::ReplyHandler replyHandler) :
            inMsg(msg), interface(intf), method(methodName), handler(handler), replyFunc(replyHandler), responseID(qcc::RandHexString(8).c_str()), responseCounter(1) {
            lamps.clear();
            lamps.push_back(lampId);
            args.clear();
        }

        QueuedMethodCall(const ajn::Message& msg, LSFStringList lampList, std::string intf, std::string methodName, MethodHandler handler, ajn::MessageReceiver::ReplyHandler replyHandler) :
            inMsg(msg), interface(intf), method(methodName), handler(handler), replyFunc(replyHandler), responseID(qcc::RandHexString(8).c_str()), responseCounter(lampList.size()) {
            lamps.clear();
            lamps = lampList;
            args.clear();
        }

        ajn::Message inMsg;
        LSFStringList lamps;
        std::string interface;
        std::string method;
        std::vector<ajn::MsgArg> args;
        MethodHandler handler;
        ajn::MessageReceiver::ReplyHandler replyFunc;
        LSFString responseID;
        ResponseCounter responseCounter;
    };

    void SendMethodReply(LSFResponseCode responseCode, ajn::Message msg, std::list<ajn::MsgArg>& stdArgs, std::list<ajn::MsgArg>& custArgs);

    LSFResponseCode DoMethodCallAsync(QueuedMethodCall* call);

    LSFResponseCode QueueLampMethod(QueuedMethodCall* queuedCall);

    void HandleReplyWithLampResponseCode(ajn::Message& msg, void* context);
    void HandleGetReply(ajn::Message& msg, void* context);
    void HandleReplyWithVariant(ajn::Message& msg, void* context);
    void HandleReplyWithKeyValuePairs(ajn::Message& msg, void* context);

    void DecrementWaitingAndSendResponse(QueuedMethodCall* queuedCall, uint32_t success, uint32_t failure, uint32_t notFound, const ajn::MsgArg* arg = NULL);

    class QueueHandler;
    QueueHandler* queueHandler;
    WorkerQueue<QueuedMethodCall> methodQueue;

    struct LampConnection {
        LSFString id;
        ajn::ProxyBusObject object;
        ajn::ProxyBusObject configObject;
        ajn::ProxyBusObject aboutObject;
        LSFString wkn;
        LSFString name;
        ajn::SessionId sessionID;
    };

    typedef std::map<LSFString, LampConnection*> LampMap;
    LampMap activeLamps;
    Mutex lampLock;

    typedef std::map<LSFString, ResponseCounter> ResponseMap;
    ResponseMap responseMap;
    Mutex responseLock;

    class ServiceHandler;
    ServiceHandler* serviceHandler;

    LSFKeyListener keyListener;
};

}

#endif
