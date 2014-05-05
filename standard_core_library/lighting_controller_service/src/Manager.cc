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

#include <Manager.h>

#include <qcc/StringUtil.h>
#include <qcc/Debug.h>

#include <ControllerService.h>

using namespace lsf;

#define QCC_MODULE "MANAGER"

Manager::Manager(ControllerService& controllerSvc) : controllerService(controllerSvc)
{

}

void Manager::MethodReplyPassthrough(ajn::Message& msg, void* context)
{
    controllerService.GetBusAttachment().EnableConcurrentCallbacks();
    size_t numArgs;
    const ajn::MsgArg* args;
    msg->GetArgs(numArgs, args);

    ajn::Message* origMessage = static_cast<ajn::Message*>(context);
    controllerService.SendMethodReply(*origMessage, args, numArgs);
    delete origMessage;
}

LSF_ID Manager::GenerateUniqueID(const LSF_Name& prefix) const
{
    // generate a GUID string with a given prefix
    qcc::String str = qcc::RandHexString(ID_STR_LEN);
    return prefix + str.c_str();
}
