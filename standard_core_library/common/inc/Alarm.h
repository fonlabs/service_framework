#ifndef _ALARM_H_
#define _ALARM_H_
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
#include <LSFSemaphore.h>
#include <signal.h>

#include <alljoyn/Status.h>

namespace lsf {

/**
 * An alarm listener is capable of receiving alarm callbacks
 */
class AlarmListener {

  public:
    /**
     * Virtual destructor for derivable class.
     */
    virtual ~AlarmListener() { }

    virtual void AlarmTriggered(void) = 0;
};

class Alarm : public Thread {
  public:

    Alarm(AlarmListener* alarmListener);

    ~Alarm();

    void SetAlarm(uint8_t timeInSecs);

    void Run(void);

    void Stop(void);

    void Join(void);

  private:

    volatile sig_atomic_t isRunning;
    AlarmListener* alarmListener;
    volatile sig_atomic_t progressInTime;
    volatile sig_atomic_t timeToTrack;
    volatile sig_atomic_t alarmReset;
};

}


#endif
