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

#include <LampGroupManager.h>
#include <ControllerService.h>
#include <qcc/atomic.h>
#include <qcc/Debug.h>
#include <SceneManager.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "LAMP_GROUP_MANAGER"

LampGroupManager::LampGroupManager(ControllerService& controllerSvc, LampManager& lampMgr, const char* ifaceName, SceneManager* sceneMgrPtr) :
    Manager(controllerSvc), lampManager(lampMgr), interfaceName(ifaceName), sceneManagerPtr(sceneMgrPtr)
{
}

LSFResponseCode LampGroupManager::Reset(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    LSFResponseCode responseCode = LSF_OK;
    QStatus tempStatus = lampGroupsLock.Lock();
    if (ER_OK == tempStatus) {
        /*
         * Record the IDs of all the LampGroups that are being deleted
         */
        LSFStringList lampGroupsList;
        for (LampGroupMap::iterator it = lampGroups.begin(); it != lampGroups.end(); ++it) {
            lampGroupsList.push_back(it->first);
        }

        /*
         * Clear the LampGroups
         */
        lampGroups.clear();
        tempStatus = lampGroupsLock.Unlock();
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: lampGroupsLock.Unlock() failed", __FUNCTION__));
        }

        /*
         * Send the LampGroups deleted signal
         */
        if (lampGroupsList.size()) {
            tempStatus = controllerService.SendSignal(interfaceName, "LampGroupsDeleted", lampGroupsList);
            if (ER_OK != tempStatus) {
                QCC_LogError(tempStatus, ("%s: Unable to send LampGroupsDeleted signal", __FUNCTION__));
            }
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(tempStatus, ("%s: lampGroupsLock.Lock() failed", __FUNCTION__));
    }

    return responseCode;
}

LSFResponseCode LampGroupManager::IsDependentOnLampGroup(LSFString& lampGroupID)
{
    LSFResponseCode responseCode = LSF_OK;
    responseCode = sceneManagerPtr->IsDependentOnLampGroup(lampGroupID);
    return responseCode;
}

void LampGroupManager::GetAllLampGroupIDs(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFStringList idList;
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = lampGroupsLock.Lock();
    if (ER_OK == status) {
        for (LampGroupMap::iterator it = lampGroups.begin(); it != lampGroups.end(); ++it) {
            idList.push_back(it->first.c_str());
        }
        status = lampGroupsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: lampGroupsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: lampGroupsLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(msg, responseCode, idList);
}

void LampGroupManager::GetLampGroupName(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFString name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSFString lampGroupID(id);

    LSFString language = static_cast<LSFString>(args[1].v_string.str);
    if (0 != strcmp("en", language.c_str())) {
        QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __FUNCTION__, language.c_str()));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else {
        QStatus status = lampGroupsLock.Lock();
        if (ER_OK == status) {
            LampGroupMap::iterator it = lampGroups.find(id);
            if (it != lampGroups.end()) {
                name = it->second.first;
                responseCode = LSF_OK;
            }
            status = lampGroupsLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: lampGroupsLock.Unlock() failed", __FUNCTION__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: lampGroupsLock.Lock() failed", __FUNCTION__));
        }
    }

    controllerService.SendMethodReplyWithResponseCodeIDLanguageAndName(msg, responseCode, lampGroupID, language, name);
}

void LampGroupManager::SetLampGroupName(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool nameChanged = false;
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSFString lampGroupID(id);

    const char* name;
    args[1].Get("s", &name);

    LSFString language = static_cast<LSFString>(args[2].v_string.str);
    if (0 != strcmp("en", language.c_str())) {
        QCC_LogError(ER_FAIL, ("%s: Language %s not supported", __FUNCTION__, language.c_str()));
        responseCode = LSF_ERR_INVALID_ARGS;
    } else {
        if (strlen(name) > LSF_MAX_NAME_LENGTH) {
            responseCode = LSF_ERR_INVALID_ARGS;
            QCC_LogError(ER_FAIL, ("%s: strlen(name) > LSF_MAX_NAME_LENGTH", __FUNCTION__));
        } else {
            QStatus status = lampGroupsLock.Lock();
            if (ER_OK == status) {
                LampGroupMap::iterator it = lampGroups.find(id);
                if (it != lampGroups.end()) {
                    it->second.first = LSFString(name);
                    responseCode = LSF_OK;
                    nameChanged = true;
                }
                status = lampGroupsLock.Unlock();
                if (ER_OK != status) {
                    QCC_LogError(status, ("%s: lampGroupsLock.Unlock() failed", __FUNCTION__));
                }
            } else {
                responseCode = LSF_ERR_BUSY;
                QCC_LogError(status, ("%s: lampGroupsLock.Lock() failed", __FUNCTION__));
            }
        }
    }

    controllerService.SendMethodReplyWithResponseCodeIDAndName(msg, responseCode, lampGroupID, language);

    if (nameChanged) {
        LSFStringList idList;
        idList.push_back(lampGroupID);
        controllerService.SendSignal(interfaceName, "LampGroupsNameChanged", idList);
    }
}

void LampGroupManager::CreateLampGroup(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_OK;

    bool created = false;
    LSFString lampGroupID;

    const ajn::MsgArg* inputArgs;
    size_t numInputArgs;
    msg->GetArgs(numInputArgs, inputArgs);

    QStatus status = lampGroupsLock.Lock();
    if (ER_OK == status) {
        if (lampGroups.size() < MAX_SUPPORTED_NUM_LSF_ENTITY) {
            lampGroupID = GenerateUniqueID("LAMP_GROUP");
            LampGroup lampGroup(inputArgs[0], inputArgs[1]);
            lampGroups[lampGroupID].first = lampGroupID;
            lampGroups[lampGroupID].second = lampGroup;
            created = true;
        } else {
            QCC_LogError(ER_FAIL, ("%s: No slot for new LampGroup", __FUNCTION__));
            responseCode = LSF_ERR_NO_SLOT;
        }
        status = lampGroupsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: lampGroupsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: lampGroupsLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, lampGroupID);

    if (created) {
        LSFStringList idList;
        idList.push_back(lampGroupID);
        controllerService.SendSignal(interfaceName, "LampGroupsCreated", idList);
    }
}

void LampGroupManager::UpdateLampGroup(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    bool updated = false;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSFString lampGroupID(id);
    LampGroup lampGroup(args[1], args[2]);

    QStatus status = lampGroupsLock.Lock();
    if (ER_OK == status) {
        LampGroupMap::iterator it = lampGroups.find(id);
        if (it != lampGroups.end()) {
            lampGroups[lampGroupID].second = lampGroup;
            responseCode = LSF_OK;
            updated = true;
        }
        status = lampGroupsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: lampGroupsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: lampGroupsLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, lampGroupID);

    if (updated) {
        LSFStringList idList;
        idList.push_back(lampGroupID);
        controllerService.SendSignal(interfaceName, "LampGroupsUpdated", idList);
    }
}

void LampGroupManager::DeleteLampGroup(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_OK;

    bool deleted = false;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* lampGroupId;
    args[0].Get("s", &lampGroupId);

    LSFString lampGroupID(lampGroupId);

    responseCode = IsDependentOnLampGroup(lampGroupID);

    if (LSF_OK == responseCode) {
        QStatus status = lampGroupsLock.Lock();
        if (ER_OK == status) {
            LampGroupMap::iterator it = lampGroups.find(lampGroupId);
            if (it != lampGroups.end()) {
                lampGroups.erase(it);
                deleted = true;
            } else {
                responseCode = LSF_ERR_NOT_FOUND;
            }
            status = lampGroupsLock.Unlock();
            if (ER_OK != status) {
                QCC_LogError(status, ("%s: lampGroupsLock.Unlock() failed", __FUNCTION__));
            }
        } else {
            responseCode = LSF_ERR_BUSY;
            QCC_LogError(status, ("%s: lampGroupsLock.Lock() failed", __FUNCTION__));
        }
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, lampGroupID);

    if (deleted) {
        LSFStringList idList;
        idList.push_back(lampGroupID);
        controllerService.SendSignal(interfaceName, "LampGroupsDeleted", idList);
    }
}

void LampGroupManager::GetLampGroup(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    MsgArg outArgs[4];

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    QStatus status = lampGroupsLock.Lock();
    if (ER_OK == status) {
        LampGroupMap::iterator it = lampGroups.find(id);
        if (it != lampGroups.end()) {
            it->second.second.Get(&outArgs[2], &outArgs[3]);
            responseCode = LSF_OK;
        } else {
            outArgs[2].Set("as", 0, NULL);
            outArgs[3].Set("as", 0, NULL);
        }
        status = lampGroupsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: lampGroupsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: lampGroupsLock.Lock() failed", __FUNCTION__));
    }

    outArgs[0].Set("u", responseCode);
    outArgs[1].Set("s", id);

    controllerService.SendMethodReply(msg, outArgs, 4);
}

void LampGroupManager::ResetLampGroupState(Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* lampGroupID;
    args[0].Get("s", &lampGroupID);

    LSFStringList Lamps;
    LSFResponseCode rc = LSF_OK;
    lampGroupsLock.Lock();
    LampGroupMap::iterator it = lampGroups.find(lampGroupID);
    if (it != lampGroups.end()) {
        GetAllGroupLamps(it->second.second, Lamps);
    } else {
        rc = LSF_ERR_NOT_FOUND;
    }
    lampGroupsLock.Unlock();


    if (rc != LSF_OK) {
        MsgArg replyArg("u", rc);
        controllerService.SendMethodReply(msg, &replyArg, 1);
    }
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFString id;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, id);
}

void LampGroupManager::TransitionLampGroupState(Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* lampGroupID;
    args[0].Get("s", &lampGroupID);

    LSFStringList Lamps;

    LSFResponseCode rc = LSF_OK;
    lampGroupsLock.Lock();
    LampGroupMap::iterator it = lampGroups.find(lampGroupID);
    if (it != lampGroups.end()) {
        GetAllGroupLamps(it->second.second, Lamps);
    } else {
        rc = LSF_ERR_NOT_FOUND;
    }
    lampGroupsLock.Unlock();


    if (rc != LSF_OK) {
        MsgArg replyArg("u", rc);
        controllerService.SendMethodReply(msg, &replyArg, 1);
    }
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFString id;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, id);
}

void LampGroupManager::TransitionLampGroupStateToPreset(Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* lampGroupID;
    args[0].Get("s", &lampGroupID);
    const char* presetID;
    args[1].Get("s", &presetID);

    LampState state;
    LSFResponseCode rc = LSF_OK;

    LSFStringList Lamps;
    if (rc == LSF_OK) {
        lampGroupsLock.Lock();
        LampGroupMap::iterator it = lampGroups.find(lampGroupID);
        if (it != lampGroups.end()) {
            GetAllGroupLamps(it->second.second, Lamps);
        } else {
            rc = LSF_ERR_NOT_FOUND;
        }
        lampGroupsLock.Unlock();
    }

    if (rc != LSF_OK) {
        MsgArg replyArg("u", rc);
        controllerService.SendMethodReply(msg, &replyArg, 1);
    }
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFString id;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, id);
}

void LampGroupManager::TransitionLampGroupStateField(Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* lampGroupID;
    args[0].Get("s", &lampGroupID);

    LSFStringList Lamps;

    LSFResponseCode rc = LSF_OK;
    lampGroupsLock.Lock();
    LampGroupMap::iterator it = lampGroups.find(lampGroupID);
    if (it != lampGroups.end()) {
        GetAllGroupLamps(it->second.second, Lamps);
    } else {
        rc = LSF_ERR_NOT_FOUND;
    }
    lampGroupsLock.Unlock();

    if (rc != LSF_OK) {
        MsgArg replyArg("u", rc);
        controllerService.SendMethodReply(msg, &replyArg, 1);
    }
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFString id;
    LSFString name;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeIDAndName(msg, responseCode, id, name);
}

void LampGroupManager::ResetLampGroupFieldState(Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* lampGroupID;
    args[0].Get("s", &lampGroupID);
    const char* field_name;
    args[1].Get("s", &field_name);

    LSFStringList Lamps;
    LSFResponseCode rc = LSF_OK;
    lampGroupsLock.Lock();
    LampGroupMap::iterator it = lampGroups.find(lampGroupID);
    if (it != lampGroups.end()) {
        GetAllGroupLamps(it->second.second, Lamps);
    } else {
        rc = LSF_ERR_NOT_FOUND;
    }
    lampGroupsLock.Unlock();


    // we now have a std::list of all Lamps in the group and its subgroups
    if (!Lamps.empty()) {
        MsgArg methodArgs[5];
        methodArgs[0].Set("s", field_name);
    }

    if (rc != LSF_OK) {
        MsgArg replyArg("u", rc);
        controllerService.SendMethodReply(msg, &replyArg, 1);
    }
#endif
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFString id;
    LSFString name;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeIDAndName(msg, responseCode, id, name);
}

void LampGroupManager::GetAllGroupLamps(const LampGroup& group, LSFStringList& Lamps) const
{
    QCC_DbgPrintf(("Add implementation"));
}

bool LampGroupManager::IsGroupValidHelper(const LSFString& id, VisitedMap& visited, VisitedMap& callStack) const
{
#if 0
    if (visited[id] == false) {
        // Mark the current Lamp as visited and part of recursion stack
        visited[id] = true;
        callStack[id] = true;

        // check that the subgroup exists
        LampGroupMap::const_iterator nit = lampGroups.find(id);
        if (nit == lampGroups.end()) {
            return false;
        }

        // search the adjacent vertices of the graph
        const LSFStringList& subGroups = nit->second.second.GetLampGroups();
        for (LSFStringList::const_iterator it = subGroups.begin(); it != subGroups.end(); ++it) {
            if (!visited[*it] && IsGroupValidHelper(*it, visited, callStack)) {
                return true;
            } else if (callStack[*it]) {
                return true;
            }
        }
    }

    callStack[id] = false;
#endif
    return false;
}

void LampGroupManager::AddLampGroup(const LSFString& id, const std::string& name, const LampGroup& group)
{
    std::pair<LSFString, LampGroup> thePair(name, group);
    lampGroups[id] = thePair;
}

/*
 * Check if a Group is valid.  This primarily means that we want to ensure the tree
 * is NOT a cyclic graph.
 *
 * Also check that all subgroups are valid.
 * TODO? Check if all member Lamps are valid?
 *
 * This should be called only when lampGroupsLock is locked!
 */
bool LampGroupManager::IsGroupValid(const LampGroup& group) const
{
    bool valid = true;
#if 0
    VisitedMap visited;
    VisitedMap callStack;
    const LSFStringList& subGroups = group.GetLampGroups();
    for (LSFStringList::const_iterator it = subGroups.begin(); valid && it != subGroups.end(); ++it) {
        valid = IsGroupValidHelper(*it, visited, callStack);
    }
#endif
    return valid;
}
