#ifndef _MANAGER_H_
#define _MANAGER_H_
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

#include <alljoyn/Message.h>
#include <alljoyn/InterfaceDescription.h>
#include <alljoyn/MessageReceiver.h>

#include <LSFResponseCodes.h>
#include <LSFTypes.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <Mutex.h>

namespace lsf {

class ControllerService;

class Manager : public ajn::MessageReceiver {

    static const size_t ID_STR_LEN = 8;

  public:

    Manager(ControllerService& controllerSvc, const std::string& filePath = "");

    void ScheduleFileRead(ajn::Message& message);

    void ScheduleFileWrite(bool blobUpdate = false);

    //protected:

    LSFString GenerateUniqueID(const LSFString& prefix) const;

    ControllerService& controllerService;

    bool updated;

    Mutex readMutex;
    bool read;

    const std::string filePath;

    bool ValidateFileAndRead(std::istringstream& filestream);

    bool ValidateFileAndReadInternal(uint32_t& checksum, uint64_t& timestamp, std::istringstream& filestream);

    uint32_t GetChecksum(const std::string& str);

    virtual bool GetString(std::string& output, uint32_t& checksum, uint64_t& timestamp) { return false; };

    void GetBlobInfoInternal(uint32_t& checksum, uint64_t& time);

    void WriteFileWithChecksumAndTimestamp(const std::string& str, uint32_t checksum, uint64_t timestamp);

    void MethodReplyPassthrough(ajn::Message& msg, void* context);

    uint32_t checkSum;
    uint64_t timeStamp;
    bool blobUpdateCycle;

    std::list<ajn::Message> readBlobMessages;
};

}


#endif
