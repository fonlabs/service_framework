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
#include <PresetManager.h>
#include <SceneManager.h>
#include <MasterSceneManager.h>
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

    void ConnectedToControllerServiceCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        LSFString id = controllerServiceDeviceID;
        LSFString name = controllerServiceName;
        printf("\n%s: controllerServiceDeviceID = %s controllerServiceName = %s\n", __FUNCTION__, id.c_str(), name.c_str());
        connectedToControllerService = true;
        gotReply = true;
        gotSignal = true;
    }

    void ConnectToControllerServiceFailedCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        LSFString id = controllerServiceDeviceID;
        LSFString name = controllerServiceName;
        printf("\n%s: controllerServiceDeviceID = %s controllerServiceName = %s\n", __FUNCTION__, id.c_str(), name.c_str());
        gotReply = true;
        gotSignal = true;
    }

    void DisconnectedFromControllerServiceCB(const LSFString& controllerServiceDeviceID, const LSFString& controllerServiceName) {
        LSFString id = controllerServiceDeviceID;
        LSFString name = controllerServiceName;
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

    void GetAllLampIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& lampIDs) {
        printf("\n%s(): responseCode = %s, listsize=%d", __FUNCTION__, LSFResponseCodeText(responseCode), lampIDs.size());
        if (responseCode == LSF_OK) {
            LSFStringList::const_iterator it = lampIDs.begin();
            uint8_t count = 1;
            for (; it != lampIDs.end(); ++it) {
                printf("\n(%d)%s", count, (*it).c_str());
                count++;
            }
            printf("\n");
        }
        gotReply = true;
    }

    void GetLampSupportedLanguagesReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFStringList& supportedLanguages) {
        printf("\n%s(): responseCode = %s, listsize=%d", __FUNCTION__, LSFResponseCodeText(responseCode), supportedLanguages.size());
        if (responseCode == LSF_OK) {
            LSFStringList::const_iterator it = supportedLanguages.begin();
            uint8_t count = 1;
            for (; it != supportedLanguages.end(); ++it) {
                printf("\n(%d)%s", count, (*it).c_str());
                count++;
            }
            printf("\n");
        }
        gotReply = true;
    }

    void GetLampManufacturerReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFString& language, const LSFString& manufacturer) {
        LSFString id = lampID;

        printf("\n%s: responseCode = %s lampID = %s language = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), language.c_str());

        if (responseCode == LSF_OK) {
            printf("\nmanufacturer = %s", manufacturer.c_str());
        }
        gotReply = true;
    }

    void GetLampNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFString& language, const LSFString& lampName) {
        LSFString id = lampID;

        printf("\n%s: responseCode = %s lampID = %s language = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), language.c_str());

        if (responseCode == LSF_OK) {
            printf("\nlampName = %s", lampName.c_str());
        }
        gotReply = true;
    }

    void SetLampNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LSFString& language) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s language = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), language.c_str());
        gotReply = true;
        if (responseCode != ER_OK) {
            gotSignal = true;
        }
    }

    void LampsNameChangedCB(const LSFStringList& lampIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = lampIDs.begin();
        for (; it != lampIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void GetLampDetailsReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampDetails& lampDetails) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf("\n%s: lampDetails = %s", __FUNCTION__, lampDetails.c_str());
        }
        gotReply = true;
    }

    void GetLampParametersReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampParameters& lampParameters) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf("\n%s: parameters = %s", __FUNCTION__, lampParameters.c_str());
        }
        gotReply = true;
    }

    void GetLampParametersBrightnessLumensFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& value) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" value = %d", value);
        }
        gotReply = true;
    }

    void GetLampParametersEnergyUsageMilliwattsFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& value) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" value = %d", value);
        }
        gotReply = true;
    }

    void GetLampStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampState& lampState) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf("\nstate=%s\n", lampState.c_str());
        }
        gotReply = true;
    }

    void GetLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const bool& onOff) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" onOff = %d", onOff);
        }
        gotReply = true;
    }

    void GetLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& hue) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" hue = %d", hue);
        }
        gotReply = true;
    }

    void GetLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& saturation) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" saturation = %d", saturation);
        }
        gotReply = true;
    }

    void GetLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& brightness) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" brightness = %d", brightness);
        }
        gotReply = true;
    }

    void GetLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& colorTemp) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        if (responseCode == LSF_OK) {
            printf(" colorTemp = %d", colorTemp);
        }
        gotReply = true;
    }

    void ResetLampStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void LampsStateChangedCB(const LSFStringList& lampIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = lampIDs.begin();
        for (; it != lampIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void TransitionLampStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (responseCode != LSF_OK) {
            gotSignal = true;
        }
    }

    void PulseLampWithStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
    }

    void StrobeLampWithStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
    }

    void CycleLampWithStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
    }

    void PulseLampWithPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
    }

    void StrobeLampWithPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
    }

    void CycleLampWithPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
    }

    void TransitionLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void TransitionLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void TransitionLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void TransitionLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void TransitionLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void GetLampFaultsReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampFaultCodeList& faultCodes) {
        LSFString id = lampID;
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

    void ClearLampFaultReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const LampFaultCode& faultCode) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s faultCode=0x%x", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), faultCode);
        gotReply = true;
    }

    void GetLampRemainingLifeReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& remainingLife) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s remainingLife=0x%x", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), remainingLife);
        gotReply = true;
    }

    void GetLampServiceVersionReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID, const uint32_t& lampServiceVersion) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s lampServiceVersion=0x%x", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), lampServiceVersion);
        gotReply = true;
    }

    void ResetLampStateOnOffFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ResetLampStateHueFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ResetLampStateSaturationFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ResetLampStateBrightnessFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ResetLampStateColorTempFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void TransitionLampStateToPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampID) {
        LSFString id = lampID;
        printf("\n%s: responseCode = %s lampID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }
};

class LampGroupManagerCallbackHandler : public LampGroupManagerCallback {
    void GetLampGroupReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID, const LampGroup& lampGroup) {
        printf("\n%s(): responseCode = %s, lampGroupID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), lampGroupID.c_str());
        if (responseCode == LSF_OK) {
            printf("\nlampGroup=%s", lampGroup.c_str());
        }
        gotReply = true;
    }

    void GetAllLampGroupIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& lampGroupIDs) {
        printf("\n%s(): responseCode = %s, listsize=%d", __FUNCTION__, LSFResponseCodeText(responseCode), lampGroupIDs.size());
        if (responseCode == LSF_OK) {
            LSFStringList::const_iterator it = lampGroupIDs.begin();
            uint8_t count = 1;
            for (; it != lampGroupIDs.end(); ++it) {
                printf("\n(%d)%s", count, (*it).c_str());
                count++;
            }
            printf("\n");
        }
        gotReply = true;
    }

    void GetLampGroupNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID, const LSFString& language, const LSFString& lampGroupName) {
        LSFString id = lampGroupID;

        printf("\n%s: responseCode = %s lampGroupID = %s language = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), language.c_str());

        if (responseCode == LSF_OK) {
            printf("\nlampGroupName = %s", lampGroupName.c_str());
        }
        gotReply = true;
    }

    void SetLampGroupNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID, const LSFString& language) {
        LSFString id = lampGroupID;
        printf("\n%s: responseCode = %s lampGroupID = %s language = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), language.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void LampGroupsNameChangedCB(const LSFStringList& lampGroupIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = lampGroupIDs.begin();
        for (; it != lampGroupIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void CreateLampGroupReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        printf("\n%s: responseCode=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode));
        if (LSF_OK == responseCode) {
            printf("lampGroupID=%s\n", lampGroupID.c_str());
        } else {
            gotSignal = true;
        }
        gotReply = true;
    }

    void LampGroupsCreatedCB(const LSFStringList& lampGroupIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = lampGroupIDs.begin();
        for (; it != lampGroupIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void UpdateLampGroupReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        printf("\n%s(): responseCode = %s, lampGroupID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), lampGroupID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void LampGroupsUpdatedCB(const LSFStringList& lampGroupIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = lampGroupIDs.begin();
        for (; it != lampGroupIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void DeleteLampGroupReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        printf("\n%s(): responseCode = %s, lampGroupID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), lampGroupID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void LampGroupsDeletedCB(const LSFStringList& lampGroupIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = lampGroupIDs.begin();
        for (; it != lampGroupIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }
#if 0
    void GetLampGroupNameReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID, const LSFString& lampGroupName) {
        LSFString id = lampGroupID;

        printf("\n%s: responseCode = %s lampGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());

        if (responseCode == LSF_OK) {
            LSFString name = lampGroupName;
            printf("\nlampGroupName = %s", name.c_str());
        }
        gotReply = true;
    }

    void ResetLampGroupFieldStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        LSFString id = lampGroupID;
        LSFString name = stateFieldName;
        printf("\n%s: responseCode = %s lampGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), name.c_str());
        gotReply = true;
    }

    void TransitionLampGroupStateFieldReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        LSFString id = lampGroupID;
        LSFString name = stateFieldName;
        printf("\n%s: responseCode = %s lampGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), name.c_str());
        gotReply = true;
    }
#endif

    void PulseLampGroupWithStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        LSFString id = lampGroupID;
        printf("\n%s: responseCode = %s lampGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
    }

    void StrobeLampGroupWithStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        LSFString id = lampGroupID;
        printf("\n%s: responseCode = %s lampGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
    }

    void CycleLampGroupWithStateReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        LSFString id = lampGroupID;
        printf("\n%s: responseCode = %s lampGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
    }

    void PulseLampGroupWithPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        LSFString id = lampGroupID;
        printf("\n%s: responseCode = %s lampGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
    }

    void StrobeLampGroupWithPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        LSFString id = lampGroupID;
        printf("\n%s: responseCode = %s lampGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
    }

    void CycleLampGroupWithPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& lampGroupID) {
        LSFString id = lampGroupID;
        printf("\n%s: responseCode = %s lampGroupID = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str());
        gotReply = true;
    }
};

class PresetManagerCallbackHandler : public PresetManagerCallback {

    void GetPresetReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID, const LampState& preset) {
        printf("\n%s(): responseCode = %s, presetID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), presetID.c_str());
        if (responseCode == LSF_OK) {
            printf("\npreset=%s", preset.c_str());
        }
        gotReply = true;
    }

    void GetAllPresetIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& presetIDs) {
        printf("\n%s(): responseCode = %s, listsize=%d", __FUNCTION__, LSFResponseCodeText(responseCode), presetIDs.size());
        if (responseCode == LSF_OK) {
            LSFStringList::const_iterator it = presetIDs.begin();
            uint8_t count = 1;
            for (; it != presetIDs.end(); ++it) {
                printf("\n(%d)%s", count, (*it).c_str());
                count++;
            }
            printf("\n");
        }
        gotReply = true;
    }

    void GetPresetNameReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID, const LSFString& language, const LSFString& presetName) {
        LSFString id = presetID;

        printf("\n%s: responseCode = %s presetID = %s language = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), language.c_str());

        if (responseCode == LSF_OK) {
            printf("\npresetName = %s", presetName.c_str());
        }
        gotReply = true;
    }

    void SetPresetNameReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID, const LSFString& language) {
        LSFString id = presetID;
        printf("\n%s: responseCode = %s presetID = %s language = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), language.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void PresetsNameChangedCB(const LSFStringList& presetIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = presetIDs.begin();
        for (; it != presetIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void CreatePresetReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID) {
        printf("\n%s: responseCode=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode));
        if (LSF_OK == responseCode) {
            printf("presetID=%s\n", presetID.c_str());
        } else {
            gotSignal = true;
        }
        gotReply = true;
    }

    void PresetsCreatedCB(const LSFStringList& presetIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = presetIDs.begin();
        for (; it != presetIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void UpdatePresetReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID) {
        printf("\n%s(): responseCode = %s, presetID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), presetID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void PresetsUpdatedCB(const LSFStringList& presetIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = presetIDs.begin();
        for (; it != presetIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void DeletePresetReplyCB(const LSFResponseCode& responseCode, const LSFString& presetID) {
        printf("\n%s(): responseCode = %s, presetID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), presetID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void PresetsDeletedCB(const LSFStringList& presetIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = presetIDs.begin();
        for (; it != presetIDs.end(); ++it) {
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
    void GetSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID, const Scene& scene) {
        printf("\n%s(): responseCode = %s, sceneID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), sceneID.c_str());
        if (responseCode == LSF_OK) {
            printf("\nscene=%s", scene.c_str());
        }
        gotReply = true;
    }

    void GetAllSceneIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& sceneIDs) {
        printf("\n%s(): responseCode = %s, listsize=%d", __FUNCTION__, LSFResponseCodeText(responseCode), sceneIDs.size());
        if (responseCode == LSF_OK) {
            LSFStringList::const_iterator it = sceneIDs.begin();
            uint8_t count = 1;
            for (; it != sceneIDs.end(); ++it) {
                printf("\n(%d)%s", count, (*it).c_str());
                count++;
            }
            printf("\n");
        }
        gotReply = true;
    }

    void GetSceneNameReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID, const LSFString& language, const LSFString& sceneName) {
        LSFString id = sceneID;

        printf("\n%s: responseCode = %s sceneID = %s language = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), language.c_str());

        if (responseCode == LSF_OK) {
            printf("\nsceneName = %s", sceneName.c_str());
        }
        gotReply = true;
    }

    void SetSceneNameReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID, const LSFString& language) {
        LSFString id = sceneID;
        printf("\n%s: responseCode = %s sceneID = %s language = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), language.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ScenesNameChangedCB(const LSFStringList& sceneIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = sceneIDs.begin();
        for (; it != sceneIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void CreateSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID) {
        printf("\n%s: responseCode=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode));
        if (LSF_OK == responseCode) {
            printf("sceneID=%s\n", sceneID.c_str());
        } else {
            gotSignal = true;
        }
        gotReply = true;
    }

    void ScenesCreatedCB(const LSFStringList& sceneIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = sceneIDs.begin();
        for (; it != sceneIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void UpdateSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID) {
        printf("\n%s(): responseCode = %s, sceneID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), sceneID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ScenesUpdatedCB(const LSFStringList& sceneIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = sceneIDs.begin();
        for (; it != sceneIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void DeleteSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& sceneID) {
        printf("\n%s(): responseCode = %s, sceneID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), sceneID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void ScenesDeletedCB(const LSFStringList& sceneIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = sceneIDs.begin();
        for (; it != sceneIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }
};

class MasterSceneManagerCallbackHandler : public MasterSceneManagerCallback {
    void GetMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID, const MasterScene& masterScene) {
        printf("\n%s(): responseCode = %s, masterSceneID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), masterSceneID.c_str());
        if (responseCode == LSF_OK) {
            printf("\nmasterScene=%s", masterScene.c_str());
        }
        gotReply = true;
    }

    void GetAllMasterSceneIDsReplyCB(const LSFResponseCode& responseCode, const LSFStringList& masterSceneIDs) {
        printf("\n%s(): responseCode = %s, listsize=%d", __FUNCTION__, LSFResponseCodeText(responseCode), masterSceneIDs.size());
        if (responseCode == LSF_OK) {
            LSFStringList::const_iterator it = masterSceneIDs.begin();
            uint8_t count = 1;
            for (; it != masterSceneIDs.end(); ++it) {
                printf("\n(%d)%s", count, (*it).c_str());
                count++;
            }
            printf("\n");
        }
        gotReply = true;
    }

    void GetMasterSceneNameReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID, const LSFString& language, const LSFString& masterSceneName) {
        LSFString id = masterSceneID;

        printf("\n%s: responseCode = %s masterSceneID = %s language = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), language.c_str());

        if (responseCode == LSF_OK) {
            printf("\nmasterSceneName = %s", masterSceneName.c_str());
        }
        gotReply = true;
    }

    void SetMasterSceneNameReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID, const LSFString& language) {
        LSFString id = masterSceneID;
        printf("\n%s: responseCode = %s masterSceneID = %s language = %s", __FUNCTION__, LSFResponseCodeText(responseCode), id.c_str(), language.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void MasterScenesNameChangedCB(const LSFStringList& masterSceneIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = masterSceneIDs.begin();
        for (; it != masterSceneIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void CreateMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID) {
        printf("\n%s: responseCode=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode));
        if (LSF_OK == responseCode) {
            printf("masterSceneID=%s\n", masterSceneID.c_str());
        } else {
            gotSignal = true;
        }
        gotReply = true;
    }

    void MasterScenesCreatedCB(const LSFStringList& masterSceneIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = masterSceneIDs.begin();
        for (; it != masterSceneIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void UpdateMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID) {
        printf("\n%s(): responseCode = %s, masterSceneID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), masterSceneID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void MasterScenesUpdatedCB(const LSFStringList& masterSceneIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = masterSceneIDs.begin();
        for (; it != masterSceneIDs.end(); ++it) {
            printf("\n%s", (*it).c_str());
        }
        printf("\n");
        gotSignal = true;
    }

    void DeleteMasterSceneReplyCB(const LSFResponseCode& responseCode, const LSFString& masterSceneID) {
        printf("\n%s(): responseCode = %s, masterSceneID=%s\n", __FUNCTION__, LSFResponseCodeText(responseCode), masterSceneID.c_str());
        gotReply = true;
        if (LSF_OK != responseCode) {
            gotSignal = true;
        }
    }

    void MasterScenesDeletedCB(const LSFStringList& masterSceneIDs) {
        printf("\n%s", __FUNCTION__);
        LSFStringList::const_iterator it = masterSceneIDs.begin();
        for (; it != masterSceneIDs.end(); ++it) {
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
    printf("(4):   GetLampManufacturer\n");
    printf("(5):   GetLampSupportedLanguages\n");
    printf("(6):   GetLampName\n");
    printf("(7):   SetLampName\n");
    printf("(8):   GetLampState\n");
    printf("(9):   GetLampStateOnOffField\n");
    printf("(10):  GetLampStateHueField\n");
    printf("(11):  GetLampStateSaturationField\n");
    printf("(12):  GetLampStateBrightnessField\n");
    printf("(13):  GetLampStateColorTempField\n");
    printf("(14):  ResetLampState\n");
    printf("(15):  ResetLampStateOnOffField\n");
    printf("(16):  ResetLampStateHueField\n");
    printf("(17):  ResetLampStateSaturationField\n");
    printf("(18):  ResetLampStateBrightnessField\n");
    printf("(19):  ResetLampStateColorTempField\n");
    printf("(20):  TransitionLampState\n");
    printf("(21):  PulseLampWithState\n");
    printf("(22):  StrobeLampWithState\n");
    printf("(23):  CycleLampWithState\n");
    printf("(24):  PulseLampWithPreset\n");
    printf("(25):  StrobeLampWithPreset\n");
    printf("(26):  CycleLampWithPreset\n");
    printf("(27):  TransitionLampStateOnOffField\n");
    printf("(28):  TransitionLampStateHueField\n");
    printf("(29):  TransitionLampStateSaturationField\n");
    printf("(30):  TransitionLampStateBrightnessField\n");
    printf("(31):  TransitionLampStateColorTempField\n");
    printf("(32):  TransitionLampStateToPreset\n");
    printf("(33):  GetLampFaults\n");
    printf("(34):  ClearLampFault\n");
    printf("(35):  GetLampRemainingLife\n");
    printf("(36):  GetLampServiceVersion\n");
    printf("(37):  GetLampDetails\n");
    printf("(38):  GetLampParameters\n");
    printf("(39):  GetLampParametersEnergyUsageMilliwattsField\n");
    printf("(40):  GetLampParametersBrightnessLumensField\n");
    printf("(41):  GetAllLampGroupIDs\n");
    printf("(42):  GetLampGroupName\n");
    printf("(43):  SetLampGroupName\n");
    printf("(44):  CreateLampGroup\n");
    printf("(45):  UpdateLampGroup\n");
    printf("(46):  DeleteLampGroup\n");
    printf("(47):  GetLampGroup\n");
    printf("(48):  ResetLampGroupState\n");
    printf("(49):  ResetLampGroupStateField\n");
    printf("(50):  TransitionLampGroupState\n");
    printf("(51):  PulseLampGroupWithState\n");
    printf("(52):  StrobeLampGroupWithState\n");
    printf("(53):  CycleLampGroupWithState\n");
    printf("(54):  PulseLampGroupWithPreset\n");
    printf("(55):  StrobeLampGroupWithPreset\n");
    printf("(56):  CycleLampGroupWithPreset\n");
    printf("(57):  TransitionLampGroupStateToPreset\n");
    printf("(58):  TransitionLampGroupStateField\n");
    printf("(59):  GetDefaultLampState\n");
    printf("(60):  SetDefaultLampState\n");
    printf("(61):  GetAllPresetIDs\n");
    printf("(62):  GetPresetName\n");
    printf("(63):  SetPresetName\n");
    printf("(64):  CreatePreset\n");
    printf("(65):  UpdatePreset\n");
    printf("(66):  DeletePreset\n");
    printf("(67):  GetPreset\n");
    printf("(68):  GetAllSceneIDs\n");
    printf("(69):  GetSceneName\n");
    printf("(70):  SetSceneName\n");
    printf("(71):  CreateScene\n");
    printf("(72):  UpdateScene\n");
    printf("(73):  DeleteScene\n");
    printf("(74):  GetScene\n");
    printf("(75):  ApplyScene\n");
    printf("(76):  GetAllMasterSceneIDs\n");
    printf("(77):  GetMasterSceneName\n");
    printf("(78):  SetMasterSceneName\n");
    printf("(79):  CreateMasterScene\n");
    printf("(80):  UpdateMasterScene\n");
    printf("(81):  DeleteMasterScene\n");
    printf("(82):  GetMasterScene\n");
    printf("(83):  ApplyMasterScene\n");
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
    PresetManagerCallbackHandler presetManagerCBHandler;
    SceneManagerCallbackHandler sceneManagerCBHandler;
    MasterSceneManagerCallbackHandler masterSceneManagerCBHandler;

    ControllerClient client(bus, controllerClientCBHandler);
    ControllerServiceManager controllerServiceManager(client, controllerServiceManagerCBHandler);
    LampManager lampManager(client, lampManagerCBHandler);
    LampGroupManager lampGroupManager(client, lampGroupManagerCBHandler);
    PresetManager presetManager(client, presetManagerCBHandler);
    SceneManager sceneManager(client, sceneManagerCBHandler);
    MasterSceneManager masterSceneManager(client, masterSceneManagerCBHandler);

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
                printf("\nInvoking GetLampManufacturer(%s)\n", id.c_str());
                status = lampManager.GetLampManufacturer(id.c_str());
                waitForReply = true;
            } else if (cmd == "5") {
                String id = NextTok(line);
                printf("\nInvoking GetLampSupportedLanguages(%s)\n", id.c_str());
                lampManager.GetLampSupportedLanguages(id.c_str());
                waitForReply = true;
            } else if (cmd == "6") {
                String id = NextTok(line);
                printf("\nInvoking GetLampName(%s)\n", id.c_str());
                status = lampManager.GetLampName(id.c_str(), "en");
                waitForReply = true;
            } else if (cmd == "7") {
                String id = NextTok(line);
                String name = NextTok(line);
                printf("\nInvoking SetLampName(%s, %s)\n", id.c_str(), name.c_str());
                lampManager.SetLampName(id.c_str(), name.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "8") {
                String id = NextTok(line);
                printf("\nInvoking GetLampState(%s)\n", id.c_str());
                status = lampManager.GetLampState(id.c_str());
                waitForReply = true;
            } else if (cmd == "9") {
                String id = NextTok(line);
                printf("\nInvoking GetLampStateOnOffField(%s)\n", id.c_str());
                lampManager.GetLampStateOnOffField(id.c_str());
                waitForReply = true;
            } else if (cmd == "10") {
                String id = NextTok(line);
                printf("\nInvoking GetLampStateHueField(%s)\n", id.c_str());
                lampManager.GetLampStateHueField(id.c_str());
                waitForReply = true;
            } else if (cmd == "11") {
                String id = NextTok(line);
                printf("\nInvoking GetLampStateSaturationField(%s)\n", id.c_str());
                lampManager.GetLampStateSaturationField(id.c_str());
                waitForReply = true;
            } else if (cmd == "12") {
                String id = NextTok(line);
                printf("\nInvoking GetLampStateBrightnessField(%s)\n", id.c_str());
                lampManager.GetLampStateBrightnessField(id.c_str());
                waitForReply = true;
            } else if (cmd == "13") {
                String id = NextTok(line);
                printf("\nInvoking GetLampStateColorTempField(%s)\n", id.c_str());
                lampManager.GetLampStateColorTempField(id.c_str());
                waitForReply = true;
            } else if (cmd == "14") {
                String id = NextTok(line);
                printf("\nInvoking ResetLampState(%s)\n", id.c_str());
                lampManager.ResetLampState(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "15") {
                String id = NextTok(line);
                printf("\nInvoking ResetLampStateOnOffField(%s)\n", id.c_str());
                lampManager.ResetLampStateOnOffField(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "16") {
                String id = NextTok(line);
                printf("\nInvoking ResetLampStateHueField(%s)\n", id.c_str());
                lampManager.ResetLampStateHueField(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "17") {
                String id = NextTok(line);
                printf("\nInvoking ResetLampStateSaturationField(%s)\n", id.c_str());
                lampManager.ResetLampStateSaturationField(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "18") {
                String id = NextTok(line);
                printf("\nInvoking ResetLampStateBrightnessField(%s)\n", id.c_str());
                lampManager.ResetLampStateBrightnessField(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "19") {
                String id = NextTok(line);
                printf("\nInvoking ResetLampStateColorTempField(%s)\n", id.c_str());
                lampManager.ResetLampStateColorTempField(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "20") {
                String id = NextTok(line);
                LampState state(false, 7, 7, 7, 7);
                printf("\nInvoking TransitionLampState(%s)\n", id.c_str());
                lampManager.TransitionLampState(id.c_str(), state, 5);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "21") {
                String id = NextTok(line);
                LampState fromState(false, 7, 7, 7, 7);
                LampState toState(false, 5, 5, 5, 5);
                printf("\nInvoking PulseLampWithState(%s)\n", id.c_str());
                lampManager.PulseLampWithState(id.c_str(), fromState, toState, 100, 10, 20);
                waitForReply = true;
            } else if (cmd == "22") {
                String id = NextTok(line);
                LampState fromState(false, 7, 7, 7, 7);
                LampState toState(false, 5, 5, 5, 5);
                printf("\nInvoking StrobeLampWithState(%s)\n", id.c_str());
                lampManager.StrobeLampWithState(id.c_str(), fromState, toState, 100, 20);
                waitForReply = true;
            } else if (cmd == "23") {
                String id = NextTok(line);
                LampState stateA(false, 7, 7, 7, 7);
                LampState stateB(false, 9, 9, 9, 9);
                printf("\nInvoking CycleLampWithState(%s)\n", id.c_str());
                lampManager.CycleLampWithState(id.c_str(), stateA, stateB, 100, 80, 20);
                waitForReply = true;
            } else if (cmd == "24") {
                String id = NextTok(line);
                String fromState = NextTok(line);
                String toState = NextTok(line);
                printf("\nInvoking PulseLampWithPreset(%s)\n", id.c_str());
                lampManager.PulseLampWithPreset(id.c_str(), LSFString(fromState.c_str()), LSFString(toState.c_str()), 100, 10, 20);
                waitForReply = true;
            } else if (cmd == "25") {
                String id = NextTok(line);
                String fromState = NextTok(line);
                String toState = NextTok(line);
                printf("\nInvoking StrobeLampWithPreset(%s)\n", id.c_str());
                lampManager.StrobeLampWithPreset(id.c_str(), LSFString(fromState.c_str()), LSFString(toState.c_str()), 100, 20);
                waitForReply = true;
            } else if (cmd == "26") {
                String id = NextTok(line);
                String stateA = NextTok(line);
                String stateB = NextTok(line);
                printf("\nInvoking CycleLampWithPreset(%s)\n", id.c_str());
                lampManager.CycleLampWithPreset(id.c_str(), LSFString(stateA.c_str()), LSFString(stateB.c_str()), 100, 80, 20);
                waitForReply = true;
            } else if (cmd == "27") {
                String id = NextTok(line);
                printf("\nInvoking TransitionLampStateOnOffField(%s)\n", id.c_str());
                lampManager.TransitionLampStateOnOffField(id.c_str(), false);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "28") {
                String id = NextTok(line);
                printf("\nInvoking TransitionLampStateHueField(%s)\n", id.c_str());
                lampManager.TransitionLampStateHueField(id.c_str(), 10, 100);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "29") {
                String id = NextTok(line);
                printf("\nInvoking TransitionLampStateSaturationField(%s)\n", id.c_str());
                lampManager.TransitionLampStateSaturationField(id.c_str(), 20);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "30") {
                String id = NextTok(line);
                printf("\nInvoking TransitionLampStateBrightnessField(%s)\n", id.c_str());
                lampManager.TransitionLampStateBrightnessField(id.c_str(), 30, 100);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "31") {
                String id = NextTok(line);
                printf("\nInvoking TransitionLampStateColorTempField(%s)\n", id.c_str());
                lampManager.TransitionLampStateColorTempField(id.c_str(), 50);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "32") {
                String id = NextTok(line);
                String presetID = NextTok(line);
                printf("\nInvoking TransitionLampStateToPreset(%s, %s)\n", id.c_str(), presetID.c_str());
                lampManager.TransitionLampStateToPreset(id.c_str(), presetID.c_str(), 25);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "33") {
                String id = NextTok(line);
                printf("\nInvoking GetLampFaults(%s)\n", id.c_str());
                status = lampManager.GetLampFaults(id.c_str());
                waitForReply = true;
            } else if (cmd == "34") {
                String id = NextTok(line);
                LampFaultCode code = qcc::StringToU32(NextTok(line));
                printf("\nInvoking ClearLampFault(%s, %d)\n", id.c_str(), code);
                status = lampManager.ClearLampFault(id.c_str(), code);
                waitForReply = true;
            } else if (cmd == "35") {
                String id = NextTok(line);
                printf("\nInvoking GetLampRemainingLife(%s)\n", id.c_str());
                status = lampManager.GetLampRemainingLife(id.c_str());
                waitForReply = true;
            } else if (cmd == "36") {
                String id = NextTok(line);
                printf("\nInvoking GetLampServiceVersion(%s)\n", id.c_str());
                status = lampManager.GetLampServiceVersion(id.c_str());
                waitForReply = true;
            } else if (cmd == "37") {
                String id = NextTok(line);
                printf("\nInvoking GetLampDetails(%s)\n", id.c_str());
                status = lampManager.GetLampDetails(id.c_str());
                waitForReply = true;
            } else if (cmd == "38") {
                String id = NextTok(line);
                status = lampManager.GetLampParameters(id.c_str());
                printf("\nInvoking GetLampParameters(%s)\n", id.c_str());
                waitForReply = true;
            } else if (cmd == "39") {
                String id = NextTok(line);
                printf("\nInvoking GetLampParametersEnergyUsageMilliwattsField(%s)\n", id.c_str());
                lampManager.GetLampParametersEnergyUsageMilliwattsField(id.c_str());
                waitForReply = true;
            } else if (cmd == "40") {
                String id = NextTok(line);
                printf("\nInvoking GetLampParametersBrightnessLumensField(%s)\n", id.c_str());
                lampManager.GetLampParametersBrightnessLumensField(id.c_str());
                waitForReply = true;
            } else if (cmd == "41") {
                printf("\nInvoking GetAllLampGroupIDs()\n");
                status = lampGroupManager.GetAllLampGroupIDs();
                waitForReply = true;
            } else if (cmd == "42") {
                String id = NextTok(line);
                printf("\nInvoking GetLampGroupName(%s)\n", id.c_str());
                status = lampGroupManager.GetLampGroupName(id.c_str());
                waitForReply = true;
            } else if (cmd == "43") {
                String id = NextTok(line);
                String name = NextTok(line);
                printf("\nInvoking SetLampGroupName(%s, %s)\n", id.c_str(), name.c_str());
                status = lampGroupManager.SetLampGroupName(id.c_str(), name.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "44") {
                LSFStringList lamps;
                lamps.push_back("abc");
                lamps.push_back("xyz");
                LampGroup group(lamps, lamps);
                printf("\nInvoking CreateLampGroup(%s)\n", group.c_str());
                status = lampGroupManager.CreateLampGroup(group);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "45") {
                String id = NextTok(line);
                LSFStringList lamps;
                lamps.push_back("123");
                lamps.push_back("456");
                LampGroup group(lamps, lamps);
                printf("\nInvoking UpdateLampGroup(%s, %s)\n", id.c_str(), group.c_str());
                status = lampGroupManager.UpdateLampGroup(id.c_str(), group);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "46") {
                String id = NextTok(line);
                printf("\nInvoking DeleteLampGroup(%s)\n", id.c_str());
                status = lampGroupManager.DeleteLampGroup(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "47") {
                String id = NextTok(line);
                printf("\nInvoking GetLampGroup(%s)\n", id.c_str());
                status = lampGroupManager.GetLampGroup(id.c_str());
                waitForReply = true;
#if 0
            } else if (cmd == "48") {
                ResetLampGroupState;
            } else if (cmd == "49") {
                String id = NextTok(line);
                String fieldname = NextTok(line);
                printf("\nInvoking ResetLampGroupFieldState(%s, %s)\n", id.c_str(), fieldname.c_str());
                lampGroupManager.ResetLampGroupFieldState(id.c_str(), fieldname.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "50") {
                TransitionLampGroupState;
#endif
            } else if (cmd == "51") {
                String id = NextTok(line);
                LampState fromState(false, 7, 7, 7, 7);
                LampState toState(false, 5, 5, 5, 5);
                printf("\nInvoking PulseLampGroupWithState(%s)\n", id.c_str());
                lampGroupManager.PulseLampGroupWithState(id.c_str(), fromState, toState, 100, 10, 20);
                waitForReply = true;
            } else if (cmd == "52") {
                String id = NextTok(line);
                LampState fromState(false, 7, 7, 7, 7);
                LampState toState(false, 5, 5, 5, 5);
                printf("\nInvoking StrobeLampGroupWithState(%s)\n", id.c_str());
                lampGroupManager.StrobeLampGroupWithState(id.c_str(), fromState, toState, 100, 20);
                waitForReply = true;
            } else if (cmd == "53") {
                String id = NextTok(line);
                LampState stateA(false, 7, 7, 7, 7);
                LampState stateB(false, 9, 9, 9, 9);
                printf("\nInvoking CycleLampGroupWithState(%s)\n", id.c_str());
                lampGroupManager.CycleLampGroupWithState(id.c_str(), stateA, stateB, 100, 80, 20);
                waitForReply = true;
            } else if (cmd == "54") {
                String id = NextTok(line);
                String fromState = NextTok(line);
                String toState = NextTok(line);
                printf("\nInvoking PulseLampGroupWithPreset(%s)\n", id.c_str());
                lampGroupManager.PulseLampGroupWithPreset(id.c_str(), LSFString(fromState.c_str()), LSFString(toState.c_str()), 100, 10, 20);
                waitForReply = true;
            } else if (cmd == "55") {
                String id = NextTok(line);
                String fromState = NextTok(line);
                String toState = NextTok(line);
                printf("\nInvoking StrobeLampGroupWithPreset(%s)\n", id.c_str());
                lampGroupManager.StrobeLampGroupWithPreset(id.c_str(), LSFString(fromState.c_str()), LSFString(toState.c_str()), 100, 20);
                waitForReply = true;
            } else if (cmd == "56") {
                String id = NextTok(line);
                String stateA = NextTok(line);
                String stateB = NextTok(line);
                printf("\nInvoking CycleLampGroupWithPreset(%s)\n", id.c_str());
                lampGroupManager.CycleLampGroupWithPreset(id.c_str(), LSFString(stateA.c_str()), LSFString(stateB.c_str()), 100, 80, 20);
                waitForReply = true;
#if 0
            } else if (cmd == "57") {
                TransitionLampGroupStateToPreset;
            } else if (cmd == "58") {
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
            } else if (cmd == "59") {
                printf("\nInvoking GetDefaultLampState()\n");
                status = presetManager.GetDefaultLampState();
                waitForReply = true;
            } else if (cmd == "60") {
                printf("\nInvoking SetDefaultLampState()\n");
                LampState state(true, 3, 3, 3, 3);
                status = presetManager.SetDefaultLampState(state);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "61") {
                printf("\nInvoking GetAllPresetIDs()\n");
                status = presetManager.GetAllPresetIDs();
                waitForReply = true;
            } else if (cmd == "62") {
                String id = NextTok(line);
                printf("\nInvoking GetPresetName(%s)\n", id.c_str());
                status = presetManager.GetPresetName(id.c_str());
                waitForReply = true;
            } else if (cmd == "63") {
                String id = NextTok(line);
                String name = NextTok(line);
                printf("\nInvoking SetPresetName(%s, %s)\n", id.c_str(), name.c_str());
                status = presetManager.SetPresetName(id.c_str(), name.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "64") {
                LampState state(true, 5, 5, 5, 5);
                printf("\nInvoking CreatePreset(%s)\n", state.c_str());
                status = presetManager.CreatePreset(state);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "65") {
                String id = NextTok(line);
                LampState state(true, 6, 6, 6, 6);
                printf("\nInvoking UpdatePreset(%s, %s)\n", id.c_str(), state.c_str());
                status = presetManager.UpdatePreset(id.c_str(), state);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "66") {
                String id = NextTok(line);
                printf("\nInvoking DeletePreset(%s)\n", id.c_str());
                status = presetManager.DeletePreset(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "67") {
                String id = NextTok(line);
                printf("\nInvoking GetPreset(%s)\n", id.c_str());
                status = presetManager.GetPreset(id.c_str());
                waitForReply = true;
            } else if (cmd == "68") {
                printf("\nInvoking GetAllSceneIDs()\n");
                status = sceneManager.GetAllSceneIDs();
                waitForReply = true;
            } else if (cmd == "69") {
                String id = NextTok(line);
                printf("\nInvoking GetSceneName(%s)\n", id.c_str());
                status = sceneManager.GetSceneName(id.c_str());
                waitForReply = true;
            } else if (cmd == "70") {
                String id = NextTok(line);
                String name = NextTok(line);
                printf("\nInvoking SetSceneName(%s, %s)\n", id.c_str(), name.c_str());
                status = sceneManager.SetSceneName(id.c_str(), name.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "71") {
                LSFStringList lampList1, lampList2;
                lampList1.push_back("abc");
                lampList2.push_back("abc");
                LSFStringList lampGroupList1, lampGroupList2;
                lampGroupList1.push_back("xyz");
                lampGroupList2.push_back("xyz");
                LampState state(false, 6, 6, 6, 6);
                LampState toState(false, 5, 5, 5, 5);
                LSFString presetID = LSFString("123");
                LSFString toPresetID = LSFString("xyz");
                uint32_t transPeriod = 35;
                TransitionLampsLampGroupsToState transitionToStateComponent(lampList1, lampGroupList1, state, transPeriod);
                TransitionLampsLampGroupsToPreset transitionToPresetComponent(lampList2, lampGroupList2, presetID, transPeriod);
                uint32_t period = 100;
                uint32_t duration = 20;
                uint32_t numPulses = 25;
                PulseLampsLampGroupsWithState pulseWithStateComponent(lampList2, lampGroupList2, state, toState, period, duration, numPulses);
                PulseLampsLampGroupsWithPreset pulseWithPresetComponent(lampList2, lampGroupList2, presetID, toPresetID, period, duration, numPulses);

                uint32_t numStrobes = 25;
                StrobeLampsLampGroupsWithState strobeWithStateComponent(lampList2, lampGroupList2, state, toState, period, numStrobes);
                StrobeLampsLampGroupsWithPreset strobeWithPresetComponent(lampList2, lampGroupList2, presetID, toPresetID, period, numStrobes);

                uint32_t numCycles = 10;
                CycleLampsLampGroupsWithState cycleWithStateComponent(lampList2, lampGroupList2, state, toState, period, duration, numCycles);
                CycleLampsLampGroupsWithPreset cycleWithPresetComponent(lampList2, lampGroupList2, presetID, toPresetID, period, duration, numCycles);

                TransitionLampsLampGroupsToStateList transitionToStateList;
                transitionToStateList.push_back(transitionToStateComponent);

                TransitionLampsLampGroupsToPresetList transitionToPresetList;
                transitionToPresetList.push_back(transitionToPresetComponent);

                PulseLampsLampGroupsWithStateList pulseWithStateList;
                pulseWithStateList.push_back(pulseWithStateComponent);

                PulseLampsLampGroupsWithPresetList pulseWithPresetList;
                pulseWithPresetList.push_back(pulseWithPresetComponent);

                StrobeLampsLampGroupsWithStateList strobeWithStateList;
                strobeWithStateList.push_back(strobeWithStateComponent);

                StrobeLampsLampGroupsWithPresetList strobeWithPresetList;
                strobeWithPresetList.push_back(strobeWithPresetComponent);

                CycleLampsLampGroupsWithStateList cycleWithStateList;
                cycleWithStateList.push_back(cycleWithStateComponent);

                CycleLampsLampGroupsWithPresetList cycleWithPresetList;
                cycleWithPresetList.push_back(cycleWithPresetComponent);

                Scene scene(transitionToStateList, transitionToPresetList, pulseWithStateList, pulseWithPresetList, strobeWithStateList, strobeWithPresetList, cycleWithStateList, cycleWithPresetList);

                printf("\nInvoking CreateScene(%s)\n", scene.c_str());
                status = sceneManager.CreateScene(scene);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "72") {
                String id = NextTok(line);

                LSFStringList lampList;
                lampList.push_back("ABC");
                LSFStringList lampGroupList;
                lampGroupList.push_back("XYZ");
                LampState state(false, 7, 7, 7, 7);
                LampState toState(false, 5, 5, 5, 5);
                LSFString presetID = LSFString("123");
                LSFString toPresetID = LSFString("xyz");
                TransitionLampsLampGroupsToState transitionToStateComponent(lampList, lampGroupList, state);
                TransitionLampsLampGroupsToPreset transitionToPresetComponent(lampList, lampGroupList, presetID);
                uint32_t period = 100;
                uint32_t duration = 20;
                uint32_t numPulses = 25;
                PulseLampsLampGroupsWithState pulseWithStateComponent(lampList, lampGroupList, state, toState, period, duration, numPulses);
                PulseLampsLampGroupsWithPreset pulseWithPresetComponent(lampList, lampGroupList, presetID, toPresetID, period, duration, numPulses);

                uint32_t numStrobes = 25;
                StrobeLampsLampGroupsWithState strobeWithStateComponent(lampList, lampGroupList, state, toState, period, numStrobes);
                StrobeLampsLampGroupsWithPreset strobeWithPresetComponent(lampList, lampGroupList, presetID, toPresetID, period, numStrobes);

                uint32_t numCycles = 10;
                CycleLampsLampGroupsWithState cycleWithStateComponent(lampList, lampGroupList, state, toState, period, duration, numCycles);
                CycleLampsLampGroupsWithPreset cycleWithPresetComponent(lampList, lampGroupList, presetID, toPresetID, period, duration, numCycles);

                TransitionLampsLampGroupsToStateList transitionToStateList;
                transitionToStateList.push_back(transitionToStateComponent);

                TransitionLampsLampGroupsToPresetList transitionToPresetList;
                transitionToPresetList.push_back(transitionToPresetComponent);

                PulseLampsLampGroupsWithStateList pulseWithStateList;
                pulseWithStateList.push_back(pulseWithStateComponent);

                PulseLampsLampGroupsWithPresetList pulseWithPresetList;
                pulseWithPresetList.push_back(pulseWithPresetComponent);

                StrobeLampsLampGroupsWithStateList strobeWithStateList;
                strobeWithStateList.push_back(strobeWithStateComponent);

                StrobeLampsLampGroupsWithPresetList strobeWithPresetList;
                strobeWithPresetList.push_back(strobeWithPresetComponent);

                CycleLampsLampGroupsWithStateList cycleWithStateList;
                cycleWithStateList.push_back(cycleWithStateComponent);

                CycleLampsLampGroupsWithPresetList cycleWithPresetList;
                cycleWithPresetList.push_back(cycleWithPresetComponent);

                Scene scene(transitionToStateList, transitionToPresetList, pulseWithStateList, pulseWithPresetList, strobeWithStateList, strobeWithPresetList, cycleWithStateList, cycleWithPresetList);

                printf("\nInvoking UpdateScene(%s, %s)\n", id.c_str(), scene.c_str());
                status = sceneManager.UpdateScene(id.c_str(), scene);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "73") {
                String id = NextTok(line);
                printf("\nInvoking DeleteScene(%s)\n", id.c_str());
                status = sceneManager.DeleteScene(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "74") {
                String id = NextTok(line);
                printf("\nInvoking GetScene(%s)\n", id.c_str());
                status = sceneManager.GetScene(id.c_str());
                waitForReply = true;
#if 0
            } else if (cmd == "75") {
                ApplyScene;
#endif
            } else if (cmd == "76") {
                printf("\nInvoking GetAllMasterSceneIDs()\n");
                status = masterSceneManager.GetAllMasterSceneIDs();
                waitForReply = true;
            } else if (cmd == "77") {
                String id = NextTok(line);
                printf("\nInvoking GetMasterSceneName(%s)\n", id.c_str());
                status = masterSceneManager.GetMasterSceneName(id.c_str());
                waitForReply = true;
            } else if (cmd == "78") {
                String id = NextTok(line);
                String name = NextTok(line);
                printf("\nInvoking SetMasterSceneName(%s, %s)\n", id.c_str(), name.c_str());
                status = masterSceneManager.SetMasterSceneName(id.c_str(), name.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "79") {
                LSFStringList scenes;
                scenes.push_back("abc");
                scenes.push_back("xyz");
                MasterScene group(scenes);
                printf("\nInvoking CreateMasterScene(%s)\n", group.c_str());
                status = masterSceneManager.CreateMasterScene(group);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "80") {
                String id = NextTok(line);
                LSFStringList scenes;
                scenes.push_back("123");
                scenes.push_back("456");
                MasterScene group(scenes);
                printf("\nInvoking UpdateMasterScene(%s, %s)\n", id.c_str(), group.c_str());
                status = masterSceneManager.UpdateMasterScene(id.c_str(), group);
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "81") {
                String id = NextTok(line);
                printf("\nInvoking DeleteMasterScene(%s)\n", id.c_str());
                status = masterSceneManager.DeleteMasterScene(id.c_str());
                waitForReply = true;
                waitForSignal = true;
            } else if (cmd == "82") {
                String id = NextTok(line);
                printf("\nInvoking GetMasterScene(%s)\n", id.c_str());
                status = masterSceneManager.GetMasterScene(id.c_str());
                waitForReply = true;
#if 0
            } else if (cmd == "83") {
                ApplyMasterScene;
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
