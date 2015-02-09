#ifndef LSF_CONDITION_H
#define LSF_CONDITION_H
/**
 * \defgroup ControllerService
 * ControllerService code
 */
/**
 * \ingroup ControllerService
 */
/**
 * @file
 * This file provides definitions for the Condition class
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

#include <pthread.h>

namespace lsf {

class Mutex;
/**
 * abstraction of posix pthread_cond_t
 */
class Condition {
  public:
    /**
     * class constructor
     */
    Condition();
    /**
     * class destructor
     */
    ~Condition();
    /**
     * unblock at least one of the threads that are blocked on the specified condition variable cond (if any threads are blocked on cond).
     */
    void Signal();
    /**
     * unblock all threads currently blocked on the specified condition variable condition
     */
    void Broadcast();
    /**
     * wait on condition
     */
    void Wait(Mutex& mutex);

  private:

    pthread_cond_t condition;
};

}

#endif
