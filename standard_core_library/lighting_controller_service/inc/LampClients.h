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

class LampClientsCallback {
  public:
    virtual ~LampClientsCallback() { }

    virtual void TransitionLampStateReplyCB(ajn::Message& origMsg, LSFResponseCode rc) = 0;
    virtual void TransitionLampStateFieldReplyCB(ajn::Message& origMsg, const char* field, LSFResponseCode rc) = 0;

    virtual void GetLampStateReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode rc) = 0;
    virtual void GetLampStateFieldReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode rc) = 0;
    virtual void GetLampDetailsReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode rc) = 0;
    virtual void GetLampParametersReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode rc) = 0;
    virtual void GetLampParametersFieldReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode rc) = 0;

    virtual void GetLampNameReplyCB(ajn::Message& origMsg, const char* name, LSFResponseCode rc) = 0;
    virtual void SetLampNameReplyCB(ajn::Message& origMsg, LSFResponseCode rc) = 0;

    virtual void GetLampFaultsReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode rc) = 0;
    virtual void ClearLampFaultsReplyCB(ajn::Message& origMsg, LSFResponseCode rc, LampFaultCode code) = 0;
};


class LampClients : public Manager, public ajn::BusAttachment::JoinSessionAsyncCB, public ajn::SessionListener, public ajn::ProxyBusObject::Listener {
  public:

    LampClients(ControllerService& controllerSvc, LampClientsCallback& callback);

    ~LampClients();

    void GetAllLampIDs(LSF_ID_List& lamps);

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
     * @param msg   The original message
     */
    LSFResponseCode GetLampName(const LSF_ID& lampID, ajn::Message& msg);

    /**
     * Set the Lamp name
     *
     * @param lampID    The lamp id
     * @param name      The new name
     * @param inMsg     The original message
     */
    LSFResponseCode SetLampName(const LSF_ID& lampID, const char* name, ajn::Message& inMsg);

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
    LSFResponseCode GetLampState(const LSF_ID& lampID, ajn::Message& inMsg);

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
    LSFResponseCode GetLampDetails(const LSF_ID& lampID, ajn::Message& inMsg);

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
    LSFResponseCode GetLampParameters(const LSF_ID& lampID, ajn::Message& inMsg);

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
    LSFResponseCode GetLampStateField(const LSF_ID& lampID, const std::string& field, ajn::Message& inMsg);

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
    LSFResponseCode GetLampParametersField(const LSF_ID& lampID, const std::string& field, ajn::Message& inMsg);

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
    LSFResponseCode TransitionLampState(const ajn::Message& inMsg, const LSF_ID_List& lamps, uint64_t timestamp, const ajn::MsgArg& state, uint32_t period);

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
    LSFResponseCode TransitionLampFieldState(const ajn::Message& inMsg, const LSF_ID_List& lamps, uint64_t timestamp, const char* field, const ajn::MsgArg& value, uint32_t period);

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
    LSFResponseCode GetLampFaults(const LSF_ID& lampID, ajn::Message& inMsg);

    /**
     * Clear the given lamp fault specified in inMsg
     *
     * @param lampID    The lamp id
     * @param inMsg     The original message that led to this call
     *
     * @return          LSF_OK if the request was sent
     *
     * The reply will trigger a call to LampClientsCallback::ClearLampFaultsReplyCB.
     * The callback will only happen if LSF_OK is returned
     */
    LSFResponseCode ClearLampFaults(const LSF_ID& lampID, ajn::Message& inMsg);

  private:


    void HandleAboutAnnounce(const LSF_ID lampID, const LSF_Name& lampName, uint16_t port, const char* busName);

    void LampStateChangedSignalHandler(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& msg);

    typedef std::list<ajn::ProxyBusObject> ObjectMap;

    struct QueuedMethodCall {
        typedef void (LampClients::* MethodHandler)(QueuedMethodCall*);

        typedef void (LampClientsCallback::* CallbackMethod)(ajn::Message&, const ajn::MsgArg&, LSFResponseCode);

        QueuedMethodCall(const ajn::Message& msg, MethodHandler handler, CallbackMethod cb = NULL) :
            inMsg(msg), handler(handler), callbackMethod(cb) { }

        ajn::Message inMsg;
        // verify whether this is safe
        ObjectMap objects;

        ObjectMap configObjects;

        std::string interface;
        std::string property;
        std::vector<ajn::MsgArg> args;

        MethodHandler handler;
        CallbackMethod callbackMethod;

        ajn::MsgArg errorOutput;
    };

    LSFResponseCode QueueLampMethod(const LSF_ID& lampId, QueuedMethodCall* queuedCall);

    void DoTransitionLampState(QueuedMethodCall* call);
    void DoSetLampMultipleReply(ajn::Message& msg, void* context);

    void DoGetProperties(QueuedMethodCall* call);
    //void DoGetPropertiesReply(QStatus status, ajn::ProxyBusObject* obj, const ajn::MsgArg& value, void* context);
    void DoGetPropertiesReply(ajn::Message& msg, void* context);

    void DoClearLampFaults(QueuedMethodCall* call);
    void DoClearLampFaultsReply(ajn::Message& msg, void* context);

    void DoGetLampFaults(QueuedMethodCall* call);
    void DoGetLampFaultsReply(ajn::Message& msg, void* context);

    void DoGetLampName(QueuedMethodCall* call);
    void GetConfigurationsReply(ajn::Message& msg, void* context);
    void DoSetLampName(QueuedMethodCall* call);
    void UpdateConfigurationsReply(ajn::Message& msg, void* context);

    struct ResponseCounter {
        ResponseCounter() {
            numWaiting = 0;
            successCount = 0;
            failCount = 0;
        }

        volatile int32_t numWaiting;

        // could be updated from multiple threads
        volatile int32_t successCount;
        volatile int32_t failCount;
        uint32_t total;

        QueuedMethodCall* call;
    };

    void DecrementWaitingAndSendResponse(ResponseCounter* counter);

    class QueueHandler;
    QueueHandler* queueHandler;
    WorkerQueue<QueuedMethodCall> methodQueue;

    struct LampConnection {
        LSF_ID id;
        ajn::ProxyBusObject object;
        ajn::ProxyBusObject configObject;
        LSF_Name wkn;
        LSF_Name name;
    };

    typedef std::map<LSF_ID, LampConnection*> LampMap;
    LampMap activeLamps;

    // map the session id to the Lamp ID
    typedef std::map<ajn::SessionId, LSF_ID> SessionIDMap;
    SessionIDMap sessionLampMap;
    Mutex lampLock;

    class ServiceHandler;
    ServiceHandler* serviceHandler;

    LSFKeyListener keyListener;

    LampClientsCallback& callback;
};

}

#endif
