#ifndef LEADER_ELECTION_OBJECT_H
#define LEADER_ELECTION_OBJECT_H
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

namespace lsf {

class ControllerService;

class LeaderElectionObject : public ajn::BusObject {
  public:

    LeaderElectionObject(ControllerService& controller);

    ~LeaderElectionObject();

    void OnAnnounced(ajn::SessionPort port, const char* busName, uint64_t rank, bool isLeader, const char* deviceId);

    QStatus Start();

    QStatus Stop();

    QStatus Join();

    void GetChecksumAndModificationTimestamp(const ajn::InterfaceDescription::Member* member, ajn::Message& msg);
    void GetBlob(const ajn::InterfaceDescription::Member* member, ajn::Message& msg);

    QStatus SendBlobUpdate(ajn::SessionId session, LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp);

    void SendGetBlobReply(ajn::Message& message, LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp);

    void Overthrow(const ajn::InterfaceDescription::Member* member, ajn::Message& msg);

  private:

    void OnOverthrowReply(ajn::Message& message, void* context);

    void OnGetBlobReply(ajn::Message& message, void* context);

    void OnGetChecksumAndModificationTimestampReply(ajn::Message& message, void* context);

    void OnBlobChanged(const ajn::InterfaceDescription::Member* member, const char* sourcePath, ajn::Message& msg);

    ControllerService& controller;
    BusAttachment& bus;

    class Handler;
    Handler* handler;

    // everything below is related to leader election
    struct ControllerEntry {
        ajn::SessionPort port;
        qcc::String busName;
        qcc::String deviceId;
        uint64_t rank;
        bool isLeader;
    };

    void PossiblyOverthrow();
    void DoLeaderElection();
    void ClearCurrentLeader();
    void OnSessionLost(SessionId sessionId);
    void OnSessionJoined(QStatus status, SessionId sessionId);
    void OnSessionMemberRemoved(SessionId sessionId, const char* uniqueName);

    // map deviceId -> ControllerEntry
    typedef std::map<qcc::String, ControllerEntry> ControllerEntryMap;
    ControllerEntryMap controllers;
    ControllerEntry currentLeader;

    uint64_t myRank;

    ajn::ProxyBusObject* leaderObj;

    Mutex controllersLock;

    const ajn::InterfaceDescription::Member* blobChangedSignal;

    class WorkerThread;
    WorkerThread* thread;
};

}


#endif
