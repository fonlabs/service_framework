#ifndef _MASTER_SCENE_MANAGER_H_
#define _MASTER_SCENE_MANAGER_H_
/**
 * \ingroup ControllerService
 */
/**
 * @file
 * This file provides definitions for master scene manager
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
#include <alljoyn/lighting/service/SceneManager.h>

#include <Mutex.h>
#include <alljoyn/lighting/LSFTypes.h>

#include <string>
#include <map>

namespace lsf {
/**
 * Master Scene Manager
 */
class MasterSceneManager : public Manager {

  public:
    /**
     * MasterSceneManager constructor
     */
    MasterSceneManager(ControllerService& controllerSvc, SceneManager& sceneMgr, const std::string& masterSceneFile);
    /**
     * Clearing all master scenes.
     * @return
     *      LSF_OK - in case operation succeeded
     */
    LSFResponseCode Reset(void);
    /**
     * Check if the master scene is depend on scene. \n
     * @param sceneID - the scene id to check dependency on \n
     * @return LSF_OK if there is not dependency. \n
     */
    LSFResponseCode IsDependentOnScene(LSFString& sceneID);
    /**
     * Get All Master scene IDs. \n
     * Return asynchronous reply with response code: \n
     *  LSF_OK - operation succeeded
     */
    void GetAllMasterSceneIDs(ajn::Message& message);
    /**
     * Get master scene name. \n
     * @param message type Message contains: scene unique id (type 's') and requested language (type 's') \n
     * Return asynchronous reply with response code: \n
     *  LSF_OK - operation succeeded \n
     */
    void GetMasterSceneName(ajn::Message& message);
    /**
     * Set Scene name. \n
     * @param message with  MsgArgs -  unique id (signature 's'), name (signature 's'), language (signature 's') . \n
     * Return asynchronous reply with response code: \n
     *  LSF_OK - operation succeeded \n
     */
    void SetMasterSceneName(ajn::Message& message);
    /**
     * Delete master scene \n
     * @param message type Message. Contains one MsgArg with scene id. \n
     * Return asynchronous reply with response code: \n
     *  LSF_OK - operation succeeded \n
     */
    void DeleteMasterScene(ajn::Message& message);
    /**
     * Create master scene and sending signal 'MasterScenesCreated' \n
     * @param message
     *
     * Return asynchronous reply with response code: \n
     *  LSF_OK - operation succeeded \n
     */
    void CreateMasterScene(ajn::Message& message);
    /**
     * Modify an existing master scene and then sending signal 'MasterScenesUpdated'. \n
     */
    void UpdateMasterScene(ajn::Message& message);
    /**
     * Get master scene. - reply asynchronously \n
     * @param message type Message contains MsgArg with parameter unique id (type 's') \n
     *  return LSF_OK \n
     */
    void GetMasterScene(ajn::Message& message);
    /**
     * Apply master scene. \n
     * @param message type Message with MsgArg parameter - scene id (type 's') \n
     * reply asynchronously with response code: \n
     *  LSF_OK - on success \n
     */
    void ApplyMasterScene(ajn::Message& message);
    /**
     * Send Master Scene Applied Signal. \n
     * Sending signals in case a master scene applied: \n
     *      'MasterScenesApplied' signal - in case that master scene applied. \n
     *
     * @param sceneorMasterSceneId - the applied scene id or master scene id
     */
    void SendMasterSceneAppliedSignal(LSFString& sceneorMasterSceneId);
    /**
     * Get All master scenes. \n
     * Return asynchronous answer - the all master scenes by its reference parameter \n
     * @param masterSceneMap of type MasterSceneMap - reference of MasterSceneMap to get all master scenes \n
     * @return LSF_OK on succedd.
     */
    LSFResponseCode GetAllMasterScenes(MasterSceneMap& masterSceneMap);
    /**
     * Read Saved Data. \n
     * Reads saved info from persistent data
     */
    void ReadSavedData();
    /**
     * Read Write File \n
     * Reading master scenes information from the persistent data and might update other interested controller services by sending blob messages.
     */
    void ReadWriteFile();
    /**
     * Get the version of the master scene inerface. \n
     * Return asynchronously. \n
     * @return version of the master scene inerface
     */
    uint32_t GetControllerServiceMasterSceneInterfaceVersion(void);
    /**
     * Get string representation of master scene objects. \n
     * @param output - string representation of master scene objects
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
        masterScenesLock.Lock();
        GetBlobInfoInternal(checksum, timestamp);
        masterScenesLock.Unlock();
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

    MasterSceneMap masterScenes;
    Mutex masterScenesLock;
    SceneManager& sceneManager;
    size_t blobLength;

    std::string GetString(const MasterSceneMap& items);
    std::string GetString(const std::string& name, const std::string& id, const MasterScene& msc);
};

}

#endif
