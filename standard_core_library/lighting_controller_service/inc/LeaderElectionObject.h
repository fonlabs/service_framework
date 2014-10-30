#ifndef LEADER_ELECTION_OBJECT_H
#define LEADER_ELECTION_OBJECT_H
/**
 * \ingroup ControllerService
 */
/**
 * @file
 * This file provides definitions for leader election object
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

#include <alljoyn/about/AnnounceHandler.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/BusObject.h>

#include <LSFTypes.h>
#include <Mutex.h>
#include <Alarm.h>
#include <OEM_CS_Config.h>
#include <Rank.h>

namespace lsf {

class ControllerService;
/**
 * LeaderElectionObject class. \n
 * Implementing the algorithm of leader election. \n
 * Implementing the election behavior of the entity as a leader and also as a follower. \n
 */
class LeaderElectionObject : public ajn::BusObject, public Thread, public AlarmListener, public OEM_CS_NetworkCallback {
  public:
    /**
     * constructor
     */
    LeaderElectionObject(ControllerService& controller);
    /**
     * destructor
     */
    ~LeaderElectionObject();
    /**
     * On announcement callback
     */
    void OnAnnounced(ajn::SessionPort port, const char* busName, Rank rank, bool isLeader, const char* deviceId);
    /**
     * Start threads
     */
    QStatus Start(Rank& rank);
    /**
     * Stop threads
     */
    void Stop(void);
    /**
     * Join threads
     */
    void Join(void);
    /**
     * get blob reply. \n
     * Get data and metadata about lamps
     */
    void SendGetBlobReply(ajn::Message& message, LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp);
    /**
     * Send blob update. \n
     * Get data and metadata about lamps
     */
    QStatus SendBlobUpdate(ajn::SessionId session, LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp);
    /**
     * send blob update. \n
     * Send data and metadata about lamps
     */
    QStatus SendBlobUpdate(LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp);
    /**
     * On session member removed
     */
    void OnSessionMemberRemoved(SessionId sessionId, const char* uniqueName);
    /**
     * Run thread
     */
    void Run(void);
    /**
     * Alarm triggered callback
     */
    void AlarmTriggered(void);
    /**
     * Get my rank
     */
    Rank& GetRank(void) {
        return myRank;
    }

    void Connected(void);

    void Disconnected(void);

  private:

    /**
     * Clear the state of the LeaderElectionObject
     */
    void ClearState(void);

    struct Synchronization {
        volatile int32_t numWaiting;
    };

    void GetChecksumAndModificationTimestamp(const ajn::InterfaceDescription::Member* member, ajn::Message& msg);
    void OnGetChecksumAndModificationTimestampReply(ajn::Message& message, void* context);

    void Overthrow(const ajn::InterfaceDescription::Member* member, ajn::Message& msg);
    void OnOverthrowReply(ajn::Message& message, void* context);

    void GetBlob(const ajn::InterfaceDescription::Member* member, ajn::Message& msg);
    void OnGetBlobReply(ajn::Message& message, void* context);

    void OnBlobChanged(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& msg);

    ControllerService& controller;
    BusAttachment& bus;

    class Handler;
    Handler* handler;

    // everything below is related to leader election
    struct ControllerEntry {
      public:
        ajn::SessionPort port;
        qcc::String busName;
        qcc::String deviceId;
        Rank rank;
        bool isLeader;

        void Clear() {
            busName = "";
            deviceId = "";
            rank = Rank();
            isLeader = false;
            port = 0;
        }
    };

    void OnSessionLost(SessionId sessionId);
    void OnSessionJoined(QStatus status, SessionId sessionId, void* context);

    typedef std::list<ajn::Message> OverThrowList;
    typedef std::map<Rank, ControllerEntry> ControllersMap;
    typedef std::map<Rank, uint32_t> SuccessfulJoinSessionReplies;
    typedef std::list<Rank> FailedJoinSessionReplies;
    typedef std::list<uint32_t> SessionLostList;
    typedef std::map<uint32_t, const char*> SessionMemberRemovedMap;
    typedef struct _CurrentLeader {
        ControllerEntry controllerDetails;
        ajn::ProxyBusObject proxyObj;

        void Clear(void) {
            controllerDetails.Clear();
            proxyObj = ProxyBusObject();
        }
    } CurrentLeader;

    Mutex currentLeaderMutex;
    CurrentLeader currentLeader;

    Mutex failedJoinSessionMutex;
    FailedJoinSessionReplies failedJoinSessions;

    Mutex successfulJoinSessionMutex;
    SuccessfulJoinSessionReplies successfulJoinSessions;

    Mutex sessionLostMutex;
    SessionLostList sessionLostList;

    Mutex sessionMemberRemovedMutex;
    SessionMemberRemovedMap sessionMemberRemoved;

    Mutex controllersMapMutex;
    ControllersMap controllersMap;

    Mutex overThrowListMutex;
    OverThrowList overThrowList;

    Rank myRank;

    Mutex connectionStateMutex;
    volatile sig_atomic_t isRunning;

    const ajn::InterfaceDescription::Member* blobChangedSignal;

    LSFSemaphore wakeSem;

    Mutex electionAlarmMutex;
    Alarm electionAlarm;

    volatile sig_atomic_t alarmTriggered;
    volatile sig_atomic_t isLeader;
    volatile sig_atomic_t startElection;
    volatile sig_atomic_t okToSetAlarm;
    volatile sig_atomic_t gotOverthrowReply;
    Mutex outGoingLeaderMutex;
    qcc::String outGoingLeaderBusName;
    Rank outgoingLeaderRank;
    Mutex upComingLeaderMutex;
    qcc::String upComingLeaderBusName;
    Rank upcomingLeaderRank;
};

}


#endif
