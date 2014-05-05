#ifndef _SAVED_STATE_MANAGER_H_
#define _SAVED_STATE_MANAGER_H_
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

#include <Manager.h>

#include <Mutex.h>
#include <LSFTypes.h>

#include <string>
#include <map>

namespace lsf {

class SavedStateManager : public Manager {
    friend class LampManager;
  public:

    SavedStateManager(ControllerService& controllerSvc, const char* ifaceName);

    QStatus Reset(void);

    void GetAllSavedStateIDs(ajn::Message& msg);
    void GetSavedStateName(ajn::Message& msg);
    void SetSavedStateName(ajn::Message& msg);
    void CreateSavedState(ajn::Message& msg);
    void UpdateSavedState(ajn::Message& msg);
    void DeleteSavedState(ajn::Message& msg);
    void GetSavedState(ajn::Message& msg);

    LSFResponseCode GetSavedStateInternal(const LSF_ID& stateid, LampState& state);

    void GetDefaultLampState(ajn::Message& msg);
    void SetDefaultLampState(ajn::Message& msg);

  private:

    LSFResponseCode GetDefaultLampStateInternal(LampState& state);
    QStatus SetDefaultLampStateInternal(LampState& state);

    typedef std::map<LSF_ID, std::pair<LSF_Name, LampState> > SavedStateMap;

    SavedStateMap savedStates;
    Mutex savedStatesLock;
    Mutex defaultLampStateLock;
    LampState defaultLampState;
    const char* interfaceName;
};

}


#endif
