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
#include <Manager.h>
#include <Thread.h>
#include <LSFSemaphore.h>

#include <string>
#include <map>
#include <vector>

#define INITIAL_PASSCODE "000000"

namespace lsf {
/**
 * struct is used to contain a list of lamps and the requested field state details
 */
typedef struct _TransitionStateFieldParams {
    /**
     * _TransitionStateFieldParams CTOR
     */
    _TransitionStateFieldParams(LSFStringList& lampList, uint64_t& timeStamp, const char* fieldName, ajn::MsgArg& fieldValue, uint32_t& transPeriod) :
        lamps(lampList), timestamp(timeStamp), field(fieldName), value(fieldValue), period(transPeriod) { }

    LSFStringList lamps;    /**< List of lamps */
    uint64_t timestamp;     /**< time for the transition */
    const char* field;      /**< field to act on */
    ajn::MsgArg value;      /**< value to move to */
    uint32_t period;        /**< period of time */
} TransitionStateFieldParams;

/**
 * struct is used to contain a list of lamps and the requested state details
 */
typedef struct _TransitionStateParams {
    /**
     * _TransitionStateParams CTOR
     */
    _TransitionStateParams(LSFStringList& lampList, uint64_t& timeStamp, ajn::MsgArg& lampState, uint32_t& transPeriod) :
        lamps(lampList), timestamp(timeStamp), state(lampState), period(transPeriod) { }

    LSFStringList lamps;    /**< List of lamps */
    uint64_t timestamp;     /**< time for the transition */
    ajn::MsgArg state;      /**< new state to transit to */
    uint32_t period;        /**< period of time */
} TransitionStateParams;

/**
 * struct is used to contain requested pulse state parameters
 */
typedef struct _PulseStateParams {
    /**
     * CTOR to struct PulseStateParams
     * @param   lampList
     * @param   oldLampState
     * @param   newLampState
     * @param   pulsePeriod
     * @param   pulseDuration
     * @param   numPul
     * @param   timeStamp
     */
    _PulseStateParams(LSFStringList& lampList, ajn::MsgArg& oldLampState, ajn::MsgArg& newLampState, uint32_t& pulsePeriod, uint32_t& pulseDuration, uint32_t& numPul, uint64_t& timeStamp) :
        lamps(lampList), oldState(oldLampState), newState(newLampState), period(pulsePeriod), duration(pulseDuration), numPulses(numPul), timestamp(timeStamp) { }

    LSFStringList lamps;    /**< List of lamps */
    ajn::MsgArg oldState;   /**< Old state */
    ajn::MsgArg newState;   /**< New state */
    uint32_t period;        /**< period of pulse time */
    uint32_t duration;      /**< duration of pulse time */
    uint32_t numPulses;     /**< number of pulses */
    uint64_t timestamp;     /**< time for the pulse */
} PulseStateParams;

typedef std::list<TransitionStateFieldParams> TransitionStateFieldParamsList;
typedef std::list<TransitionStateParams> TransitionStateParamsList;
typedef std::list<PulseStateParams> PulseStateParamsList;

/**
 * class is used as clietn side to the lamp service
 */
class LampClients : public Manager, public ajn::BusAttachment::JoinSessionAsyncCB, public ajn::SessionListener,
    public ajn::ProxyBusObject::Listener, public lsf::Thread, public BusAttachment::PingAsyncCB {
  public:
    /**
     * LampClients constructor
     */
    LampClients(ControllerService& controllerSvc);
    /**
     * LampClients destructor
     */
    ~LampClients();
    /**
     * request all lamp ids
     */
    void RequestAllLampIDs(ajn::Message& message);
    /**
     * Start the Lamp Clients
     *
     * @param   keyStoreFileLocation - key for the security
     * @return  ER_OK if successful, error otherwise
     */
    QStatus Start(const char* keyStoreFileLocation);
    /**
     * Run thread method
     */
    void Run(void);

    /**
     * Stop the Lamp Clients
     */
    void Stop(void);
    /**
     * Join thread method
     */
    void Join(void);

    /**
     * Called when JoinSessionAsync() completes.
     *
     * @param status       ER_OK if successful
     * @param sessionId    Unique identifier for session.
     * @param opts         Session options.
     * @param context      User defined context which will be passed as-is to callback.
     */
    void JoinSessionCB(QStatus status, ajn::SessionId sessionId, const ajn::SessionOpts& opts, void* context);

    /**
     * Called by the bus when an existing session becomes disconnected.
     *
     * @param sessionId     Id of session that was lost.
     * @param reason        The reason for the session being lost
     */
    void SessionLost(ajn::SessionId sessionId, SessionLostReason reason);

    /**
     * Get the Lamp name
     *
     * @param lampID    The lamp id
     * @param language  The language of the requested information
     * @param msg   The original message
     */
    void GetLampName(const LSFString& lampID, const LSFString& language, ajn::Message& msg);

    /**
     * Set the Lamp name
     *
     * @param lampID    The lamp id
     * @param name      The new name
     * @param language  The language of the given information
     * @param inMsg     The original message
     */
    void SetLampName(const LSFString& lampID, const LSFString& name, const LSFString& language, ajn::Message& inMsg);

    /**
     * Get the Lamp manufacturer
     *
     * @param lampID    The lamp id
     * @param language  the language of the requested information
     * @param msg   The original message
     */
    void GetLampManufacturer(const LSFString& lampID, const LSFString& language, ajn::Message& msg);

    /**
     * Get the Lamp manufacturer
     *
     * @param lampID    The lamp id
     * @param msg   The original message
     */
    void GetLampSupportedLanguages(const LSFString& lampID, ajn::Message& msg);

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
    void GetLampState(const LSFString& lampID, ajn::Message& inMsg);

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
    void GetLampDetails(const LSFString& lampID, ajn::Message& inMsg);

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
    void GetLampParameters(const LSFString& lampID, ajn::Message& inMsg);

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
    void GetLampStateField(const LSFString& lampID, const LSFString& field, ajn::Message& inMsg);

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
    void GetLampParametersField(const LSFString& lampID, const LSFString& field, ajn::Message& inMsg);

    /**
     * change lamp state
     */
    void ChangeLampState(const ajn::Message& inMsg, bool groupOperation, bool sceneOperation, TransitionStateParamsList& transitionStateParams,
                         TransitionStateFieldParamsList& transitionStateFieldparams, PulseStateParamsList& pulseParams, LSFString sceneOrMasterSceneID = LSFString());

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
    void GetLampFaults(const LSFString& lampID, ajn::Message& inMsg);

    /**
     * Get lamp version
     */
    void GetLampVersion(const LSFString& lampID, ajn::Message& inMsg);

    /**
     * Clear the given lamp fault specified in inMsg
     *
     * @param lampID    The lamp id
     * @param faultCode  The lamp code to clear
     * @param inMsg     The original message that led to this call
     *
     * @return          LSF_OK if the request was sent
     *
     * The reply will trigger a call to LampClientsCallback::ClearLampFaultReplyCB.
     * The callback will only happen if LSF_OK is returned
     */
    void ClearLampFault(const LSFString& lampID, LampFaultCode faultCode, ajn::Message& inMsg);

    /**
     * Get all lamps
     */
    void GetAllLamps(LampNameMap& lamps);

    /**
     * introspect callback
     */
    void IntrospectCB(QStatus status, ajn::ProxyBusObject* obj, void* context);
    /**
     * connect to lamps
     */
    void ConnectToLamps(void);
    /**
     * disconnect from lamps
     */
    void DisconnectFromLamps(void);
    /**
     * unregister announce handler
     */
    QStatus UnregisterAnnounceHandler(void);
    /**
     * register announce handler
     */
    QStatus RegisterAnnounceHandler(void);

  private:

    void* LampClientsThread(void* data);

    void HandleAboutAnnounce(const LSFString lampID, const LSFString& lampName, uint16_t port, const char* busName);

    void LampStateChangedSignalHandler(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& msg);

    typedef std::list<ajn::ProxyBusObject> ObjectMap;

    struct ResponseCounter {
        ResponseCounter() :
            numWaiting(0), successCount(0), failCount(0), notFoundCount(0), total(0) {
            sceneOrMasterSceneID.clear();
        }

        void AddLamps(uint32_t numLamps)
        {
            total += numLamps;
            numWaiting += numLamps;
        }

        volatile int32_t numWaiting;
        volatile uint32_t successCount;
        volatile uint32_t failCount;
        volatile uint32_t notFoundCount;
        uint32_t total;

        std::list<ajn::MsgArg> standardReplyArgs;
        std::list<ajn::MsgArg> customReplyArgs;
        LSFString sceneOrMasterSceneID;
    };

    struct QueuedMethodCallElement {
        QueuedMethodCallElement() {
            lamps.clear();
            args.clear();
        }

        QueuedMethodCallElement(LSFStringList lampList, std::string intf, std::string methodName) :
            lamps(lampList), interface(intf), method(methodName) { }

        QueuedMethodCallElement(LSFString lamp, std::string intf, std::string methodName) :
            interface(intf), method(methodName) {
            lamps.clear();
            lamps.push_back(lamp);
        }

        LSFStringList lamps;
        std::string interface;
        std::string method;
        std::vector<ajn::MsgArg> args;
    };

    typedef std::list<QueuedMethodCallElement> QueuedMethodCallElementList;

    struct QueuedMethodCall {
        QueuedMethodCall(const ajn::Message& msg, ajn::MessageReceiver::ReplyHandler replyHandler) :
            inMsg(msg), replyFunc(replyHandler), responseID(qcc::RandHexString(8).c_str()), responseCounter() {
        }

        void AddMethodCallElement(QueuedMethodCallElement& element) {
            methodCallElements.push_back(element);
            responseCounter.AddLamps(element.lamps.size());
        }

        ajn::Message inMsg;
        ajn::MessageReceiver::ReplyHandler replyFunc;
        LSFString responseID;
        ResponseCounter responseCounter;
        QueuedMethodCallElementList methodCallElements;
        uint32_t methodCallCount;
    };

    struct QueuedMethodCallContext {
        QueuedMethodCallContext(LSFString lampId, QueuedMethodCall* qCallPtr, LSFString met) :
            lampID(lampId), queuedCallPtr(qCallPtr), method(met) { }

        LSFString lampID;
        QueuedMethodCall* queuedCallPtr;
        LSFString method;
    };

    void SendMethodReply(LSFResponseCode responseCode, ajn::Message msg, std::list<ajn::MsgArg>& stdArgs, std::list<ajn::MsgArg>& custArgs);

    LSFResponseCode DoMethodCallAsync(QueuedMethodCall* call);

    void QueueLampMethod(QueuedMethodCall* queuedCall);

    void HandleReplyWithLampResponseCode(ajn::Message& msg, void* context);
    void HandleGetReply(ajn::Message& msg, void* context);
    void HandleReplyWithVariant(ajn::Message& msg, void* context);
    void HandleReplyWithKeyValuePairs(ajn::Message& msg, void* context);

    void DecrementWaitingAndSendResponse(QueuedMethodCall* queuedCall, uint32_t success, uint32_t failure, uint32_t notFound, const ajn::MsgArg* arg = NULL);

    void PingCB(QStatus status, void* context);

    struct LampConnection {
        LSFString lampId;
        ajn::ProxyBusObject object;
        ajn::ProxyBusObject configObject;
        ajn::ProxyBusObject aboutObject;
        LSFString busName;
        LSFString name;
        uint16_t port;
        ajn::SessionId sessionID;
        uint32_t methodCallCount;
    };

    typedef std::map<LSFString, LampConnection*> LampMap;
    LampMap activeLamps;

    LampMap aboutsList;
    Mutex aboutsListLock;

    std::list<LampConnection*> joinSessionCBList;
    Mutex joinSessionCBListLock;

    std::set<uint32_t> lostSessionList;
    Mutex lostSessionListLock;

    typedef std::map<LSFString, ResponseCounter> ResponseMap;
    ResponseMap responseMap;
    Mutex responseLock;

    class ServiceHandler;
    ServiceHandler* serviceHandler;

    LSFKeyListener keyListener;

    Mutex queueLock;
    std::list<QueuedMethodCall*> methodQueue;
    bool isRunning;

    bool lampStateChangedSignalHandlerRegistered;

    std::list<ajn::Message> getAllLampIDsRequests;
    Mutex getAllLampIDsLock;

    Mutex joinSessionInProgressLock;
    std::set<LSFString> joinSessionInProgressList;

    LSFSemaphore wakeUp;

    Mutex connectToLampsLock;
    bool connectToLamps;

    typedef struct _PingCtx {
        _PingCtx(uint32_t count, LSFString lampId) :
            callCount(count), lampID(lampId) { }
        uint32_t callCount;
        LSFString lampID;
    } PingCtx;
};

}

#endif
