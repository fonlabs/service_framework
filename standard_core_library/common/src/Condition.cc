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

#include <Condition.h>

#include <time.h>

using namespace lsf;

#define QCC_MODULE "CONDITION"

Condition::Condition()
{
    pthread_cond_init(&condition, NULL);
}


Condition::~Condition()
{
    pthread_cond_destroy(&condition);
}

int Condition::Wait(Mutex& mutex, uint32_t timeout)
{
    pthread_mutex_t* thelock = mutex.GetMutex();

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    // now add the offset
    ts.tv_sec += timeout / 1000;
    ts.tv_nsec += (timeout % 1000) * 1000;

    // WHY is ts an ABSTIME?
    return pthread_cond_timedwait(&condition, thelock, &ts);
}

int Condition::Signal()
{
    return pthread_cond_signal(&condition);
}

int Condition::Broadcast()
{
    return pthread_cond_broadcast(&condition);
}
