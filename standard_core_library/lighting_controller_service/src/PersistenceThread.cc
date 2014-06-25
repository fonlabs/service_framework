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

#include <PersistenceThread.h>
#include <ControllerService.h>

using namespace lsf;

PersistenceThread::PersistenceThread(ControllerService& service)
    : service(service),
    running(true)
{
}

PersistenceThread::~PersistenceThread()
{
    Join();
}


void PersistenceThread::Run()
{
    while (running) {
        // wait!
        semaphore.Wait();

        if (running) {
            service.GetLampGroupManager().WriteFile();
            service.GetMasterSceneManager().WriteFile();
            service.GetPresetManager().WriteFile();
            service.GetSceneManager().WriteFile();
        }
    }
}

void PersistenceThread::SignalWrite()
{
    // signal
    semaphore.Post();
}

void PersistenceThread::Stop()
{
    running = false;
    SignalWrite();
}

