#ifndef _WORKER_QUEUE_H_
#define _WORKER_QUEUE_H_
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

#include <Thread.h>
#include <Mutex.h>
#include <Condition.h>

#include <list>

namespace lsf {

/**
 * This class will offload processing of <T> itemse
 * to another thread.
 */
template <typename T>
class WorkerQueue : public Thread {

  public:

    class Handler {
      public:
        virtual ~Handler() { }
        virtual int HandleItem(T* item) = 0;
    };

    WorkerQueue(Handler& handler);

    /**
     * Add an item to the queue
     *
     * @param item  The item to add
     * @return      ER_OK if the item was successfully added
     */
    QStatus AddItem(T* item);

    // from lsf::Thread
    virtual void Run();
    virtual void Stop();

  private:
    Handler& handler;
    Mutex mutex;
    Condition cond;
    bool isRunning;

    typedef std::list<T*> ItemList;
    ItemList queue;
};

template <typename T>
WorkerQueue<T>::WorkerQueue(Handler& handler) : handler(handler), isRunning(false)
{
}

template <typename T>
QStatus WorkerQueue<T>::AddItem(T* item)
{
    QStatus status = mutex.Lock();

    if (status == ER_OK) {
        queue.push_back(item);
        cond.Signal();
        mutex.Unlock();
    }

    return status;
}

template <typename T>
void WorkerQueue<T>::Run()
{
    isRunning = true;

    while (isRunning) {
        mutex.Lock();

        while (isRunning && queue.empty()) {
            cond.Wait(mutex, 100);
        }

        if (!isRunning) {
            break;
        }

        T* item = queue.front();
        queue.pop_front();
        mutex.Unlock();

        // call the handler with the item
        handler.HandleItem(item);
    }

    queue.clear();
}

template <typename T>
void WorkerQueue<T>::Stop()
{
    isRunning = false;
    cond.Signal();
}

}

#endif
