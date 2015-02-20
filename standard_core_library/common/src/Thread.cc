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
#include <qcc/Debug.h>

using namespace lsf;

#define QCC_MODULE "LSF_THREAD"

void* Thread::RunThread(void* data)
{
    QCC_DbgPrintf(("%s", __func__));
    Thread* thread = static_cast<Thread*>(data);
    thread->Run();
    return NULL;
}

Thread::Thread()
{
    QCC_DbgPrintf(("%s", __func__));
}

Thread::~Thread()
{
    QCC_DbgPrintf(("%s", __func__));
}

QStatus Thread::Start()
{
    QCC_DbgPrintf(("%s", __func__));
    int ret = pthread_create(&thread, NULL, &Thread::RunThread, this);
    return ret == 0 ? ER_OK : ER_FAIL;
}

void Thread::Join()
{
    QCC_DbgPrintf(("%s", __func__));
    pthread_join(thread, NULL);
}

