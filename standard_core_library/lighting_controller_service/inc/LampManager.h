#ifndef _LAMP_MANAGER_H_
#define _LAMP_MANAGER_H_
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
#include <PresetManager.h>
#include <Mutex.h>
#include <LampClients.h>

#include <string>
#include <map>
#include <vector>

namespace lsf {

class LampManager : public Manager, public LampClientsCallback {
  public:

    LampManager(ControllerService& controllerSvc, PresetManager& presetMgr, const char* ifaceName);

    ~LampManager();

    /*
     * Start the Lamp Manager
     *
     * @param   None
     * @return  ER_OK if successful, error otherwise
     */
    QStatus Start(void);

    /*
     * Stop the Lamp Manager
     *
     * @param   None
     * @return  ER_OK if successful, error otherwise
     */
    QStatus Stop(void);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.GetAllLampIDs
     *
     * @param message   The params
     */
    void GetAllLampIDs(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.GetLampFaults
     *
     * @param message   The params
     */
    void GetLampFaults(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.ClearLampFault
     *
     * @param message   The params
     */
    void ClearLampFault(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.GetLampSupportedLanguages
     *
     * @param message   The params
     */
    void GetLampSupportedLanguages(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.GetLampName
     *
     * @param message   The params
     */
    void GetLampManufacturer(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.GetLampName
     *
     * @param message   The params
     */
    void GetLampName(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.SetLampName
     * @param message   The params
     */
    void SetLampName(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.GetLampDetails
     *
     * @param message   The params
     */
    void GetLampDetails(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.GetLampParameters
     *
     * @param message   The params
     */
    void GetLampParameters(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.GetLampParametersField
     *
     * @param message   The params
     */
    void GetLampParametersField(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.GetLampState
     *
     * @param message   The params
     */
    void GetLampState(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.GetLampStateField
     *
     * @param message   The params
     */
    void GetLampStateField(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.TransitionLampState
     *
     * @param message   The params
     */
    void TransitionLampState(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.TransitionLampStateToPreset
     *
     * @param message   The params
     */
    void TransitionLampStateToPreset(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.TransitionLampStateField
     *
     * @param message   The params
     */
    void TransitionLampStateField(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.ResetLampState
     *
     * @param message   The params
     */
    void ResetLampState(ajn::Message& message);

    /**
     * Process an AllJoyn call to org.allseen.LSF.ControllerService.ResetLampStateField
     *
     * @param message   The params
     */
    void ResetLampStateField(ajn::Message& message);





    virtual void TransitionLampStateReplyCB(ajn::Message& origMsg, LSFResponseCode rc);
    virtual void TransitionLampStateFieldReplyCB(ajn::Message& origMsg, const char* field, LSFResponseCode rc);

    virtual void GetLampStateReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode rc);
    virtual void GetLampStateFieldReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode rc);
    virtual void GetLampDetailsReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode rc);
    virtual void GetLampParametersReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode rc);
    virtual void GetLampParametersFieldReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode rc);
    virtual void GetLampNameReplyCB(ajn::Message& origMsg, const char* name, LSFResponseCode rc);
    virtual void GetLampManufacturerReplyCB(ajn::Message& origMsg, const char* manufacturer, LSFResponseCode rc);
    virtual void SetLampNameReplyCB(ajn::Message& origMsg, LSFResponseCode rc);

    virtual void GetLampFaultsReplyCB(ajn::Message& origMsg, const ajn::MsgArg& replyMsg, LSFResponseCode rc);
    virtual void ClearLampFaultReplyCB(ajn::Message& origMsg, LSFResponseCode rc, LampFaultCode code);

  private:

    class LampsAndState {
      public:
        LampsAndState(LSFStringList lampList, LampState lampState, uint32_t period) :
            lamps(lampList), state(lampState), transitionPeriod(period) { }

        LSFStringList lamps;
        LampState state;
        uint32_t transitionPeriod;
    };

    class LampsAndPreset {
      public:
        LampsAndPreset(LSFStringList lampList, LSFString presetID, uint32_t period) :
            lamps(lampList), presetID(presetID), transitionPeriod(period) { }

        LSFStringList lamps;
        LSFString presetID;
        uint32_t transitionPeriod;
    };

    class LampsAndStateField {
      public:
        LampsAndStateField(LSFStringList lampList, LSFString fieldName, ajn::MsgArg arg, uint32_t period) :
            lamps(lampList), stateFieldName(fieldName), stateFieldValue(arg), transitionPeriod(period) { }

        LSFStringList lamps;
        LSFString stateFieldName;
        ajn::MsgArg stateFieldValue;
        uint32_t transitionPeriod;
    };

    LSFResponseCode ResetLampStateInternal(ajn::Message& message, LSFStringList lamps);

    LSFResponseCode ResetLampStateFieldInternal(ajn::Message& message, LSFStringList lamps, LSFString stateFieldName);

    LSFResponseCode TransitionLampStateAndFieldInternal(ajn::Message& message, LampsAndState* stateComponent, LampsAndPreset* presetComponent, LampsAndStateField* stateFieldComponent);

    LampClients lampClients;
    PresetManager& presetManager;
    const char* interfaceName;

};

}

#endif
