#ifndef _LSF_TYPES_H_
#define _LSF_TYPES_H_
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

#include <memory>
#include <string>
#include <string.h>
#include <map>
#include <list>
#include <set>

#include <alljoyn/MsgArg.h>
#include <alljoyn/Session.h>
#include <qcc/StringUtil.h>

#include <LampValues.h>
#include <LSFResponseCodes.h>

using namespace qcc;
using namespace ajn;

#define LSF_CASE(_case) case _case: return # _case

namespace lsf {
/*
 * Typedef for LSFString type
 */
typedef std::string LSFString;

/*
 * Typedef for LSFStringList type
 */
typedef std::list<LSFString> LSFStringList;

/*
 * Typedef for LampFaultCodeList type
 */
typedef std::list<LampFaultCode> LampFaultCodeList;

/*
 * Typedef for LampNameMap type
 */
typedef std::map<LSFString, LSFString> LampNameMap;

/*
 * String used as an identifier for the current state Preset
 */
extern const LSFString CurrentStateIdentifier;

/*
 * Controller Service Object Path
 */
extern const char* ControllerServiceObjectPath;

/*
 * Controller Service Interface Name
 */
extern const char* ControllerServiceInterfaceName;

/*
 * Controller Service Lamp Interface Name
 */
extern const char* ControllerServiceLampInterfaceName;

/*
 * Controller Service LampGroup Interface Name
 */
extern const char* ControllerServiceLampGroupInterfaceName;

/*
 * Controller Service Preset Interface Name
 */
extern const char* ControllerServicePresetInterfaceName;

/*
 * Controller Service Scene Interface Name
 */
extern const char* ControllerServiceSceneInterfaceName;

/*
 * Controller Service Master Scene Interface Name
 */
extern const char* ControllerServiceMasterSceneInterfaceName;

/*
 * Controller Service Session Port
 */
extern ajn::SessionPort ControllerServiceSessionPort;

/*
 * Controller Service Interface Version
 */
extern const uint32_t ControllerServiceInterfaceVersion;

/*
 * Controller Service Lamp Interface Version
 */
extern const uint32_t ControllerServiceLampInterfaceVersion;

/*
 * Controller Service Lamp Group Interface Version
 */
extern const uint32_t ControllerServiceLampGroupInterfaceVersion;

/*
 * Controller Service Preset Interface Version
 */
extern const uint32_t ControllerServicePresetInterfaceVersion;

/*
 * Controller Service Scene Interface Version
 */
extern const uint32_t ControllerServiceSceneInterfaceVersion;

/*
 * Controller Service Master Scene Interface Version
 */
extern const uint32_t ControllerServiceMasterSceneInterfaceVersion;

/*
 * Controller Service Leader Election And State Sync Interface Version
 */
extern const uint32_t ControllerServiceLeaderElectionAndStateSyncInterfaceVersion;

/*
 * Lamp Service Object Path
 */
extern const char* LampServiceObjectPath;

/*
 * Lamp Service Interface Name
 */
extern const char* LampServiceInterfaceName;

/*
 * Lamp Service State Interface Name
 */
extern const char* LampServiceStateInterfaceName;

/*
 * Lamp Service Parameters Interface Name
 */
extern const char* LampServiceParametersInterfaceName;

/*
 * Lamp Service Details Interface Name
 */
extern const char* LampServiceDetailsInterfaceName;

/*
 * Lamp Service Session Port
 */
extern ajn::SessionPort LampServiceSessionPort;

/*
 * Config Service Object Path
 */
extern const char* ConfigServiceObjectPath;

/*
 * Config Service Interface Name
 */
extern const char* ConfigServiceInterfaceName;

/*
 * Onboarding Service Object Path
 */
extern const char* OnboardingServiceObjectPath;

/*
 * Onboarding Service Interface Name
 */
extern const char* OnboardingServiceInterfaceName;

/*
 * About Object Path
 */
extern const char* AboutObjectPath;

/*
 * About Interface Name
 */
extern const char* AboutInterfaceName;

/*
 * About Icon Object Path
 */
extern const char* AboutIconObjectPath;

/*
 * About Icon Interface Name
 */
extern const char* AboutIconInterfaceName;

/*
 * Leader Election And State Sync Object Path
 */
extern const char* LeaderElectionAndStateSyncObjectPath;

/*
 * Leader Election And State Sync Interface Name
 */
extern const char* LeaderElectionAndStateSyncInterfaceName;

/*
 * Apply Scene Event Action Interface Name
 */
extern const char* ApplySceneEventActionInterfaceName;

/*
 * Apply Scene Event Action Object Path
 */
extern const char* ApplySceneEventActionObjectPath;

/*
 * Enum defining the LSF Blob Type
 */
typedef enum _LSFBlobType {
    LSF_PRESET,      /**< Preset type */
    LSF_LAMP_GROUP,  /**< Lamp Group type */
    LSF_SCENE,       /**< Scene type */
    LSF_MASTER_SCENE /**< Master Scene type */
} LSFBlobType;

/*
 * Creates a unique list from the input list by removing duplicate entries
 *
 * @param uniqueList Container to return the unique list in
 * @param fromList   Input list
 *
 * @return None
 */
void CreateUniqueList(LSFStringList& uniqueList, LSFStringList& fromList);

/*
 * Creates a unique list from the input array by removing duplicate entries
 *
 * @param uniqueList Container to return the unique list in
 * @param idsArray   Array of ids
 * @param idsSize    Size of the array
 *
 * @return None
 */
void CreateUniqueList(LSFStringList& uniqueList, ajn::MsgArg* idsArray, size_t idsSize);

/*
 * Creates new duplicate string from the input string
 *
 * @param str Input string
 *
 * @return New duplicate string
 */
char* strdupnew(const char* str);

/*
 * Class defining the Lamp State
 */
class LampState {

  public:
    /*
     * Contructor
     */
    LampState();

    /*
     * Parameterized Contructor
     *
     * @param onOff      ON/OFF field
     * @param hue        Hue
     * @param saturation Saturation
     * @param colorTemp  Color Temperature
     * @param brightness Brightness
     */
    LampState(bool onOff, uint32_t hue, uint32_t saturation, uint32_t colorTemp, uint32_t brightness);

    /*
     * Parameterized Contructor
     *
     * @param arg Msgarg holding a dictionary
     */
    LampState(const ajn::MsgArg& arg);

    /*
     * Copy Contructor
     *
     * @param other Lamp State to copy from
     */
    LampState(const LampState& other);

    /*
     * Assignment operator
     *
     * @param other Lamp State to assign from
     */
    LampState& operator=(const LampState& other);

    /*
     * Return the details of the Lamp State as a string
     *
     * @return string
     */
    const char* c_str(void) const;

    /*
     * Set the Lamp State
     *
     * @param arg Msgarg holding a dictionary
     */
    void Set(const ajn::MsgArg& arg);

    /*
     * Get the Lamp State as a MsgArg
     *
     * @param arg*       Msgarg holding a dictionary
     * @param ownership  Set ownership flag in the MsgArg
     */
    void Get(ajn::MsgArg* arg, bool ownership = false) const;

    /*
     * ON/OFF
     */
    bool onOff;

    /*
     * Hue
     */
    uint32_t hue;

    /*
     * Saturation
     */
    uint32_t saturation;

    /*
     * Color Temperature
     */
    uint32_t colorTemp;

    /*
     * Brightness
     */
    uint32_t brightness;

    /*
     * Indicates if the state is a NULL state
     */
    bool nullState;

};

/*
 * Typedef for PresetMap type
 */
typedef std::map<LSFString, std::pair<LSFString, LampState> > PresetMap;

/*
 * Class defining the Lamp Parameters
 */
class LampParameters {

  public:
    /*
     * Constructor
     */
    LampParameters();

    /*
     * Parameterized Constructor
     *
     * @param arg MsgArg with a dictionary
     */
    LampParameters(const ajn::MsgArg& arg);

    /*
     * Copy Constructor
     *
     * @param other Lamp Parameters to copy from
     */
    LampParameters(const LampParameters& other);

    /*
     * Assignment operator
     *
     * @param other Lamp Parameters to assign from
     */
    LampParameters& operator=(const LampParameters& other);

    /*
     * Return the details of the Lamp Parameters as a string
     *
     * @return string
     */
    const char* c_str(void) const;

    /*
     * Set the Lamp Parameters
     *
     * @param arg Msgarg holding a dictionary
     */
    void Set(const ajn::MsgArg& arg);

    /*
     * Get the Lamp Parameters as a MsgArg
     *
     * @param arg*       Msgarg holding a dictionary
     * @param ownership  Set ownership flag in the MsgArg
     */
    void Get(ajn::MsgArg* arg, bool ownership = false) const;

    /*
     * Energy Usage in  Milliwatts
     */
    uint32_t energyUsageMilliwatts;

    /*
     * Brightness in Lumens
     */
    uint32_t lumens;
};

/*
 * Class defining the Lamp Details
 */
class LampDetails {

  public:

    /*
     * Constructor
     */
    LampDetails();

    /*
     * Parameterized Constructor
     *
     * @param arg MsgArg with a dictionary
     */
    LampDetails(const ajn::MsgArg& arg);

    /*
     * Copy Constructor
     *
     * @param other Lamp Details to copy from
     */
    LampDetails(const LampDetails& other);

    /*
     * Assignment operator
     *
     * @param other Lamp Details to assign from
     */
    LampDetails& operator=(const LampDetails& other);

    /*
     * Return the details of the Lamp Details as a string
     *
     * @return string
     */
    const char* c_str(void) const;

    /*
     * Set the Lamp Details
     *
     * @param arg Msgarg holding a dictionary
     */
    void Set(const ajn::MsgArg& arg);

    /*
     * Get the Lamp Details as a MsgArg
     *
     * @param arg*       Msgarg holding a dictionary
     * @param ownership  Set ownership flag in the MsgArg
     */
    void Get(ajn::MsgArg* arg, bool ownership = false) const;

    /*
     * Lamp Make
     */
    LampMake make;

    /*
     * Lamp Model
     */
    LampModel model;

    /*
     * Device Type
     */
    DeviceType type;

    /*
     * Lamp Type
     */
    LampType lampType;

    /*
     * Lamp Base Type
     */
    BaseType lampBaseType;

    /*
     * Lamp Beam Angle
     */
    uint32_t lampBeamAngle;

    /*
     * Denotes if the lamp is dimmable
     */
    bool dimmable;

    /*
     * Denotes if the lamp supports color
     */
    bool color;

    /*
     * Denotes if the lamp supports variable color temperature
     */
    bool variableColorTemp;

    /*
     * Denotes if the lamp supports effects
     */
    bool hasEffects;

    /*
     * Max Voltage
     */
    uint32_t maxVoltage;

    /*
     * Min Voltage
     */
    uint32_t minVoltage;

    /*
     * Wattage
     */
    uint32_t wattage;

    /*
     * Incandescent Equivalent
     */
    uint32_t incandescentEquivalent;

    /*
     * Max Lumens
     */
    uint32_t maxLumens;

    /*
     * Min Temperature
     */
    uint32_t minTemperature;

    /*
     * Max Temperature
     */
    uint32_t maxTemperature;

    /*
     * Color Rendering Index
     */
    uint32_t colorRenderingIndex;

    /*
     * Lamp ID
     */
    LSFString lampID;
};

/*
 * Class defining a Lamp Group
 */
class LampGroup {
  public:

    /*
     * Constructor
     */
    LampGroup();

    /*
     * Parameterized Constructor
     *
     * @param lampList      MsgArg with a array of Lamp IDs
     * @param lampGroupList MsgArg with a array of Lamp Group IDs
     */
    LampGroup(const ajn::MsgArg& lampList, const ajn::MsgArg& lampGroupList);

    /*
     * Parameterized Constructor
     *
     * @param lampList      List of Lamp IDs
     * @param lampGroupList List of Lamp Group IDs
     */
    LampGroup(LSFStringList lampList, LSFStringList lampGroupList);

    /*
     * Destructor
     */
    ~LampGroup() {
        lamps.clear();
        lampGroups.clear();
    }

    /*
     * Return the details of the Lamp Group as a string
     *
     * @return string
     */
    const char* c_str(void) const;

    /*
     * Copy Constructor
     *
     * @param other Lamp Group to copy from
     */
    LampGroup(const LampGroup& other);

    /*
     * Assignment operator
     *
     * @param other Lamp Group to assign from
     */
    LampGroup& operator=(const LampGroup& other);

    /*
     * Set the Lamp Group
     *
     * @param lampList      Msgarg with an array of Lamp IDs
     * @param lampGroupList Msgarg with an array of Lamp Group IDs
     */
    void Set(const ajn::MsgArg& lampList, const ajn::MsgArg& lampGroupList);

    /*
     * Get the Lamp Group as a MsgArg
     *
     * @param lampList       Msgarg holding an array of Lamp IDs
     * @param lampGroupList  Msgarg holding an array of Lamp Group IDs
     */
    void Get(ajn::MsgArg* lampList, ajn::MsgArg* lampGroupList) const;

    /*
     * Return true if the Lamp Group contains the specified lampGroupID
     *
     * @param  lampGroupID    Lamp Group ID
     * @return LSF_ERR_DEPENDENCY if the Lamp Group contains lampGroupID, LSF_OK otherwise
     */
    LSFResponseCode IsDependentLampGroup(LSFString& lampGroupID);

    /*
     * List of Lamp IDs
     */
    LSFStringList lamps;

    /*
     * List of Lamp Group IDs
     */
    LSFStringList lampGroups;
};

/*
 * Typedef for LampGroupMap type
 */
typedef std::map<LSFString, std::pair<LSFString, LampGroup> > LampGroupMap;

/*
 * Class defining a Transition To State component of a Scene
 */
class TransitionLampsLampGroupsToState {
  public:
    /*
     * Parameterized Constructor
     *
     * @param lampList      List of Lamp IDs
     * @param lampGroupList List of Lamp Group IDs
     * @param lampState     Lamp State to transition to
     * @param transPeriod   Transition Period to transition over
     */
    TransitionLampsLampGroupsToState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& lampState, uint32_t& transPeriod);

    /*
     * Parameterized Constructor
     *
     * @param lampList      List of Lamp IDs
     * @param lampGroupList List of Lamp Group IDs
     * @param lampState     Lamp State to transition to
     */
    TransitionLampsLampGroupsToState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& lampState);

    /*
     * Parameterized Constructor
     *
     * @param component      MsgArg with an array of Lamp IDs,
     *                       array of Lamp Group IDs and a
     *                       dictionary with Lamp State
     */
    TransitionLampsLampGroupsToState(const ajn::MsgArg& component);

    /*
     * Return the details of the Transition To State component as a string
     *
     * @return string
     */
    const char* c_str(void) const;

    /*
     * Copy Constructor
     *
     * @param other Component to copy from
     */
    TransitionLampsLampGroupsToState(const TransitionLampsLampGroupsToState& other);

    /*
     * Assignment operator
     *
     * @param other Component to assign from
     */
    TransitionLampsLampGroupsToState& operator=(const TransitionLampsLampGroupsToState& other);

    /*
     * Set the Lamp Group
     *
     * @param component      MsgArg with an array of Lamp IDs,
     *                       array of Lamp Group IDs and a
     *                       dictionary with Lamp State
     */
    void Set(const ajn::MsgArg& component);

    /*
     * Get the component as a MsgArg
     *
     * @param component       Msgarg holding the component
     */
    void Get(ajn::MsgArg* component) const;

    /*
     * List of Lamps
     */
    LSFStringList lamps;

    /*
     * List of Lamp Groups
     */
    LSFStringList lampGroups;

    /*
     * Lamp State to transition to
     */
    LampState state;

    /*
     * Transition period to transition over
     */
    uint32_t transitionPeriod;

    /*
     * Indicated invalid arguments
     */
    bool invalidArgs;
};

class TransitionLampsLampGroupsToPreset {
  public:
    TransitionLampsLampGroupsToPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& presetID, uint32_t& transPeriod);

    TransitionLampsLampGroupsToPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& presetID);

    TransitionLampsLampGroupsToPreset(const ajn::MsgArg& component);

    const char* c_str(void) const;

    TransitionLampsLampGroupsToPreset(const TransitionLampsLampGroupsToPreset& other);
    TransitionLampsLampGroupsToPreset& operator=(const TransitionLampsLampGroupsToPreset& other);

    void Set(const ajn::MsgArg& component);
    void Get(ajn::MsgArg* component) const;

    LSFStringList lamps;
    LSFStringList lampGroups;
    LSFString presetID;
    uint32_t transitionPeriod;
    bool invalidArgs;
};

class PulseLampsLampGroupsWithState {
  public:
    PulseLampsLampGroupsWithState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& fromLampState, LampState& toLampState, uint32_t& period, uint32_t& duration, uint32_t& numOfPulses);

    PulseLampsLampGroupsWithState(LSFStringList& lampList, LSFStringList& lampGroupList, LampState& toLampState, uint32_t& period, uint32_t& duration, uint32_t& numOfPulses);

    PulseLampsLampGroupsWithState(const ajn::MsgArg& component);

    const char* c_str(void) const;

    PulseLampsLampGroupsWithState(const PulseLampsLampGroupsWithState& other);
    PulseLampsLampGroupsWithState& operator=(const PulseLampsLampGroupsWithState& other);

    void Set(const ajn::MsgArg& component);
    void Get(ajn::MsgArg* component) const;

    LSFStringList lamps;
    LSFStringList lampGroups;
    LampState fromState;
    LampState toState;
    uint32_t pulsePeriod;
    uint32_t pulseDuration;
    uint32_t numPulses;
    bool invalidArgs;
};

class PulseLampsLampGroupsWithPreset {
  public:
    PulseLampsLampGroupsWithPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& fromPreset, LSFString& toPreset, uint32_t& period, uint32_t& duration, uint32_t& numOfPulses);

    PulseLampsLampGroupsWithPreset(LSFStringList& lampList, LSFStringList& lampGroupList, LSFString& toPreset, uint32_t& period, uint32_t& duration, uint32_t& numOfPulses);

    PulseLampsLampGroupsWithPreset(const ajn::MsgArg& component);

    const char* c_str(void) const;

    PulseLampsLampGroupsWithPreset(const PulseLampsLampGroupsWithPreset& other);
    PulseLampsLampGroupsWithPreset& operator=(const PulseLampsLampGroupsWithPreset& other);

    void Set(const ajn::MsgArg& component);
    void Get(ajn::MsgArg* component) const;

    LSFStringList lamps;
    LSFStringList lampGroups;
    LSFString fromPreset;
    LSFString toPreset;
    uint32_t pulsePeriod;
    uint32_t pulseDuration;
    uint32_t numPulses;
    bool invalidArgs;
};

typedef std::list<TransitionLampsLampGroupsToState> TransitionLampsLampGroupsToStateList;
typedef std::list<TransitionLampsLampGroupsToPreset> TransitionLampsLampGroupsToPresetList;
typedef std::list<PulseLampsLampGroupsWithState> PulseLampsLampGroupsWithStateList;
typedef std::list<PulseLampsLampGroupsWithPreset> PulseLampsLampGroupsWithPresetList;

class Scene {
  public:
    Scene();
    Scene(const ajn::MsgArg& transitionToStateComponentList, const ajn::MsgArg& transitionToPresetComponentList,
          const ajn::MsgArg& pulseWithStateComponentList, const ajn::MsgArg& pulseWithPresetComponentList);
    Scene(TransitionLampsLampGroupsToStateList& transitionToStateComponentList, TransitionLampsLampGroupsToPresetList& transitionToPresetComponentList,
          PulseLampsLampGroupsWithStateList& pulseWithStateComponentList, PulseLampsLampGroupsWithPresetList& pulseWithPresetComponentList);

    Scene(const Scene& other);
    Scene& operator=(const Scene& other);

    const char* c_str(void) const;

    void Set(const ajn::MsgArg& transitionToStateComponentList, const ajn::MsgArg& transitionToPresetComponentList,
             const ajn::MsgArg& pulseWithStateComponentList, const ajn::MsgArg& pulseWithPresetComponentList);
    void Get(ajn::MsgArg* transitionToStateComponentList, ajn::MsgArg* transitionToPresetComponentList,
             ajn::MsgArg* pulseWithStateComponentList, ajn::MsgArg* pulseWithPresetComponentList) const;

    LSFResponseCode IsDependentOnPreset(LSFString& presetID);
    LSFResponseCode IsDependentOnLampGroup(LSFString& lampGroupID);

    TransitionLampsLampGroupsToStateList transitionToStateComponent;
    TransitionLampsLampGroupsToPresetList transitionToPresetComponent;
    PulseLampsLampGroupsWithStateList pulseWithStateComponent;
    PulseLampsLampGroupsWithPresetList pulseWithPresetComponent;
    bool invalidArgs;
};

/*
 * std::map<sceneID, std::pair<sceneName, Scene>>
 */
typedef std::map<LSFString, std::pair<LSFString, Scene> > SceneMap;

class MasterScene {
  public:
    MasterScene();
    MasterScene(const ajn::MsgArg& sceneList);
    MasterScene(LSFStringList sceneList);

    ~MasterScene() { scenes.clear(); }

    const char* c_str(void) const;

    MasterScene(const MasterScene& other);
    MasterScene& operator=(const MasterScene& other);

    void Set(const ajn::MsgArg& sceneList);
    void Get(ajn::MsgArg* sceneList) const;

    LSFResponseCode IsDependentOnScene(LSFString& sceneID);

    LSFStringList scenes;
};

/*
 * std::map<masterSceneID, std::pair<masterSceneName, masterScene>>
 */
typedef std::map<LSFString, std::pair<LSFString, MasterScene> > MasterSceneMap;

}

#endif
