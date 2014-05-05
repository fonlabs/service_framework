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

#include <Mutex.h>

using namespace lsf;

#define QCC_MODULE "MUTEX"

Mutex::Mutex()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutexattr_destroy(&attr);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&mutex);
}

QStatus Mutex::Lock()
{
    int ret = pthread_mutex_trylock(&mutex);

    if (ret == 0) {
        return ER_OK;
    } else if ((ret == EBUSY) || (ret == EAGAIN)) {
        return ER_WOULDBLOCK;
    } else {
        return ER_FAIL;
    }
}

QStatus Mutex::Unlock()
{
    int ret = pthread_mutex_unlock(&mutex);

    if (ret == 0) {
        return ER_OK;
    } else {
        return ER_FAIL;
    }
}
