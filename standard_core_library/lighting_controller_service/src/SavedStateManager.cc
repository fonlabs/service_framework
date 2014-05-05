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

#include <SavedStateManager.h>
#include <ControllerService.h>
#include <qcc/Debug.h>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "SAVED_STATE_MANAGER"

SavedStateManager::SavedStateManager(ControllerService& controllerSvc, const char* ifaceName) : Manager(controllerSvc), interfaceName(ifaceName)
{
    defaultLampState = { false, 0x100, 0x100, 0x100, 0x100 };
}

QStatus SavedStateManager::Reset(void)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    QStatus status = ER_OK;
    QStatus tempStatus = savedStatesLock.Lock();
    if (ER_OK == tempStatus) {
        /*
         * Record the IDs of all the Saved States that are being deleted
         */
        LSF_ID_List savedStatesList;
        for (SavedStateMap::iterator it = savedStates.begin(); it != savedStates.end(); ++it) {
            savedStatesList.push_back(it->first);
        }

        /*
         * Clear the SavedStates
         */
        savedStates.clear();
        tempStatus = savedStatesLock.Unlock();
        if (ER_OK != tempStatus) {
            status = tempStatus;
            QCC_LogError(tempStatus, ("%s: savedStatesLock.Unlock() failed", __FUNCTION__));
        }

        /*
         * Send the SavedStates deleted signal
         */
        tempStatus = controllerService.SendSignal(interfaceName, "SavedStatesDeleted", savedStatesList);
        if (ER_OK != tempStatus) {
            status = tempStatus;
            QCC_LogError(tempStatus, ("%s: Unable to send SavedStatesDeleted signal", __FUNCTION__));
        }
    } else {
        status = tempStatus;
        QCC_LogError(tempStatus, ("%s: savedStatesLock.Lock() failed", __FUNCTION__));
    }

    return status;
}

LSFResponseCode SavedStateManager::GetSavedStateInternal(const LSF_ID& savedStateid, LampState& savedState)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;
    QStatus status = savedStatesLock.Lock();
    if (ER_OK == status) {
        SavedStateMap::iterator it = savedStates.find(savedStateid);
        if (it != savedStates.end()) {
            savedState = it->second.second;
            responseCode = LSF_OK;
        }
        status = savedStatesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: savedStatesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: savedStatesLock.Lock() failed", __FUNCTION__));
    }
    return responseCode;
}

void SavedStateManager::GetAllSavedStateIDs(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSF_ID_List idList;
    LSFResponseCode responseCode = LSF_OK;

    QStatus status = savedStatesLock.Lock();
    if (ER_OK == status) {
        for (SavedStateMap::iterator it = savedStates.begin(); it != savedStates.end(); ++it) {
            idList.push_back(it->first.c_str());
        }
        status = savedStatesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: savedStatesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: savedStatesLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndListOfIDs(msg, responseCode, idList);
}

void SavedStateManager::GetSavedStateName(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSF_Name name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSF_ID savedStateID(id);

    QStatus status = savedStatesLock.Lock();
    if (ER_OK == status) {
        SavedStateMap::iterator it = savedStates.find(id);
        if (it != savedStates.end()) {
            name = it->second.first;
            responseCode = LSF_OK;
        }
        status = savedStatesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: savedStatesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: savedStatesLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeIDAndName(msg, responseCode, savedStateID, name);
}

void SavedStateManager::SetSavedStateName(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSF_ID savedStateID(id);

    const char* name;
    args[1].Get("s", &name);

    QStatus status = savedStatesLock.Lock();
    if (ER_OK == status) {
        SavedStateMap::iterator it = savedStates.find(id);
        if (it != savedStates.end()) {
            it->second.first = LSF_Name(name);
            responseCode = LSF_OK;

            LSF_ID_List idList;
            idList.push_back(savedStateID);
            controllerService.SendSignal(interfaceName, "SavedStatesNameChanged", idList);
        }
        status = savedStatesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: savedStatesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: savedStatesLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, savedStateID);
}

void SavedStateManager::CreateSavedState(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_OK;

    const ajn::MsgArg* inputArgs;
    size_t numInputArgs;
    msg->GetArgs(numInputArgs, inputArgs);

    LampState savedState(inputArgs[0]);
    LSF_ID savedStateID = GenerateUniqueID("SAVED_STATE");

    QStatus status = savedStatesLock.Lock();
    if (ER_OK == status) {
        savedStates[savedStateID].first = savedStateID;
        savedStates[savedStateID].second = savedState;

        LSF_ID_List idList;
        idList.push_back(savedStateID);
        controllerService.SendSignal(interfaceName, "SavedStatesCreated", idList);

        status = savedStatesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: savedStatesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: savedStatesLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, savedStateID);
}

void SavedStateManager::UpdateSavedState(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSF_ID savedStateID(id);
    LampState savedState(args[1]);

    QStatus status = savedStatesLock.Lock();
    if (ER_OK == status) {
        SavedStateMap::iterator it = savedStates.find(id);
        if (it != savedStates.end()) {
            savedStates[savedStateID].second = savedState;
            responseCode = LSF_OK;

            LSF_ID_List idList;
            idList.push_back(savedStateID);
            controllerService.SendSignal(interfaceName, "SavedStatesUpdated", idList);
        }
        status = savedStatesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: savedStatesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: savedStatesLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, savedStateID);
}

void SavedStateManager::DeleteSavedState(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));
    LSF_Name name;
    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    LSF_ID savedStateID(id);

    QStatus status = savedStatesLock.Lock();
    if (ER_OK == status) {
        SavedStateMap::iterator it = savedStates.find(id);
        if (it != savedStates.end()) {
            savedStates.erase(it);
            responseCode = LSF_OK;

            LSF_ID_List idList;
            idList.push_back(savedStateID);
            controllerService.SendSignal(interfaceName, "SavedStatesDeleted", idList);
        }
        status = savedStatesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: savedStatesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: savedStatesLock.Lock() failed", __FUNCTION__));
    }

    controllerService.SendMethodReplyWithResponseCodeAndID(msg, responseCode, savedStateID);
}

void SavedStateManager::GetSavedState(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_NOT_FOUND;

    MsgArg outArgs[3];

    size_t numArgs;
    const MsgArg* args;
    msg->GetArgs(numArgs, args);

    const char* id;
    args[0].Get("s", &id);

    QStatus status = savedStatesLock.Lock();
    if (ER_OK == status) {
        SavedStateMap::iterator it = savedStates.find(id);
        if (it != savedStates.end()) {
            it->second.second.Get(&outArgs[2]);
            responseCode = LSF_OK;
        } else {
            outArgs[2].Set("a{sv}", 0, NULL);
        }
        status = savedStatesLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: savedStatesLock.Unlock() failed", __FUNCTION__));
        }
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: savedStatesLock.Lock() failed", __FUNCTION__));
    }

    outArgs[0].Set("u", responseCode);
    outArgs[1].Set("s", id);

    controllerService.SendMethodReply(msg, outArgs, 3);
}

void SavedStateManager::GetDefaultLampState(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_BUSY;
    LampState savedState;

    MsgArg outArgs[2];

    if (LSF_OK == GetDefaultLampStateInternal(savedState)) {
        savedState.Get(&outArgs[1]);
        responseCode = LSF_OK;
    } else {
        outArgs[1].Set("a{sv}", 0, NULL);
    }

    outArgs[0].Set("u", responseCode);

    controllerService.SendMethodReply(msg, outArgs, 2);
}

void SavedStateManager::SetDefaultLampState(Message& msg)
{
    QCC_DbgPrintf(("%s: %s", __FUNCTION__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_ERR_BUSY;

    const ajn::MsgArg* inputArg;
    size_t numInputArgs;
    msg->GetArgs(numInputArgs, inputArg);

    MsgArg arg;

    LampState savedState(*inputArg);
    if (ER_OK == SetDefaultLampStateInternal(savedState)) {
        responseCode = LSF_OK;
    }

    controllerService.SendMethodReplyWithUint32Value(msg, (uint32_t &)responseCode);
}

LSFResponseCode SavedStateManager::GetDefaultLampStateInternal(LampState& savedState)
{
    QCC_DbgPrintf(("%s", __FUNCTION__));
    LSFResponseCode responseCode = LSF_ERR_FAILURE;
    QStatus status = defaultLampStateLock.Lock();
    if (ER_OK == status) {
        savedState = defaultLampState;
        status = defaultLampStateLock.Unlock();
        if (ER_OK != status) {
            QCC_LogError(status, ("%s: defaultLampStateLock.Unlock() failed", __FUNCTION__));
        }
        responseCode = LSF_OK;
    } else {
        responseCode = LSF_ERR_BUSY;
        QCC_LogError(status, ("%s: defaultLampStateLock.Lock() failed", __FUNCTION__));
    }
    return responseCode;
}

QStatus SavedStateManager::SetDefaultLampStateInternal(LampState& savedState)
{
    QCC_DbgPrintf(("%s: savedState=%s", __FUNCTION__, savedState.c_str()));
    QStatus status = ER_OK;
    QStatus tempStatus = defaultLampStateLock.Lock();
    if (ER_OK == tempStatus) {
        /*
         * Set the defaultLampState to factory settings
         * TODO: Provision for factory setting
         */
        defaultLampState = savedState;
        tempStatus = defaultLampStateLock.Unlock();
        if (ER_OK != tempStatus) {
            status = tempStatus;
            QCC_LogError(tempStatus, ("%s: defaultLampStateLock.Unlock() failed", __FUNCTION__));
        }

        /*
         * Send the DefaultLampStateChangedSignal
         */
        QCC_DbgPrintf(("%s: Sending the DefaultLampStateChangedSignal", __FUNCTION__));
        tempStatus = controllerService.SendSignalWithoutArg(interfaceName, "DefaultLampStateChanged");
        if (ER_OK != tempStatus) {
            QCC_LogError(tempStatus, ("%s: Unable to send DefaultLampStateChanged signal", __FUNCTION__));
        }
    } else {
        status = tempStatus;
        QCC_LogError(tempStatus, ("%s: defaultLampStateLock.Lock() failed", __FUNCTION__));
    }

    return status;
}


