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
#include <signal.h>

namespace lsf {

class ControllerService;

uint64_t GetTimestamp64(void);
/**
 * a base class to derive by manager types of classes
 */
class Manager : public ajn::MessageReceiver {

    static const size_t ID_STR_LEN = 8;

  protected:
    static const size_t MAX_FILE_LEN = 1024 * 127; /**< Max file len */

  public:
    /**
     * Manager constructor
     */
    Manager(ControllerService& controllerSvc, const std::string& filePath = "");
    /**
     * Schedule File Read
     */
    void ScheduleFileRead(ajn::Message& message);
    /**
     * Trigger update for persistent data
     */
    void TriggerUpdate(void);
    /**
     * Schedule File Write
     */
    void ScheduleFileWrite(bool blobUpdate = false, bool initState = false);

    //protected:
    /**
     * Generate GUID Id
     */
    LSFString GenerateUniqueID(const LSFString& prefix) const;

    ControllerService& controllerService; /**< controller service reference */

    bool updated; /**< true after ScheduleFileWrite */

    Mutex readMutex; /**< read mutex */
    bool read; /**< true after ScheduleFileRead */

    const std::string filePath; /**< the file location */
    /**
     * Reading from file
     */
    bool ValidateFileAndRead(std::istringstream& filestream);
    /**
     * Reading from file
     */
    bool ValidateFileAndReadInternal(uint32_t& checksum, uint64_t& timestamp, std::istringstream& filestream);
    /**
     * Get checksum of file
     */
    uint32_t GetChecksum(const std::string& str);
    /**
     * Get string from file
     */
    virtual bool GetString(std::string& output, uint32_t& checksum, uint64_t& timestamp) { return false; };
    /**
     * Get file information
     */
    void GetBlobInfoInternal(uint32_t& checksum, uint64_t& time);
    /**
     * Write File With Checksum And Timestamp
     */
    void WriteFileWithChecksumAndTimestamp(const std::string& str, uint32_t checksum, uint64_t timestamp);

    uint32_t checkSum; /**< checkSum of the file */
    uint64_t timeStamp; /**< timestamp of the file */
    bool blobUpdateCycle; /**< blob Update Cycle */
    bool initialState; /**< initial state */

    std::list<ajn::Message> readBlobMessages; /**< Read blob messages */

    volatile sig_atomic_t sendUpdate; /**< send update */
};

}


#endif
