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

#include <LSFSemaphore.h>
#include <qcc/Debug.h>

#include <time.h>

using namespace lsf;

#define QCC_MODULE "LSF_SEMAPHORE"

LSFSemaphore::LSFSemaphore()
{
    QCC_DbgPrintf(("%s", __func__));
    sem_init(&mutex, 1, 1);
}

LSFSemaphore::~LSFSemaphore()
{
    QCC_DbgPrintf(("%s", __func__));
    sem_destroy(&mutex);
}

void LSFSemaphore::Wait(void)
{
    QCC_DbgPrintf(("%s", __func__));
    sem_wait(&mutex);
}

void LSFSemaphore::Post(void)
{
    QCC_DbgPrintf(("%s", __func__));
    sem_post(&mutex);
}
