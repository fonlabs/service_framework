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

#include <LeaderElectionObject.h>
#include <ServiceDescription.h>
#include <qcc/Debug.h>
#include <ControllerService.h>
#include <OEM_CS_Config.h>

#include <Thread.h>
#include <LSFSemaphore.h>


#define QCC_MODULE "LEADER_ELECTION"

#define ELECTION_INTERVAL_IN_SECONDS 1

#define LEADER_ANNOUNCEMENT_WAIT_INTERVAL_IN_SECONDS 2

#define OVERTHROW_TIMEOUT_IN_SEC 5

#define OVERTHROW_TIMEOUT_IN_M_SEC 5000

bool g_IsLeader = false;

using namespace lsf;
using namespace ajn;

static const char* ControllerServiceInterface[] = {
    ControllerServiceInterfaceName
};

class LeaderElectionObject::Handler : public ajn::services::AnnounceHandler,
    public BusAttachment::JoinSessionAsyncCB,
    public BusAttachment::SetLinkTimeoutAsyncCB,
    public SessionListener {
  public:
    Handler(LeaderElectionObject& elector) : elector(elector) { }

    virtual void Announce(uint16_t version, uint16_t port, const char* busName, const ObjectDescriptions& objectDescs, const AboutData& aboutData);

    virtual void JoinSessionCB(QStatus status, SessionId sessionId, const SessionOpts& opts, void* context) {
        QCC_DbgTrace(("%s: (status=%s, sessionId=%u)", __func__, QCC_StatusText(status), sessionId));
        elector.bus.EnableConcurrentCallbacks();
        if (context) {
            elector.OnSessionJoined(status, sessionId, context);
        }
    }

    virtual void SetLinkTimeoutCB(QStatus status, uint32_t timeout, void* context) {
        QCC_DbgTrace(("SetLinkTimeoutCB(%s, %u)", QCC_StatusText(status), timeout));
    }

    virtual void SessionMemberRemoved(SessionId sessionId, const char* uniqueName) {
        QCC_DbgTrace(("%s: (sessionId=%u, uniqueName=%s)", __func__, sessionId, uniqueName));
        elector.bus.EnableConcurrentCallbacks();
        // we need to trigger a new election if the host has left the session
        elector.OnSessionMemberRemoved(sessionId, uniqueName);
    }

    virtual void SessionLost(SessionId sessionId, SessionLostReason reason) {
        QCC_DbgTrace(("%s: (sessionId=%u, reason=%u)", __func__, sessionId, reason));
        elector.bus.EnableConcurrentCallbacks();
        elector.OnSessionLost(sessionId);
    }

    LeaderElectionObject& elector;
};

void LeaderElectionObject::Handler::Announce(
    uint16_t version,
    uint16_t port,
    const char* busName,
    const ObjectDescriptions& objectDescs,
    const AboutData& aboutData)
{
    QCC_DbgTrace(("%s", __func__));
    elector.bus.EnableConcurrentCallbacks();
    ObjectDescriptions::const_iterator oit = objectDescs.find(ControllerServiceObjectPath);
    if (elector.bus.GetUniqueName() != busName && oit != objectDescs.end()) {
        AboutData::const_iterator ait;
        bool isLeader = false;
        const char* deviceId;

        QCC_DbgPrintf(("%s: About Data Dump", __func__));
        for (ait = aboutData.begin(); ait != aboutData.end(); ait++) {
            QCC_DbgPrintf(("%s: %s", ait->first.c_str(), ait->second.ToString().c_str()));
        }

        ait = aboutData.find("DeviceId");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: DeviceId missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("s", &deviceId);

        uint64_t higherBits = 0, lowerBits = 0;
        ait = aboutData.find("RankHigherBits");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: RankHigherBits missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("t", &higherBits);

        ait = aboutData.find("RankLowerBits");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: RankLowerBits missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("t", &lowerBits);

        Rank rank(higherBits, lowerBits);

        ait = aboutData.find("IsLeader");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: IsLeader missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("b", &isLeader);

        if ((rank > elector.GetRank()) || isLeader) {
            QCC_DbgPrintf(("%s: Received a potential leader or existent leader announcement", __func__, busName, port));
            elector.OnAnnounced(port, busName, rank, isLeader, deviceId);
        }
    }
}

LeaderElectionObject::LeaderElectionObject(ControllerService& controller)
    : BusObject(LeaderElectionAndStateSyncObjectPath),
    controller(controller),
    bus(controller.GetBusAttachment()),
    handler(new Handler(*this)),
    myRank(),
    isRunning(false),
    blobChangedSignal(NULL),
    electionAlarm(this),
    alarmTriggered(false),
    isLeader(false),
    startElection(true),
    okToSetAlarm(true),
    gotOverthrowReply(false)
{
    QCC_DbgTrace(("%s", __func__));
    currentLeader.Clear();
    controllersMap.clear();
    outGoingLeader.Clear();
    upComingLeader.Clear();
    controller.SetAllowUpdates(false);
}

LeaderElectionObject::~LeaderElectionObject()
{
    QCC_DbgTrace(("%s", __func__));

    QStatus status = bus.UnregisterSignalHandler(
        this,
        static_cast<MessageReceiver::SignalHandler>(&LeaderElectionObject::OnBlobChanged),
        blobChangedSignal,
        LeaderElectionAndStateSyncObjectPath);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to unregister BlobChanged Handler", __func__));
    }

    electionAlarmMutex.Lock();
    electionAlarm.Stop();
    electionAlarm.Join();
    electionAlarmMutex.Unlock();

    if (handler) {
        delete handler;
        handler = NULL;
    }
}

void LeaderElectionObject::OnAnnounced(ajn::SessionPort port, const char* busName, Rank rank, bool isLeader, const char* deviceId)
{
    QCC_DbgPrintf(("%s: (%u, %s, %s, %s)", __func__, port, busName, (isLeader ? "true" : "false"), rank.c_str()));

    ControllerEntry newEntry;
    newEntry.busName = busName;
    newEntry.deviceId = deviceId;
    newEntry.isLeader = isLeader;
    newEntry.port = port;
    newEntry.rank = rank;
    newEntry.announcementTimestamp = GetTimestampInMs();

    Rank lastTrackedRank = myRank;

    controllersMapMutex.Lock();
    if (controllersMap.size()) {
        lastTrackedRank = (controllersMap.rbegin())->first;
        QCC_DbgPrintf(("%s: Last tracked rank set to %s", __func__, lastTrackedRank.c_str()));
    }
    std::pair<ControllersMap::iterator, bool> ins = controllersMap.insert(std::make_pair(rank, newEntry));
    if (ins.second == false) {
        ins.first->second = newEntry;
    }
    controllersMapMutex.Unlock();

    if (okToSetAlarm && (!isLeader)) {
        if ((rank > lastTrackedRank) || (rank == lastTrackedRank)) {
            electionAlarmMutex.Lock();
            QCC_DbgPrintf(("%s: Reloading alarm", __func__));
            electionAlarm.SetAlarm(LEADER_ANNOUNCEMENT_WAIT_INTERVAL_IN_SECONDS);
            electionAlarmMutex.Unlock();
        }
    } else {
        wakeSem.Post();
    }
}

void LeaderElectionObject::OnSessionLost(SessionId sessionId)
{
    QCC_DbgTrace(("%s: (%u)", __func__, sessionId));
    sessionLostMutex.Lock();
    sessionLostList.push_back(static_cast<uint32_t>(sessionId));
    sessionLostMutex.Unlock();
    wakeSem.Post();
}

void LeaderElectionObject::OnSessionMemberRemoved(SessionId sessionId, const char* uniqueName)
{
    QCC_DbgTrace(("%s: (%u, %s)", __func__, sessionId, uniqueName));
    sessionMemberRemovedMutex.Lock();
    sessionMemberRemoved.insert(std::make_pair(static_cast<uint32_t>(sessionId), uniqueName));
    sessionMemberRemovedMutex.Unlock();
    wakeSem.Post();
}

void LeaderElectionObject::OnGetBlobReply(ajn::Message& message, void* context)
{
    QCC_DbgTrace(("%s", __func__));
    bus.EnableConcurrentCallbacks();

    QCC_DbgPrintf(("%s: %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
        size_t numArgs;
        const MsgArg* args;
        message->GetArgs(numArgs, args);

        if (controller.CheckNumArgsInMessage(numArgs, 4)  != LSF_OK) {
            return;
        }

        if (args[1].v_string.len) {
            switch (args[0].v_uint32) {
            case LSF_PRESET:
                controller.GetPresetManager().HandleReceivedBlob(args[1].v_string.str, args[2].v_uint32, args[3].v_uint64);
                break;

            case LSF_MASTER_SCENE:
                controller.GetMasterSceneManager().HandleReceivedBlob(args[1].v_string.str, args[2].v_uint32, args[3].v_uint64);
                break;

            case LSF_LAMP_GROUP:
                controller.GetLampGroupManager().HandleReceivedBlob(args[1].v_string.str, args[2].v_uint32, args[3].v_uint64);
                break;

            case LSF_SCENE:
                controller.GetSceneManager().HandleReceivedBlob(args[1].v_string.str, args[2].v_uint32, args[3].v_uint64);
                break;
            }
        }
    }

    Synchronization* sync = static_cast<Synchronization*>(context);
    if (0 == qcc::DecrementAndFetch(&sync->numWaiting)) {
        // we're finished synchronizing!
        QCC_DbgPrintf(("Finished synchronizing!"));
        delete sync;

        ControllerEntry outGoingLeaderCopy;
        outGoingLeaderMutex.Lock();
        outGoingLeaderCopy = outGoingLeader;
        outGoingLeaderMutex.Unlock();

        if (!outGoingLeaderCopy.busName.empty()) {
            QStatus status = currentLeader.proxyObj.MethodCallAsync(
                LeaderElectionAndStateSyncInterfaceName,
                "Overthrow",
                this,
                static_cast<MessageReceiver::ReplyHandler>(&LeaderElectionObject::OnOverthrowReply),
                NULL, 0, NULL, OVERTHROW_TIMEOUT_IN_M_SEC);

            if (status != ER_OK) {
                QCC_LogError(status, ("%s: MethodCallAsync for Overthrow failed", __func__));
                gotOverthrowReply = true;
                wakeSem.Post();
            }
        }
    }
}

void LeaderElectionObject::OnGetChecksumAndModificationTimestampReply(ajn::Message& message, void* context)
{
    QCC_DbgTrace(("%s", __func__));
    bus.EnableConcurrentCallbacks();
    QCC_DbgPrintf(("%s: %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
        size_t numArgs;
        const MsgArg* args;
        message->GetArgs(numArgs, args);

        if (controller.CheckNumArgsInMessage(numArgs, 1)  != LSF_OK) {
            return;
        }

        MsgArg* elems;
        size_t numElems;
        args[0].Get("a(uut)", &numElems, &elems);
        std::list<LSFBlobType> storesToFetch;
        std::list<LSFBlobType> storesToSend;

        for (size_t i = 0; i < numElems; ++i) {
            LSFBlobType type;
            uint32_t checksum;
            uint64_t timestamp;
            elems[i].Get("(uut)", &type, &checksum, &timestamp);

            uint32_t myChecksum;
            uint64_t myTimestamp;

            switch (type) {
            case LSF_PRESET:
                controller.GetPresetManager().GetBlobInfo(myChecksum, myTimestamp);
                break;

            case LSF_MASTER_SCENE:
                controller.GetMasterSceneManager().GetBlobInfo(myChecksum, myTimestamp);
                break;

            case LSF_LAMP_GROUP:
                controller.GetLampGroupManager().GetBlobInfo(myChecksum, myTimestamp);
                break;

            case LSF_SCENE:
                controller.GetSceneManager().GetBlobInfo(myChecksum, myTimestamp);
                break;
            }

            uint64_t currentTimeStamp = GetTimestampInMs();
            QCC_DbgPrintf(("%s: type=%d checksum=%u timestamp=%llu", __func__, type, checksum, timestamp));
            QCC_DbgPrintf(("%s: type=%d myChecksum=%u myTimestamp=%llu GetTimestampInMs=%llu", __func__, type, myChecksum, myTimestamp, currentTimeStamp));

            if ((myTimestamp != 0) && ((timestamp == 0) || ((currentTimeStamp - myTimestamp) < timestamp)) && (myChecksum != 0)) {
                QCC_DbgPrintf(("%s: Need to send a blob!", __func__));
                storesToSend.push_back(type);
            } else {
                QCC_DbgPrintf(("%s: No need to send blob!", __func__));
            }

            myTimestamp = currentTimeStamp - myTimestamp;
            if ((timestamp != 0) && (myTimestamp > timestamp) && (checksum != 0)) {
                QCC_DbgPrintf(("%s: Need to fetch blob!", __func__));
                storesToFetch.push_back(type);
            } else {
                QCC_DbgPrintf(("%s: No need to fetch blob!", __func__));
            }
        }

        QCC_DbgPrintf(("%s: Going to send blobs according to list of types I prepared", __func__));
        if (!storesToSend.empty()) {
            for (std::list<LSFBlobType>::iterator it = storesToSend.begin(); it != storesToSend.end(); ++it) {
                switch (*it) {
                case LSF_PRESET:
                    controller.GetPresetManager().TriggerUpdate();
                    break;

                case LSF_LAMP_GROUP:
                    controller.GetLampGroupManager().TriggerUpdate();
                    break;

                case LSF_SCENE:
                    controller.GetSceneManager().TriggerUpdate();
                    break;

                case LSF_MASTER_SCENE:
                    controller.GetMasterSceneManager().TriggerUpdate();
                    break;

                default:
                    QCC_LogError(ER_FAIL, ("%s: Unsupported blob type requested", __func__));
                    break;
                }
            }
        } else {
            QCC_DbgTrace(("%s: Nothing to send", __func__));
        }

        QCC_DbgPrintf(("%s: Going to fetch blobs according to list of types I prepared", __func__));
        if (!storesToFetch.empty()) {
            Synchronization* sync = new Synchronization();
            if (!sync) {
                QCC_LogError(ER_FAIL, ("%s: Could not allocate memory for new Synchronization context", __func__));
                return;
            }
            sync->numWaiting = storesToFetch.size();
            QCC_DbgPrintf(("Going to synchronize %d types", sync->numWaiting));

            uint8_t methodCallFailCount = 0;

            for (std::list<LSFBlobType>::iterator it = storesToFetch.begin(); it != storesToFetch.end(); ++it) {
                LSFBlobType type = *it;
                MsgArg arg("u", type);

                QStatus status = ER_OK;

                currentLeaderMutex.Lock();
                if (currentLeader.proxyObj.IsValid()) {
                    status = currentLeader.proxyObj.MethodCallAsync(
                        LeaderElectionAndStateSyncInterfaceName,
                        "GetBlob",
                        this,
                        static_cast<MessageReceiver::ReplyHandler>(&LeaderElectionObject::OnGetBlobReply),
                        &arg,
                        1,
                        sync,
                        OVERTHROW_TIMEOUT_IN_M_SEC);
                } else {
                    status = ER_FAIL;
                }
                currentLeaderMutex.Unlock();
                if (status != ER_OK) {
                    methodCallFailCount++;
                    qcc::DecrementAndFetch(&sync->numWaiting);
                    QCC_LogError(ER_FAIL, ("%s: Method Call Async failed", __func__));
                }
            }

            if (methodCallFailCount == storesToFetch.size()) {
                QCC_LogError(ER_FAIL, ("%s: All Method Call Asyncs failed", __func__));
                delete sync;
                gotOverthrowReply = true;
                wakeSem.Post();
            }
        } else {
            QCC_DbgTrace(("%s: Nothing to fetch", __func__));
            gotOverthrowReply = true;
            wakeSem.Post();
        }
    } else {
        QCC_DbgTrace(("%s: GetChecksumAndModificationTimestamp method call timed out", __func__));
        gotOverthrowReply = true;
        wakeSem.Post();
    }
}

void LeaderElectionObject::AlarmTriggered(void)
{
    QCC_DbgPrintf(("%s", __func__));
    alarmTriggered = true;
    wakeSem.Post();
}

void LeaderElectionObject::Connected(void)
{
    QCC_DbgPrintf(("%s", __func__));

    if (!(controller.IsRunning())) {
        QCC_LogError(ER_FAIL, ("%s: The Controller Service is not running", __func__));
        return;
    }

    connectionStateMutex.Lock();
    if (!isRunning) {
        isRunning = true;

        QStatus status = services::AnnouncementRegistrar::RegisterAnnounceHandler(bus, *handler, ControllerServiceInterface, 1);
        if (status != ER_OK) {
            QCC_LogError(status, ("%s: Failed to RegisterAnnounceHandler", __func__));
        }

        status = Thread::Start();
        if (status != ER_OK) {
            isRunning = false;
            QCC_LogError(status, ("%s: Unable to start Run() thread", __func__));
        }
    }
    wakeSem.Post();
    connectionStateMutex.Unlock();
}

void LeaderElectionObject::Disconnected(void)
{
    QCC_DbgPrintf(("%s", __func__));

    if (!(controller.IsRunning())) {
        QCC_LogError(ER_FAIL, ("%s: The Controller Service is not running", __func__));
        return;
    }

    connectionStateMutex.Lock();
    Stop();
    Join();
    connectionStateMutex.Unlock();
}

void LeaderElectionObject::ClearState(void)
{
    QCC_DbgPrintf(("%s", __func__));

    /* Clean up all the stale data */
    overThrowListMutex.Lock();
    overThrowList.clear();
    overThrowListMutex.Unlock();

    upComingLeaderMutex.Lock();
    upComingLeader.Clear();
    upComingLeaderMutex.Unlock();

    electionAlarmMutex.Lock();
    electionAlarm.SetAlarm(0);
    electionAlarmMutex.Unlock();

    controllersMapMutex.Lock();
    controllersMap.clear();
    controllersMapMutex.Unlock();

    alarmTriggered = false;
    gotOverthrowReply = false;
    okToSetAlarm = true;
    startElection = true;

    currentLeaderMutex.Lock();
    currentLeader.Clear();
    currentLeaderMutex.Unlock();

    outGoingLeaderMutex.Lock();
    outGoingLeader.Clear();
    outGoingLeaderMutex.Unlock();

    failedJoinSessionMutex.Lock();
    failedJoinSessions.clear();
    failedJoinSessionMutex.Unlock();

    sessionAlreadyJoinedRepliesMutex.Lock();
    sessionAlreadyJoinedReplies.clear();
    sessionAlreadyJoinedRepliesMutex.Unlock();

    successfulJoinSessionMutex.Lock();
    successfulJoinSessions.clear();
    successfulJoinSessionMutex.Unlock();

    sessionLostMutex.Lock();
    sessionLostList.clear();
    sessionLostMutex.Unlock();

    sessionMemberRemovedMutex.Lock();
    sessionMemberRemoved.clear();
    sessionMemberRemovedMutex.Unlock();
}

void LeaderElectionObject::Run(void)
{
    QCC_DbgPrintf(("%s", __func__));

    while (isRunning) {
        wakeSem.Wait();
        QCC_DbgPrintf(("%s: wakeSem posted", __func__));

        /*
         * We are shutting down. So announce self as non-leader before going away
         */
        if (!isRunning) {
            if (isLeader) {
                controller.LeaveSession();
                controller.GetLampManager().DisconnectFromLamps();
                controller.SetAllowUpdates(false);
                controller.SetIsLeader(false);
                isLeader = false;
                g_IsLeader = false;
            } else {
                ajn::SessionId sessionId = 0;
                currentLeaderMutex.Lock();
                sessionId = currentLeader.proxyObj.GetSessionId();
                currentLeader.Clear();
                currentLeaderMutex.Unlock();

                if (sessionId) {
                    QCC_DbgPrintf(("%s: Tearing down session with current leader", __func__));
                    controller.DoLeaveSessionAsync(sessionId);
                }
            }

            ClearState();

            goto _Exit;
        }

        bool loopBack = true;

        while (loopBack) {
            loopBack = false;
            if (isLeader) {
                QCC_DbgPrintf(("%s: I am the leader", __func__));

                bool overThrown = false;
                bool leaderFound = false;

                OverThrowList overThrowListCopy;
                overThrowListMutex.Lock();
                overThrowListCopy = overThrowList;
                overThrowList.clear();
                overThrowListMutex.Unlock();

                ControllerEntry upcomingLeaderCopy;
                upComingLeaderMutex.Lock();
                upcomingLeaderCopy = upComingLeader;
                upComingLeaderMutex.Unlock();

                MsgArg arg;
                while (overThrowListCopy.size()) {
                    if ((0 == strcmp(overThrowListCopy.front()->GetSender(), upcomingLeaderCopy.busName.c_str()))) {
                        arg.Set("b", true);
                        overThrown = true;
                        controller.LeaveSession();
                        QCC_DbgPrintf(("%s: Announcing self as non-leader after a successful overthrow", __func__));
                        controller.GetLampManager().DisconnectFromLamps();
                        controller.SetAllowUpdates(false);
                        controller.SetIsLeader(false);
                        isLeader = false;
                        g_IsLeader = false;

                        upComingLeaderMutex.Lock();
                        upComingLeader.Clear();
                        upComingLeaderMutex.Unlock();

                        electionAlarmMutex.Lock();
                        electionAlarm.SetAlarm(0);
                        electionAlarmMutex.Unlock();
                        /*
                         * Loopback so that we now become a follower to the new leader
                         */
                        loopBack = true;
                    } else {
                        arg.Set("b", false);
                    }
                    controller.SendMethodReply(overThrowListCopy.front(), &arg, 1);
                    overThrowListCopy.pop_front();
                }

                if (!overThrown) {
                    ControllersMap::reverse_iterator rit;
                    /*
                     * Check to see if we found another leader announcement
                     */
                    ControllerEntry controllerDetails;
                    controllersMapMutex.Lock();
                    for (rit = controllersMap.rbegin(); rit != controllersMap.rend(); rit++) {
                        if (rit->second.isLeader) {
                            leaderFound = true;
                            controllerDetails = rit->second;
                            break;
                        }
                    }
                    controllersMapMutex.Unlock();

                    if (leaderFound && (myRank < controllerDetails.rank)) {
                        /*
                         * We are being overthrown but we did not know about it as the new leader was probably not
                         * successful in connecting to us. Tear down our session and announce as a non-leader
                         */
                        controller.LeaveSession();
                        QCC_DbgPrintf(("%s: Announcing self as non-leader because I saw another leader announcement", __func__));
                        controller.GetLampManager().DisconnectFromLamps();
                        controller.SetAllowUpdates(false);
                        controller.SetIsLeader(false);
                        isLeader = false;
                        g_IsLeader = false;

                        upComingLeaderMutex.Lock();
                        upComingLeader.Clear();
                        upComingLeaderMutex.Unlock();

                        electionAlarmMutex.Lock();
                        electionAlarm.SetAlarm(0);
                        electionAlarmMutex.Unlock();
                        /*
                         * Loopback so that we now become a follower to the new leader
                         */
                        loopBack = true;
                    } else {
                        /*
                         * Check if some one is in the process of performing a coup on us
                         */
                        ControllerEntry upcomingLeaderCopy;
                        upComingLeaderMutex.Lock();
                        upcomingLeaderCopy = upComingLeader;
                        upComingLeaderMutex.Unlock();

                        controllersMapMutex.Lock();
                        for (rit = controllersMap.rbegin(); rit != controllersMap.rend(); rit++) {
                            if ((rit->second.rank > myRank) && (0 != strcmp(upcomingLeaderCopy.busName.c_str(), rit->second.busName.c_str()))) {
                                upComingLeaderMutex.Lock();
                                upComingLeader = rit->second;
                                upComingLeaderMutex.Unlock();
                                controller.SetAllowUpdates(false);
                                /*
                                 * Set an alarm to take necessary action if the coup does not go through
                                 */
                                electionAlarmMutex.Lock();
                                QCC_DbgPrintf(("%s: Extended overthrow alarm", __func__));
                                electionAlarm.SetAlarm(OVERTHROW_TIMEOUT_IN_SEC);
                                electionAlarmMutex.Unlock();
                                QCC_DbgPrintf(("%s: Identified upcoming leader %s", __func__, upComingLeader.busName.c_str()));
                                break;
                            }
                        }
                        controllersMapMutex.Unlock();
                    }

                    if (alarmTriggered) {
                        alarmTriggered = false;
                        if (!overThrown && !leaderFound) {
                            QCC_DbgPrintf(("%s: Upcoming leader %s failed to take over", __func__, upComingLeader.busName.c_str()));
                            controllersMapMutex.Lock();
                            ControllersMap::iterator it = controllersMap.find(upComingLeader.rank);
                            if (it != controllersMap.end()) {
                                if (it->second.isLeader) {
                                    /*
                                     * The leader announcement came through just as we were going to timeout.
                                     * So loopback
                                     */
                                    loopBack = true;
                                } else {
                                    if (upComingLeader.announcementTimestamp == it->second.announcementTimestamp) {
                                        controllersMap.erase(it);
                                    }
                                    upComingLeaderMutex.Lock();
                                    upComingLeader.Clear();
                                    upComingLeaderMutex.Unlock();

                                    controller.SetAllowUpdates(true);
                                }
                            }
                            controllersMapMutex.Unlock();
                        }
                    }
                }
            } else {
                if (gotOverthrowReply && (outGoingLeader.rank.IsInitialized())) {
                    QCC_DbgPrintf(("%s: gotOverthrowReply", __func__));
                    gotOverthrowReply = false;

                    ajn::SessionId sessionId = 0;
                    currentLeaderMutex.Lock();
                    sessionId = currentLeader.proxyObj.GetSessionId();
                    currentLeader.Clear();
                    currentLeaderMutex.Unlock();

                    if (sessionId) {
                        QCC_DbgPrintf(("%s: Tearing down session with current leader", __func__));
                        controller.DoLeaveSessionAsync(sessionId);
                    }
                    /*
                     * Coup was successful. Take over as leader
                     */
                    controllersMapMutex.Lock();
                    ControllersMap::iterator it = controllersMap.find(outGoingLeader.rank);
                    if ((it != controllersMap.end()) && (it->second.isLeader) && (it->second.announcementTimestamp == outGoingLeader.announcementTimestamp)) {
                        QCC_DbgPrintf(("%s: Removed outgoing leader with rank %s from controllersMap", __func__, outGoingLeader.rank.c_str()));
                        controllersMap.erase(it);
                    }
                    controllersMapMutex.Unlock();

                    outGoingLeaderMutex.Lock();
                    outGoingLeader.Clear();
                    outGoingLeaderMutex.Unlock();

                    QCC_DbgPrintf(("%s: Announcing self as leader", __func__));
                    controller.GetLampManager().ConnectToLamps();
                    controller.SetAllowUpdates(true);
                    controller.SetIsLeader(true);
                    isLeader = true;
                    g_IsLeader = true;
                    okToSetAlarm = false;
                    electionAlarmMutex.Lock();
                    electionAlarm.SetAlarm(0);
                    electionAlarmMutex.Unlock();
                } else if (startElection) {
                    QCC_DbgPrintf(("%s: startElection", __func__));
                    startElection = false;
                    QCC_DbgPrintf(("%s: Announcing self as non-leader", __func__));
                    controller.SetAllowUpdates(false);
                    controller.GetLampManager().DisconnectFromLamps();
                    controller.SetIsLeader(false);
                    electionAlarmMutex.Lock();
                    electionAlarm.SetAlarm(ELECTION_INTERVAL_IN_SECONDS);
                    electionAlarmMutex.Unlock();
                } else {
                    QCC_DbgPrintf(("%s: Third loop", __func__));
                    bool connectingToLeader = false;
                    bool lookingForALeader = false;

                    /*
                     * Find out if we are already connected to or trying to connect to a
                     * Leader
                     */
                    currentLeaderMutex.Lock();
                    if (!(currentLeader.controllerDetails.rank.IsInitialized())) {
                        lookingForALeader = true;
                    } else {
                        if (!(currentLeader.proxyObj.IsValid())) {
                            connectingToLeader = true;
                        }
                    }
                    currentLeaderMutex.Unlock();

                    if (lookingForALeader) {
                        QCC_DbgPrintf(("%s: lookingForALeader", __func__));
                        ControllersMap::reverse_iterator rit;
                        /*
                         * Check to see if we found a leader
                         */
                        ControllerEntry controllerDetails;
                        bool leaderFound = false;
                        controllersMapMutex.Lock();
                        for (rit = controllersMap.rbegin(); rit != controllersMap.rend(); rit++) {
                            if (rit->second.isLeader) {
                                leaderFound = true;
                                controllerDetails = rit->second;
                                break;
                            }
                        }
                        controllersMapMutex.Unlock();

                        if (leaderFound) {
                            QCC_DbgPrintf(("%s: Found an entry with rank=%s and leader bit set", __func__, rit->second.rank.c_str()));
                            /*
                             * Check if we need to perform a coup
                             */
                            if (myRank > controllerDetails.rank) {
                                outGoingLeaderMutex.Lock();
                                outGoingLeader = controllerDetails;
                                outGoingLeaderMutex.Unlock();
                            }
                            /*
                             * We found a leader. Try JoinSessionAsync with the leader
                             * and turn off the alarm if the call is successful
                             */
                            okToSetAlarm = false;

                            currentLeaderMutex.Lock();
                            currentLeader.Clear();
                            currentLeader.controllerDetails = controllerDetails;
                            currentLeaderMutex.Unlock();

                            ControllerEntry* ctx = new ControllerEntry();
                            *ctx = controllerDetails;
                            SessionOpts opts;
                            opts.isMultipoint = true;
                            QStatus status = bus.JoinSessionAsync(controllerDetails.busName.c_str(), controllerDetails.port, handler, opts, handler, ctx);
                            if (status != ER_OK) {
                                QCC_LogError(status, ("%s: JoinSessionAsync failed", __func__));
                                delete ctx;

                                controllersMapMutex.Lock();
                                ControllersMap::iterator fit = controllersMap.find(controllerDetails.rank);
                                if (controllerDetails.announcementTimestamp == fit->second.announcementTimestamp) {
                                    controllersMap.erase(fit);
                                }
                                controllersMapMutex.Unlock();

                                currentLeaderMutex.Lock();
                                currentLeader.Clear();
                                currentLeaderMutex.Unlock();
                            } else {
                                QCC_DbgPrintf(("%s: JoinSessionAsync successful", __func__));
                                electionAlarmMutex.Lock();
                                electionAlarm.SetAlarm(0);
                                electionAlarmMutex.Unlock();
                            }
                        } else {
                            bool takeOverAsLeader = false;
                            /*
                             * Check if we need to take over as the leader
                             */
                            controllersMapMutex.Lock();
                            /*
                             * If controllersMap is empty, we are the highest ranking Controller Service or
                             * if controllersMap has only one entry which does not have the leader bit set,
                             * we need to take over as the leader
                             */
                            if (controllersMap.empty() || ((controllersMap.size() == 1) && (!((controllersMap.begin())->second.isLeader)))) {
                                takeOverAsLeader = true;
                                controllersMap.clear();
                                alarmTriggered = false;
                            }
                            controllersMapMutex.Unlock();

                            if (takeOverAsLeader) {
                                QCC_DbgPrintf(("%s: Announcing self as leader", __func__));
                                controller.GetLampManager().ConnectToLamps();
                                controller.SetAllowUpdates(true);
                                controller.SetIsLeader(true);
                                isLeader = true;
                                g_IsLeader = true;
                                okToSetAlarm = false;
                                electionAlarmMutex.Lock();
                                electionAlarm.SetAlarm(0);
                                electionAlarmMutex.Unlock();
                            }
                        }
                    } else if (connectingToLeader) {
                        QCC_DbgPrintf(("%s: connectingToLeader", __func__));
                        /*
                         * Go through all the JoinSession replies
                         */
                        FailedJoinSessionReplies failedJoinSessionsCopy;
                        failedJoinSessionMutex.Lock();
                        failedJoinSessionsCopy = failedJoinSessions;
                        failedJoinSessions.clear();
                        failedJoinSessionMutex.Unlock();

                        FailedJoinSessionReplies sessionAlreadyJoinedRepliesCopy;
                        sessionAlreadyJoinedRepliesMutex.Lock();
                        sessionAlreadyJoinedRepliesCopy = sessionAlreadyJoinedReplies;
                        sessionAlreadyJoinedReplies.clear();
                        sessionAlreadyJoinedRepliesMutex.Unlock();

                        SuccessfulJoinSessionReplies successfulJoinSessionsCopy;
                        successfulJoinSessionMutex.Lock();
                        successfulJoinSessionsCopy = successfulJoinSessions;
                        successfulJoinSessions.clear();
                        successfulJoinSessionMutex.Unlock();

                        ControllerEntry failedConnectToLeader;

                        currentLeaderMutex.Lock();
                        while (failedJoinSessionsCopy.size()) {
                            if ((static_cast<ControllerEntry>(failedJoinSessionsCopy.front()).rank == currentLeader.controllerDetails.rank)
                                && (static_cast<ControllerEntry>(failedJoinSessionsCopy.front()).announcementTimestamp == currentLeader.controllerDetails.announcementTimestamp)) {
                                QCC_DbgPrintf(("%s: JoinSession failed with current leader", __func__));
                                failedConnectToLeader = currentLeader.controllerDetails;
                                currentLeader.Clear();
                                break;
                            }
                            failedJoinSessionsCopy.pop_front();
                        }
                        currentLeaderMutex.Unlock();

                        ControllerEntry outGoingLeaderCopy;
                        outGoingLeaderMutex.Lock();
                        outGoingLeaderCopy = outGoingLeader;
                        outGoingLeaderMutex.Unlock();

                        if (failedConnectToLeader.rank.IsInitialized()) {
                            if (!outGoingLeaderCopy.busName.empty()) {
                                /*
                                 * We are performing a coup but could not successfully talk to the
                                 * outgoing leader. So forcefully take over as the leader now
                                 */
                                controllersMapMutex.Lock();
                                ControllersMap::iterator it = controllersMap.find(outGoingLeader.rank);
                                if ((it != controllersMap.end()) && (it->second.isLeader) && (it->second.announcementTimestamp == outGoingLeader.announcementTimestamp)) {
                                    QCC_DbgPrintf(("%s: Removed outgoing leader with rank %s from controllersMap", __func__, outGoingLeader.rank.c_str()));
                                    controllersMap.erase(it);
                                }
                                controllersMapMutex.Unlock();

                                outGoingLeaderMutex.Lock();
                                outGoingLeader.Clear();
                                outGoingLeaderMutex.Unlock();

                                QCC_DbgPrintf(("%s: Announcing self as leader", __func__));
                                controller.GetLampManager().ConnectToLamps();
                                controller.SetAllowUpdates(true);
                                controller.SetIsLeader(true);
                                isLeader = true;
                                g_IsLeader = true;
                                okToSetAlarm = false;
                                electionAlarmMutex.Lock();
                                electionAlarm.SetAlarm(0);
                                electionAlarmMutex.Unlock();
                            }

                            controllersMapMutex.Lock();
                            ControllersMap::iterator it = controllersMap.find(failedConnectToLeader.rank);
                            /*
                             * We should delete the entry from controllersMap only if the leader bit is
                             * set. Otherwise it means that someone took over from this leader when
                             * we were trying to JoinSession
                             */
                            if ((it != controllersMap.end()) && (it->second.isLeader) && (it->second.announcementTimestamp == failedConnectToLeader.announcementTimestamp)) {
                                controllersMap.erase(it);
                            }
                            controllersMapMutex.Unlock();

                            /*
                             * Loop back to see if we have any other leader in the controllersMap
                             */
                            loopBack = true;
                        } else if (!sessionAlreadyJoinedRepliesCopy.empty()) {
                            /*
                             * Loop back to re-attempt a connect to the leader that returned session_already_joined error
                             */
                            loopBack = true;
                        } else {
                            currentLeaderMutex.Lock();
                            for (SuccessfulJoinSessionReplies::iterator it = successfulJoinSessionsCopy.begin(); it != successfulJoinSessionsCopy.end(); it++) {
                                if (((it->first) == currentLeader.controllerDetails.rank) && ((it->second.first.announcementTimestamp) == currentLeader.controllerDetails.announcementTimestamp)) {
                                    ajn::SessionId sessionId = static_cast<ajn::SessionId>(it->second.second);
                                    QCC_DbgPrintf(("%s: JoinSession successful with current leader", __func__));
                                    currentLeader.proxyObj = ProxyBusObject(bus, currentLeader.controllerDetails.busName.c_str(), LeaderElectionAndStateSyncObjectPath, sessionId);

                                    const InterfaceDescription* stateSyncInterface = bus.GetInterface(LeaderElectionAndStateSyncInterfaceName);
                                    currentLeader.proxyObj.AddInterface(*stateSyncInterface);

                                    ControllerEntry outGoingLeaderCopy;
                                    outGoingLeaderMutex.Lock();
                                    outGoingLeaderCopy = outGoingLeader;
                                    outGoingLeaderMutex.Unlock();

                                    if (!outGoingLeaderCopy.busName.empty()) {
                                        /*
                                         * Try to get current state from outgoing leader
                                         */
                                        QStatus status = currentLeader.proxyObj.MethodCallAsync(
                                            LeaderElectionAndStateSyncInterfaceName,
                                            "GetChecksumAndModificationTimestamp",
                                            this,
                                            static_cast<MessageReceiver::ReplyHandler>(&LeaderElectionObject::OnGetChecksumAndModificationTimestampReply),
                                            NULL, 0, NULL, OVERTHROW_TIMEOUT_IN_M_SEC);

                                        if (status != ER_OK) {
                                            QCC_LogError(status, ("%s: MethodCallAsync for GetChecksumAndModificationTimestamp failed", __func__));
                                            /*
                                             * Sending OverThrow failed. Forcefully take over as leader after tearing down
                                             * the session with the leader
                                             */
                                            ajn::SessionId sessionId = 0;
                                            currentLeaderMutex.Lock();
                                            sessionId = currentLeader.proxyObj.GetSessionId();
                                            currentLeader.Clear();
                                            currentLeaderMutex.Unlock();

                                            if (sessionId) {
                                                QCC_DbgPrintf(("%s: Tearing down session with current leader", __func__));
                                                controller.DoLeaveSessionAsync(sessionId);
                                            }

                                            controllersMapMutex.Lock();
                                            ControllersMap::iterator it = controllersMap.find(outGoingLeader.rank);
                                            if ((it != controllersMap.end()) && (it->second.isLeader) && (it->second.announcementTimestamp == outGoingLeader.announcementTimestamp)) {
                                                QCC_DbgPrintf(("%s: Removed outgoing leader with rank %s from controllersMap", __func__, outGoingLeader.rank.c_str()));
                                                controllersMap.erase(it);
                                            }
                                            controllersMapMutex.Unlock();

                                            outGoingLeaderMutex.Lock();
                                            outGoingLeader.Clear();
                                            outGoingLeaderMutex.Unlock();

                                            QCC_DbgPrintf(("%s: Announcing self as leader", __func__));
                                            controller.GetLampManager().ConnectToLamps();
                                            controller.SetAllowUpdates(true);
                                            controller.SetIsLeader(true);
                                            isLeader = true;
                                            g_IsLeader = true;
                                            okToSetAlarm = false;
                                            electionAlarmMutex.Lock();
                                            electionAlarm.SetAlarm(0);
                                            electionAlarmMutex.Unlock();
                                        }
                                    } else {
                                        // we don't need to wait for this
                                        QStatus status = bus.SetLinkTimeoutAsync(sessionId, LSF_MIN_LINK_TIMEOUT_IN_SECONDS, handler, NULL);
                                        if (status != ER_OK) {
                                            QCC_LogError(status, ("%s: SetLinkTimeoutAsync failed", __func__));
                                        }

                                        status = currentLeader.proxyObj.MethodCallAsync(
                                            LeaderElectionAndStateSyncInterfaceName,
                                            "GetChecksumAndModificationTimestamp",
                                            this,
                                            static_cast<MessageReceiver::ReplyHandler>(&LeaderElectionObject::OnGetChecksumAndModificationTimestampReply),
                                            NULL,
                                            0);
                                        if (status != ER_OK) {
                                            QCC_LogError(status, ("%s: MethodCallAsync for GetChecksumAndModificationTimestamp failed", __func__));
                                        }
                                    }

                                    successfulJoinSessionsCopy.erase(it);
                                    break;
                                }
                            }
                            currentLeaderMutex.Unlock();

                            /*
                             * Cleaning up the remaining stray sessions
                             */
                            for (SuccessfulJoinSessionReplies::iterator it = successfulJoinSessionsCopy.begin(); it != successfulJoinSessionsCopy.end(); it++) {
                                QCC_DbgPrintf(("%s: DoLeaveSessionAsync on stray session %d", __func__, it->second.second));
                                controller.DoLeaveSessionAsync(static_cast<ajn::SessionId>(it->second.second));
                            }
                        }
                    } else {
                        QCC_DbgPrintf(("%s: connectedToLeader", __func__));
                        /*
                         * Check if we have lost our session with the leader or if the leader has left the session
                         */
                        SessionLostList sessionLostListCopy;
                        sessionLostMutex.Lock();
                        sessionLostListCopy = sessionLostList;
                        sessionLostList.clear();
                        sessionLostMutex.Unlock();

                        QCC_DbgPrintf(("%s: Made sessionLostList copy", __func__));

                        SessionMemberRemovedMap sessionMemberRemovedCopy;
                        sessionMemberRemovedMutex.Lock();
                        sessionMemberRemovedCopy = sessionMemberRemoved;
                        sessionMemberRemoved.clear();
                        sessionMemberRemovedMutex.Unlock();

                        QCC_DbgPrintf(("%s: Made sessionMemberRemoved copy", __func__));

                        bool lostSessionWithLeader = false;
                        ajn::SessionId sessionId = 0;
                        const char* leaderUniqueName = NULL;

                        currentLeaderMutex.Lock();
                        sessionId = static_cast<uint32_t>(currentLeader.proxyObj.GetSessionId());
                        leaderUniqueName = currentLeader.controllerDetails.busName.c_str();
                        currentLeaderMutex.Unlock();

                        QCC_DbgPrintf(("%s: Made copy of current leader details", __func__));

                        SessionLostList::iterator it = std::find(sessionLostListCopy.begin(), sessionLostListCopy.end(), static_cast<uint32_t>(sessionId));
                        if (it != sessionLostListCopy.end()) {
                            QCC_DbgPrintf(("%s: Lost Session with the current leader", __func__));
                            lostSessionWithLeader = true;
                        }

                        if (!lostSessionWithLeader) {
                            QCC_DbgPrintf(("%s: No useful session losts. Looking at sessionMemberRemovedCopy", __func__));
                            SessionMemberRemovedMap::iterator it = sessionMemberRemovedCopy.find(static_cast<uint32_t>(currentLeader.proxyObj.GetSessionId()));
                            if ((it != sessionMemberRemovedCopy.end()) && (0 == strcmp(it->second, leaderUniqueName))) {
                                QCC_DbgPrintf(("%s: Current Leader left the session", __func__));
                                controller.DoLeaveSessionAsync(sessionId);
                                lostSessionWithLeader = true;
                            }
                        }

                        if (lostSessionWithLeader) {
                            QCC_DbgPrintf(("%s: SessionLost", __func__));
                            ControllerEntry failedConnectToLeader;

                            currentLeaderMutex.Lock();
                            QCC_DbgPrintf(("%s: Cleared current leader with rank %s", __func__, currentLeader.controllerDetails.rank.c_str()));
                            failedConnectToLeader = currentLeader.controllerDetails;
                            currentLeader.Clear();
                            currentLeaderMutex.Unlock();

                            controllersMapMutex.Lock();
                            ControllersMap::iterator it = controllersMap.find(failedConnectToLeader.rank);
                            /*
                             * We should delete the entry from controllersMap only if the leader bit is
                             * set and announcement timestamp is the same.
                             */
                            if ((it != controllersMap.end()) && (it->second.isLeader) && (it->second.announcementTimestamp == failedConnectToLeader.announcementTimestamp)) {
                                QCC_DbgPrintf(("%s: Removing entry with rank %s from controllersMap", __func__, it->second.rank.c_str()));
                                controllersMap.erase(it);
                            }
                            controllersMapMutex.Unlock();

                            /*
                             * Loop Back to find and connect to a new leader who may have come up or
                             * take over as the leader
                             */
                            loopBack = true;
                        } else {
                            QCC_DbgPrintf(("%s: No SessionLost", __func__));
                            /*
                             * Check if a new leader with rank higher that whom we think is the leader has come up. If so,
                             * leave our current session and join the new leader
                             */
                            ControllerEntry currentLeaderCopy;
                            currentLeaderMutex.Lock();
                            currentLeaderCopy = currentLeader.controllerDetails;
                            currentLeaderMutex.Unlock();

                            ControllerEntry entry;
                            entry.Clear();
                            controllersMapMutex.Lock();
                            ControllersMap::iterator it = controllersMap.find(currentLeaderCopy.rank);
                            if (it != controllersMap.end()) {
                                for (ControllersMap::reverse_iterator rit = controllersMap.rbegin(); rit != controllersMap.rend(); rit++) {
                                    if ((rit->second.rank > currentLeaderCopy.rank) && (rit->second.isLeader)) {
                                        QCC_DbgPrintf(("%s: Found a leader that has higher rank than current leader", __func__));
                                        entry = rit->second;
                                        break;
                                    }
                                }
                            }
                            if ((entry.rank.IsInitialized()) && (it->second.isLeader) && (it->second.announcementTimestamp == currentLeaderCopy.announcementTimestamp)) {
                                QCC_DbgPrintf(("%s: Removing current leader %s entry from controllersMap", __func__, currentLeaderCopy.rank.c_str()));
                                controllersMap.erase(it);
                            }
                            controllersMapMutex.Unlock();

                            if (entry.rank.IsInitialized()) {
                                ajn::SessionId session = 0;
                                currentLeaderMutex.Lock();
                                session = currentLeader.proxyObj.GetSessionId();
                                currentLeader.Clear();
                                currentLeaderMutex.Unlock();

                                if (session) {
                                    controller.DoLeaveSessionAsync(session);
                                    loopBack = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

_Exit:
    QCC_DbgPrintf(("%s: Exiting", __func__));
}

// called when we join another controller's session
void LeaderElectionObject::OnSessionJoined(QStatus status, SessionId sessionId, void* context)
{
    ControllerEntry* ctx = static_cast<ControllerEntry*>(context);
    QCC_DbgPrintf(("%s: (status=%s sessionId=%u rank=%s)", __func__, QCC_StatusText(status), sessionId, (*ctx).rank.c_str()));

    if (status == ER_OK) {
        successfulJoinSessionMutex.Lock();
        successfulJoinSessions.insert(std::make_pair((*ctx).rank, std::make_pair((*ctx), static_cast<uint32_t>(sessionId))));
        successfulJoinSessionMutex.Unlock();
    } else {
        if (status == ER_ALLJOYN_JOINSESSION_REPLY_ALREADY_JOINED) {
            sessionAlreadyJoinedRepliesMutex.Lock();
            sessionAlreadyJoinedReplies.push_back(*ctx);
            sessionAlreadyJoinedRepliesMutex.Unlock();
        } else {
            failedJoinSessionMutex.Lock();
            failedJoinSessions.push_back(*ctx);
            failedJoinSessionMutex.Unlock();
        }
    }

    delete ctx;
    wakeSem.Post();
}

QStatus LeaderElectionObject::Start(Rank& rank)
{
    QCC_DbgTrace(("%s", __func__));

    // can't get this in c'tor because it might not be initialized yet
    myRank = rank;

    QStatus status;
    status = bus.CreateInterfacesFromXml(LeaderElectionAndStateSyncDescription.c_str());
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to CreateInterfacesFromXml", __func__));
        return status;
    }

    const InterfaceDescription* stateSyncInterface = bus.GetInterface(LeaderElectionAndStateSyncInterfaceName);

    if (!stateSyncInterface) {
        QCC_LogError(status, ("%s: Failed to get a valid state sync interface", __func__));
        return status;
    }

    AddInterface(*stateSyncInterface);

    const MethodEntry methodEntries[] = {
        { stateSyncInterface->GetMember("GetChecksumAndModificationTimestamp"), static_cast<MessageReceiver::MethodHandler>(&LeaderElectionObject::GetChecksumAndModificationTimestamp) },
        { stateSyncInterface->GetMember("GetBlob"), static_cast<MessageReceiver::MethodHandler>(&LeaderElectionObject::GetBlob) },
        { stateSyncInterface->GetMember("Overthrow"), static_cast<MessageReceiver::MethodHandler>(&LeaderElectionObject::Overthrow) }
    };

    status = AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(MethodEntry));
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to AddMethodHandlers", __func__));
        return status;
    }

    blobChangedSignal = stateSyncInterface->GetSignal("BlobChanged");
    status = bus.RegisterSignalHandler(
        this,
        static_cast<MessageReceiver::SignalHandler>(&LeaderElectionObject::OnBlobChanged),
        blobChangedSignal,
        LeaderElectionAndStateSyncObjectPath);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to register BlobChanged signal handler", __func__));
        return status;
    }

    status = bus.RegisterBusObject(*this);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to register BusObject for the Leader Object", __func__));
        return status;
    }

    if (OEM_CS_IsNetworkConnected()) {
        QCC_DbgPrintf(("%s: Device is connected to the network. Starting the leader election thread", __func__));
        Connected();
    } else {
        QCC_DbgPrintf(("%s: Device is not connected to the network. Waiting for the connected callback from the firmware", __func__));
    }

    return status;
}

void LeaderElectionObject::Stop()
{
    QCC_DbgTrace(("%s", __func__));
    if (isRunning) {
        isRunning = false;

        QStatus status = services::AnnouncementRegistrar::UnRegisterAnnounceHandler(bus, *handler, ControllerServiceInterface, 1);
        if (status != ER_OK) {
            QCC_LogError(status, ("%s: Failed to UnRegisterAnnounceHandler", __func__));
        }

        wakeSem.Post();
    }
}

void LeaderElectionObject::Join()
{
    Thread::Join();
}

void LeaderElectionObject::Overthrow(const InterfaceDescription::Member* member, ajn::Message& msg)
{
    QCC_DbgTrace(("%s", __func__));

    ControllerEntry upcomingLeaderCopy;
    upComingLeaderMutex.Lock();
    upcomingLeaderCopy = upComingLeader;
    upComingLeaderMutex.Unlock();

    if (0 == strcmp(msg->GetSender(), upcomingLeaderCopy.busName.c_str())) {
        electionAlarmMutex.Lock();
        QCC_DbgPrintf(("%s: Extended overthrow alarm", __func__));
        electionAlarm.SetAlarm(OVERTHROW_TIMEOUT_IN_SEC);
        electionAlarmMutex.Unlock();
    }

    overThrowListMutex.Lock();
    overThrowList.push_back(msg);
    overThrowListMutex.Unlock();

    wakeSem.Post();
}

void LeaderElectionObject::OnOverthrowReply(Message& message, void* context)
{
    QCC_DbgPrintf(("%s: %s", __func__, (MESSAGE_METHOD_RET == message->GetType()) ? message->ToString().c_str() : "ERROR"));

    if (message->GetType() == ajn::MESSAGE_METHOD_RET) {
        size_t numArgs;
        const MsgArg* args;
        message->GetArgs(numArgs, args);

        if (controller.CheckNumArgsInMessage(numArgs, 1)  != LSF_OK) {
            return;
        }

        bool success;
        args[0].Get("b", &success);

        QCC_DbgTrace(("%s: success = %d", __func__, success));
    } else {
        QCC_DbgTrace(("%s: Overthrow method call timed out", __func__));
    }

    gotOverthrowReply = true;
    wakeSem.Post();
}

QStatus LeaderElectionObject::SendBlobUpdate(SessionId session, LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp)
{
    if (!controller.IsLeader()) {
        return ER_OK;
    }

    QCC_DbgTrace(("%s: Signal(session=%u)", __func__, session));
    MsgArg args[4];
    args[0].Set("u", static_cast<uint32_t>(type));
    args[1].Set("s", strdupnew(blob.c_str()));
    args[1].SetOwnershipFlags(MsgArg::OwnsData);
    args[2].Set("u", checksum);
    args[3].Set("t", timestamp);

    return Signal(NULL, session, *blobChangedSignal, args, 4);
}

QStatus LeaderElectionObject::SendBlobUpdate(LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp)
{
    QCC_DbgTrace(("%s", __func__));

    ajn::SessionId session = 0;

    currentLeaderMutex.Lock();
    if (currentLeader.proxyObj.IsValid()) {
        session = currentLeader.proxyObj.GetSessionId();
    }
    currentLeaderMutex.Unlock();

    if (session) {
        MsgArg args[4];
        args[0].Set("u", static_cast<uint32_t>(type));
        args[1].Set("s", strdupnew(blob.c_str()));
        args[1].SetOwnershipFlags(MsgArg::OwnsData);
        args[2].Set("u", checksum);
        args[3].Set("t", timestamp);
        QCC_DbgTrace(("%s: Signal(session=%u)", __func__, session));
        return Signal(NULL, session, *blobChangedSignal, args, 4);
    }

    return ER_FAIL;
}

void LeaderElectionObject::SendGetBlobReply(ajn::Message& message, LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp)
{
    QCC_DbgTrace(("%s", __func__));
    MsgArg args[4];
    args[0].Set("u", static_cast<uint32_t>(type));
    args[1].Set("s", strdupnew(blob.c_str()));
    args[1].SetOwnershipFlags(MsgArg::OwnsData);
    args[2].Set("u", checksum);
    args[3].Set("t", timestamp);

    controller.SendMethodReply(message, args, 4);
}

void LeaderElectionObject::GetChecksumAndModificationTimestamp(const ajn::InterfaceDescription::Member* member, ajn::Message& message)
{
    QCC_DbgTrace(("%s", __func__));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    ControllerEntry upcomingLeaderCopy;
    upComingLeaderMutex.Lock();
    upcomingLeaderCopy = upComingLeader;
    upComingLeaderMutex.Unlock();

    if (0 == strcmp(message->GetSender(), upcomingLeaderCopy.busName.c_str())) {
        electionAlarmMutex.Lock();
        QCC_DbgPrintf(("%s: Extended overthrow alarm", __func__));
        electionAlarm.SetAlarm(OVERTHROW_TIMEOUT_IN_SEC);
        electionAlarmMutex.Unlock();
    }

    MsgArg outArg;
    MsgArg* out = new MsgArg[4];

    uint32_t presetchecksum;
    uint64_t presettimestamp;
    controller.GetPresetManager().GetBlobInfo(presetchecksum, presettimestamp);

    uint32_t groupchecksum;
    uint64_t grouptimestamp;
    controller.GetLampGroupManager().GetBlobInfo(groupchecksum, grouptimestamp);

    uint32_t scenechecksum;
    uint64_t scenetimestamp;
    controller.GetSceneManager().GetBlobInfo(scenechecksum, scenetimestamp);

    uint32_t masterscenechecksum;
    uint64_t masterscenetimestamp;
    controller.GetMasterSceneManager().GetBlobInfo(masterscenechecksum, masterscenetimestamp);

    uint64_t currentTimestamp = GetTimestampInMs();
    uint64_t blobTimeStamp = 0;
    if (presettimestamp != 0) {
        blobTimeStamp = currentTimestamp - presettimestamp;
    }
    out[0].Set("(uut)", static_cast<uint32_t>(LSF_PRESET), presetchecksum, blobTimeStamp);
    blobTimeStamp = 0;
    if (grouptimestamp != 0) {
        blobTimeStamp = currentTimestamp - grouptimestamp;
    }
    out[1].Set("(uut)", static_cast<uint32_t>(LSF_LAMP_GROUP), groupchecksum, blobTimeStamp);
    blobTimeStamp = 0;
    if (scenetimestamp != 0) {
        blobTimeStamp = currentTimestamp - scenetimestamp;
    }
    out[2].Set("(uut)", static_cast<uint32_t>(LSF_SCENE), scenechecksum, blobTimeStamp);
    blobTimeStamp = 0;
    if (masterscenetimestamp != 0) {
        blobTimeStamp = currentTimestamp - masterscenetimestamp;
    }
    out[3].Set("(uut)", static_cast<uint32_t>(LSF_MASTER_SCENE), masterscenechecksum, blobTimeStamp);
    outArg.Set("a(uut)", 4, out);
    outArg.SetOwnershipFlags(MsgArg::OwnsArgs | MsgArg::OwnsData, true);

    MethodReply(message, &outArg, 1);
}

void LeaderElectionObject::GetBlob(const ajn::InterfaceDescription::Member* member, ajn::Message& message)
{
    QCC_DbgTrace(("%s", __func__));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controller.CheckNumArgsInMessage(numArgs, 1)  != LSF_OK) {
        return;
    }

    ControllerEntry upcomingLeaderCopy;
    upComingLeaderMutex.Lock();
    upcomingLeaderCopy = upComingLeader;
    upComingLeaderMutex.Unlock();

    if (0 == strcmp(message->GetSender(), upcomingLeaderCopy.busName.c_str())) {
        electionAlarmMutex.Lock();
        QCC_DbgPrintf(("%s: Extended overthrow alarm", __func__));
        electionAlarm.SetAlarm(OVERTHROW_TIMEOUT_IN_SEC);
        electionAlarmMutex.Unlock();
    }

    switch (static_cast<LSFBlobType>(args[0].v_uint32)) {
    case LSF_PRESET:
        controller.GetPresetManager().ScheduleFileRead(message);
        break;

    case LSF_LAMP_GROUP:
        controller.GetLampGroupManager().ScheduleFileRead(message);
        break;

    case LSF_SCENE:
        controller.GetSceneManager().ScheduleFileRead(message);
        break;

    case LSF_MASTER_SCENE:
        controller.GetMasterSceneManager().ScheduleFileRead(message);
        break;

    default:
        QCC_LogError(ER_FAIL, ("%s: Unsupported blob type requested", __func__));
        break;
    }
}

void LeaderElectionObject::OnBlobChanged(const InterfaceDescription::Member* member, const char* sourcePath, Message& message)
{
    QCC_DbgTrace(("%s", __func__));
    bus.EnableConcurrentCallbacks();
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    if (controller.CheckNumArgsInMessage(numArgs, 4)  != LSF_OK) {
        return;
    }

    LSFBlobType type = static_cast<LSFBlobType>(args[0].v_uint32);
    std::string blob = args[1].v_string.str;
    uint32_t checksum = args[2].v_uint32;
    uint64_t timestamp = args[3].v_uint64;

    switch (type) {
    case LSF_PRESET:
        controller.GetPresetManager().HandleReceivedBlob(blob, checksum, timestamp);
        break;

    case LSF_LAMP_GROUP:
        controller.GetLampGroupManager().HandleReceivedBlob(blob, checksum, timestamp);
        break;

    case LSF_SCENE:
        controller.GetSceneManager().HandleReceivedBlob(blob, checksum, timestamp);
        break;

    case LSF_MASTER_SCENE:
        controller.GetMasterSceneManager().HandleReceivedBlob(blob, checksum, timestamp);
        break;

    default:
        QCC_LogError(ER_FAIL, ("%s: Unsupported blob type requested", __func__));
        break;
    }
}
