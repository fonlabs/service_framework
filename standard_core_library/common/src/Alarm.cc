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

#include <Alarm.h>
#include <qcc/Debug.h>

using namespace lsf;

#define QCC_MODULE "LSF_ALARM"

Alarm::Alarm(AlarmListener* alarmListener) :
    isRunning(true),
    alarmListener(alarmListener),
    progressInTime(0),
    timeToTrack(0),
    alarmReset(false)
{
    QCC_DbgPrintf(("%s", __func__));
    Thread::Start();
}

Alarm::~Alarm()
{
    QCC_DbgPrintf(("%s", __func__));
}

void Alarm::Run()
{
    QCC_DbgPrintf(("%s", __func__));

    while (isRunning) {
        QCC_DbgPrintf(("%s", __func__));
        usleep(1000000);
        if (alarmReset) {
            QCC_DbgPrintf(("%s: Alarm Reloaded", __func__));
            progressInTime = 0;
            alarmReset = false;
        } else {
            if (timeToTrack) {
                progressInTime++;
                if (progressInTime == timeToTrack) {
                    QCC_DbgPrintf(("%s: Calling AlarmTriggered", __func__));
                    progressInTime = 0;
                    timeToTrack = 0;
                    alarmListener->AlarmTriggered();
                }
            }
        }
    }
}

void Alarm::Join()
{
    QCC_DbgPrintf(("%s", __func__));
    Thread::Join();
}

void Alarm::Stop()
{
    isRunning = false;
    progressInTime = 0;
    timeToTrack = 0;
    alarmReset = false;
}

void Alarm::SetAlarm(uint8_t timeInSecs)
{
    timeToTrack = timeInSecs;
    alarmReset = true;
}
