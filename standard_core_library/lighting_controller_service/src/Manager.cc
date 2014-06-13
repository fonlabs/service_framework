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

#include <string>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <zlib.h>

#include <ControllerService.h>

using namespace lsf;

#define QCC_MODULE "MANAGER"

Manager::Manager(ControllerService& controllerSvc, const std::string& filePath)
    : controllerService(controllerSvc),
    updated(false),
    filePath(filePath)
{

}

void Manager::WriteFileWithChecksum(const std::string& str)
{
    std::ofstream fstream(filePath.c_str(), std::ios_base::out);
    if (!fstream.is_open()) {
        printf("File not found: %s\n", filePath.c_str());
        QCC_DbgPrintf(("File not found: %s\n", filePath.c_str()));
        return;
    }

    // calculate the adler checksum and write it before the file contents
    uint64_t adler = adler32(1, (uint8_t*) str.c_str(), str.length());
    fstream << adler << std::endl;
    fstream << str;

    fstream.close();
}

bool Manager::ValidateFileAndRead(std::istringstream& filestream)
{
    if (filePath.empty()) {
        return false;
    }

    std::ifstream stream(filePath.c_str());

    if (!stream.is_open()) {
        QCC_DbgPrintf(("File not found: %s\n", filePath.c_str()));
        return false;
    }


    uint64_t checksum;
    stream >> checksum;

    // put the rest of the file into the filestream
    std::stringbuf rest;
    stream >> &rest;
    std::string data = rest.str();
    filestream.str(data);

    // check the adler checksum
    uint64_t adler = adler32(1, (uint8_t*) data.c_str(), data.length());

    stream.close();
    return adler == checksum;
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

LSFString Manager::GenerateUniqueID(const LSFString& prefix) const
{
    // generate a GUID string with a given prefix
    qcc::String str = qcc::RandHexString(ID_STR_LEN);
    return prefix + str.c_str();
}

void Manager::ScheduleFileUpdate()
{
    updated = true;
    controllerService.ScheduleFileWrite(this);
}
