/*****************************************************************************
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

#include <ControllerClient.h>
#include <LampManager.h>
#include <LampGroupManager.h>
#include <SavedStateManager.h>
#include <SceneManager.h>
#include <SceneGroupManager.h>
#include <ControllerServiceManager.h>

#include <qcc/StringUtil.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/ProxyBusObject.h>
#include <qcc/Debug.h>


using namespace qcc;
using namespace lsf;
using namespace ajn;

#define QCC_MODULE "SAMPLE"

bool connectedToControllerService = false;
bool gotReply = false;
bool gotSignal = false;

char* get_line(char* str, size_t num, FILE* fp)
{
    char* p = fgets(str, num, fp);

    // fgets will capture the '\n' character if the string entered is shorter than
    // num. Remove the '\n' from the end of the line and replace it with nul '\0'.
    if (p != NULL) {
        size_t last = strlen(str) - 1;
        if (str[last] == '\n') {
            str[last] = '\0';
        }
    }
    return p;
}

static qcc::String NextTok(qcc::String& inStr)
{
    qcc::String ret;
    size_t off = inStr.find_first_of(' ');
    if (off == qcc::String::npos) {
        ret = inStr;
        inStr.clear();
    } else {
        ret = inStr.substr(0, off);
        inStr = qcc::Trim(inStr.substr(off));
    }
    return qcc::Trim(ret);
}

class ControllerClientCallbackHandler : public ControllerClientCallback {
  public:

    ~ControllerClientCallbackHandler() { }

    void ConnectedToControllerServiceCB(const LSF_ID& controllerServiceDeviceID, const LSF_Name& controllerServiceName) {
        LSF_ID id = controllerServiceDeviceID;
        LSF_Name name = controllerServiceName;
        printf("\n%s: controllerServiceDeviceID = %s controllerServiceName = %s\n", __FUNCTION__, id.c_str(), name.c_str());
        connectedToControllerService = true;
        gotReply = true;
        gotSignal = true;
    }

    void ConnectToControllerServiceFailedCB(const LSF_ID& controllerServiceDeviceID, const LSF_Name& controllerServiceName) {
        LSF_ID id = controllerServiceDeviceID;
        LSF_Name name = controllerServiceName;
        printf("\n%s: controllerServiceDeviceID = %s controllerServiceName = %s\n", __FUNCTION__, id.c_str(), name.c_str());
        gotReply = true;
        gotSignal = true;
    }

    void DisconnectedFromControllerServiceCB(const LSF_ID& controllerServiceDeviceID, const LSF_Name& controllerServiceName) {
        LSF_ID id = controllerServiceDeviceID;
        LSF_Name name = controllerServiceName;
        printf("\n%s: controllerServiceDeviceID = %s controllerServiceName = %s\n", __FUNCTION__, id.c_str(), name.c_str());
        connectedToControllerService = false;
        gotReply = true;
        gotSignal = true;
    }

    void ControllerClientErrorCB(const ErrorCodeList& errorCodeList) {
        printf("\n%s:", __FUNCTION__);
        ErrorCodeList::const_iterator it = errorCodeList.begin();
        for (; it != errorCodeList.end(); ++it) {
            printf("\n%s", ControllerClientErrorText(*it));
        }
        printf("\n");
        gotReply = true;
        gotSignal = true;
    }
};

class ControllerServiceManagerCallbackHandler : public ControllerServiceManagerCallback {

    void GetControllerServiceVersionReplyCB(const uint32_t& version) {
        printf("\n%s: version = %d\n", __FUNCTION__, version);
        gotReply = true;
    }

    void LightingResetControllerServiceReplyCB(const LSFResponseCode& responseCode) {
        printf("\n%s: responseCode = %s\n", __FUNCTION__, LSFResponseCodeText(responseCode));
        gotReply = true;
    }

    void ControllerServiceLightingResetCB(void) {
        printf("\n%s\n", __FUNCTION__);
        gotSignal = true;
    }

    void ControllerServiceNameChangedCB(void) {
        printf("\n%s", __FUNCTION__);
        gotSignal = true;
    }
};

class LampManagerCallbackHandler : public LampManagerCallback {

    void GetAllLampIDsReplyCB(const LSFResponseCode& responseCode, const LSF_ID_List& lampIDs) {
        printf("\n%s(): responseCode = %s, listsize=%lu", __FUNCTION__, LSFResponseCodeText(responseCode), lampIDs.size());
        if (responseCode == LSF_OK) {
            LSF_ID_List::const_iterator it = lampIDs.begin();
            uint8_t count = 1;
            for (; it != lampIDs.end(); ++it) {
                printf("\n(%d)%s", count, (*it).c_str());
                count++;
            }
            printf("\n");
        }
        gotReply = true;
    }

    void GetLampNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const LSF_Name& lampName) {
        LSF_ID id = lampID;

        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());

        if (responseCode == LSF_OK) {
            LSF_Name name = lampName;
            printf("\nlampName = %s", name.c_str());
        }
        gotReply = true;
    }

    void SetLampNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void LampsNameChangedCB(const LSF_ID_List& lampIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = lampIDs.begin();
        for (; it != lampIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void GetLampDetailsReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const LampDetails& lampDetails) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf("\n%s: lampDetails = %s", __FUNCTION__, lampDetails.c_str());
        }
        gotReply = true;
    }

    void GetLampParametersReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const LampParameters& lampParameters) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf("\n%s: parameters = %s", __FUNCTION__, lampParameters.c_str());
        }
        gotReply = true;
    }

    void GetLampParametersBrightnessLumensFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const uint32_t& value) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" value = %d", value);
        }
        gotReply = true;
    }

    void GetLampParametersEnergyUsageMilliwattsFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const uint32_t& value) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" value = %d", value);
        }
        gotReply = true;
    }

    void GetLampStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const LampState& lampState) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf("\nstate=%s\n", lampState.c_str());
        }
        gotReply = true;
    }

    void GetLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const bool& onOff) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" onOff = %d", onOff);
        }
        gotReply = true;
    }

    void GetLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const uint32_t& hue) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" hue = %d", hue);
        }
        gotReply = true;
    }

    void GetLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const uint32_t& saturation) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" saturation = %d", saturation);
        }
        gotReply = true;
    }

    void GetLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const uint32_t& brightness) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" brightness = %d", brightness);
        }
        gotReply = true;
    }

    void GetLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const uint32_t& colorTemp) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" colorTemp = %d", colorTemp);
        }
        gotReply = true;
    }

    void ResetLampStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void LampsStateChangedCB(const LSF_ID_List& lampIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = lampIDs.begin();
        for (; it != lampIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void TransitionLampStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (responseCode != LSF_OK) {
            gotSignal = true;
        }
    }

    void TransitionLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void TransitionLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void TransitionLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void TransitionLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void TransitionLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void GetLampFaultsReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const LampFaultCodeList& faultCodes) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            LampFaultCodeList::const_iterator it = faultCodes.begin();
            for (; it != faultCodes.end(); ++it) {
                printf("\n%d", *it);
            }
            printf("\n");
        }
        gotReply = true;
    }

    void ClearLampFaultReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID, const LampFaultCode& faultCode) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s faultCode=0x%x", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), faultCode);
        gotReply = true;
    }

    void ResetLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ResetLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ResetLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ResetLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ResetLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void TransitionLampStateToSavedStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampID) {
        LSF_ID id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }
};

class LampGroupManagerCallbackHandler : public LampGroupManagerCallback {
    void GetLampGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID, const LampGroup& lampGroup) {
        printf("\n%s(): responseCode = %s, lampGroupID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), lampGroupID.c_str());
        if (responseCode == LSF_OK) {
            printf("\nlampGroup=%s", lampGroup.c_str());
        }
        gotReply = true;
    }

    void GetAllLampGroupIDsReplyCB(const LSFResponseCode& responseCode, const LSF_ID_List& lampGroupIDs) {
        printf("\n%s(): responseCode = %s, listsize=%d", __FUNCTION__, LSFResponseCodeText(responseCode), lampGroupIDs.size());
        if (responseCode == LSF_OK) {
            LSF_ID_List::const_iterator it = lampGroupIDs.begin();
            uint8_t count = 1;
            for (; it != lampGroupIDs.end(); ++it) {
                printf("\n(%d)%s", count, (*it).c_str());
                count++;
            }
            printf("\n");
        }
        gotReply = true;
    }

    void GetLampGroupNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID, const LSF_Name& lampGroupName) {
        LSF_ID id = lampGroupID;

        printf("\n%s: responseCode = %s lampGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());

        if (responseCode == LSF_OK) {
            LSF_Name name = lampGroupName;
            printf("\nlampGroupName = %s", name.c_str());
        }
        gotReply = true;
    }

    void SetLampGroupNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) {
        printf("\n%s(): responseCode = %s, lampGroupID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), lampGroupID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void LampGroupsNameChangedCB(const LSF_ID_List& lampGroupIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = lampGroupIDs.begin();
        for (; it != lampGroupIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void CreateLampGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) {
        printf("\n%s: responseCode=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode));
        if (LSF_OK == responseCode) {
            printf("lampGroupID=%s\n", lampGroupID.c_str());
        } else {
            gotSignal = true;
        }
        gotReply = true;
    }

    void LampGroupsCreatedCB(const LSF_ID_List& lampGroupIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = lampGroupIDs.begin();
        for (; it != lampGroupIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void UpdateLampGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) {
        printf("\n%s(): responseCode = %s, lampGroupID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), lampGroupID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void LampGroupsUpdatedCB(const LSF_ID_List& lampGroupIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = lampGroupIDs.begin();
        for (; it != lampGroupIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void DeleteLampGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) {
        printf("\n%s(): responseCode = %s, lampGroupID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), lampGroupID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void LampGroupsDeletedCB(const LSF_ID_List& lampGroupIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = lampGroupIDs.begin();
        for (; it != lampGroupIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }
#if 0
    void GetLampGroupNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID, const LSF_Name& lampGroupName) {
        LSF_ID id = lampGroupID;

        printf("\n%s: responseCode = %s lampGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());

        if (responseCode == LSF_OK) {
            LSF_Name name = lampGroupName;
            printf("\nlampGroupName = %s", name.c_str());
        }
        gotReply = true;
    }

    void ResetLampGroupFieldStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) {
        LSF_ID id = lampGroupID;
        LSF_Name name = stateFieldName;
        printf("\n%s: responseCode = %s lampGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), name.c_str());
        gotReply = true;
    }

    void TransitionLampGroupStateFieldReplyCB(const LSFResponseCode& responseCode, const LSF_ID& lampGroupID) {
        LSF_ID id = lampGroupID;
        LSF_Name name = stateFieldName;
        printf("\n%s: responseCode = %s lampGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), name.c_str());
        gotReply = true;
    }
#endif
};

class SavedStateManagerCallbackHandler : public SavedStateManagerCallback {

    void GetSavedStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& savedStateID, const LampState& savedState) {
        printf("\n%s(): responseCode = %s, savedStateID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), savedStateID.c_str());
        if (responseCode == LSF_OK) {
            printf("\nsavedState=%s", savedState.c_str());
        }
        gotReply = true;
    }

    void GetAllSavedStateIDsReplyCB(const LSFResponseCode& responseCode, const LSF_ID_List& savedStateIDs) {
        printf("\n%s(): responseCode = %s, listsize=%d", __FUNCTION__, LSFResponseCodeText(responseCode), savedStateIDs.size());
        if (responseCode == LSF_OK) {
            LSF_ID_List::const_iterator it = savedStateIDs.begin();
            uint8_t count = 1;
            for (; it != savedStateIDs.end(); ++it) {
                printf("\n(%d)%s", count, (*it).c_str());
                count++;
            }
            printf("\n");
        }
        gotReply = true;
    }

    void GetSavedStateNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& savedStateID, const LSF_Name& savedStateName) {
        LSF_ID id = savedStateID;

        printf("\n%s: responseCode = %s savedStateID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());

        if (responseCode == LSF_OK) {
            LSF_Name name = savedStateName;
            printf("\nsavedStateName = %s", name.c_str());
        }
        gotReply = true;
    }

    void SetSavedStateNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& savedStateID) {
        printf("\n%s(): responseCode = %s, savedStateID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), savedStateID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void SavedStatesNameChangedCB(const LSF_ID_List& savedStateIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = savedStateIDs.begin();
        for (; it != savedStateIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void CreateSavedStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& savedStateID) {
        printf("\n%s: responseCode=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode));
        if (LSF_OK == responseCode) {
            printf("savedStateID=%s\n", savedStateID.c_str());
        } else {
            gotSignal = true;
        }
        gotReply = true;
    }

    void SavedStatesCreatedCB(const LSF_ID_List& savedStateIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = savedStateIDs.begin();
        for (; it != savedStateIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void UpdateSavedStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& savedStateID) {
        printf("\n%s(): responseCode = %s, savedStateID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), savedStateID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void SavedStatesUpdatedCB(const LSF_ID_List& savedStateIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = savedStateIDs.begin();
        for (; it != savedStateIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void DeleteSavedStateReplyCB(const LSFResponseCode& responseCode, const LSF_ID& savedStateID) {
        printf("\n%s(): responseCode = %s, savedStateID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), savedStateID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void SavedStatesDeletedCB(const LSF_ID_List& savedStateIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = savedStateIDs.begin();
        for (; it != savedStateIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void GetDefaultLampStateReplyCB(const LSFResponseCode& responseCode, const LampState& defaultState) {
        printf("\n%s: responseCode = %s", __FUNCTION__, LSFResponseCodeText(responseCode));
        if (responseCode == LSF_OK) {
            printf("\nstate=%s\n", defaultState.c_str());
        }
        gotReply = true;
    }

    void SetDefaultLampStateReplyCB(const LSFResponseCode& responseCode) {
        printf("\n%s: responseCode = %s\n", __FUNCTION__, LSFResponseCodeText(responseCode));
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void DefaultLampStateChangedCB(void) {
        printf("\n%s", __FUNCTION__);
        gotSignal = true;
    }
};

class SceneManagerCallbackHandler : public SceneManagerCallback {
    void GetSceneReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneID, const Scene& scene) {
        printf("\n%s(): responseCode = %s, sceneID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), sceneID.c_str());
        if (responseCode == LSF_OK) {
            printf("\nscene=%s", scene.c_str());
        }
        gotReply = true;
    }

    void GetAllSceneIDsReplyCB(const LSFResponseCode& responseCode, const LSF_ID_List& sceneIDs) {
        printf("\n%s(): responseCode = %s, listsize=%d", __FUNCTION__, LSFResponseCodeText(responseCode), sceneIDs.size());
        if (responseCode == LSF_OK) {
            LSF_ID_List::const_iterator it = sceneIDs.begin();
            uint8_t count = 1;
            for (; it != sceneIDs.end(); ++it) {
                printf("\n(%d)%s", count, (*it).c_str());
                count++;
            }
            printf("\n");
        }
        gotReply = true;
    }

    void GetSceneNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneID, const LSF_Name& sceneName) {
        LSF_ID id = sceneID;

        printf("\n%s: responseCode = %s sceneID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());

        if (responseCode == LSF_OK) {
            LSF_Name name = sceneName;
            printf("\nsceneName = %s", name.c_str());
        }
        gotReply = true;
    }

    void SetSceneNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneID) {
        printf("\n%s(): responseCode = %s, sceneID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), sceneID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ScenesNameChangedCB(const LSF_ID_List& sceneIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = sceneIDs.begin();
        for (; it != sceneIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void CreateSceneReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneID) {
        printf("\n%s: responseCode=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode));
        if (LSF_OK == responseCode) {
            printf("sceneID=%s\n", sceneID.c_str());
        } else {
            gotSignal = true;
        }
        gotReply = true;
    }

    void ScenesCreatedCB(const LSF_ID_List& sceneIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = sceneIDs.begin();
        for (; it != sceneIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void UpdateSceneReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneID) {
        printf("\n%s(): responseCode = %s, sceneID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), sceneID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ScenesUpdatedCB(const LSF_ID_List& sceneIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = sceneIDs.begin();
        for (; it != sceneIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void DeleteSceneReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneID) {
        printf("\n%s(): responseCode = %s, sceneID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), sceneID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ScenesDeletedCB(const LSF_ID_List& sceneIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = sceneIDs.begin();
        for (; it != sceneIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }
};

class SceneGroupManagerCallbackHandler : public SceneGroupManagerCallback {
    void GetSceneGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneGroupID, const SceneGroup& sceneGroup) {
        printf("\n%s(): responseCode = %s, sceneGroupID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), sceneGroupID.c_str());
        if (responseCode == LSF_OK) {
            printf("\nsceneGroup=%s", sceneGroup.c_str());
        }
        gotReply = true;
    }

    void GetAllSceneGroupIDsReplyCB(const LSFResponseCode& responseCode, const LSF_ID_List& sceneGroupIDs) {
        printf("\n%s(): responseCode = %s, listsize=%d", __FUNCTION__, LSFResponseCodeText(responseCode), sceneGroupIDs.size());
        if (responseCode == LSF_OK) {
            LSF_ID_List::const_iterator it = sceneGroupIDs.begin();
            uint8_t count = 1;
            for (; it != sceneGroupIDs.end(); ++it) {
                printf("\n(%d)%s", count, (*it).c_str());
                count++;
            }
            printf("\n");
        }
        gotReply = true;
    }

    void GetSceneGroupNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneGroupID, const LSF_Name& sceneGroupName) {
        LSF_ID id = sceneGroupID;

        printf("\n%s: responseCode = %s sceneGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());

        if (responseCode == LSF_OK) {
            LSF_Name name = sceneGroupName;
            printf("\nsceneGroupName = %s", name.c_str());
        }
        gotReply = true;
    }

    void SetSceneGroupNameReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneGroupID) {
        printf("\n%s(): responseCode = %s, sceneGroupID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), sceneGroupID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void SceneGroupsNameChangedCB(const LSF_ID_List& sceneGroupIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = sceneGroupIDs.begin();
        for (; it != sceneGroupIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void CreateSceneGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneGroupID) {
        printf("\n%s: responseCode=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode));
        if (LSF_OK == responseCode) {
            printf("sceneGroupID=%s\n", sceneGroupID.c_str());
        } else {
            gotSignal = true;
        }
        gotReply = true;
    }

    void SceneGroupsCreatedCB(const LSF_ID_List& sceneGroupIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = sceneGroupIDs.begin();
        for (; it != sceneGroupIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void UpdateSceneGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneGroupID) {
        printf("\n%s(): responseCode = %s, sceneGroupID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), sceneGroupID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void SceneGroupsUpdatedCB(const LSF_ID_List& sceneGroupIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = sceneGroupIDs.begin();
        for (; it != sceneGroupIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void DeleteSceneGroupReplyCB(const LSFResponseCode& responseCode, const LSF_ID& sceneGroupID) {
        printf("\n%s(): responseCode = %s, sceneGroupID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), sceneGroupID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void SceneGroupsDeletedCB(const LSF_ID_List& sceneGroupIDs) {
        printf("\n%s", __FUNCTION__);
        LSF_ID_List::const_iterator it = sceneGroupIDs.begin();
        for (; it != sceneGroupIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }
};

void PrintHelp() {
    printf("\nEnter one of the following command numbers to test end-to-end functionality:\n");
    printf("(1):   GetControllerServiceVersion\n");
    printf("(2):   LightingResetControllerService\n");
    printf("(3):   GetAllLampIDs\n");
    printf("(4):   GetLampName\n");
    printf("(5):   SetLampName\n");
    printf("(6):   GetLampState\n");
    printf("(7):   GetLampStateOnOffField\n");
    printf("(8):   GetLampStateHueField\n");
    printf("(9):   GetLampStateSaturationField\n");
    printf("(10):  GetLampStateBrightnessField\n");
    printf("(11):  GetLampStateColorTempField\n");
    printf("(12):  ResetLampState\n");
    printf("(13):  ResetLampStateOnOffField\n");
    printf("(14):  ResetLampStateHueField\n");
    printf("(15):  ResetLampStateSaturationField\n");
    printf("(16):  ResetLampStateBrightnessField\n");
    printf("(17):  ResetLampStateColorTempField\n");
    printf("(18):  TransitionLampState\n");
    printf("(19):  TransitionLampStateOnOffField\n");
    printf("(20):  TransitionLampStateHueField\n");
    printf("(21):  TransitionLampStateSaturationField\n");
    printf("(22):  TransitionLampStateBrightnessField\n");
    printf("(23):  TransitionLampStateColorTempField\n");
    printf("(24):  TransitionLampStateToSavedState\n");
    printf("(25):  GetLampFaults\n");
    printf("(26):  ClearLampFault\n");
    printf("(27):  GetLampDetails\n");
    printf("(28):  GetLampParameters\n");
    printf("(29):  GetLampParametersEnergyUsageMilliwattsField\n");
    printf("(30):  GetLampParametersBrightnessLumensField\n");
    printf("(31):  GetAllLampGroupIDs\n");
    printf("(32):  GetLampGroupName\n");
    printf("(33):  SetLampGroupName\n");
    printf("(34):  CreateLampGroup\n");
    printf("(35):  UpdateLampGroup\n");
    printf("(36):  DeleteLampGroup\n");
    printf("(37):  GetLampGroup\n");
    printf("(38):  ResetLampGroupState\n");
    printf("(39):  ResetLampGroupStateField\n");
    printf("(40):  TransitionLampGroupState\n");
    printf("(41):  TransitionLampGroupStateToSavedState\n");
    printf("(42):  TransitionLampGroupStateField\n");
    printf("(43):  GetDefaultLampState\n");
    printf("(44):  SetDefaultLampState\n");
    printf("(45):  GetAllSavedStateIDs\n");
    printf("(46):  GetSavedStateName\n");
    printf("(47):  SetSavedStateName\n");
    printf("(48):  CreateSavedState\n");
    printf("(49):  UpdateSavedState\n");
    printf("(50):  DeleteSavedState\n");
    printf("(51):  GetSavedState\n");
    printf("(52):  GetAllSceneIDs\n");
    printf("(53):  GetSceneName\n");
    printf("(54):  SetSceneName\n");
    printf("(55):  CreateScene\n");
    printf("(56):  UpdateScene\n");
    printf("(57):  DeleteScene\n");
    printf("(58):  GetScene\n");
    printf("(59):  ApplyScene\n");
    printf("(60):  GetAllSceneGroupIDs\n");
    printf("(61):  GetSceneGroupName\n");
    printf("(62):  SetSceneGroupName\n");
    printf("(63):  CreateSceneGroup\n");
    printf("(64):  UpdateSceneGroup\n");
    printf("(65):  DeleteSceneGroup\n");
    printf("(66):  GetSceneGroup\n");
    printf("(67):  ApplySceneGroup\n");
}

int main()
{
    bool waitForReply = false;
    bool waitForSignal = false;
    bool unrecognizedCommand = false;

    BusAttachment bus("ClientTest", true);
    bus.Start();
    bus.Connect();

    ControllerClientCallbackHandler controllerClientCBHandler;
    ControllerServiceManagerCallbackHandler controllerServiceManagerCBHandler;
    LampManagerCallbackHandler lampManagerCBHandler;
    LampGroupManagerCallbackHandler lampGroupManagerCBHandler;
    SavedStateManagerCallbackHandler savedStateManagerCBHandler;
    SceneManagerCallbackHandler sceneManagerCBHandler;
    SceneGroupManagerCallbackHandler sceneGroupManagerCBHandler;

    ControllerClient client(bus, controllerClientCBHandler);
    ControllerServiceManager controllerServiceManager(client, controllerServiceManagerCBHandler);
    LampManager lampManager(client, lampManagerCBHandler);
    LampGroupManager lampGroupManager(client, lampGroupManagerCBHandler);
    SavedStateManager savedStateManager(client, savedStateManagerCBHandler);
    SceneManager sceneManager(client, sceneManagerCBHandler);
    SceneGroupManager sceneGroupManager(client, sceneGroupManagerCBHandler);

    printf("\nController Client Version = %d\n", client.GetVersion());

    printf("\nWaiting for the Controller Client to connect to a Controller Service...\n");

    //Wait for the Controller Client to find and connect to a Controller Service
    while (!connectedToControllerService) ;

    printf("\nController Client setup successful.\n");

    ControllerClientStatus status = CONTROLLER_CLIENT_OK;
    const int bufSize = 1024;
    char buf[bufSize];

    while (true) {
        gotReply = false;
        gotSignal = false;

        waitForReply = false;
        waitForSignal = false;

        sleep(1);
        PrintHelp();

        if (get_line(buf, bufSize, stdin)) {
            String line(buf);
            String cmd = NextTok(line);

            if (cmd == "1") {
                printf("\nInvoking GetControllerServiceVersion()\n");
                status = controllerServiceManager.GetControllerServiceVersion();
                waitForReply = true;
            } else if (cmd == "2") {
                status = controllerServiceManager.LightingResetControllerService();
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "3") {
                printf("\nInvoking GetAllLampIDs()\n");
                status = lampManager.GetAllLampIDs();
                waitForReply = true;
            } else if (cmd == "4") {
                String id = NextTok(line);
                printf("\nInvoking GetLampName(%s)\n", id.c_str());
                status = lampManager.GetLampName(id.c_str());
                waitForReply = true;
            } else if (cmd == "5") {
                String id = NextTok(line);
                String name = NextTok(line);
                printf("\nInvoking SetLampName(%s, %s)\n", id.c_str(), name.c_str());
                lampManager.SetLampName(id.c_str(), name.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "6") {
                String id = NextTok(line);
                printf("\nInvoking GetLampState(%s)\n", id.c_str());
                status = lampManager.GetLampState(id.c_str());
                waitForReply = true;
            } else if (cmd == "7") {
                String id = NextTok(line);
                printf("\nInvoking GetLampStateOnOffField(%s)\n", id.c_str());
                lampManager.GetLampStateOnOffField(id.c_str());
                waitForReply = true;
            } else if (cmd == "8") {
                String id = NextTok(line);
                printf("\nInvoking GetLampStateHueField(%s)\n", id.c_str());
                lampManager.GetLampStateHueField(id.c_str());
                waitForReply = true;
            } else if (cmd == "9") {
                String id = NextTok(line);
                printf("\nInvoking GetLampStateSaturationField(%s)\n", id.c_str());
                lampManager.GetLampStateSaturationField(id.c_str());
                waitForReply = true;
            } else if (cmd == "10") {
                String id = NextTok(line);
                printf("\nInvoking GetLampStateBrightnessField(%s)\n", id.c_str());
                lampManager.GetLampStateBrightnessField(id.c_str());
                waitForReply = true;
            } else if (cmd == "11") {
                String id = NextTok(line);
                printf("\nInvoking GetLampStateColorTempField(%s)\n", id.c_str());
                lampManager.GetLampStateColorTempField(id.c_str());
                waitForReply = true;
            } else if (cmd == "12") {
                String id = NextTok(line);
                printf("\nInvoking ResetLampState(%s)\n", id.c_str());
                lampManager.ResetLampState(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "13") {
                String id = NextTok(line);
                printf("\nInvoking ResetLampStateOnOffField(%s)\n", id.c_str());
                lampManager.ResetLampStateOnOffField(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "14") {
                String id = NextTok(line);
                printf("\nInvoking ResetLampStateHueField(%s)\n", id.c_str());
                lampManager.ResetLampStateHueField(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "15") {
                String id = NextTok(line);
                printf("\nInvoking ResetLampStateSaturationField(%s)\n", id.c_str());
                lampManager.ResetLampStateSaturationField(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "16") {
                String id = NextTok(line);
                printf("\nInvoking ResetLampStateBrightnessField(%s)\n", id.c_str());
                lampManager.ResetLampStateBrightnessField(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "17") {
                String id = NextTok(line);
                printf("\nInvoking ResetLampStateColorTempField(%s)\n", id.c_str());
                lampManager.ResetLampStateColorTempField(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "18") {
                String id = NextTok(line);
                LampState state;
                printf("\nInvoking TransitionLampState(%s)\n", id.c_str());
                lampManager.TransitionLampState(id.c_str(), state, 5);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "19") {
                String id = NextTok(line);
                printf("\nInvoking TransitionLampStateOnOffField(%s)\n", id.c_str());
                lampManager.TransitionLampStateOnOffField(id.c_str(), false);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "20") {
                String id = NextTok(line);
                printf("\nInvoking TransitionLampStateHueField(%s)\n", id.c_str());
                lampManager.TransitionLampStateHueField(id.c_str(), 10, 100);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "21") {
                String id = NextTok(line);
                printf("\nInvoking TransitionLampStateSaturationField(%s)\n", id.c_str());
                lampManager.TransitionLampStateSaturationField(id.c_str(), 20);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "22") {
                String id = NextTok(line);
                printf("\nInvoking TransitionLampStateBrightnessField(%s)\n", id.c_str());
                lampManager.TransitionLampStateBrightnessField(id.c_str(), 30, 100);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "23") {
                String id = NextTok(line);
                printf("\nInvoking TransitionLampStateColorTempField(%s)\n", id.c_str());
                lampManager.TransitionLampStateColorTempField(id.c_str(), 50);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "24") {
                String id = NextTok(line);
                String savedStateID = NextTok(line);
                printf("\nInvoking TransitionLampStateToSavedState(%s, %s)\n", id.c_str(), savedStateID.c_str());
                lampManager.TransitionLampStateToSavedState(id.c_str(), savedStateID.c_str(), 25);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "25") {
                String id = NextTok(line);
                printf("\nInvoking GetLampFaults(%s)\n", id.c_str());
                status = lampManager.GetLampFaults(id.c_str());
                waitForReply = true;
            } else if (cmd == "26") {
                String id = NextTok(line);
                LampFaultCode code = qcc::StringToU32(NextTok(line));
                printf("\nInvoking ClearLampFault(%s, %d)\n", id.c_str(), code);
                status = lampManager.ClearLampFault(id.c_str(), code);
                waitForReply = true;
            } else if (cmd == "27") {
                String id = NextTok(line);
                printf("\nInvoking GetLampDetails(%s)\n", id.c_str());
                status = lampManager.GetLampDetails(id.c_str());
                waitForReply = true;
            } else if (cmd == "28") {
                String id = NextTok(line);
                status = lampManager.GetLampParameters(id.c_str());
                printf("\nInvoking GetLampParameters(%s)\n", id.c_str());
                waitForReply = true;
            } else if (cmd == "29") {
                String id = NextTok(line);
                printf("\nInvoking GetLampParametersEnergyUsageMilliwattsField(%s)\n", id.c_str());
                lampManager.GetLampParametersEnergyUsageMilliwattsField(id.c_str());
                waitForReply = true;
            } else if (cmd == "30") {
                String id = NextTok(line);
                printf("\nInvoking GetLampParametersBrightnessLumensField(%s)\n", id.c_str());
                lampManager.GetLampParametersBrightnessLumensField(id.c_str());
                waitForReply = true;
            } else if (cmd == "31") {
                printf("\nInvoking GetAllLampGroupIDs()\n");
                status = lampGroupManager.GetAllLampGroupIDs();
                waitForReply = true;
            } else if (cmd == "32") {
                String id = NextTok(line);
                printf("\nInvoking GetLampGroupName(%s)\n", id.c_str());
                status = lampGroupManager.GetLampGroupName(id.c_str());
                waitForReply = true;
            } else if (cmd == "33") {
                String id = NextTok(line);
                String name = NextTok(line);
                printf("\nInvoking SetLampGroupName(%s, %s)\n", id.c_str(), name.c_str());
                status = lampGroupManager.SetLampGroupName(id.c_str(), name.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "34") {
                LSF_ID_List lamps;
                lamps.push_back("abc");
                lamps.push_back("xyz");
                LampGroup group(lamps, lamps);
                printf("\nInvoking CreateLampGroup(%s)\n", group.c_str());
                status = lampGroupManager.CreateLampGroup(group);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "35") {
                String id = NextTok(line);
                LSF_ID_List lamps;
                lamps.push_back("123");
                lamps.push_back("456");
                LampGroup group(lamps, lamps);
                printf("\nInvoking UpdateLampGroup(%s, %s)\n", id.c_str(), group.c_str());
                status = lampGroupManager.UpdateLampGroup(id.c_str(), group);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "36") {
                String id = NextTok(line);
                printf("\nInvoking DeleteLampGroup(%s)\n", id.c_str());
                status = lampGroupManager.DeleteLampGroup(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "37") {
                String id = NextTok(line);
                printf("\nInvoking GetLampGroup(%s)\n", id.c_str());
                status = lampGroupManager.GetLampGroup(id.c_str());
                waitForReply = true;
#if 0
            } else if (cmd == "27") {
                ResetLampGroupState;
            } else if (cmd == "28") {
                String id = NextTok(line);
                String fieldname = NextTok(line);
                printf("\nInvoking ResetLampGroupFieldState(%s, %s)\n", id.c_str(), fieldname.c_str());
                lampGroupManager.ResetLampGroupFieldState(id.c_str(), fieldname.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "29") {
                TransitionLampGroupState;
            } else if (cmd == "30") {
                TransitionLampGroupStateToSavedState;
            } else if (cmd == "31") {
                String id = NextTok(line);
                String fieldname = NextTok(line);
                printf("\nInvoking TransitionLampGroupStateField(%s, %s)\n", id.c_str(), fieldname.c_str());
                uint32_t value = 55;
                MsgArg arg;
                arg.Set("u", value);
                lampGroupManager.TransitionLampGroupStateField(id.c_str(), fieldname.c_str(), arg);
                waitForReply = true;
                waitForSignal = true;
#endif
            } else if (cmd == "43") {
                printf("\nInvoking GetDefaultLampState()\n");
                status = savedStateManager.GetDefaultLampState();
                waitForReply = true;
            } else if (cmd == "44") {
                printf("\nInvoking SetDefaultLampState()\n");
                LampState state(true, 3, 3, 3, 3);
                status = savedStateManager.SetDefaultLampState(state);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "45") {
                printf("\nInvoking GetAllSavedStateIDs()\n");
                status = savedStateManager.GetAllSavedStateIDs();
                waitForReply = true;
            } else if (cmd == "46") {
                String id = NextTok(line);
                printf("\nInvoking GetSavedStateName(%s)\n", id.c_str());
                status = savedStateManager.GetSavedStateName(id.c_str());
                waitForReply = true;
            } else if (cmd == "47") {
                String id = NextTok(line);
                String name = NextTok(line);
                printf("\nInvoking SetSavedStateName(%s, %s)\n", id.c_str(), name.c_str());
                status = savedStateManager.SetSavedStateName(id.c_str(), name.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "48") {
                LampState state(true, 5, 5, 5, 5);
                printf("\nInvoking CreateSavedState(%s)\n", state.c_str());
                status = savedStateManager.CreateSavedState(state);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "49") {
                String id = NextTok(line);
                LampState state(true, 6, 6, 6, 6);
                printf("\nInvoking UpdateSavedState(%s, %s)\n", id.c_str(), state.c_str());
                status = savedStateManager.UpdateSavedState(id.c_str(), state);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "50") {
                String id = NextTok(line);
                printf("\nInvoking DeleteSavedState(%s)\n", id.c_str());
                status = savedStateManager.DeleteSavedState(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "51") {
                String id = NextTok(line);
                printf("\nInvoking GetSavedState(%s)\n", id.c_str());
                status = savedStateManager.GetSavedState(id.c_str());
                waitForReply = true;
            } else if (cmd == "52") {
                printf("\nInvoking GetAllSceneIDs()\n");
                status = sceneManager.GetAllSceneIDs();
                waitForReply = true;
            } else if (cmd == "53") {
                String id = NextTok(line);
                printf("\nInvoking GetSceneName(%s)\n", id.c_str());
                status = sceneManager.GetSceneName(id.c_str());
                waitForReply = true;
            } else if (cmd == "54") {
                String id = NextTok(line);
                String name = NextTok(line);
                printf("\nInvoking SetSceneName(%s, %s)\n", id.c_str(), name.c_str());
                status = sceneManager.SetSceneName(id.c_str(), name.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "55") {
                LSF_ID_List lampList;
                lampList.push_back("abc");
                LSF_ID_List lampGroupList;
                lampGroupList.push_back("xyz");
                LampState state(false, 6, 6, 6, 6);
                LSF_ID savedStateID("123");
                uint32_t transPeriod = 35;
                LampsLampGroupsAndState stateComponent(lampList, lampGroupList, state, transPeriod);
                LampsLampGroupsAndSavedState savedStateComponent(lampList, lampGroupList, savedStateID, transPeriod);

                LampsLampGroupsAndStateList stateList;
                stateList.push_back(stateComponent);

                LampsLampGroupsAndSavedStateList savedStateList;
                savedStateList.push_back(savedStateComponent);

                Scene scene(stateList, savedStateList);

                printf("\nInvoking CreateScene(%s)\n", scene.c_str());
                status = sceneManager.CreateScene(scene);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "56") {
                String id = NextTok(line);

                LSF_ID_List lampList;
                lampList.push_back("ABC");
                LSF_ID_List lampGroupList;
                lampGroupList.push_back("XYZ");
                LampState state(false, 7, 7, 7, 7);
                LSF_ID savedStateID("xxx");
                LampsLampGroupsAndState stateComponent(lampList, lampGroupList, state);
                LampsLampGroupsAndSavedState savedStateComponent(lampList, lampGroupList, savedStateID);

                LampsLampGroupsAndStateList stateList;
                stateList.push_back(stateComponent);

                LampsLampGroupsAndSavedStateList savedStateList;
                savedStateList.push_back(savedStateComponent);

                Scene scene(stateList, savedStateList);

                printf("\nInvoking UpdateScene(%s, %s)\n", id.c_str(), scene.c_str());
                status = sceneManager.UpdateScene(id.c_str(), scene);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "57") {
                String id = NextTok(line);
                printf("\nInvoking DeleteScene(%s)\n", id.c_str());
                status = sceneManager.DeleteScene(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "58") {
                String id = NextTok(line);
                printf("\nInvoking GetScene(%s)\n", id.c_str());
                status = sceneManager.GetScene(id.c_str());
                waitForReply = true;
#if 0
            } else if (cmd == "46") {
                ApplyScene;
#endif
            } else if (cmd == "60") {
                printf("\nInvoking GetAllSceneGroupIDs()\n");
                status = sceneGroupManager.GetAllSceneGroupIDs();
                waitForReply = true;
            } else if (cmd == "61") {
                String id = NextTok(line);
                printf("\nInvoking GetSceneGroupName(%s)\n", id.c_str());
                status = sceneGroupManager.GetSceneGroupName(id.c_str());
                waitForReply = true;
            } else if (cmd == "62") {
                String id = NextTok(line);
                String name = NextTok(line);
                printf("\nInvoking SetSceneGroupName(%s, %s)\n", id.c_str(), name.c_str());
                status = sceneGroupManager.SetSceneGroupName(id.c_str(), name.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "63") {
                LSF_ID_List scenes;
                scenes.push_back("abc");
                scenes.push_back("xyz");
                SceneGroup group(scenes);
                printf("\nInvoking CreateSceneGroup(%s)\n", group.c_str());
                status = sceneGroupManager.CreateSceneGroup(group);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "64") {
                String id = NextTok(line);
                LSF_ID_List scenes;
                scenes.push_back("123");
                scenes.push_back("456");
                SceneGroup group(scenes);
                printf("\nInvoking UpdateSceneGroup(%s, %s)\n", id.c_str(), group.c_str());
                status = sceneGroupManager.UpdateSceneGroup(id.c_str(), group);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "65") {
                String id = NextTok(line);
                printf("\nInvoking DeleteSceneGroup(%s)\n", id.c_str());
                status = sceneGroupManager.DeleteSceneGroup(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "66") {
                String id = NextTok(line);
                printf("\nInvoking GetSceneGroup(%s)\n", id.c_str());
                status = sceneGroupManager.GetSceneGroup(id.c_str());
                waitForReply = true;
#if 0
            } else if (cmd == "54") {
                ApplySceneGroup;
#endif
            } else if (cmd == "help") {
                PrintHelp();
            } else if (cmd == "exit") {
                break;
            } else {
//                /unrecognizedCommand = true;
            }

            if (unrecognizedCommand) {
                printf("\nInvalid Command\n");
                unrecognizedCommand = false;
            } else {
                if (status == CONTROLLER_CLIENT_OK) {
                    printf("\nSent command successfully\n");
                    if (waitForReply) {
                        while (!gotReply) ;
                    }
                    if (waitForSignal) {
                        while (!gotSignal) ;
                    }
                } else {
                    printf("\nCommand send failed\n");
                }
            }

        }
    }

    bus.Stop();
    bus.Join();
}
