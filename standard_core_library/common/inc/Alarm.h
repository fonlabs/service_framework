#ifndef _ALARM_H_
#define _ALARM_H_
/**
 * \defgroup Common
 * Common code to client and server controller
 */
/**
 * \ingroup Common
 */
/**
 * \file  common/inc/Alarm.h
 * This file provides definitions for alarm infrastructure
 */
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
     * Virtual destructor for derivable class
     */
    virtual ~AlarmListener() { }

    /**
     * Invoked when an alarm is triggered
     */
    virtual void AlarmTriggered(void) = 0;
};

/**
 * Class used to implement an Alarm that is
 * capable of handling time in seconds.
 */
class Alarm : public Thread {
  public:

    /**
     * Contructor that accepts an alarmListener
     */
    Alarm(AlarmListener* alarmListener);

    /**
     * Destructor
     */
    ~Alarm();

    /**
     * Set an Alarm
     *
     * @param timeInSecs Alarm time in seconds
     */
    void SetAlarm(uint8_t timeInSecs);

    /**
     * Start the Alarm thread
     */
    void Run(void);

    /**
     * Stop the Alarm thread
     */
    void Stop(void);

    /**
     * Join the Alarm thread
     */
    void Join(void);

  private:

    /*
     * Indicates if the Alarm thread is running
     */
    volatile sig_atomic_t isRunning;

    /*
     * Alarm Listener
     */
    AlarmListener* alarmListener;

    /*
     * Internal variable used to track progress in time
     */
    volatile sig_atomic_t progressInTime;

    /*
     * Internal holder of the Alarm time
     */
    volatile sig_atomic_t timeToTrack;

    /*
     * Indicates if the Alarm time is reset
     */
    volatile sig_atomic_t alarmReset;
};

}


#endif
