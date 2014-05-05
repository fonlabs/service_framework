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

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "LAMP_GROUP_MANAGER"

LampGroupManager::LampGroupManager(ControllerService& controllerSvc, LampManager& lampMgr, const char* ifaceName) :
    Manager(controllerSvc), lampManager(lampMgr), interfaceName(ifaceName)
{
}

void LampGroupManager::ResetLampGroupState(Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* lampGroupID;
    args[0].Get("s", &lampGroupID);

    LSF_ID_List Lamps;
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
    LSF_ID id;
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

    LSF_ID_List Lamps;

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
    LSF_ID id;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, id);
}

void LampGroupManager::TransitionLampGroupStateToSavedState(Message& msg)
{
#if 0
    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* lampGroupID;
    args[0].Get("s", &lampGroupID);
    const char* stateID;
    args[1].Get("s", &stateID);

    LampState state;
    LSFResponseCode rc = LSF_OK;

    LSF_ID_List Lamps;
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
    LSF_ID id;
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

    LSF_ID_List Lamps;

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
    LSF_ID id;
    LSF_Name name;
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

    LSF_ID_List Lamps;
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
    LSF_ID id;
    LSF_Name name;
    LSFResponseCode responseCode = LSF_OK;
    controllerService.SendMethodReplyWithResponseCodeIDAndName(msg, responseCode, id, name);
}

void LampGroupManager::GetAllLampGroupIDs(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSF_ID_List idList;
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
    LSF_Name name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSF_ID lampGroupID(id);

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

    controllerService.SendMethodReplyWithResponseCodeIDAndName(msg, responseCode, lampGroupID, name);
}

void LampGroupManager::SetLampGroupName(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSF_ID lampGroupID(id);

    const char* name;
    args[1].Get("s", &name);

    QStatus status = lampGroupsLock.Lock();
    if (ER_OK == status) {
        LampGroupMap::iterator it = lampGroups.find(id);
        if (it != lampGroups.end()) {
            it->second.first = LSF_Name(name);
            responseCode = LSF_OK;

            LSF_ID_List idList;
            idList.push_back(lampGroupID);
            controllerService.SendSignal(interfaceName, "LampGroupsNameChanged", idList);
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
}

void LampGroupManager::CreateLampGroup(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_OK;

    const ajn::MsgArg* inputArgs;
    size_t numInputArgs;
    msg->GetArgs(numInputArgs, inputArgs);

    LampGroup lampGroup(inputArgs[0], inputArgs[1]);
    LSF_ID lampGroupID = GenerateUniqueID("LAMP_GROUP");

    QStatus status = lampGroupsLock.Lock();
    if (ER_OK == status) {
        lampGroups[lampGroupID].first = lampGroupID;
        lampGroups[lampGroupID].second = lampGroup;

        LSF_ID_List idList;
        idList.push_back(lampGroupID);
        controllerService.SendSignal(interfaceName, "LampGroupsCreated", idList);

        status = lampGroupsLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: lampGroupsLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: lampGroupsLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, lampGroupID);
}

void LampGroupManager::UpdateLampGroup(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSF_ID lampGroupID(id);
    LampGroup lampGroup(args[1], args[2]);

    QStatus status = lampGroupsLock.Lock();
    if (ER_OK == status) {
        LampGroupMap::iterator it = lampGroups.find(id);
        if (it != lampGroups.end()) {
            lampGroups[lampGroupID].second = lampGroup;
            responseCode = LSF_OK;

            LSF_ID_List idList;
            idList.push_back(lampGroupID);
            controllerService.SendSignal(interfaceName, "LampGroupsUpdated", idList);
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
}

void LampGroupManager::DeleteLampGroup(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSF_Name name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSF_ID lampGroupID(id);

    QStatus status = lampGroupsLock.Lock();
    if (ER_OK == status) {
        LampGroupMap::iterator it = lampGroups.find(id);
        if (it != lampGroups.end()) {
            lampGroups.erase(it);
            responseCode = LSF_OK;

            LSF_ID_List idList;
            idList.push_back(lampGroupID);
            controllerService.SendSignal(interfaceName, "LampGroupsDeleted", idList);
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

void LampGroupManager::GetAllGroupLamps(const LampGroup& group, LSF_ID_List& Lamps) const
{
    QCC_DbgPrintf(("Add implementation"));
}

bool LampGroupManager::IsGroupValidHelper(const LSF_ID& id, VisitedMap& visited, VisitedMap& callStack) const
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
        const LSF_ID_List& subGroups = nit->second.second.GetLampGroups();
        for (LSF_ID_List::const_iterator it = subGroups.begin(); it != subGroups.end(); ++it) {
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
    const LSF_ID_List& subGroups = group.GetLampGroups();
    for (LSF_ID_List::const_iterator it = subGroups.begin(); valid && it != subGroups.end(); ++it) {
        valid = IsGroupValidHelper(*it, visited, callStack);
    }
#endif
    return valid;
}
