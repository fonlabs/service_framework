#ifndef PERSISTENCE_THREAD_H
#define PERSISTENCE_THREAD_H
/**
 * \ingroup ControllerService
 */
/**
 * @file
 * This file provides definitions for persistent thread
 */
/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
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

#include <alljoyn/lighting/Thread.h>
#include <alljoyn/lighting/LSFSemaphore.h>

namespace lsf {

class ControllerService;
/**
 * Thread dedicated for persisted data
 */
class PersistenceThread : public Thread {
  public:
    /**
     * class constructor
     */
    PersistenceThread(ControllerService& service);
    /**
     * class destructor
     */
    virtual ~PersistenceThread();
    /**
     * Signal to the thread to be active
     */
    void SignalReadWrite();
    /**
     * Thread run method
     */
    virtual void Run();
    /**
     * Stop thread
     */
    virtual void Stop();
    /**
     * Join thread after stopped
     */
    virtual void Join();

  private:
    ControllerService& service;
    bool running;
    LSFSemaphore semaphore;
};

}


#endif
