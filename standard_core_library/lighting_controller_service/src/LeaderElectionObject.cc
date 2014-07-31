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

bool g_IsLeader = false;

using namespace lsf;
using namespace ajn;

static const char* ControllerServiceInterface[] = {
    ControllerServiceInterfaceName
};

class LeaderElectionObject::WorkerThread : public Thread {
  public:
    WorkerThread(LeaderElectionObject& object, services::AnnounceHandler& handler) : object(object), handler(handler), running(true) { }

    virtual ~WorkerThread() { }

    virtual void Run();
    virtual void Stop();

    void Go() { semaphore.Post(); }

  protected:

    LeaderElectionObject& object;
    services::AnnounceHandler& handler;
    volatile bool running;
    LSFSemaphore semaphore;
};

void LeaderElectionObject::WorkerThread::Run()
{
    while (running) {
        // wait for something to happen
        semaphore.Wait();

        if (!running) {
            return;
        }

        bool found;
        do {
            object.ClearCurrentState();
            services::AboutServiceApi::getInstance()->Announce();
            // wait one second for announcements to come in
            usleep(1000000);

            found = object.DoLeaderElection();

            // start over!
            // increment the semaphore so it will go again
        } while (!found);
    }
}

void LeaderElectionObject::WorkerThread::Stop()
{
    running = false;
    semaphore.Post();
}

class LeaderElectionObject::Handler : public ajn::services::AnnounceHandler,
    public BusAttachment::JoinSessionAsyncCB,
    public BusAttachment::SetLinkTimeoutAsyncCB,
    public SessionListener {
  public:
    Handler(LeaderElectionObject& elector) : elector(elector) { }

    // AnnounceHandler
    virtual void Announce(uint16_t version, uint16_t port, const char* busName, const ObjectDescriptions& objectDescs, const AboutData& aboutData);

    // BusAttachment::JoinSessionAsyncCB
    // when we are the joiner...
    virtual void JoinSessionCB(QStatus status, SessionId sessionId, const SessionOpts& opts, void* context) {
        QCC_DbgTrace(("JoinSessionCB(%s, %u)", QCC_StatusText(status), sessionId));
        elector.bus.EnableConcurrentCallbacks();
        elector.OnSessionJoined(status, sessionId);
    }

    virtual void SetLinkTimeoutCB(QStatus status, uint32_t timeout, void* context) {
        QCC_DbgTrace(("SetLinkTimeoutCB(%s, %u)", QCC_StatusText(status), timeout));
    }

    // SessionListener
    virtual void SessionLost(SessionId sessionId, SessionLostReason reason) {
        QCC_DbgTrace(("SessionLost(%u)", sessionId));
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
        uint64_t rank = 0;
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

        ait = aboutData.find("Rank");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: Rank missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("t", &rank);

        ait = aboutData.find("IsLeader");
        if (ait == aboutData.end()) {
            QCC_LogError(ER_FAIL, ("%s: IsLeader missing in About Announcement", __func__));
            return;
        }
        ait->second.Get("b", &isLeader);

        QCC_DbgPrintf(("%s: Received Announce: busName=%s port=%u", __func__, busName, port));
        elector.OnAnnounced(port, busName, rank, isLeader, deviceId);
    }
}



LeaderElectionObject::LeaderElectionObject(ControllerService& controller)
    : BusObject(LeaderElectionAndStateSyncObjectPath),
    controller(controller),
    bus(controller.GetBusAttachment()),
    handler(new Handler(*this)),
    myRank(0UL),
    leaderObj(NULL),
    blobChangedSignal(NULL),
    thread(new WorkerThread(*this, *handler))
{
    QCC_DbgTrace(("%s", __func__));
    ClearCurrentLeader();
}

LeaderElectionObject::~LeaderElectionObject()
{
    QCC_DbgTrace(("%s", __func__));
    if (handler) {
        delete handler;
        handler = NULL;
    }

    if (thread) {
        delete thread;
        thread = NULL;
    }

    if (leaderObj) {
        delete leaderObj;
        leaderObj = NULL;
    }
}


void LeaderElectionObject::OnAnnounced(ajn::SessionPort port, const char* busName, uint64_t rank, bool isLeader, const char* deviceId)
{
    QCC_DbgPrintf(("OnAnnounced(%u, %s, %s, %lu)", port, busName, (isLeader ? "true" : "false"), rank));

    if (isLeader) {
        controllersLock.Lock();
        ControllerEntry& entry = controllers[busName];
        entry.port = port;
        entry.busName = busName;
        entry.deviceId = deviceId;
        entry.rank = rank;
        entry.isLeader = isLeader;

        if (currentLeader.busName == busName) {
            // handle reannouncements
            // we are already connected/connecting to the leader
            controllersLock.Unlock();
            return;
        }

        currentLeader = entry;
        const SessionId oldSession = leaderObj ? leaderObj->GetSessionId() : 0;
        if (leaderObj) {
            delete leaderObj;
            leaderObj = NULL;
        }
        controllersLock.Unlock();

        // moving to a new session
        // this won't catch a join in progress
        if (oldSession) {
            QCC_DbgPrintf(("Leaving session %u", oldSession));
            bus.LeaveSession(oldSession);
        }

        SessionOpts opts;
        opts.isMultipoint = true;
        QCC_DbgPrintf(("Connecting to leader %s with rank %lu", busName, rank));
        bus.JoinSessionAsync(busName, port, handler, opts, handler);
    } else {
        controllersLock.Lock();
        ControllerEntry& entry = controllers[busName];
        entry.port = port;
        entry.busName = busName;
        entry.deviceId = deviceId;
        entry.rank = rank;
        entry.isLeader = isLeader;
        controllersLock.Unlock();
    }
}

void LeaderElectionObject::OnSessionLost(SessionId sessionId)
{
    QCC_DbgTrace(("LeaderElectionObject::OnSessionLost(%u)", sessionId));
    controllersLock.Lock();
    if (leaderObj && leaderObj->GetSessionId() == sessionId) {
        ClearCurrentLeader();
        delete leaderObj;
        leaderObj = NULL;
    }
    controllersLock.Unlock();

    // fire off the search thread
    thread->Go();
}

bool LeaderElectionObject::DoLeaderElection()
{
    QCC_DbgTrace(("%s", __func__));
    controllersLock.Lock();

    ControllerEntry* max_rank = NULL;
    uint64_t leaderRank = currentLeader.rank;

    // get the entry with the highest rank
    ControllerEntryMap::iterator it = controllers.begin();
    for (; it != controllers.end(); ++it) {
        ControllerEntry& entry = it->second;
        if (max_rank) {
            if (max_rank->rank < entry.rank) {
                max_rank = &entry;
            }
        } else {
            max_rank = &entry;
        }
    }

    controllersLock.Unlock();

    // nobody else there; I am the leader
    // or if the highest rank is lower than mine
    if ((max_rank == NULL || max_rank->rank < myRank) && !leaderRank) {
        QCC_DbgPrintf(("Set Leader bit = true and announce\n"));
        g_IsLeader = true;
        controller.SetIsLeader(true);
        controller.SetAllowUpdates(true);
        controller.GetLampManager().ConnectToLamps();
        return true;
    } else if (leaderRank) {
        // do nothing; we're already connected to a leader
        return true;
    }

    // else somebody else is the leader so we should wait for it to announce with isLeader=1
    // but what if the announcement never comes?

    // by returning false, we tell the thread to try again.
    return false;
}

void LeaderElectionObject::OnSessionMemberAdded(SessionId sessionId, const char* uniqueName)
{
    QCC_DbgTrace(("LeaderElectionObject::OnSessionMemberAdded(%u, %s)", sessionId, uniqueName));
    controllersLock.Lock();

    // clients won't appear in this map!
    ControllerEntryMap::const_iterator it = controllers.find(uniqueName);
    if (it != controllers.end()) {
        const ControllerEntry& entry = it->second;

        QCC_DbgTrace(("RANK: %lu, myRank: %lu", entry.rank, myRank));
        if (entry.rank > myRank) {
            controller.SetAllowUpdates(false);
        }
    }

    controllersLock.Unlock();
}

void LeaderElectionObject::OnSessionMemberRemoved(SessionId sessionId, const char* uniqueName)
{
    QCC_DbgTrace(("LeaderElectionObject::OnSessionMemberRemoved(%u, %s)", sessionId, uniqueName));
    controllersLock.Lock();

    // if we lost the leader it won't be in the map.
    // however if it's not the leader we want to forget about it unless it announces itself again.
    ControllerEntryMap::iterator it = controllers.find(uniqueName);
    if (it != controllers.end()) {
        controllers.erase(it);
    }

    if (leaderObj && leaderObj->GetServiceName() == uniqueName) {
        ClearCurrentLeader();
        // no need to treat this any different if the session was lost
        controllersLock.Unlock();
        // we don't want to keep this session alive anymore!
        OnSessionLost(sessionId);
    } else {
        controllersLock.Unlock();
    }
}

struct Synchronization {
    volatile int32_t numWaiting;
};

void LeaderElectionObject::OnGetBlobReply(ajn::Message& message, void* context)
{
    QCC_DbgTrace(("%s", __func__));
    bus.EnableConcurrentCallbacks();

    if (message->GetType() == MESSAGE_METHOD_RET) {
        size_t numArgs;
        const MsgArg* args;
        message->GetArgs(numArgs, args);

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
        // wait another second for announcements to come in
        thread->Go();
        // we're finished synchronizing!
        QCC_DbgPrintf(("Finished synchronizing!"));
        delete sync;

        PossiblyOverthrow();
    }
}


void LeaderElectionObject::OnGetChecksumAndModificationTimestampReply(ajn::Message& message, void* context)
{
    QCC_DbgTrace(("%s", __func__));
    bus.EnableConcurrentCallbacks();
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    MsgArg* elems;
    size_t numElems;
    args[0].Get("a(uut)", &numElems, &elems);
    std::list<LSFBlobType> storesToFetch;

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

        myTimestamp = GetTimestamp64() - myTimestamp;
        if (myTimestamp > timestamp && checksum != 0) {
            // need to call!
            storesToFetch.push_back(type);
        }
    }

    if (!storesToFetch.empty()) {
        Synchronization* sync = new Synchronization();
        if (!sync) {
            QCC_LogError(ER_FAIL, ("%s: Could not allocate memory for new Synchronization context", __func__));
            return;
        }
        sync->numWaiting = storesToFetch.size();
        QCC_DbgPrintf(("Going to synchronize %d types", sync->numWaiting));

        bool methodCallFailCount = 0;

        for (std::list<LSFBlobType>::iterator it = storesToFetch.begin(); it != storesToFetch.end(); ++it) {
            LSFBlobType type = *it;
            MsgArg arg("u", type);

            controllersLock.Lock();
            if (leaderObj) {
                QStatus status = leaderObj->MethodCallAsync(
                    LeaderElectionAndStateSyncInterfaceName,
                    "GetBlob",
                    this,
                    static_cast<MessageReceiver::ReplyHandler>(&LeaderElectionObject::OnGetBlobReply),
                    &arg,
                    1,
                    sync);
                if (status != ER_OK) {
                    methodCallFailCount++;
                    QCC_LogError(ER_FAIL, ("%s: Method Call Async failed", __func__));
                }
            }
            controllersLock.Unlock();
        }

        if (methodCallFailCount == storesToFetch.size()) {
            QCC_LogError(ER_FAIL, ("%s: All Method Call Asyncs failed", __func__));
            delete sync;
        }
    } else {
        // no need to sync anything; check if we're the new leader
        PossiblyOverthrow(); services::AboutServiceApi::getInstance()->Announce();
    }
}


// called when somebody joins *our* session
void LeaderElectionObject::OnSessionJoined(ajn::SessionId session, const char* joiner)
{
    QCC_DbgPrintf(("ControllerService::OnSessionJoined(%u, %s)", session, joiner));
    //OnSessionMemberAdded(session, joiner);
}

// called when we join another controller's session
void LeaderElectionObject::OnSessionJoined(QStatus status, SessionId sessionId)
{
    QCC_DbgPrintf(("ControllerService::OnJoined(%s, %u, %s)", QCC_StatusText(status), sessionId, currentLeader.busName.c_str()));

    if (status == ER_OK) {
        controllersLock.Lock();

        // we are connected to the new leader!
        if (leaderObj) {
            delete leaderObj;
            leaderObj = NULL;
        }

        leaderObj = new ProxyBusObject(bus, currentLeader.busName.c_str(), LeaderElectionAndStateSyncObjectPath, sessionId);

        if (!leaderObj) {
            controllersLock.Unlock();
            QCC_LogError(ER_FAIL, ("%s: Could not allocate memory for new ProxyBusObject", __func__));
            return;
        }

        // add the synchronization interface
        const InterfaceDescription* stateSyncInterface = bus.GetInterface(LeaderElectionAndStateSyncInterfaceName);
        leaderObj->AddInterface(*stateSyncInterface);
        controllersLock.Unlock();

        // we don't need to wait for this
        bus.SetLinkTimeoutAsync(sessionId, LINK_TIMEOUT, handler, NULL);

        leaderObj->MethodCallAsync(
            LeaderElectionAndStateSyncInterfaceName,
            "GetChecksumAndModificationTimestamp",
            this,
            static_cast<MessageReceiver::ReplyHandler>(&LeaderElectionObject::OnGetChecksumAndModificationTimestampReply),
            NULL,
            0);

        // we don't need to walk the list again because Join will be called by the Announce/Lost handlers
        // Remember, we could have multiple Joins in flight at the same time.  Only the one for the real leader will be kept
    } else {
        controllersLock.Lock();
        ControllerEntryMap::iterator it = controllers.find(currentLeader.busName);
        if (it != controllers.end()) {
            controllers.erase(it);
        }

        ClearCurrentLeader();
        controllersLock.Unlock();

        // wait another second for announcements to come in
        thread->Go();
    }
}

// do not call this method if controllersLock isn't locked!
void LeaderElectionObject::ClearCurrentLeader()
{
    QCC_DbgTrace(("%s", __func__));
    currentLeader.busName = "";
    currentLeader.rank = 0;
    currentLeader.isLeader = false;
    currentLeader.port = 0;
}

QStatus LeaderElectionObject::Start()
{
    QCC_DbgTrace(("%s", __func__));
    // can't get this in c'tor because it might not be initialized yet
    myRank = controller.GetRank();

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

    // announce before registering the handler and hope that my peers see this before I see an announcement with IsLeader=1
    services::AboutServiceApi::getInstance()->Announce();

    status = services::AnnouncementRegistrar::RegisterAnnounceHandler(bus, *handler, ControllerServiceInterface, 1);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to start the register Announce Handler", __func__));
        return status;
    }

    status = thread->Start();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to start the worker thread", __func__));
        return status;
    }

    thread->Go();
    return status;
}

QStatus LeaderElectionObject::Stop()
{
    QCC_DbgTrace(("%s", __func__));
    QStatus status = ER_OK;

    thread->Stop();

    return status;
}

QStatus LeaderElectionObject::Join()
{
    QStatus status = ER_OK;

    thread->Join();

    status = bus.UnregisterSignalHandler(
        this,
        static_cast<MessageReceiver::SignalHandler>(&LeaderElectionObject::OnBlobChanged),
        blobChangedSignal,
        LeaderElectionAndStateSyncObjectPath);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to start the unregister BlobChanged Handler", __func__));
    }

    return status;
}

void LeaderElectionObject::Overthrow(const InterfaceDescription::Member* member, ajn::Message& msg)
{
    QCC_DbgTrace(("%s", __func__));

    controller.SetAllowUpdates(false);

    g_IsLeader = false;
    controller.Overthrow();

    MsgArg arg("b", true);
    controller.SendMethodReply(msg, &arg, 1);

    thread->Go();
}

void LeaderElectionObject::PossiblyOverthrow()
{
    controllersLock.Lock();
    if (currentLeader.rank < myRank) {
        QCC_DbgPrintf(("Now must overthrow leader of rank %lu", currentLeader.rank));
        // no reply to this method!
        // restart the leader election process
        if (leaderObj) {
            leaderObj->MethodCallAsync(
                LeaderElectionAndStateSyncInterfaceName,
                "Overthrow",
                this,
                static_cast<MessageReceiver::ReplyHandler>(&LeaderElectionObject::OnOverthrowReply));
        }
    }

    controllersLock.Unlock();
}

void LeaderElectionObject::OnOverthrowReply(Message& message, void* context)
{
    QCC_DbgTrace(("%s", __func__));
    size_t numArgs;
    const MsgArg* args;
    message->GetArgs(numArgs, args);

    // now wait for announcements to refresh then announce we are the leader
    thread->Go();
}

QStatus LeaderElectionObject::SendBlobUpdate(SessionId session, LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp)
{
    if (!controller.IsLeader()) {
        return ER_OK;
    }

    QCC_DbgTrace(("%s", __func__));
    MsgArg args[4];
    args[0].Set("u", static_cast<uint32_t>(type));
    args[1].Set("s", strdupnew(blob.c_str()));
    args[1].SetOwnershipFlags(MsgArg::OwnsData);
    args[2].Set("u", checksum);
    args[3].Set("t", timestamp);

    return Signal(NULL, session, *blobChangedSignal, args, 4);
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

    uint64_t currentTimestamp = GetTimestamp64();
    out[0].Set("(uut)", static_cast<uint32_t>(LSF_PRESET), presetchecksum, currentTimestamp - presettimestamp);
    out[1].Set("(uut)", static_cast<uint32_t>(LSF_LAMP_GROUP), groupchecksum, currentTimestamp - grouptimestamp);
    out[2].Set("(uut)", static_cast<uint32_t>(LSF_SCENE), scenechecksum, currentTimestamp - scenetimestamp);
    out[3].Set("(uut)", static_cast<uint32_t>(LSF_MASTER_SCENE), masterscenechecksum, currentTimestamp - masterscenetimestamp);
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

void LeaderElectionObject::ClearCurrentState(void)
{
    controllersLock.Lock();
    ClearCurrentLeader();
    controllers.clear();
    controllersLock.Unlock();
    QStatus status = services::AnnouncementRegistrar::UnRegisterAnnounceHandler(bus, *handler, ControllerServiceInterface, 1);
    if (ER_OK == status) {
        QCC_DbgPrintf(("%s: UnRegisterAnnounceHandler successful", __func__));
        status = services::AnnouncementRegistrar::RegisterAnnounceHandler(bus, *handler, ControllerServiceInterface, 1);
        if (status == ER_OK) {
            QCC_DbgPrintf(("%s: RegisterAnnounceHandler successful", __func__));
        }
    }
}
