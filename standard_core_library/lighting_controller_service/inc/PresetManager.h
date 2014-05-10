#ifndef _PRESET_MANAGER_H_
#define _PRESET_MANAGER_H_
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

class SceneManager;

class PresetManager : public Manager {
    friend class LampManager;
  public:

    PresetManager(ControllerService& controllerSvc, const char* ifaceName, SceneManager* sceneMgrPtr);

    LSFResponseCode Reset(void);

    void GetAllPresetIDs(ajn::Message& msg);
    void GetPresetName(ajn::Message& msg);
    void SetPresetName(ajn::Message& msg);
    void CreatePreset(ajn::Message& msg);
    void UpdatePreset(ajn::Message& msg);
    void DeletePreset(ajn::Message& msg);
    void GetPreset(ajn::Message& msg);

    LSFResponseCode GetPresetInternal(const LSFString& presetID, LampState& state);

    void GetDefaultLampState(ajn::Message& msg);
    void SetDefaultLampState(ajn::Message& msg);

    void AddPreset(const LSFString& presetId, const std::string& presetName, const LampState& state);

  private:

    LSFResponseCode GetDefaultLampStateInternal(LampState& state);
    LSFResponseCode SetDefaultLampStateInternal(LampState& state);

    typedef std::map<LSFString, std::pair<LSFString, LampState> > PresetMap;

    PresetMap presets;
    Mutex presetsLock;
    Mutex defaultLampStateLock;
    LampState defaultLampState;
    const char* interfaceName;
    SceneManager* sceneManagerPtr;
};

}


#endif
