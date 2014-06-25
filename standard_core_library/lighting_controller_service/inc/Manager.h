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

namespace lsf {

class ControllerService;

class Manager : public ajn::MessageReceiver {

    static const size_t ID_STR_LEN = 8;

  public:

    Manager(ControllerService& controllerSvc, const std::string& filePath = "");

    void ScheduleFileUpdate();

    //protected:

    LSFString GenerateUniqueID(const LSFString& prefix) const;

    ControllerService& controllerService;

    bool updated;

    const std::string filePath;

    bool ValidateFileAndRead(std::istringstream& filestream);

    uint32_t GetChecksum(const std::string& str);

    virtual std::string GetString() { return ""; }

    void GetBlobInfo(uint32_t& checksum, uint64_t& time);

    void GetBlob(ajn::MsgArg& blob, ajn::MsgArg& checksum, ajn::MsgArg& time);

    void WriteFileWithChecksum(const std::string& str, uint32_t checksum);

    void MethodReplyPassthrough(ajn::Message& msg, void* context);
};

}


#endif
