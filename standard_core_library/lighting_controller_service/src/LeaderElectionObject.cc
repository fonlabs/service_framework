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

#define QCC_MODULE "LEADER_ELECTION"

bool g_IsLeader = false;

using namespace lsf;
using namespace ajn;

class LeaderElectionObject::Handler : public ajn::services::AnnounceHandler, public BusAttachment::JoinSessionAsyncCB, public SessionListener {
  public:
    Handler(LeaderElectionObject& elector) : elector(elector) { }

    // AnnounceHandler
    virtual void Announce(uint16_t version, uint16_t port, const char* busName, const ObjectDescriptions& objectDescs, const AboutData& aboutData);

    // BusAttachment::JoinSessionAsyncCB
    // when we are the joiner...
    virtual void JoinSessionCB(QStatus status, SessionId sessionId, const SessionOpts& opts, void* context) {
        QCC_DbgTrace(("%s", __func__));
        elector.bus.EnableConcurrentCallbacks();
        ControllerEntry* joined = static_cast<ControllerEntry*>(context);
        elector.OnSessionJoined(status, sessionId, joined);
    }

    // SessionListener
    virtual void SessionLost(SessionId sessionId, SessionLostReason reason) {
        QCC_DbgTrace(("%s", __func__));
        elector.bus.EnableConcurrentCallbacks();
        elector.OnSessionLost(sessionId);
    }

    virtual void SessionMemberRemoved(SessionId sessionId, const char* uniqueName) {
        QCC_DbgTrace(("%s", __func__));
        elector.bus.EnableConcurrentCallbacks();
        elector.OnSessionMemberRemoved(sessionId, uniqueName);
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
        uint32_t isLeader = 0;
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
        ait->second.Get("u", &isLeader);

        QCC_DbgPrintf(("%s: Received Announce: busName=%s port=%u", __func__, busName, port));
        elector.OnAnnounced(port, busName, rank, isLeader, deviceId);
    }
}



LeaderElectionObject::LeaderElectionObject(ControllerService& controller)
    : BusObject(LeaderElectionAndStateSyncObjectPath),
    controller(controller),
    bus(controller.GetBusAttachment()),
    handler(new Handler(*this)),
    isLeader(true),
    myRank(0UL),
    leaderObj(NULL),
    blobChangedSignal(NULL)
{
    QCC_DbgTrace(("%s", __func__));
    ClearCurrentLeader();
}

LeaderElectionObject::~LeaderElectionObject()
{
    QCC_DbgTrace(("%s", __func__));
    delete handler;
}


void LeaderElectionObject::OnAnnounced(ajn::SessionPort port, const char* busName, uint64_t rank, uint32_t isLeader, const char* deviceId)
{
    QCC_DbgPrintf(("OnAnnounced(%u, %s, %lu)", port, busName, rank));
    controllersLock.Lock();

    // if we're already connecting to a lower-ranked CS when the announcement comes in,
    // don't worry about it!  When the session is accepted it will be cleaned up
    // by OnSessionJoined

    nameToId[busName] = deviceId;

    // find or insert
    ControllerEntry& entry = controllers[deviceId];
    entry.port = port;
    entry.busName = busName;
    entry.deviceId = deviceId;
    entry.rank = rank;
    entry.isLeader = isLeader;
    entry.joining = false;
    controllersLock.Unlock();

    JoinLeaderSession();
}

void LeaderElectionObject::OnSessionLost(SessionId sessionId)
{
    QCC_DbgTrace(("%s", __func__));
    controllersLock.Lock();
    if (leaderObj != NULL) {
        qcc::String busName = leaderObj->GetServiceName();
        delete leaderObj;
        leaderObj = NULL;
        RemoveUniqueName(busName);
    }

    ClearCurrentLeader();
    controllersLock.Unlock();

    // now find the new leader
    JoinLeaderSession();
}

// call while locked!
void LeaderElectionObject::RemoveUniqueName(const qcc::String& uniqueName)
{
    QCC_DbgTrace(("%s", __func__));
    BusNameToDeviceId::iterator bit = nameToId.find(uniqueName);
    if (bit != nameToId.end()) {
        ControllerEntryMap::iterator it = controllers.find(bit->second);
        if (it != controllers.end()) {
            QCC_DbgPrintf(("Removing mapping: %s -> %s", uniqueName.c_str(), bit->second.c_str()));
            controllers.erase(it);
        }

        nameToId.erase(bit);
    }
}

void LeaderElectionObject::OnSessionMemberRemoved(SessionId sessionId, const char* uniqueName)
{
    QCC_DbgTrace(("%s", __func__));
    controllersLock.Lock();

    // we don't care if a peer was lost; only if the leader has left
    if (currentLeader.busName == uniqueName) {
        // no need to treat this any different if the session was lost
        controllersLock.Unlock();
        // we don't want to keep this session alive anymore!
        OnSessionLost(sessionId);
    } else {
        RemoveUniqueName(uniqueName);
        controllersLock.Unlock();
    }
}

void LeaderElectionObject::OnSessionJoined(QStatus status, SessionId sessionId, ControllerEntry* joined)
{
    QCC_DbgPrintf(("ControllerService::OnJoined(%s, %u, %s)", QCC_StatusText(status), sessionId, joined->busName.c_str()));
    joined->joining = false;

    if (status == ER_OK) {
        controllersLock.Lock();
        SessionId oldSession = leaderObj ? leaderObj->GetSessionId() : 0;
        QCC_DbgPrintf(("%s: leader rank: %lu; joined rank: %lu\n", __func__, currentLeader.rank, joined->rank));

        if ((currentLeader.rank <= joined->rank) && (currentLeader.isLeader && joined->isLeader)) {
            currentLeader = *joined;

            // we are connected to the new leader!
            delete leaderObj;
            leaderObj = new ProxyBusObject(bus, currentLeader.busName.c_str(), LeaderElectionAndStateSyncObjectPath, sessionId);

            // add the synchronization interface
            const InterfaceDescription* stateSyncInterface = bus.GetInterface(LeaderElectionAndStateSyncInterfaceName);
            leaderObj->AddInterface(*stateSyncInterface);
        } else {
            // we don't care about this session anymore!
            // leave it!
            oldSession = sessionId;
        }

        controllersLock.Unlock();

        if (oldSession) {
            bus.LeaveSession(oldSession);
        }

        // we don't need to walk the list again because Join will be called by the Announce/Lost handlers
        // Remember, we could have multiple Joins in flight at the same time.  Only the one for the real leader will be kept
    } else {
        controllersLock.Lock();
        // something has gone wrong; remove the controller from the list
        RemoveUniqueName(joined->busName);
        controllersLock.Unlock();

        // do nothing?  assume another leader is coming up?
        // if we are connecting to a CS who decides they are no longer the leader, this can be rejected
        JoinLeaderSession();
    }
}


// do not call this method if controllersLock isn't locked!
LeaderElectionObject::ControllerEntry* LeaderElectionObject::GetMaxRankedEntry()
{
    QCC_DbgTrace(("%s", __func__));
    ControllerEntry* max_rank = NULL;

    // now decide what to do!
    ControllerEntryMap::iterator it = controllers.begin();
    for (; it != controllers.end(); ++it) {
        ControllerEntry& entry = it->second;
        if (max_rank) {
            if ((max_rank->isLeader && entry.isLeader) && (max_rank->rank < entry.rank)) {
                max_rank = &entry;
            }
        } else {
            max_rank = &entry;
        }
    }

    return max_rank;
}

// do not call this method if controllersLock isn't locked!
void LeaderElectionObject::ClearCurrentLeader()
{
    QCC_DbgTrace(("%s", __func__));
    currentLeader.busName = "";
    currentLeader.rank = 0;
    currentLeader.isLeader = 0;
    currentLeader.port = 0;
    currentLeader.joining = false;
}

void LeaderElectionObject::JoinLeaderSession()
{
    QCC_DbgPrintf(("%s: Ignoring announcement as we are already in a session with a Controller Service", __func__));
#if 0
    controllersLock.Lock();
    ControllerEntry* max_rank = GetMaxRankedEntry();

    QCC_DbgPrintf(("Max rank: %lu; myRank=%lu, isLeader=%u, currentleader=%lu",
                   (max_rank ? max_rank->rank : 0UL), myRank, (uint32_t) isLeader, currentLeader.rank));

    bool change_session = false;
    if (max_rank == NULL) {
        // nobody else is out there; we must be the leader
        isLeader = true;
        g_IsLeader = true;
        change_session = true;
    } else if (myRank > max_rank->rank && !isLeader) {
        // i have the highest rank; I am the leader
        isLeader = true;
        g_IsLeader = true;
        change_session = true;
    } else if (myRank < max_rank->rank && isLeader) {
        // somebody else outranks me; I am no longer the leader
        isLeader = false;
        g_IsLeader = false;
        change_session = true;
    } else if (!isLeader) {
        // the leader has changed
        if (currentLeader.rank < max_rank->rank) {
            change_session = true;
        }
    }

    SessionId serviceSession = leaderObj ? leaderObj->GetSessionId() : 0;
    // need to clean this up
    delete leaderObj;
    leaderObj = NULL;
    controllersLock.Unlock();

    // best to do this after releasing the lock
    if (change_session) {
        if (serviceSession) {
            bus.LeaveSession(serviceSession);
            ClearCurrentLeader();
        }

        // don't try to join the same session twice!
        if (!isLeader && max_rank != NULL && !max_rank->joining) {
            SessionOpts options;
            options.isMultipoint = true;
            QStatus status = bus.JoinSessionAsync(max_rank->busName.c_str(), max_rank->port, handler, options, handler, max_rank);
            if (status != ER_OK) {
                QCC_LogError(status, ("%s: JoinSessionAsync\n", __func__));
            } else {
                max_rank->joining = true;
            }
        }
        // else nothing to join
    }
#endif
}


bool LeaderElectionObject::IsLeader()
{
    controllersLock.Lock();
    bool b = isLeader;
    controllersLock.Unlock();
    return b;
}

static const char* ControllerServiceInterface[] = {
    ControllerServiceInterfaceName
};

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
    AddInterface(*stateSyncInterface);

    const MethodEntry methodEntries[] = {
        { stateSyncInterface->GetMember("GetChecksumAndModificationTimestamp"),
          static_cast<MessageReceiver::MethodHandler>(&LeaderElectionObject::GetChecksumAndModificationTimestamp) },
        { stateSyncInterface->GetMember("GetBlob"),
          static_cast<MessageReceiver::MethodHandler>(&LeaderElectionObject::GetBlob) },
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

    status = services::AnnouncementRegistrar::RegisterAnnounceHandler(bus, *handler, ControllerServiceInterface, 1);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to start the register Announce Handler", __func__));
        return status;
    }

    return status;
}

QStatus LeaderElectionObject::Stop()
{
    QCC_DbgTrace(("%s", __func__));
    QStatus status = ER_OK;

    status = bus.UnregisterSignalHandler(
        this,
        static_cast<MessageReceiver::SignalHandler>(&LeaderElectionObject::OnBlobChanged),
        blobChangedSignal,
        LeaderElectionAndStateSyncObjectPath);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to start the unregister BlobChanged Handler", __func__));
    }

    status = services::AnnouncementRegistrar::UnRegisterAnnounceHandler(bus, *handler, ControllerServiceInterface, 1);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to start the unregister Announce Handler", __func__));
    }

    return status;
}

QStatus LeaderElectionObject::SendBlobUpdate(SessionId session, LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp)
{
    QCC_DbgTrace(("%s", __func__));
    MsgArg args[4];
    args[0].Set("u", static_cast<uint32_t>(type));
    args[1].Set("s", strdup(blob.c_str()));
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
    args[1].Set("s", strdup(blob.c_str()));
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
    outArg.SetOwnershipFlags(MsgArg::OwnsArgs | MsgArg::OwnsData);

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
    std::string blob = args[0].v_string.str;
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
