#ifndef _SCENE_MANAGER_H_
#define _SCENE_MANAGER_H_
/**
 * \ingroup ControllerService
 */
/**
 * \file lighting_controller_service/inc/SceneManager.h
 * This file provides definitions for scene manager
 */
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

#include <alljoyn/lighting/service/Manager.h>
#include <alljoyn/lighting/service/LampGroupManager.h>

#include <Mutex.h>
#include <alljoyn/lighting/LSFTypes.h>

#include <string>
#include <map>

namespace lsf {

class MasterSceneManager;
class SceneObject;
/**
 * scene management class
 */
class SceneManager : public Manager {
    friend class MasterSceneManager;
    friend class SceneObject;
  public:
    /**
     * SceneManager CTOR
     */
    SceneManager(ControllerService& controllerSvc, LampGroupManager& lampGroupMgr, MasterSceneManager* masterSceneMgr, const std::string& sceneFile);
    /**
     * UnregsiterSceneEventActionObjects. \n
     * The method is called during controller service stop(). \n
     * Deletes all SceneObject instances. \n
     * No event and action is possible after this method.
     */
    void UnregsiterSceneEventActionObjects(void);
    /**
     * Clearing all scenes.
     * @return
     *      LSF_OK - in case operation succeeded
     */
    LSFResponseCode Reset(void);
    /**
     * Check if there a scene that depends on specific present. \n
     * @param presetID - the lamp present id. \n
     * @return LSF_OK if there is not dependency. \n
     *         LSF_ERR_DEPENDENCY if there is dependency
     */
    LSFResponseCode IsDependentOnPreset(LSFString& presetID);
    /**
     * Check if there a scene that depends on specific lamp group. \n
     * @param lampGroupID - the lamp group id
     * @return LSF_OK if there is not dependency
     *         LSF_ERR_DEPENDENCY if there is dependency
     */
    LSFResponseCode IsDependentOnLampGroup(LSFString& lampGroupID);
    /**
     * Get All Scene IDs. \n
     * Return asynchronous reply with response code: \n
     *  LSF_OK - operation succeeded
     */
    void GetAllSceneIDs(ajn::Message& message);
    /**
     * Get Scene name. \n
     * @param message type Message contains: scene unique id (type 's') and requested language (type 's') \n
     * Return asynchronous reply with response code: \n
     *  LSF_OK - operation succeeded \n
     *  LSF_ERR_NOT_FOUND  - the scene not found \n
     *  LSF_ERR_INVALID_ARGS - Language not supported
     */
    void GetSceneName(ajn::Message& message);
    /**
     * Set Scene name. \n
     * @param message with  MsgArgs -  unique id (signature 's'), name (signature 's'), language (signature 's'). \n
     * Return asynchronous reply with response code: \n
     *  LSF_OK - operation succeeded \n
     *  LSF_ERR_INVALID_ARGS - Language not supported, length exceeds LSF_MAX_NAME_LENGTH \n
     *  LSF_ERR_EMPTY_NAME - scene name is empty \n
     *  LSF_ERR_RESOURCES - blob length is longer than MAX_FILE_LEN
     */
    void SetSceneName(ajn::Message& message);
    /**
     * Delete Scene \n
     * @param message type Message. Contains one MsgArg with scene id. \n
     * Return asynchronous reply with response code: \n
     *  LSF_OK - operation succeeded \n
     *  LSF_ERR_NOT_FOUND - can't find scene id
     */
    void DeleteScene(ajn::Message& message);
    /**
     * Create Scene and sending signal 'ScenesCreated' \n
     * @param message (type Message) with 4 message arguments as parameters (type ajn::MsgArg). \n
     * The arguments should have the following types: \n
     *      TransitionLampsLampGroupsToState \n
     *      TransitionLampsLampGroupsToPreset \n
     *      PulseLampsLampGroupsWithState \n
     *      PulseLampsLampGroupsWithPreset
     *
     * Return asynchronous reply with response code: \n
     *  LSF_OK - operation succeeded \n
     *  LSF_ERR_INVALID_ARGS - Language not supported, scene name is empty, Invalid Scene components specified, ame length exceeds \n
     *  LSF_ERR_RESOURCES - Could not allocate memory \n
     *  LSF_ERR_NO_SLOT - No slot for new Scene
     */
    void CreateScene(ajn::Message& message);
    /**
     * Modify an existing scene and then sending signal 'ScenesUpdated' \n
     * @param message (type Message) with 4 message arguments as parameters (type ajn::MsgArg). \n
     * The arguments should have the following types: \n
     *      TransitionLampsLampGroupsToState \n
     *      TransitionLampsLampGroupsToPreset \n
     *      PulseLampsLampGroupsWithState \n
     *      PulseLampsLampGroupsWithPreset
     */
    void UpdateScene(ajn::Message& message);
    /**
     * Get Scene. - reply asynchronously with scene content: \n
     *  TransitionLampsLampGroupsToState \n
     *  TransitionLampsLampGroupsToPreset \n
     *  PulseLampsLampGroupsWithState \n
     *  PulseLampsLampGroupsWithPreset \n
     * @param message type Message contains MsgArg with parameter unique id (type 's') \n
     *  return LSF_OK \n
     *  return LSF_ERR_NOT_FOUND - scene not found
     */
    void GetScene(ajn::Message& message);
    /**
     * Apply Scene. \n
     * @param message type Message with MsgArg parameter - scene id (type 's') \n
     * reply asynchronously with response code: \n
     *  LSF_OK - on success \n
     *  LSF_ERR_NOT_FOUND - scene id not found in current list of scenes
     */
    void ApplyScene(ajn::Message& message);
    /**
     * Send Scene Or Master Scene Applied Signal. \n
     * Sending signals in case a scene applied: \n
     *       Sessionless signal 'SceneApplied' for event and action mechanism. \n
     *      'MasterScenesApplied' signal - in case that master scene applied. \n
     *      'ScenesApplied' signal - in case that scene applied. \n
     *
     * @param sceneorMasterSceneId - the applied scene id or master scene id
     */
    void SendSceneOrMasterSceneAppliedSignal(LSFString& sceneorMasterSceneId);
    /**
     * Get All Scenes. \n
     * Return asynchronous answer - the all scenes by its reference parameter \n
     * @param sceneMap of type SceneMap - reference of sceneMap to get all scenes \n
     * @return LSF_OK on succedd.
     */
    LSFResponseCode GetAllScenes(SceneMap& sceneMap);
    /**
     * Read Write File \n
     * Reading scenes information from the persistent data and might update other interested controller services by sending blob messages.
     */
    void ReadWriteFile();
    /**
     * Read Saved Data. \n
     * Reads saved info from persistent data
     */
    void ReadSavedData();
    /**
     * Get the version of the scene inerface. \n
     * Return asynchronously. \n
     * @return version of the scene inerface
     */
    uint32_t GetControllerServiceSceneInterfaceVersion(void);
    /**
     * Get string representation of scene objects. \n
     * @param output - string representation of scene objects
     * @param checksum - of the output
     * @param timestamp - current time
     */
    virtual bool GetString(std::string& output, uint32_t& checksum, uint64_t& timestamp);
    /**
     * Get file information. \n
     * Derived from Manager class. \n
     * Answer returns synchronously by the reference parameters.
     * @param checksum
     * @param timestamp
     */
    void GetBlobInfo(uint32_t& checksum, uint64_t& timestamp) {
        scenesLock.Lock();
        GetBlobInfoInternal(checksum, timestamp);
        scenesLock.Unlock();
    }
    /**
     * Write the blob containing scene information to persistent data. \n
     * @param blob - string containing scenes information.
     * @param checksum
     * @param timestamp
     */
    void HandleReceivedBlob(const std::string& blob, uint32_t checksum, uint64_t timestamp);

  private:

    void ReplaceMap(std::istringstream& stream);

    LSFResponseCode ApplySceneInternal(ajn::Message message, LSFStringList& sceneList, LSFString sceneOrMasterSceneId);

    typedef std::map<LSFString, SceneObject*> SceneObjectMap;

    SceneObjectMap scenes;
    Mutex scenesLock;
    LampGroupManager& lampGroupManager;
    MasterSceneManager* masterSceneManager;
    size_t blobLength;

    std::string GetString(const SceneObjectMap& items);
    std::string GetString(const std::string& name, const std::string& id, const Scene& scene);
};

/**
 * SceneObject class - Implementing event and action mechanism \n
 * Class creates unique interface name for each instance 'org.allseen.LSF.ControllerService.ApplySceneEventAction + sceneId' \n
 * Object implementation located in path '/org/allseen/LSF/ControllerService/ApplySceneEventAction/' \n
 * All included in the controller service announcement \n
 * Implements one method 'ApplyScene' as an action \n
 * and one sessionless signal 'SceneApplied' as an event.
 */
class SceneObject : public BusObject, public Translator {
  public:
    /**
     * SceneObject CTOR
     * @param sceneMgr - SceneManager object reference
     * @param sceneid - the scene id related to the class object
     * @param tempScene
     * @param name  The scene name
     */
    SceneObject(SceneManager& sceneMgr, LSFString& sceneid, Scene& tempScene, LSFString& name);
    /**
     * SceneObject DTOR
     */
    ~SceneObject();

    /**
     * apply scene implementation \n
     * triggered in case 'ApplyScene' method called
     */
    void ApplySceneHandler(const InterfaceDescription::Member* member, Message& msg);
    /**
     * Send scene applied signal \n
     * Sessionless signal 'SceneApplied' for event and action mechanism
     */
    void SendSceneAppliedSignal(void);
    /**
     * Number of target languages \n
     * @return number
     */
    size_t NumTargetLanguages() {
        return 1;
    }
    /**
     * Get target language
     * @param index
     * @param ret - language returned
     */
    void GetTargetLanguage(size_t index, qcc::String& ret) {
        ret.assign("en");
    }
    /**
     * member override from Translator class. \n
     * Describes interface elements
     */
    const char* Translate(const char* sourceLanguage, const char* targetLanguage, const char* source, qcc::String& buffer);
    /**
     * BusObject override member
     */
    void ObjectRegistered(void);

    SceneManager& sceneManager; /**< Scene manager reference count */
    LSFString sceneId;          /**< Scene id */
    Scene scene;                /**< Scene object */
    Mutex sceneNameMutex;       /**< Scene name mutex */
    LSFString sceneName;        /**< Scene name */
    const InterfaceDescription::Member* appliedSceneMember;  /**< applied scene signal */
};

}

#endif
