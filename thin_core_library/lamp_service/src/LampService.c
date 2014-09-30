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

#include <aj_target.h>
#include <aj_link_timeout.h>
#include <aj_debug.h>
#include <alljoyn.h>
#include <aj_guid.h>
#include <aj_crypto.h>
#include <aj_nvram.h>
#include <aj_config.h>
#include <aj_bus.h>
#include <aj_msg.h>
#include <aj_version.h>
#include <aj_about.h>
#include <OEM_LS_Provisioning.h>

#ifdef ONBOARDING_SERVICE
#include <alljoyn/onboarding/OnboardingService.h>
#include <aj_wifi_ctrl.h>
#endif

#include <alljoyn/notification/NotificationProducer.h>
#include <alljoyn/config/ConfigService.h>
#include <alljoyn/services_common/ServicesCommon.h>
#include <alljoyn/services_common/PropertyStore.h>
#include <alljoyn/services_common/ServicesHandlers.h>

#include <LampService.h>
#include <OEM_LS_Code.h>
#include <LampState.h>
#include <LampAboutData.h>
#include <LampOnboarding.h>

/**
 * Per-module definition of the current module for debug logging.  Must be defined
 * prior to first inclusion of aj_debug.h
 */
#define AJ_MODULE LAMP_SERVICE

/**
 * Turn on per-module debug printing by setting this variable to non-zero value
 * (usually in debugger).
 */
#ifndef NDEBUG
uint8_t dbgLAMP_SERVICE = 1;
#endif

static const uint16_t LSF_ServicePort = 42;
static uint32_t ControllerSessionID = 0;
static uint8_t SendStateChanged = FALSE;

static const char LSF_Interface_Name[] = "org.allseen.LSF.LampService";
static const uint32_t LSF_Interface_Version = 1;
static const char* const LSF_Interface[] = {
    LSF_Interface_Name,
    "@Version>u",
    "@LampServiceVersion>u",
    "?ClearLampFault LampFaultCode<u LampResponseCode>u LampFaultCode>u",
    "@LampFaults>au",
    NULL
};

static const char LSF_Parameters_Interface_Name[] = "org.allseen.LSF.LampParameters";
static const uint32_t LSF_Parameters_Interface_Version = 1;
static const char* const LSF_Parameters_Interface[] = {
    LSF_Parameters_Interface_Name,
    "@Version>u",
    "@Energy_Usage_Milliwatts>u",
    "@Brightness_Lumens>u",
    NULL
};

static const char LSF_Details_Interface_Name[] = "org.allseen.LSF.LampDetails";
static const uint32_t LSF_Details_Interface_Version = 1;
static const char* const LSF_Details_Interface[] = {
    LSF_Details_Interface_Name,
    "@Version>u",
    "@Make>u",
    "@Model>u",
    "@Type>u",
    "@LampType>u",
    "@LampBaseType>u",
    "@LampBeamAngle>u",
    "@Dimmable>b",
    "@Color>b",
    "@VariableColorTemp>b",
    "@HasEffects>b",
    "@MinVoltage>u",
    "@MaxVoltage>u",
    "@Wattage>u",
    "@IncandescentEquivalent>u",
    "@MaxLumens>u",
    "@MinTemperature>u",
    "@MaxTemperature>u",
    "@ColorRenderingIndex>u",
    "@LampID>s",
    NULL
};

static const char LSF_State_Interface_Name[] = "org.allseen.LSF.LampState";
static const uint32_t LSF_State_Interface_Version = 1;
static const char* const LSF_State_Interface[] = {
    LSF_State_Interface_Name,
    "@Version>u",
    "?TransitionLampState Timestamp<t NewState<a{sv} TransitionPeriod<u LampResponseCode>u",
    "?ApplyPulseEffect FromState<a{sv} ToState<a{sv} period<u duration<u numPulses<u timestamp<t LampResponseCode>u",
    "!LampStateChanged LampID>s",
    "@OnOff=b",
    "@Hue=u",
    "@Saturation=u",
    "@ColorTemp=u",
    "@Brightness=u",
    NULL
};

static const AJ_InterfaceDescription LSF_Interfaces[] = {
    AJ_PropertiesIface,
    LSF_Interface,
    LSF_Parameters_Interface,
    LSF_Details_Interface,
    LSF_State_Interface,
    NULL
};

static AJ_Object LSF_AllJoynObjects[] = {
    { "/org/allseen/LSF/Lamp", LSF_Interfaces, AJ_OBJ_FLAG_ANNOUNCED },
    { NULL }
};

#define LSF_MAJOR_VERSION   0    /**< major version */
#define LSF_MINOR_VERSION   0    /**< minor version */
#define LSF_RELEASE_VERSION 1    /**< release version */
#define LSF_VERSION ((LSF_MAJOR_VERSION) << 24) | ((LSF_MINOR_VERSION) << 16) | (LSF_RELEASE_VERSION)

uint32_t LAMP_GetServiceVersion(void)
{
    return (uint32_t) LSF_VERSION;
}

#define LSF_PROP_IFACE 0
#define LSF_IFACE 1
#define LSF_IFACE_PARAMS 2
#define LSF_IFACE_DETAILS 3
#define LSF_IFACE_STATE 4

#define APP_SET_PROP        AJ_APP_MESSAGE_ID(0, LSF_PROP_IFACE, AJ_PROP_SET)
#define APP_GET_PROP        AJ_APP_MESSAGE_ID(0, LSF_PROP_IFACE, AJ_PROP_GET)
#define APP_GET_PROP_ALL    AJ_APP_MESSAGE_ID(0, LSF_PROP_IFACE, AJ_PROP_GET_ALL)

#define LSF_PROP_VERSION            AJ_APP_PROPERTY_ID(0, LSF_IFACE, 0)
#define LSF_PROP_LSF_VERSION        AJ_APP_PROPERTY_ID(0, LSF_IFACE, 1)
#define LSF_METHOD_CLEARLAMPFAULTS  AJ_APP_MESSAGE_ID(0, LSF_IFACE, 2)
#define LSF_PROP_FAULTS             AJ_APP_PROPERTY_ID(0, LSF_IFACE, 3)


// Run-time Parameters
#define LSF_PROP_PARAMS_VERSION    AJ_APP_PROPERTY_ID(0, LSF_IFACE_PARAMS, 0)
#define LSF_PROP_PARAMS_ENERGY_USAGE_MILLIWATTS AJ_APP_PROPERTY_ID(0, LSF_IFACE_PARAMS, 1)
#define LSF_PROP_PARAMS_BRIGHTNESS_LUMENS     AJ_APP_PROPERTY_ID(0, LSF_IFACE_PARAMS, 2)


// Compile-time Details
#define LSF_PROP_DETAILS_VERSION        AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 0)
#define LSF_PROP_DETAILS_MAKE           AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 1)
#define LSF_PROP_DETAILS_MODEL          AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 2)
#define LSF_PROP_DETAILS_DEV_TYPE       AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 3)
#define LSF_PROP_DETAILS_LAMP_TYPE      AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 4)
#define LSF_PROP_DETAILS_BASETYPE       AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 5)
#define LSF_PROP_DETAILS_BEAMANGLE      AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 6)
#define LSF_PROP_DETAILS_DIMMABLE       AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 7)
#define LSF_PROP_DETAILS_COLOR          AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 8)
#define LSF_PROP_DETAILS_VARCOLORTEMP   AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 9)
#define LSF_PROP_DETAILS_HASEFFECTS     AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 10)
#define LSF_PROP_DETAILS_MINVOLTAGE     AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 11)
#define LSF_PROP_DETAILS_MAXVOLTAGE     AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 12)
#define LSF_PROP_DETAILS_WATTAGE        AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 13)
#define LSF_PROP_DETAILS_INCANEQV       AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 14)
#define LSF_PROP_DETAILS_MAXLUMENS      AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 15)
#define LSF_PROP_DETAILS_MINTEMP        AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 16)
#define LSF_PROP_DETAILS_MAXTEMP        AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 17)
#define LSF_PROP_DETAILS_CRI            AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 18)
#define LSF_PROP_DETAILS_LAMPID         AJ_APP_PROPERTY_ID(0, LSF_IFACE_DETAILS, 19)

// Run-time Lamp State
#define LSF_PROP_STATE_VERSION          AJ_APP_PROPERTY_ID(0, LSF_IFACE_STATE, 0)
#define LSF_METHOD_STATE_SETSTATE       AJ_APP_MESSAGE_ID(0, LSF_IFACE_STATE, 1)
#define LSF_METHOD_APPLY_PULSE          AJ_APP_MESSAGE_ID(0, LSF_IFACE_STATE, 2)
#define LSF_SIGNAL_STATE_STATECHANGED   AJ_APP_MESSAGE_ID(0, LSF_IFACE_STATE, 3)
#define LSF_PROP_STATE_ONOFF    AJ_APP_PROPERTY_ID(0, LSF_IFACE_STATE, 4)
#define LSF_PROP_STATE_HUE      AJ_APP_PROPERTY_ID(0, LSF_IFACE_STATE, 5)
#define LSF_PROP_STATE_SAT      AJ_APP_PROPERTY_ID(0, LSF_IFACE_STATE, 6)
#define LSF_PROP_STATE_TEMP     AJ_APP_PROPERTY_ID(0, LSF_IFACE_STATE, 7)
#define LSF_PROP_STATE_BRIGHT   AJ_APP_PROPERTY_ID(0, LSF_IFACE_STATE, 8)

static uint32_t MyBusAuthPwdCB(uint8_t* buf, uint32_t bufLen)
{
    const char* myPwd = "000000";
    strncpy((char*) buf, myPwd, bufLen);
    return (uint32_t) strlen(myPwd);
}

static AJ_BusAttachment Bus;

static volatile uint8_t PendingFaultNotification = FALSE;
static volatile uint8_t PendingRestartRequest = FALSE;
static volatile uint8_t PendingFactoryResetRequest = FALSE;

void LAMP_SetFaults(void)
{
    PendingFaultNotification = TRUE;
}

void LAMP_ClearFaults(void)
{
    PendingFaultNotification = FALSE;
}

void LAMP_Restart(void)
{
    PendingRestartRequest = TRUE;
}

void LAMP_FactoryReset(void)
{
    PendingFactoryResetRequest = TRUE;
}

/*
 * We currently check for faults when one second has passed between messages.
 * This is because we can only have two AJ_Message's at a time due to memory
 * limitations.  Those two messages are already used in the message handlers
 * for (1) the incoming message and (2) the reply.
 */
static void CheckForFaults(void)
{
    static uint32_t FaultNotificationSerialNumber = 0;

    // if new faults have occured, send a notification
    if (PendingFaultNotification) {
        // turn ON notification
        AJNS_NotificationContent NotificationContent;
        AJNS_DictionaryEntry NotificationTexts;
        uint16_t messageType = AJNS_NOTIFICATION_MESSAGE_TYPE_WARNING;
        uint32_t ttl = AJNS_NOTIFICATION_TTL_MAX;
        const char* notif_text = OEM_LS_GetFaultsText();
        memset(&NotificationContent, 0, sizeof(AJNS_NotificationContent));

        if (notif_text != NULL) {
            NotificationTexts.key = "LSF_FAULTS";
            NotificationTexts.value = notif_text;

            NotificationContent.originalSenderName = AJ_GetUniqueName(&Bus);
            NotificationContent.numTexts = 1;
            NotificationContent.texts = &NotificationTexts;

            // if we clear this now, the Notification will be pulled on the next
            // pass through the event loop
            PendingFaultNotification = FALSE;
            AJNS_Producer_SendNotification(&Bus, &NotificationContent, messageType, ttl, &FaultNotificationSerialNumber);
        }
    }
}

/*
 * We do this on the side, the same as checking for faults.
 */
static void CheckForStateChanged(void)
{
    if (ControllerSessionID != 0 && SendStateChanged == TRUE) {
        AJ_InfoPrintf(("\n%s\n", __func__));
        AJ_Message sig_out;
        AJ_MarshalSignal(&Bus, &sig_out, LSF_SIGNAL_STATE_STATECHANGED, NULL, ControllerSessionID, 0, 0);
        AJ_MarshalArgs(&sig_out, "s", LAMP_GetID());
        AJ_DeliverMsg(&sig_out);
        AJ_CloseMsg(&sig_out);

        // no need to send this again.
        SendStateChanged = FALSE;
    }

    // what if SendStateChanged==TRUE and no session?
    // cancel?  or wait until a session is accepted?
}


#define CONNECT_TIMEOUT    (1000 * 1000)
#define UNMARSHAL_TIMEOUT  (1000 * 2)
#define DEFAULT_TIMEOUT    (1000 * 5)


void LAMP_RunService(void)
{
    LAMP_RunServiceWithCallback(DEFAULT_TIMEOUT, NULL);
}

#define MIN_ROUTER_VERSION 10

static AJSVC_ServiceStatus LAMP_HandleMessage(AJ_Message* msg, AJ_Status* status);

static AJ_Status ParseOptions(AJ_SessionOpts* opts, AJ_Message* msg)
{
    AJ_Arg array1, struct1;
    AJ_Status status = AJ_UnmarshalContainer(msg, &array1, AJ_ARG_ARRAY);

    do {
        char* field;
        char* sig;

        status = AJ_UnmarshalContainer(msg, &struct1, AJ_ARG_DICT_ENTRY);
        if (status != AJ_OK) {
            break;
        }

        status = AJ_UnmarshalArgs(msg, "s", &field);
        if (status != AJ_OK) {
            AJ_ErrPrintf(("AJ_UnmarshalArgs: %s\n", AJ_StatusText(status)));
            break;
        }

        // Process the field!
        status = AJ_UnmarshalVariant(msg, (const char**) &sig);
        if (status != AJ_OK) {
            AJ_ErrPrintf(("AJ_UnmarshalVariant: %s\n", AJ_StatusText(status)));
            break;
        }

        if (0 == strcmp(field, "traf")) {
            uint32_t traf;
            status = AJ_UnmarshalArgs(msg, "y", &traf);
            opts->traffic = traf;
        } else if (0 == strcmp(field, "multi")) {
            uint32_t multi;
            status = AJ_UnmarshalArgs(msg, "b", &multi);
            opts->isMultipoint = multi ? TRUE : FALSE;
        } else if (0 == strcmp(field, "prox")) {
            uint32_t prox;
            status = AJ_UnmarshalArgs(msg, "y", &prox);
            opts->proximity = prox;
        } else if (0 == strcmp(field, "trans")) {
            status = AJ_UnmarshalArgs(msg, "q", &opts->transports);
        } else {
            // don't print becuase this isn't really an error
            AJ_SkipArg(msg);
        }

        status = AJ_UnmarshalCloseContainer(msg, &struct1);
        // if field invalid, throw the whole thing out and return the error
    } while (status == AJ_OK);

    AJ_UnmarshalCloseContainer(msg, &array1);

    return status;
}

static void LSF_DisconnectHandler(uint8_t restart)
{
    if (restart) {
        AJ_BusUnbindSession(&Bus, LSF_ServicePort);
    }

    AJ_AboutSetShouldAnnounce();

    AJSVC_DisconnectHandler(&Bus);
}


void LAMP_RunServiceWithCallback(uint32_t timeout, LampServiceCallback callback)
{
    AJ_Status status = AJ_OK;
    uint8_t connected = FALSE;
    AJ_Time timer;

    /*
     * One time initialization before calling any other AllJoyn APIs
     */
    AJ_Initialize();

    AJ_PrintXML(LSF_AllJoynObjects);
    AJ_RegisterObjects(LSF_AllJoynObjects, NULL);

    SetBusAuthPwdCallback(MyBusAuthPwdCB);

    LAMP_SetupAboutConfigData();

    AJNS_Producer_Start();

    LAMP_InitializeState();

    OEM_LS_Initialize();

#ifdef ONBOARDING_SERVICE
    // initialize onboarding!
    LAMP_InitOnboarding();
#endif

    // announce all of our IOE objects;
    // this call might not be necessary
    AJ_AboutSetAnnounceObjects(LSF_AllJoynObjects);

    AJ_SetMinProtoVersion(MIN_ROUTER_VERSION);

    while (TRUE) {
        AJ_Message msg;

        if (connected == FALSE) {
            status = AJSVC_RoutingNodeConnect(&Bus, routingNodePrefix, CONNECT_TIMEOUT, 2000, 60, &connected);

            if (connected == FALSE) {
                continue;
            }

            AJ_BusSetPasswordCallback(&Bus, LAMP_PasswordCallback);

            AJ_SessionOpts session_opts = { AJ_SESSION_TRAFFIC_MESSAGES, AJ_SESSION_PROXIMITY_ANY, AJ_TRANSPORT_ANY, TRUE };
            // we need to bind the session port to run a service
            AJ_InfoPrintf(("%s: AJ_BindSessionPort()\n", __func__));
            status = AJ_BusBindSessionPort(&Bus, LSF_ServicePort, &session_opts, 0);
        }

        // use a minimum two-second timeout to ensure the callback is *eventually* reached
        status = AJ_UnmarshalMsg(&Bus, &msg, min(timeout, UNMARSHAL_TIMEOUT));
        if (AJ_ERR_TIMEOUT == status && AJ_ERR_LINK_TIMEOUT == AJ_BusLinkStateProc(&Bus)) {
            status = AJ_ERR_READ;
        }
        AJ_InfoPrintf(("%s: Got Message /Timed out. status = %d\n", __func__, status));

        if (status == AJ_OK) {
            switch (msg.msgId) {

            case AJ_REPLY_ID(AJ_METHOD_ADD_MATCH):
                if (msg.hdr->msgType == AJ_MSG_ERROR) {
                    AJ_InfoPrintf(("%s: Failed to add match\n", __func__));
                    status = AJ_ERR_FAILURE;
                } else {
                    status = AJ_OK;
                }
                break;

            case AJ_REPLY_ID(AJ_METHOD_BIND_SESSION_PORT):
                if (msg.hdr->msgType == AJ_MSG_ERROR) {
                    AJ_ErrPrintf(("%s: AJ_METHOD_BIND_SESSION_PORT: AJ_ERR_FAILURE\n", __func__));
                    status = AJ_ERR_FAILURE;
                } else {
                    AJ_InfoPrintf(("%s: AJ_BusRequestName()\n", __func__));
                    // announce now
                    AJ_InfoPrintf(("%s: Initializing About!\n", __func__));
                    status = AJ_AboutInit(&Bus, LSF_ServicePort);
                }
                break;

            case AJ_METHOD_ACCEPT_SESSION:
                {
                    AJ_InfoPrintf(("%s: Got Accept Session\n", __func__));
                    uint16_t port;
                    uint32_t session;
                    char* joiner;
                    AJ_UnmarshalArgs(&msg, "qus", &port, &session, &joiner);

                    if (port == LSF_ServicePort) {
                        AJ_SessionOpts opts;
                        ParseOptions(&opts, &msg);

                        if (opts.isMultipoint) {
                            ControllerSessionID = session;
                            AJ_InfoPrintf(("%s: Accepted multipoint session id=%u from joiner=%s\n", __func__, session, joiner));
                        } else {
                            AJ_InfoPrintf(("%s: Accepted session id=%u from joiner=%s\n", __func__, session, joiner));
                        }

                        status = AJ_BusReplyAcceptSession(&msg, TRUE);
                    } else {
                        status = AJ_BusReplyAcceptSession(&msg, FALSE);
                        AJ_InfoPrintf(("%s: Accepted rejected session_id=%u joiner=%s\n", __func__, ControllerSessionID, joiner));
                    }

                    break;
                }

            case AJ_SIGNAL_SESSION_LOST_WITH_REASON:
                {
                    // this might not be an error.
                    uint32_t sessionId, reason;
                    AJ_UnmarshalArgs(&msg, "uu", &sessionId, &reason);

                    if (sessionId == ControllerSessionID) {
                        // we don't care if a point-to-point session is lost
                        ControllerSessionID = 0;
                        SendStateChanged = FALSE;
                        status = AJ_ERR_SESSION_LOST;
                    }
                    AJ_InfoPrintf(("%s: Session lost. ID = %u, reason = %u", __func__, sessionId, reason));
                    break;
                }

            default:
                {
                    // try to process with Config
                    AJSVC_ServiceStatus serv_status = AJCFG_MessageProcessor(&Bus, &msg, &status);

#ifdef ONBOARDING_SERVICE
                    if (serv_status == AJSVC_SERVICE_STATUS_NOT_HANDLED) {
                        serv_status = AJOBS_MessageProcessor(&Bus, &msg, &status);
                    }
#endif
                    if (serv_status == AJSVC_SERVICE_STATUS_NOT_HANDLED) {
                        // let the notification produer object attempt to handle this message
                        serv_status = AJNS_Producer_MessageProcessor(&Bus, &msg, &status);
                    }

                    if (serv_status == AJSVC_SERVICE_STATUS_NOT_HANDLED) {
                        // let the LSF service attempt to handle this message
                        serv_status = LAMP_HandleMessage(&msg, &status);
                    }

                    if (serv_status == AJSVC_SERVICE_STATUS_NOT_HANDLED) {
                        /*
                         * Pass to the built-in bus message handlers.
                         * This will also handle messages for the About object
                         */
                        status = AJ_BusHandleBusMessage(&msg);
                    }
                    break;
                }
            } // end switch

            // Any received packets indicates the link is active, so call to reinforce the bus link state
            AJ_NotifyLinkActive();
        }

        /*
         * Unarshaled messages must be closed to free resources
         */
        AJ_CloseMsg(&msg);

        // check for anything that must go out now
        CheckForFaults();
        CheckForStateChanged();

        // this will be called by AJ_BusHandleBusMessage, but on LSF method calls AJ_BusHandleBusMessage isn't called
        if (status == AJ_OK) {
            AJ_AboutAnnounce(&Bus);
        }

        // we might make a timer callback even if we just processed a message
        if (callback != NULL && AJ_GetElapsedTime(&timer, TRUE) >= timeout) {
            // restart the timer
            AJ_InitTimer(&timer);
            (*callback)();
        }

        if (PendingRestartRequest == TRUE) {
            PendingRestartRequest = FALSE;
            status = AJ_ERR_RESTART;
        }

        if (PendingFactoryResetRequest == TRUE) {
            PendingFactoryResetRequest = FALSE;

            AJSVC_PropertyStore_ResetAll();
            AJ_NVRAM_Clear();
            SavePersistentDeviceId();
            PropertyStore_Init();

            OEM_LS_DoFactoryReset();

            status = AJ_ERR_RESTART_APP;
        }

        if (status == AJ_ERR_READ || status == AJ_ERR_RESTART || status == AJ_ERR_RESTART_APP) {
            AJ_InfoPrintf(("%s: AllJoyn disconnect\n", __func__));
            AJ_InfoPrintf(("%s: Disconnected from Daemon:%s\n", __func__, AJ_GetUniqueName(&Bus)));

            LSF_DisconnectHandler(status != AJ_ERR_READ);
            AJSVC_RoutingNodeDisconnect(&Bus, (status != AJ_ERR_READ), 2000, 3000, &connected);

            if (status == AJ_ERR_RESTART_APP) {
                AJ_Reboot();
            }
        }
    }
}

void LAMP_SendStateChangedSignal(void)
{
    AJ_InfoPrintf(("\n%s\n", __func__));
    SendStateChanged = TRUE;
}

static AJ_Status ClearLampFault(AJ_Message* msg)
{
    LampResponseCode responseCode = LAMP_OK;
    uint32_t faultCode;
    AJ_Message reply;
    AJ_MarshalReplyMsg(msg, &reply);

    AJ_UnmarshalArgs(msg, "u", &faultCode);
    responseCode = OEM_LS_ClearFault(faultCode);

    AJ_MarshalArgs(&reply, "uu", (uint32_t) responseCode, (uint32_t) faultCode);
    AJ_DeliverMsg(&reply);
    AJ_CloseMsg(&reply);

    return AJ_OK;
}

static AJ_Status TransitionLampState(AJ_Message* msg)
{
    LampResponseCode responseCode = LAMP_OK;
    LampState new_state;
    uint64_t timestamp;
    uint32_t TransitionPeriod;

    AJ_Message reply;
    AJ_MarshalReplyMsg(msg, &reply);

    AJ_UnmarshalArgs(msg, "t", &timestamp);
    LAMP_UnmarshalState(&new_state, msg);
    AJ_UnmarshalArgs(msg, "u", &TransitionPeriod);

    // apply the new state
    responseCode = OEM_LS_TransitionState(&new_state, timestamp, TransitionPeriod);

    AJ_MarshalArgs(&reply, "u", (uint32_t) responseCode);
    AJ_DeliverMsg(&reply);
    AJ_CloseMsg(&reply);
    return AJ_OK;
}

/*
 * The Apply Pulse Effect accepts two parameters - the From State and the To State.
 * If the user wants the Lamp to pulse from the Lamp's current state to a another
 * state, the From State is specified as a NULL dictionary.  When the From State
 * is specified as NULL, the Lamp Service sets the state to the current state of
 * the Lamp and passes it on the OEM layer.
 */
static AJ_Status ApplyPulseEffect(AJ_Message* msg)
{
    LampResponseCode responseCode = LAMP_OK;
    LampState FromState, ToState;
    uint32_t period;
    uint32_t duration;
    uint32_t numPulses;
    uint64_t timestamp;

    AJ_Message reply;
    AJ_MarshalReplyMsg(msg, &reply);

    LAMP_UnmarshalState(&FromState, msg);
    LAMP_UnmarshalState(&ToState, msg);
    AJ_UnmarshalArgs(msg, "uuut", &period, &duration, &numPulses, &timestamp);

    // apply the new state
    responseCode = OEM_LS_ApplyPulseEffect(&FromState, &ToState, period, duration, numPulses, timestamp);

    AJ_MarshalArgs(&reply, "u", (uint32_t) responseCode);
    AJ_DeliverMsg(&reply);
    AJ_CloseMsg(&reply);
    return AJ_OK;
}

static AJ_Status MarshalStateField(AJ_Message* replyMsg, uint32_t propId)
{
    LampState state;
    LAMP_GetState(&state);
    AJ_InfoPrintf(("%s\n", __func__));

    switch (propId) {
    case LSF_PROP_STATE_ONOFF:
        AJ_InfoPrintf(("onOff: %s\n", (state.onOff ? "TRUE" : "FALSE")));
        return AJ_MarshalArgs(replyMsg, "b", (state.onOff ? TRUE : FALSE));

    case LSF_PROP_STATE_HUE:
        AJ_InfoPrintf(("Hue: %u\n", state.hue));
        return AJ_MarshalArgs(replyMsg, "u", state.hue);

    case LSF_PROP_STATE_SAT:
        AJ_InfoPrintf(("Saturation: %u\n", state.saturation));
        return AJ_MarshalArgs(replyMsg, "u", state.saturation);

    case LSF_PROP_STATE_TEMP:
        AJ_InfoPrintf(("Color: %u\n", state.colorTemp));
        return AJ_MarshalArgs(replyMsg, "u", state.colorTemp);

    case LSF_PROP_STATE_BRIGHT:
        AJ_InfoPrintf(("Brightness: %u\n", state.brightness));
        return AJ_MarshalArgs(replyMsg, "u", state.brightness);

    default:
        return AJ_ERR_UNEXPECTED;
    }
}

static AJ_Status PropSetHandler(AJ_Message* msg, uint32_t propId, void* context)
{
    AJ_Status status = AJ_OK;
    LampResponseCode responseCode = LAMP_OK;

    AJ_InfoPrintf(("%s\n", __func__));

    switch (propId) {
    case LSF_PROP_STATE_ONOFF:
        {
            uint32_t onoff;
            status = AJ_UnmarshalArgs(msg, "b", &onoff);
            if (status == AJ_OK) {
                responseCode = OEM_LS_SetOnOff(onoff);
            }
            break;
        }

    case LSF_PROP_STATE_HUE:
        {
            uint32_t hue;
            status = AJ_UnmarshalArgs(msg, "u", &hue);
            if (status == AJ_OK) {
                responseCode = OEM_LS_SetHue(hue);
            }
            break;
        }

    case LSF_PROP_STATE_SAT:
        {
            uint32_t saturation;
            status = AJ_UnmarshalArgs(msg, "u", &saturation);
            if (status == AJ_OK) {
                responseCode = OEM_LS_SetSaturation(saturation);
            }
            break;
        }

    case LSF_PROP_STATE_TEMP:
        {
            uint32_t colorTemp;
            status = AJ_UnmarshalArgs(msg, "u", &colorTemp);
            if (status == AJ_OK) {
                responseCode = OEM_LS_SetColorTemp(colorTemp);
            }
            break;
        }

    case LSF_PROP_STATE_BRIGHT:
        {
            uint32_t brightness;
            status = AJ_UnmarshalArgs(msg, "u", &brightness);
            if (status == AJ_OK) {
                responseCode = OEM_LS_SetBrightness(brightness);
            }
            break;
        }

    default:
        status = AJ_ERR_DISALLOWED;
        break;
    }

    // need to indicate some kind of failure
    if (responseCode != LAMP_OK) {
        status = AJ_ERR_FAILURE;
    }

    return status;
}


static AJ_Status PropGetHandler(AJ_Message* replyMsg, uint32_t propId, void* context)
{
    switch (propId) {
    // org.allseen.LSF.LampService
    case LSF_PROP_VERSION:
        AJ_InfoPrintf(("LSF_PROP_VERSION: %u\n", LSF_Interface_Version));
        return AJ_MarshalArgs(replyMsg, "u", LSF_Interface_Version);

    case LSF_PROP_LSF_VERSION:
        AJ_InfoPrintf(("LSF_PROP_LSF_VERSION: %u\n", LAMP_GetServiceVersion()));
        return AJ_MarshalArgs(replyMsg, "u", LAMP_GetServiceVersion());

    case LSF_PROP_FAULTS:
        {
            AJ_Arg array1;

            AJ_MarshalContainer(replyMsg, &array1, AJ_ARG_ARRAY);
            OEM_LS_PopulateFaults(replyMsg);
            AJ_MarshalCloseContainer(replyMsg, &array1);
            return AJ_OK;
        }

    // run-time parameters
    case LSF_PROP_PARAMS_VERSION:
        AJ_InfoPrintf(("LSF_PROP_FAULTS: %u\n", LSF_Parameters_Interface_Version));
        return AJ_MarshalArgs(replyMsg, "u", LSF_Parameters_Interface_Version);

    case LSF_PROP_PARAMS_ENERGY_USAGE_MILLIWATTS:
        AJ_InfoPrintf(("LSF_PROP_PARAMS_ENERGY_USAGE_MILLIWATTS: %u\n", OEM_LS_GetEnergyUsageMilliwatts()));
        return AJ_MarshalArgs(replyMsg, "u", OEM_LS_GetEnergyUsageMilliwatts());

    case LSF_PROP_PARAMS_BRIGHTNESS_LUMENS:
        AJ_InfoPrintf(("LSF_PROP_PARAMS_BRIGHTNESS_LUMENS: %u\n", OEM_LS_GetBrightnessLumens()));
        return AJ_MarshalArgs(replyMsg, "u", OEM_LS_GetBrightnessLumens());


    // Compile-time Details
    case LSF_PROP_DETAILS_VERSION:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_VERSION: %u\n", LSF_Parameters_Interface_Version));
        return AJ_MarshalArgs(replyMsg, "u", LSF_Details_Interface_Version);

    case LSF_PROP_DETAILS_MAKE:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_MAKE: %u\n", LampDetails.lampMake));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.lampMake);

    case LSF_PROP_DETAILS_MODEL:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_MODEL: %u\n", LampDetails.lampModel));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.lampModel);

    case LSF_PROP_DETAILS_DEV_TYPE:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_DEV_TYPE: %u\n", LampDetails.deviceType));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.deviceType);

    case LSF_PROP_DETAILS_LAMP_TYPE:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_LAMP_TYPE: %u\n", LampDetails.lampType));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.lampType);

    case LSF_PROP_DETAILS_BASETYPE:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_BASETYPE: %u\n", LampDetails.baseType));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.baseType);

    case LSF_PROP_DETAILS_BEAMANGLE:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_BEAMANGLE: %u\n", LampDetails.deviceLampBeamAngle));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.deviceLampBeamAngle);

    case LSF_PROP_DETAILS_DIMMABLE:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_DIMMABLE: %s\n", (LampDetails.deviceDimmable ? "TRUE" : "FALSE")));
        return AJ_MarshalArgs(replyMsg, "b", (LampDetails.deviceDimmable ? TRUE : FALSE));

    case LSF_PROP_DETAILS_COLOR:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_COLOR: %s\n", (LampDetails.deviceColor ? "TRUE" : "FALSE")));
        return AJ_MarshalArgs(replyMsg, "b", (LampDetails.deviceColor ? TRUE : FALSE));

    case LSF_PROP_DETAILS_VARCOLORTEMP:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_VARCOLORTEMP: %s\n", (LampDetails.variableColorTemp ? "TRUE" : "FALSE")));
        return AJ_MarshalArgs(replyMsg, "b", (LampDetails.variableColorTemp ? TRUE : FALSE));

    case LSF_PROP_DETAILS_HASEFFECTS:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_HASEFFECTS: %s\n", (LampDetails.deviceHasEffects ? "TRUE" : "FALSE")));
        return AJ_MarshalArgs(replyMsg, "b", (LampDetails.deviceHasEffects ? TRUE : FALSE));

    case LSF_PROP_DETAILS_MINVOLTAGE:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_MINVOLTAGE: %u\n", LampDetails.deviceMinVoltage));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.deviceMinVoltage);

    case LSF_PROP_DETAILS_MAXVOLTAGE:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_MAXVOLTAGE: %u\n", LampDetails.deviceMaxVoltage));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.deviceMaxVoltage);

    case LSF_PROP_DETAILS_WATTAGE:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_WATTAGE: %u\n", LampDetails.deviceWattage));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.deviceWattage);

    case LSF_PROP_DETAILS_INCANEQV:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_INCANEQV: %u\n", LampDetails.deviceIncandescentEquivalent));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.deviceIncandescentEquivalent);

    case LSF_PROP_DETAILS_MAXLUMENS:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_MAXLUMENS: %u\n", LampDetails.deviceMaxLumens));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.deviceMaxLumens);

    case LSF_PROP_DETAILS_MINTEMP:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_MINTEMP: %u\n", LampDetails.deviceMinTemperature));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.deviceMinTemperature);

    case LSF_PROP_DETAILS_MAXTEMP:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_MAXTEMP: %u\n", LampDetails.deviceMaxTemperature));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.deviceMaxTemperature);

    case LSF_PROP_DETAILS_CRI:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_CRI: %u\n", LampDetails.deviceColorRenderingIndex));
        return AJ_MarshalArgs(replyMsg, "u", LampDetails.deviceColorRenderingIndex);

    case LSF_PROP_DETAILS_LAMPID:
        AJ_InfoPrintf(("LSF_PROP_DETAILS_LAMPID: %s\n", AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEVICE_ID)));
        return AJ_MarshalArgs(replyMsg, "s", AJSVC_PropertyStore_GetValue(AJSVC_PROPERTY_STORE_DEVICE_ID));


    // LampState properties
    case LSF_PROP_STATE_VERSION:
        AJ_InfoPrintf(("LSF_PROP_STATE_VERSION: %u\n", LSF_State_Interface_Version));
        return AJ_MarshalArgs(replyMsg, "u", LSF_State_Interface_Version);

    case LSF_PROP_STATE_ONOFF:
    case LSF_PROP_STATE_HUE:
    case LSF_PROP_STATE_SAT:
    case LSF_PROP_STATE_TEMP:
    case LSF_PROP_STATE_BRIGHT:
        return MarshalStateField(replyMsg, propId);

    default:
        return AJ_ERR_UNEXPECTED;
    }
}

static AJ_Status GetAllProps(AJ_Message* msg)
{
    const char* iface;
    AJ_Message reply;
    AJ_Arg array1;

    AJ_UnmarshalArgs(msg, "s", &iface);
    AJ_InfoPrintf(("%s: Interface=%s\n", __func__, iface));

    AJ_MarshalReplyMsg(msg, &reply);
    AJ_MarshalContainer(&reply, &array1, AJ_ARG_ARRAY);

    if (0 == strcmp(iface, LSF_Interface_Name)) {
        AJ_MarshalArgs(&reply, "{sv}", "Version", "u", LSF_Interface_Version);
        AJ_MarshalArgs(&reply, "{sv}", "LampServiceVersion", "u", LAMP_GetServiceVersion());

        // now add the lamp faults to the message
        {
            AJ_Arg array2, struct1;
            AJ_MarshalContainer(&reply, &struct1, AJ_ARG_DICT_ENTRY);
            AJ_MarshalArgs(&reply, "s", "LampFaults");

            AJ_MarshalVariant(&reply, "au");

            AJ_MarshalContainer(&reply, &array2, AJ_ARG_ARRAY);
            OEM_LS_PopulateFaults(&reply);
            AJ_MarshalCloseContainer(&reply, &array2);
            AJ_MarshalCloseContainer(&reply, &struct1);
        }
    } else if (0 == strcmp(iface, LSF_Parameters_Interface_Name)) {
        AJ_MarshalArgs(&reply, "{sv}", "Version", "u", LSF_Parameters_Interface_Version);
        OEM_LS_PopulateParameters(&reply);
    } else if (0 == strcmp(iface, LSF_Details_Interface_Name)) {
        AJ_MarshalArgs(&reply, "{sv}", "Version", "u", LSF_Details_Interface_Version);
        OEM_LS_PopulateDetails(&reply);
    } else if (0 == strcmp(iface, LSF_State_Interface_Name)) {
        AJ_MarshalArgs(&reply, "{sv}", "Version", "u", LSF_State_Interface_Version);
        LampState state;
        LAMP_GetState(&state);
        LAMP_MarshalState(&state, &reply);
    }

    AJ_MarshalCloseContainer(&reply, &array1);
    AJ_DeliverMsg(&reply);
    AJ_CloseMsg(&reply);
    return AJ_OK;
}

static AJSVC_ServiceStatus LAMP_HandleMessage(AJ_Message* msg, AJ_Status* status)
{
    AJ_InfoPrintf(("\n%s\n", __func__));
    AJSVC_ServiceStatus serv_status = AJSVC_SERVICE_STATUS_HANDLED;

    switch (msg->msgId) {

    case APP_GET_PROP:
        *status = AJ_BusPropGet(msg, PropGetHandler, NULL);
        break;

    case APP_SET_PROP:
        *status = AJ_BusPropSet(msg, PropSetHandler, NULL);
        break;

    case APP_GET_PROP_ALL:
        *status = GetAllProps(msg);
        break;

    case LSF_METHOD_CLEARLAMPFAULTS:
        *status = ClearLampFault(msg);
        break;

    case LSF_METHOD_STATE_SETSTATE:
        *status = TransitionLampState(msg);
        break;

    case LSF_METHOD_APPLY_PULSE:
        *status = ApplyPulseEffect(msg);
        break;

    default:
        serv_status = AJSVC_SERVICE_STATUS_NOT_HANDLED;
        break;
    }

    return serv_status;
}
