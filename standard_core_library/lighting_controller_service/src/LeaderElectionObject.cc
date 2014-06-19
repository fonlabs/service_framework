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

using namespace lsf;
using namespace ajn;

LeaderElectionObject::LeaderElectionObject(ControllerService& controller)
    : BusObject(LeaderElectionAndStateSyncObjectPath),
    controller(controller)
{

}

QStatus LeaderElectionObject::Start()
{
    BusAttachment& bus = controller.GetBusAttachment();
    QStatus status;
    status = bus.CreateInterfacesFromXml(LeaderElectionAndStateSyncDescription.c_str());
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to CreateInterfacesFromXml", __FUNCTION__));
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
        QCC_LogError(status, ("%s: Failed to AddMethodHandlers", __FUNCTION__));
        return status;
    }

    status = bus.RegisterBusObject(*this);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to register BusObject for the Leader Object", __FUNCTION__));
        return status;
    }

    return status;
}


QStatus LeaderElectionObject::SendBlobUpdate(SessionId session, LSFBlobType type, uint32_t checksum, uint64_t timestamp)
{
    const InterfaceDescription* iface = controller.GetBusAttachment().GetInterface(LeaderElectionAndStateSyncInterfaceName);
    const InterfaceDescription::Member* signal = iface->GetMember("BlobChanged");

    MsgArg args[3];
    args[0].Set("u", static_cast<uint32_t>(type));
    args[1].Set("u", checksum);
    args[2].Set("t", timestamp);

    return Signal(NULL, session, *signal, args, 3);
}

void LeaderElectionObject::GetChecksumAndModificationTimestamp(const ajn::InterfaceDescription::Member* member, ajn::Message& msg)
{
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    MsgArg outArg;
    MsgArg* out = new MsgArg[4];

    uint32_t checksum;
    uint64_t timestamp;

    controller.GetPresetManager().GetBlobInfo(checksum, timestamp);
    out[0].Set("(uut)", static_cast<uint32_t>(LSF_PRESET), checksum, timestamp);

    controller.GetLampGroupManager().GetBlobInfo(checksum, timestamp);
    out[1].Set("(uut)", static_cast<uint32_t>(LSF_LAMP_GROUP), checksum, timestamp);

    controller.GetSceneManager().GetBlobInfo(checksum, timestamp);
    out[2].Set("(uut)", static_cast<uint32_t>(LSF_SCENE), checksum, timestamp);

    controller.GetMasterSceneManager().GetBlobInfo(checksum, timestamp);
    out[3].Set("(uut)", static_cast<uint32_t>(LSF_MASTER_SCENE), checksum, timestamp);

    outArg.Set("a(uut)", 4, out);
    outArg.SetOwnershipFlags(MsgArg::OwnsArgs | MsgArg::OwnsData);

    MethodReply(msg, &outArg, 1);
}

void LeaderElectionObject::GetBlob(const ajn::InterfaceDescription::Member* member, ajn::Message& msg)
{
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    MsgArg outArgs[3];

    switch (static_cast<LSFBlobType>(args[0].v_uint32)) {
    case LSF_PRESET:
        controller.GetPresetManager().GetBlob(outArgs[0], outArgs[1], outArgs[2]);
        break;

    case LSF_LAMP_GROUP:
        controller.GetLampGroupManager().GetBlob(outArgs[0], outArgs[1], outArgs[2]);
        break;

    case LSF_SCENE:
        controller.GetSceneManager().GetBlob(outArgs[0], outArgs[1], outArgs[2]);
        break;

    case LSF_MASTER_SCENE:
        controller.GetMasterSceneManager().GetBlob(outArgs[0], outArgs[1], outArgs[2]);
        break;

    default:
        outArgs[0].Set("s", "");
        outArgs[1].Set("t", 0UL);
        outArgs[2].Set("t", 0UL);
        break;
    }

    MethodReply(msg, outArgs, 3);
}
