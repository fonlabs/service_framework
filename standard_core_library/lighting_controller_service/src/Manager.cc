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

#include <ControllerService.h>

using namespace lsf;

#define QCC_MODULE "MANAGER"

Manager::Manager(ControllerService& controllerSvc, const std::string& filePath)
    : controllerService(controllerSvc),
    updated(false),
    read(false),
    filePath(filePath),
    checkSum(0),
    timeStamp(0),
    blobUpdateCycle(false)
{
    QCC_DbgTrace(("%s", __func__));
    readBlobMessages.clear();
}

static uint32_t GetAdler32Checksum(const uint8_t* data, size_t len) {
    QCC_DbgTrace(("%s", __func__));
    uint32_t adler = 1;
    uint32_t adlerPrime = 65521;
    while (data && len) {
        size_t l = len % 3800; // Max number of iterations before modulus must be computed
        uint32_t a = adler & 0xFFFF;
        uint32_t b = adler >> 16;
        len -= l;
        while (l--) {
            a += *data++;
            b += a;
        }
        adler = ((b % adlerPrime) << 16) | (a % adlerPrime);
    }
    QCC_DbgTrace(("%s: adler=0x%x", __func__, adler));
    return adler;
}

uint32_t Manager::GetChecksum(const std::string& str)
{
    QCC_DbgTrace(("%s", __func__));
    return GetAdler32Checksum((uint8_t*) str.c_str(), str.length());
}

void Manager::WriteFileWithChecksumAndTimestamp(const std::string& str, uint32_t checksum, uint64_t timestamp)
{
    QCC_DbgTrace(("%s", __func__));
    std::ofstream fstream(filePath.c_str(), std::ios_base::out);
    if (!fstream.is_open()) {
        QCC_LogError(ER_FAIL, ("File not found: %s\n", filePath.c_str()));
        return;
    }

    fstream << timestamp << std::endl;
    fstream << checksum << std::endl;
    fstream << str;
    fstream.close();
}

bool Manager::ValidateFileAndRead(std::istringstream& filestream)
{
    QCC_DbgTrace(("%s", __func__));
    uint32_t checksum;
    uint64_t timestamp;

    return ValidateFileAndReadInternal(checksum, timestamp, filestream);
}

bool Manager::ValidateFileAndReadInternal(uint32_t& checksum, uint64_t& timestamp, std::istringstream& filestream)
{
    QCC_DbgPrintf(("%s: filePath=%s", __func__, filePath.c_str()));

    if (filePath.empty()) {
        return false;
    }

    std::ifstream stream(filePath.c_str());

    if (!stream.is_open()) {
        QCC_LogError(ER_FAIL, ("File not found: %s\n", filePath.c_str()));
        return false;
    }

    stream >> timestamp;

    uint64_t currenttime = GetTimestamp64();
    QCC_DbgPrintf(("%s: timestamp=%lld", __func__, timestamp));
    QCC_DbgPrintf(("%s: Updated %lld ticks ago", __func__, (currenttime - timestamp)));

    stream >> checksum;

    QCC_DbgPrintf(("%s: checksum=%d", __func__, checksum));

    // put the rest of the file into the filestream
    std::stringbuf rest;
    stream >> &rest;
    std::string data = rest.str();
    filestream.str(data);

    // check the adler checksum
    uint32_t adler = GetChecksum(data);

    stream.close();
    return adler == checksum;
}

void Manager::MethodReplyPassthrough(ajn::Message& msg, void* context)
{
    QCC_DbgTrace(("%s", __func__));
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
    QCC_DbgTrace(("%s", __func__));
    // generate a GUID string with a given prefix
    qcc::String str = qcc::RandHexString(ID_STR_LEN);
    return prefix + str.c_str();
}

void Manager::ScheduleFileWrite(bool blobUpdate)
{
    QCC_DbgTrace(("%s", __func__));
    updated = true;
    blobUpdateCycle = blobUpdate;
    controllerService.ScheduleFileReadWrite(this);
}

void Manager::ScheduleFileRead(Message& message)
{
    QCC_DbgTrace(("%s", __func__));
    readMutex.Lock();
    readBlobMessages.push_back(message);
    read = true;
    controllerService.ScheduleFileReadWrite(this);
    readMutex.Unlock();
}

void Manager::GetBlobInfoInternal(uint32_t& checksum, uint64_t& timestamp)
{
    checksum = checkSum;
    timestamp = timeStamp;
}
