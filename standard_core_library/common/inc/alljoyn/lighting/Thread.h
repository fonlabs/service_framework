#ifndef _THREAD_H_
#define _THREAD_H_
/**
 * \ingroup Common
 */
/**
 * \file  common/inc/Thread.h
 * This file provides definitions for thread infrastructure
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
/**
 * \ingroup Common
 */
#include <pthread.h>

#include <alljoyn/Status.h>

namespace lsf {
/**
 * abstraction class to thread
 */
class Thread {
  public:
    /**
     * class constructor
     */
    Thread();
    /**
     * class destructor
     */
    virtual ~Thread();
    /**
     * This method is been called by the created thread after start()
     */
    virtual void Run() = 0;
    /**
     * Stop running thread.  \n
     * Can be implemented by changing a variable value that is shared with the run() method.  \n
     * This will release the thread.  \n
     */
    virtual void Stop() = 0;
    /**
     * Create and run new thread
     */
    QStatus Start();
    /**
     * Join running thread until it exits
     */
    void Join();

  private:

    static void* RunThread(void* data);

    pthread_t thread;
};

}


#endif
