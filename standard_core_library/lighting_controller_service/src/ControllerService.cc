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

#include <LSFResponseCodes.h>
#include <qcc/StringUtil.h>
#include <qcc/Debug.h>
#include <LSFTypes.h>

#include <ControllerService.h>
#include <ServiceDescription.h>
#include <DeviceIcon.h>

#include <alljoyn/notification/NotificationService.h>
#include <string>

using namespace lsf;
using namespace ajn;

#define QCC_MODULE "CONTROLLER_SERVICE"

class ControllerService::ControllerListener :
    public SessionPortListener,
    public BusListener,
    public services::AnnounceHandler,
    public SessionListener {
  public:
    ControllerListener(ControllerService* controller) : controller(controller), multipointjoiner(), multipointSessionId(0) { }

    ~ControllerListener() { }

    // SessionPortListener
    virtual bool AcceptSessionJoiner(SessionPort sessionPort, const char* joiner, const SessionOpts& opts) {
        QCC_DbgTrace(("%s:SessionPort=%d, joiner=%s", __func__, sessionPort, joiner));
        // only allow multipoint sessions
        bool acceptStatus = (sessionPort == ControllerServiceSessionPort && controller->IsLeader());
        if (acceptStatus && opts.isMultipoint) {
            multipointjoiner = joiner;
            QCC_DbgPrintf(("%s: Recorded multi-point session joiner %s\n", __func__, multipointjoiner.c_str()));
        }
        QCC_DbgPrintf(("%s: Joiner = %s: Status=%d\n", __func__, joiner, acceptStatus));
        return acceptStatus;
    }

    virtual void SessionJoined(SessionPort sessionPort, SessionId sessionId, const char* joiner) {
        QCC_DbgPrintf(("%s: sessionPort(%d), sessionId(%d), joiner(%s)", __func__, sessionPort, sessionId, joiner));
        if (multipointSessionId == 0) {
            if (0 == strcmp(multipointjoiner.c_str(), joiner)) {
                multipointSessionId = sessionId;
                QCC_DbgPrintf(("%s: Recorded multi-point session id %d\n", __func__, multipointSessionId));
                controller->SessionJoined(sessionId);
            }
        } else {
            if (multipointSessionId == sessionId) {
                QCC_DbgPrintf(("%s: Another member joined the multipoint session", __func__));
            }
        }
    }

    virtual void SessionLost(SessionId sessionId, SessionLostReason reason) {
        QCC_DbgPrintf(("%s: sessionId(%d), reason(%d)", __func__, sessionId, reason));
        if (multipointSessionId == sessionId) {
            QCC_DbgPrintf(("%s: Lost multi-point session id %d\n", __func__, multipointSessionId));
            controller->SessionLost(sessionId);
            multipointSessionId = 0;
            multipointjoiner.clear();
        }
    }

    virtual void Announce(uint16_t version, uint16_t port, const char* busName, const ObjectDescriptions& objectDescs, const AboutData& aboutData) {
        QCC_DbgTrace(("%s:version=%d, port=%d, busName=%s", __func__, version, port, busName));
        controller->bus.EnableConcurrentCallbacks();
        ObjectDescriptions::const_iterator it = objectDescs.find(OnboardingServiceObjectPath);
        if (it != objectDescs.end()) {
            // this implements an onboarding service
            // we need to determine whether it's on the same routing node

            qcc::String myName = controller->bus.GetUniqueName();
            size_t idx = myName.find_first_of('.');
            myName = myName.substr(0, idx);

            qcc::String name = busName;
            size_t idx2 = name.find_first_of('.');
            name = name.substr(0, idx2);

            if (name == myName) {
                controller->FoundLocalOnboardingService(busName, port);
            }
        }
    }

    virtual void BusDisconnected() {
        QCC_DbgPrintf(("BusDisconnected!"));
        controller->Restart();
    }

    ControllerService* controller;
    qcc::String multipointjoiner;
    SessionId multipointSessionId;
};

class ControllerService::OBSJoiner : public BusAttachment::JoinSessionAsyncCB, public ProxyBusObject::Listener {
  public:
    OBSJoiner(ControllerService& controller, const char* busName)
        : controller(controller),
        busName(busName)
    {
        QCC_DbgTrace(("%s:busName=%s", __func__, busName));
    }

    void FinishedIntrospect(QStatus status, ProxyBusObject* obj, void* context) {
        QCC_DbgTrace(("%s:status=%s", __func__, QCC_StatusText(status)));
        controller.bus.EnableConcurrentCallbacks();
        if (status == ER_OK) {
            // we are now valid!
            // set valid!
            controller.obsObjectLock.Lock();
            controller.isObsObjectReady = true;
            controller.obsObjectLock.Unlock();
            QCC_DbgPrintf(("Onboarding Service proxy object ready"));
        } else {
            // something has gone wrong.
            SessionId session = 0;
            QCC_LogError(status, ("Failed to Introspect remote Onboarding Object"));

            controller.obsObjectLock.Lock();
            if (controller.obsObject != NULL) {
                session = controller.obsObject->GetSessionId();
                delete controller.obsObject;
                controller.obsObject = NULL;
            }
            controller.obsObjectLock.Unlock();

            if (session != 0) {
                controller.bus.LeaveSession(session);
            }
        }

        delete this;
    }

    virtual void JoinSessionCB(QStatus status, SessionId sessionId, const SessionOpts& opts, void* context) {
        QCC_DbgTrace(("%s:status=%s sessionId=%d", __func__, QCC_StatusText(status), sessionId));
        OBSJoiner* joinHandler = static_cast<OBSJoiner*>(context);

        if (joinHandler == NULL) {
            return;
        }

        if (status == ER_OK) {
            controller.obsObjectLock.Lock();
            controller.isObsObjectReady = false;
            controller.obsObject = new ProxyBusObject(controller.bus, busName.c_str(), ConfigServiceObjectPath, sessionId);
            // ok to call when locked?
            QCC_DbgPrintf(("Joining Onboarding Service on %s with session %u", busName.c_str(), sessionId));

            status = controller.obsObject->IntrospectRemoteObjectAsync(
                this,
                static_cast<ProxyBusObject::Listener::IntrospectCB>(&ControllerService::OBSJoiner::FinishedIntrospect),
                NULL);

            if (status != ER_OK) {
                delete controller.obsObject;
                controller.obsObject = NULL;
                QCC_LogError(status, ("Failed to join Introspect Onboarding Object on %s on session %u", busName.c_str(), sessionId));
            }

            controller.obsObjectLock.Unlock();
        }

        delete joinHandler;

        if (status != ER_OK) {
            delete this;
        }
    }

    ControllerService& controller;
    qcc::String busName;
};




ControllerService::ControllerService(
    ajn::services::PropertyStore& propStore,
    const std::string& obsConfigFile,
    const std::string& factoryConfigFile,
    const std::string& configFile,
    const std::string& lampGroupFile,
    const std::string& presetFile,
    const std::string& sceneFile,
    const std::string& masterSceneFile) :
    BusObject(ControllerServiceObjectPath),
    elector(*this),
    bus("LightingServiceController", true),
    serviceSession(0),
    listener(new ControllerListener(this)),
    lampManager(*this, presetManager),
    lampGroupManager(*this, lampManager, &sceneManager, lampGroupFile),
    presetManager(*this, &sceneManager, presetFile),
    sceneManager(*this, lampGroupManager, &masterSceneManager, sceneFile),
    masterSceneManager(*this, sceneManager, masterSceneFile),
    internalPropertyStore(obsConfigFile, factoryConfigFile, configFile), // we don't use this!
    propertyStore(propStore),
    aboutService(NULL),
    aboutIconService(bus, DeviceIconMimeType, DeviceIconURL, DeviceIcon, DeviceIconSize),
    configService(bus, propertyStore, *this),
    notificationSender(NULL),
    obsObject(NULL),
    isObsObjectReady(false),
    isRunning(true),
    fileWriterThread(*this),
    firstAnnouncementSent(false)
{
    QCC_DbgTrace(("%s:obsConfigFile=%s, factoryConfigFile=%s, configFile=%s, lampGroupFile=%s, presetFile=%s, sceneFile=%s, masterSceneFile=%s", __func__, obsConfigFile.c_str(), factoryConfigFile.c_str(), configFile.c_str(), lampGroupFile.c_str(), presetFile.c_str(), sceneFile.c_str(), masterSceneFile.c_str()));
}

ControllerService::ControllerService(
    const std::string& obsConfigFile,
    const std::string& factoryConfigFile,
    const std::string& configFile,
    const std::string& lampGroupFile,
    const std::string& presetFile,
    const std::string& sceneFile,
    const std::string& masterSceneFile) :
    BusObject(ControllerServiceObjectPath),
    elector(*this),
    bus("LightingServiceController", true),
    serviceSession(0),
    listener(new ControllerListener(this)),
    lampManager(*this, presetManager),
    lampGroupManager(*this, lampManager, &sceneManager, lampGroupFile),
    presetManager(*this, &sceneManager, presetFile),
    sceneManager(*this, lampGroupManager, &masterSceneManager, sceneFile),
    masterSceneManager(*this, sceneManager, masterSceneFile),
    internalPropertyStore(obsConfigFile, factoryConfigFile, configFile),
    propertyStore(internalPropertyStore),
    aboutService(NULL),
    aboutIconService(bus, DeviceIconMimeType, DeviceIconURL, DeviceIcon, DeviceIconSize),
    configService(bus, propertyStore, *this),
    notificationSender(NULL),
    obsObject(NULL),
    isObsObjectReady(false),
    isRunning(true),
    fileWriterThread(*this),
    firstAnnouncementSent(false)
{
    QCC_DbgTrace(("%s:obsConfigFile=%s, factoryConfigFile=%s, configFile=%s, lampGroupFile=%s, presetFile=%s, sceneFile=%s, masterSceneFile=%s", __func__, obsConfigFile.c_str(), factoryConfigFile.c_str(), configFile.c_str(), lampGroupFile.c_str(), presetFile.c_str(), sceneFile.c_str(), masterSceneFile.c_str()));
    internalPropertyStore.Initialize();
}

void ControllerService::FoundLocalOnboardingService(const char* busName, SessionPort port)
{
    QCC_DbgPrintf(("Joining Onboarding Service on %s:%u", busName, port));

    SessionOpts opts;
    OBSJoiner* joinHandler = new OBSJoiner(*this, busName);
    QStatus status = bus.JoinSessionAsync(busName, port, NULL, opts, joinHandler);
    if (status != ER_OK) {
        QCC_LogError(status, ("Failed to join Onboarding Service on %s:%u", busName, port));
        //QCC_DbgPrintf(("Successful CreateAndAddInterface for %s", entries[i].interfaceName));
        delete joinHandler;
    }
}

void ControllerService::Initialize()
{
    QCC_DbgTrace(("%s", __func__));
    lampGroupManager.ReadSavedData();
    presetManager.ReadSavedData();
    sceneManager.ReadSavedData();
    masterSceneManager.ReadSavedData();

    AddMethodHandler("LightingResetControllerService", this, &ControllerService::LightingResetControllerService);
    AddMethodHandler("GetControllerServiceVersion", this, &ControllerService::GetControllerServiceVersion);
    AddMethodHandler("GetAllLampIDs", &lampManager, &LampManager::GetAllLampIDs);
    AddMethodHandler("GetLampSupportedLanguages", &lampManager, &LampManager::GetLampSupportedLanguages);
    AddMethodHandler("GetLampManufacturer", &lampManager, &LampManager::GetLampManufacturer);
    AddMethodHandler("GetLampName", &lampManager, &LampManager::GetLampName);
    AddMethodHandler("SetLampName", &lampManager, &LampManager::SetLampName);
    AddMethodHandler("GetLampDetails", &lampManager, &LampManager::GetLampDetails);
    AddMethodHandler("GetLampParameters", &lampManager, &LampManager::GetLampParameters);
    AddMethodHandler("GetLampParametersField", &lampManager, &LampManager::GetLampParametersField);
    AddMethodHandler("GetLampState", &lampManager, &LampManager::GetLampState);
    AddMethodHandler("GetLampStateField", &lampManager, &LampManager::GetLampStateField);
    AddMethodHandler("TransitionLampState", &lampManager, &LampManager::TransitionLampState);
    AddMethodHandler("PulseLampWithState", &lampManager, &LampManager::PulseLampWithState);
    AddMethodHandler("PulseLampWithPreset", &lampManager, &LampManager::PulseLampWithPreset);
    AddMethodHandler("TransitionLampStateToPreset", &lampManager, &LampManager::TransitionLampStateToPreset);
    AddMethodHandler("TransitionLampGroupState", &lampGroupManager, &LampGroupManager::TransitionLampGroupState);
    AddMethodHandler("PulseLampGroupWithState", &lampGroupManager, &LampGroupManager::PulseLampGroupWithState);
    AddMethodHandler("PulseLampGroupWithPreset", &lampGroupManager, &LampGroupManager::PulseLampGroupWithPreset);
    AddMethodHandler("TransitionLampGroupStateToPreset", &lampGroupManager, &LampGroupManager::TransitionLampGroupStateToPreset);
    AddMethodHandler("TransitionLampStateField", &lampManager, &LampManager::TransitionLampStateField);
    AddMethodHandler("TransitionLampGroupStateField", &lampGroupManager, &LampGroupManager::TransitionLampGroupStateField);
    AddMethodHandler("ResetLampState", &lampManager, &LampManager::ResetLampState);
    AddMethodHandler("ResetLampGroupState", &lampGroupManager, &LampGroupManager::ResetLampGroupState);
    AddMethodHandler("ResetLampStateField", &lampManager, &LampManager::ResetLampStateField);
    AddMethodHandler("ResetLampGroupStateField", &lampGroupManager, &LampGroupManager::ResetLampGroupStateField);
    AddMethodHandler("GetLampFaults", &lampManager, &LampManager::GetLampFaults);
    AddMethodHandler("ClearLampFault", &lampManager, &LampManager::ClearLampFault);
    AddMethodHandler("GetLampServiceVersion", &lampManager, &LampManager::GetLampServiceVersion);
    AddMethodHandler("GetAllLampGroupIDs", &lampGroupManager, &LampGroupManager::GetAllLampGroupIDs);
    AddMethodHandler("GetLampGroupName", &lampGroupManager, &LampGroupManager::GetLampGroupName);
    AddMethodHandler("SetLampGroupName", &lampGroupManager, &LampGroupManager::SetLampGroupName);
    AddMethodHandler("CreateLampGroup", &lampGroupManager, &LampGroupManager::CreateLampGroup);
    AddMethodHandler("UpdateLampGroup", &lampGroupManager, &LampGroupManager::UpdateLampGroup);
    AddMethodHandler("DeleteLampGroup", &lampGroupManager, &LampGroupManager::DeleteLampGroup);
    AddMethodHandler("GetLampGroup", &lampGroupManager, &LampGroupManager::GetLampGroup);
    AddMethodHandler("GetDefaultLampState", &presetManager, &PresetManager::GetDefaultLampState);
    AddMethodHandler("SetDefaultLampState", &presetManager, &PresetManager::SetDefaultLampState);
    AddMethodHandler("GetAllPresetIDs", &presetManager, &PresetManager::GetAllPresetIDs);
    AddMethodHandler("GetPresetName", &presetManager, &PresetManager::GetPresetName);
    AddMethodHandler("SetPresetName", &presetManager, &PresetManager::SetPresetName);
    AddMethodHandler("CreatePreset", &presetManager, &PresetManager::CreatePreset);
    AddMethodHandler("UpdatePreset", &presetManager, &PresetManager::UpdatePreset);
    AddMethodHandler("DeletePreset", &presetManager, &PresetManager::DeletePreset);
    AddMethodHandler("GetPreset", &presetManager, &PresetManager::GetPreset);
    AddMethodHandler("GetAllSceneIDs", &sceneManager, &SceneManager::GetAllSceneIDs);
    AddMethodHandler("GetSceneName", &sceneManager, &SceneManager::GetSceneName);
    AddMethodHandler("SetSceneName", &sceneManager, &SceneManager::SetSceneName);
    AddMethodHandler("CreateScene", &sceneManager, &SceneManager::CreateScene);
    AddMethodHandler("UpdateScene", &sceneManager, &SceneManager::UpdateScene);
    AddMethodHandler("DeleteScene", &sceneManager, &SceneManager::DeleteScene);
    AddMethodHandler("GetScene", &sceneManager, &SceneManager::GetScene);
    AddMethodHandler("ApplyScene", &sceneManager, &SceneManager::ApplyScene);
    AddMethodHandler("GetAllMasterSceneIDs", &masterSceneManager, &MasterSceneManager::GetAllMasterSceneIDs);
    AddMethodHandler("GetMasterSceneName", &masterSceneManager, &MasterSceneManager::GetMasterSceneName);
    AddMethodHandler("SetMasterSceneName", &masterSceneManager, &MasterSceneManager::SetMasterSceneName);
    AddMethodHandler("CreateMasterScene", &masterSceneManager, &MasterSceneManager::CreateMasterScene);
    AddMethodHandler("UpdateMasterScene", &masterSceneManager, &MasterSceneManager::UpdateMasterScene);
    AddMethodHandler("DeleteMasterScene", &masterSceneManager, &MasterSceneManager::DeleteMasterScene);
    AddMethodHandler("GetMasterScene", &masterSceneManager, &MasterSceneManager::GetMasterScene);
    AddMethodHandler("ApplyMasterScene", &masterSceneManager, &MasterSceneManager::ApplyMasterScene);
}

ControllerService::~ControllerService()
{
    QCC_DbgTrace(("%s", __func__));
    delete listener;
    QCC_DbgPrintf(("%s: Done deleting listener", __func__));

    for (DispatcherMap::iterator it = messageHandlers.begin(); it != messageHandlers.end(); ++it) {
        delete it->second;
    }

    messageHandlers.clear();
}

QStatus ControllerService::CreateAndAddInterface(std::string interfaceDescription, const char* interfaceName) {
    QCC_DbgTrace(("%s:interfaceDescription=%s interfaceName=%s", __func__, interfaceDescription.c_str(), interfaceName));
    QStatus status = ER_OK;

    status = bus.CreateInterfacesFromXml(interfaceDescription.c_str());
    if (status == ER_OK) {
        const InterfaceDescription* intf = bus.GetInterface(interfaceName);
        if (intf) {
            AddInterface(*intf);
        } else {
            status = ER_BUS_UNKNOWN_INTERFACE;
            QCC_LogError(status, ("GetInterface failed for %s", interfaceName));
        }
    } else {
        QCC_LogError(status, ("CreateInterfacesFromXml failed for %s", interfaceName));
    }

    return status;
}

QStatus ControllerService::CreateAndAddInterfaces(const InterfaceEntry* entries, size_t numEntries) {
    QCC_DbgTrace(("%s:numEntries=%d", __func__, numEntries));
    if (!entries) {
        return ER_BAD_ARG_1;
    }
    QStatus status = ER_OK;
    for (size_t i = 0; i < numEntries; ++i) {
        status = CreateAndAddInterface(entries[i].interfaceDescription, entries[i].interfaceName);
        if (ER_OK != status) {
            QCC_LogError(status, ("Failed to CreateAndAddInterface %s", entries[i].interfaceName));
            break;
        } else {
            QCC_DbgPrintf(("Successful CreateAndAddInterface for %s", entries[i].interfaceName));
        }
    }
    return status;
}

static const char* OnboardingInterfaces[] = {
    OnboardingServiceInterfaceName,
    ConfigServiceInterfaceName
};

QStatus ControllerService::Start(const char* keyStoreFileLocation)
{
    QCC_DbgPrintf(("%s:%s", __func__, keyStoreFileLocation));
    QStatus status = ER_OK;

    /*
     * Start the AllJoyn Bus
     */
    status = bus.Start();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to start the bus\n", __func__));
        return status;
    }

    bus.RegisterBusListener(*listener);

    /*
     * Connect to the AllJoyn Bus
     */
    status = bus.Connect();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to connect to the bus\n", __func__));
        return status;
    }

    /*
     * Create and Add the Controller Service Interfaces to the AllJoyn Bus
     */
    const InterfaceEntry interfaceEntries[] = {
        { ControllerServiceDescription, ControllerServiceInterfaceName },
        { ControllerServiceLampDescription, ControllerServiceLampInterfaceName },
        { ControllerServiceLampGroupDescription, ControllerServiceLampGroupInterfaceName },
        { ControllerServicePresetDescription, ControllerServicePresetInterfaceName },
        { ControllerServiceSceneDescription, ControllerServiceSceneInterfaceName },
        { ControllerServiceMasterSceneDescription, ControllerServiceMasterSceneInterfaceName }
    };

    status = CreateAndAddInterfaces(interfaceEntries, sizeof(interfaceEntries) / sizeof(InterfaceEntry));
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to CreateAndAddInterfaces", __func__));
        return status;
    }

    const InterfaceDescription* controllerServiceInterface = bus.GetInterface(ControllerServiceInterfaceName);
    const InterfaceDescription* controllerServiceLampInterface = bus.GetInterface(ControllerServiceLampInterfaceName);
    const InterfaceDescription* controllerServiceLampGroupInterface = bus.GetInterface(ControllerServiceLampGroupInterfaceName);
    const InterfaceDescription* controllerServicePresetInterface = bus.GetInterface(ControllerServicePresetInterfaceName);
    const InterfaceDescription* controllerServiceSceneInterface = bus.GetInterface(ControllerServiceSceneInterfaceName);
    const InterfaceDescription* controllerServiceMasterSceneInterface = bus.GetInterface(ControllerServiceMasterSceneInterfaceName);

    /*
     * Add method handlers for the various Controller Service interface methods
     */
    const MethodEntry methodEntries[] = {
        { controllerServiceInterface->GetMember("LightingResetControllerService"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceInterface->GetMember("GetControllerServiceVersion"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("GetAllLampIDs"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("GetLampSupportedLanguages"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("GetLampManufacturer"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("GetLampName"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("SetLampName"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("GetLampDetails"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("GetLampParameters"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("GetLampParametersField"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("GetLampState"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("GetLampStateField"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("TransitionLampState"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("PulseLampWithState"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("PulseLampWithPreset"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("TransitionLampStateToPreset"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("TransitionLampStateField"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("ResetLampState"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("ResetLampStateField"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("GetLampFaults"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("ClearLampFault"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampInterface->GetMember("GetLampServiceVersion"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("GetAllLampGroupIDs"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("GetLampGroupName"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("SetLampGroupName"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("CreateLampGroup"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("UpdateLampGroup"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("DeleteLampGroup"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("GetLampGroup"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("TransitionLampGroupState"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("PulseLampGroupWithState"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("PulseLampGroupWithPreset"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("TransitionLampGroupStateToPreset"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("TransitionLampGroupStateField"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("ResetLampGroupState"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceLampGroupInterface->GetMember("ResetLampGroupStateField"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServicePresetInterface->GetMember("GetDefaultLampState"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServicePresetInterface->GetMember("SetDefaultLampState"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServicePresetInterface->GetMember("GetAllPresetIDs"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServicePresetInterface->GetMember("GetPresetName"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServicePresetInterface->GetMember("SetPresetName"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServicePresetInterface->GetMember("CreatePreset"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServicePresetInterface->GetMember("UpdatePreset"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServicePresetInterface->GetMember("DeletePreset"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServicePresetInterface->GetMember("GetPreset"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceSceneInterface->GetMember("GetAllSceneIDs"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceSceneInterface->GetMember("GetSceneName"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceSceneInterface->GetMember("SetSceneName"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceSceneInterface->GetMember("CreateScene"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceSceneInterface->GetMember("UpdateScene"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceSceneInterface->GetMember("DeleteScene"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceSceneInterface->GetMember("GetScene"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceSceneInterface->GetMember("ApplyScene"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("GetAllMasterSceneIDs"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("GetMasterSceneName"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("SetMasterSceneName"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("CreateMasterScene"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("UpdateMasterScene"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("DeleteMasterScene"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("GetMasterScene"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
        { controllerServiceMasterSceneInterface->GetMember("ApplyMasterScene"), static_cast<MessageReceiver::MethodHandler>(&ControllerService::MethodCallDispatcher) },
    };
    status = AddMethodHandlers(methodEntries, sizeof(methodEntries) / sizeof(MethodEntry));
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to AddMethodHandlers", __func__));
        return status;
    }

    status = fileWriterThread.Start();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to start file writer thread", __func__));
        return status;
    }

    /*
     * Initialize About
     */
    services::AboutServiceApi::Init(bus, propertyStore);
    aboutService = services::AboutServiceApi::getInstance();
    if (aboutService) {
        std::vector<qcc::String> ifaces;
        ifaces.push_back(qcc::String(ControllerServiceInterfaceName));
        ifaces.push_back(qcc::String(ControllerServiceLampInterfaceName));
        ifaces.push_back(qcc::String(ControllerServiceLampGroupInterfaceName));
        ifaces.push_back(qcc::String(ControllerServicePresetInterfaceName));
        ifaces.push_back(qcc::String(ControllerServiceSceneInterfaceName));
        ifaces.push_back(qcc::String(ControllerServiceMasterSceneInterfaceName));
        aboutService->AddObjectDescription(ControllerServiceObjectPath, ifaces);

        ifaces.clear();
        ifaces.push_back(AboutIconInterfaceName);
        aboutService->AddObjectDescription(AboutIconObjectPath, ifaces);

        ifaces.clear();
        ifaces.push_back(ConfigServiceInterfaceName);
        aboutService->AddObjectDescription(ConfigServiceObjectPath, ifaces);

        status = aboutService->Register(ControllerServiceSessionPort);
        if (status != ER_OK) {
            QCC_LogError(status, ("%s: Failed to AddMethodHandlers", __func__));
            return status;
        }
    } else {
        status = ER_FAIL;
        QCC_LogError(status, ("%s: Failed to initialize About", __func__));
        return status;
    }

    Initialize();

    /*
     * Register the Config Service on the AllJoyn bus
     */

    status = configService.Register();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: ConfigService::Register() failed", __func__));
        return status;
    }

    status = bus.RegisterBusObject(configService);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to register Config Service on the AllJoyn Bus", __func__));
        return status;
    }

    /*
     * Register the About Service on the AllJoyn bus
     */
    status = bus.RegisterBusObject(*aboutService);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to register About Service on the AllJoyn Bus", __func__));
        return status;
    }

    status = aboutIconService.Register();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: AboutIconService::Register() failed", __func__));
        return status;
    }

    status = bus.RegisterBusObject(aboutIconService);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to register About Icon Service on the AllJoyn Bus", __func__));
        return status;
    }

    status = elector.Start();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to start LeaderElection object", __func__));
        return status;
    }

    notificationSender = services::NotificationService::getInstance()->initSend(&bus, &propertyStore);

    /*
     * Register the BusObject for the Controller Service
     */
    status = bus.RegisterBusObject(*this);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to register BusObject for the Controller Service", __func__));
        return status;
    }

    /*
     * Start the Lamp Manager
     */
    status = lampManager.Start(keyStoreFileLocation);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to start the LampManager", __func__));
    }

    status = services::AnnouncementRegistrar::RegisterAnnounceHandler(bus, *listener, OnboardingInterfaces, 2);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to start the register Announce Handler", __func__));
    }

    return status;
}

QStatus ControllerService::Stop(void)
{
    QCC_DbgPrintf(("%s", __func__));

    isRunningLock.Lock();
    isRunning = false;
    isRunningLock.Unlock();

    QStatus status = elector.Stop();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to stop the Leader Elector object", __func__));
    }

    lampManager.Stop();

    fileWriterThread.Stop();

    internalPropertyStore.Stop();

    if (bus.IsConnected()) {
        status = bus.Disconnect();
        if (status != ER_OK) {
            QCC_LogError(status, ("%s: Error disconnecting from the AllJoyn bus", __func__));
        }
    }

    status = bus.Stop();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Error stopping the AllJoyn bus", __func__));
    }

    return status;
}

QStatus ControllerService::Join(void)
{
    QStatus status = elector.Join();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to join the Leader Elector object", __func__));
    }

    lampManager.Join();

    fileWriterThread.Join();

    internalPropertyStore.Join();

    status = services::AnnouncementRegistrar::UnRegisterAnnounceHandler(bus, *listener, OnboardingInterfaces, 2);
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Failed to start the unregister Announce Handler", __func__));
    }

    if (aboutService) {
        aboutService->Unregister();
        services::AboutServiceApi::DestroyInstance();
    }

    // we need to manage the notification sender's memory
    if (notificationSender) {
        services::NotificationService::getInstance()->shutdownSender();
    }

    bus.UnregisterBusListener(*listener);

    bus.UnregisterBusObject(*this);

    status = bus.Join();
    if (status != ER_OK) {
        QCC_LogError(status, ("%s: Error joining the AllJoyn bus", __func__));
    }

    return status;
}

QStatus ControllerService::SendSignal(const char* ifaceName, const char* signalName, LSFStringList& idList)
{
    QCC_DbgTrace(("%s:ifaceName=%s signalName=%s", __func__, ifaceName, signalName));
    QStatus status = ER_BUS_NO_SESSION;

    size_t arraySize = idList.size();
    MsgArg arg;
    if (arraySize) {
        const char** ids = new const char*[arraySize];
        size_t i = 0;
        for (LSFStringList::const_iterator it = idList.begin(); it != idList.end(); ++it, ++i) {
            ids[i] = it->c_str();
        }
        arg.Set("as", arraySize, ids);
        arg.SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
    }

    serviceSessionMutex.Lock();
    if (serviceSession != 0) {
        status = Signal(NULL, serviceSession, *(bus.GetInterface(ifaceName)->GetMember(signalName)), &arg, ALLJOYN_FLAG_NO_REPLY_EXPECTED);
    }
    serviceSessionMutex.Unlock();

    if (ER_OK == status) {
        QCC_DbgPrintf(("%s: Successfully sent signal with %d entries", __func__, arraySize));
    } else {
        QCC_LogError(status, ("%s: Failed to send signal", __func__));
    }

    return status;
}

QStatus ControllerService::SendSignalWithoutArg(const char* ifaceName, const char* signalName)
{
    QCC_DbgTrace(("%s:ifaceName=%s signalName=%s", __func__, ifaceName, signalName));
    QStatus status = ER_BUS_NO_SESSION;

    serviceSessionMutex.Lock();
    if (serviceSession != 0) {
        status = Signal(NULL, serviceSession, *(bus.GetInterface(ifaceName)->GetMember(signalName)), NULL, ALLJOYN_FLAG_NO_REPLY_EXPECTED);
    }
    serviceSessionMutex.Unlock();

    if (ER_OK == status) {
        QCC_DbgPrintf(("%s: Successfully sent signal", __func__));
    } else {
        QCC_LogError(status, ("%s: Failed to send signal", __func__));
    }

    return status;
}

void ControllerService::ObjectRegistered(void)
{
    QCC_DbgPrintf(("Registered!\n"));

    SessionOpts opts;
    opts.isMultipoint = true;
    QStatus status = bus.BindSessionPort(ControllerServiceSessionPort, opts, *listener);
    QCC_DbgPrintf(("BindSessionPort: %s\n", QCC_StatusText(status)));

    status = aboutService->Announce();
    QCC_DbgPrintf(("AboutService::Announce: %s\n", QCC_StatusText(status)));

    firstAnnouncementSent = true;
}

QStatus ControllerService::Restart()
{
    QCC_DbgPrintf(("Restarting\n"));
    isRunningLock.Lock();
    isRunning = false;
    isRunningLock.Unlock();
    return ER_OK;
}

QStatus ControllerService::FactoryReset()
{
    QCC_DbgPrintf(("Factory Resetting\n"));

    // TODO: call FactoryReset on the OnboardingService
    obsObjectLock.Lock();
    // if isObsObjectReady == false, the object hasn't yet been introspected
    if (obsObject != NULL && isObsObjectReady) {
        QCC_DbgPrintf(("Calling FactoryReset to Offboard device\n"));
        // no reply; okay to call when locked
        QStatus status = obsObject->MethodCall(ConfigServiceInterfaceName, "FactoryReset", NULL, 0);
        if (status != ER_OK) {
            QCC_LogError(ER_FAIL, ("%s: Could not call FactoryReset on OBS", __func__));
        }
    }
    obsObjectLock.Unlock();

    // reset user data
    internalPropertyStore.Reset();

    // the main thread will shut us down soon
    isRunningLock.Lock();
    isRunning = false;
    isRunningLock.Unlock();
    return ER_OK;
}

QStatus ControllerService::SetPassphrase(const char* daemonRealm, size_t passcodeSize, const char* passcode, ajn::SessionId sessionId)
{
    QCC_DbgTrace(("%s", __func__));
    return ER_OK;
}

void ControllerService::Overthrow()
{
    bus.EnableConcurrentCallbacks();

    // we don't want to connect to new lamps that come up
    SetIsLeader(false);

    // we're not the leader anymore, so start disconnecting from lamps
    lampManager.DisconnectFromLamps();

    // we are being overthrown!
    serviceSessionMutex.Lock();
    const SessionId sessionId = serviceSession;
    serviceSessionMutex.Unlock();

    if (sessionId) {
        bus.LeaveSession(sessionId);
    }
}

void ControllerService::SessionJoined(SessionId sessionId)
{
    QCC_DbgTrace(("%s:sessionId-%d", __func__, sessionId));
    bus.SetSessionListener(sessionId, listener);

    // we are now serving up a multipoint session to the apps
    serviceSessionMutex.Lock();
    serviceSession = sessionId;
    serviceSessionMutex.Unlock();
}

void ControllerService::SessionLost(SessionId sessionId)
{
    // TODO: do we need to track multiple sessions?
    // Or are we ok since there is only one multipoint session?
    QCC_DbgPrintf(("%s:%u", __func__, sessionId));
    serviceSessionMutex.Lock();
    serviceSession = 0;
    serviceSessionMutex.Unlock();
}

void ControllerService::LeaveSession(ajn::SessionId sessionId)
{
    QCC_DbgTrace(("%s:sessionId=%u", __func__, sessionId));
    bus.EnableConcurrentCallbacks();
    bus.LeaveSession(sessionId);
}

void ControllerService::LightingResetControllerService(Message& msg)
{
    QCC_DbgPrintf(("%s:%s", __func__, msg->ToString().c_str()));

    LSFResponseCode responseCode = LSF_OK;
    uint8_t failure = 0;
    uint8_t numResets = 0;

    if (LSF_OK != presetManager.Reset()) {
        failure++;
    }
    numResets++;

    if (LSF_OK != presetManager.ResetDefaultState()) {
        failure++;
    }
    numResets++;

    if (LSF_OK != lampGroupManager.Reset()) {
        failure++;
    }
    numResets++;

    if (LSF_OK != sceneManager.Reset()) {
        failure++;
    }
    numResets++;

    if (LSF_OK != masterSceneManager.Reset()) {
        failure++;
    }
    numResets++;

    if (failure) {
        if (failure == numResets) {
            responseCode = LSF_ERR_FAILURE;
        } else {
            responseCode = LSF_ERR_PARTIAL;
        }
    }

    SendMethodReplyWithUint32Value(msg, (uint32_t &)responseCode);

    if (responseCode != LSF_ERR_FAILURE) {
        SendSignalWithoutArg(ControllerServiceInterfaceName, "ControllerServiceLightingReset");
    }
}

void ControllerService::GetControllerServiceVersion(Message& msg)
{
    QCC_DbgPrintf(("%s:%s", __func__, msg->ToString().c_str()));
    uint32_t version = CONTROLLER_SERVICE_VERSION;
    SendMethodReplyWithUint32Value(msg, version);
}

void ControllerService::MethodCallDispatcher(const InterfaceDescription::Member* member, Message& msg)
{
    bus.EnableConcurrentCallbacks();

    QCC_DbgPrintf(("%s: Received Method call %s from interface %s", __func__, msg->GetMemberName(), msg->GetInterface()));

    DispatcherMap::iterator it = messageHandlers.find(msg->GetMemberName());
    if (it != messageHandlers.end()) {
        MethodHandlerBase* handler = it->second;
        handler->Handle(msg);
    } else {
        QCC_LogError(ER_FAIL, ("%s: Could not find handler for method call", __func__));
    }
}

void ControllerService::SendMethodReply(const ajn::Message& msg, const ajn::MsgArg* args, size_t numArgs)
{
    QCC_DbgPrintf(("%s: Method Reply for %s", __func__, msg->GetMemberName()));
    QStatus status = ajn::BusObject::MethodReply(msg, args, numArgs);
    if (status == ER_OK) {
        QCC_DbgPrintf(("Successfully sent the reply"));
    } else {
        QCC_LogError(status, ("Error sending reply"));
    }
}

void ControllerService::SendMethodReplyWithResponseCodeAndListOfIDs(const ajn::Message& msg, LSFResponseCode& responseCode, LSFStringList& idList)
{
    QCC_DbgPrintf(("%s: Method Reply for %s", __func__, msg->GetMemberName()));

    MsgArg replyArgs[2];

    replyArgs[0].Set("u", responseCode);

    size_t arraySize = idList.size();
    if (arraySize) {
        const char** ids = new const char*[arraySize];
        size_t i = 0;
        for (LSFStringList::const_iterator it = idList.begin(); it != idList.end(); ++it, ++i) {
            ids[i] = it->c_str();
        }

        replyArgs[1].Set("as", arraySize, ids);
        replyArgs[1].SetOwnershipFlags(MsgArg::OwnsData | MsgArg::OwnsArgs);
        QCC_DbgPrintf(("%s: Sending method reply with %d entries", __func__, arraySize));
    } else {
        replyArgs[1].Set("as", 0, NULL);
    }

    QStatus status = ajn::BusObject::MethodReply(msg, replyArgs, sizeof(replyArgs) / sizeof(MsgArg));
    if (status == ER_OK) {
        QCC_DbgPrintf(("Successfully sent the reply"));
    } else {
        QCC_LogError(status, ("Error sending reply"));
    }
}

void ControllerService::SendMethodReplyWithResponseCodeIDAndName(const ajn::Message& msg, LSFResponseCode& responseCode, LSFString& lsfId, LSFString& lsfName)
{
    QCC_DbgPrintf(("%s: Method Reply for %s", __func__, msg->GetMemberName()));

    MsgArg replyArgs[3];

    replyArgs[0].Set("u", responseCode);
    replyArgs[1].Set("s", lsfId.c_str());
    replyArgs[2].Set("s", lsfName.c_str());

    QStatus status = ajn::BusObject::MethodReply(msg, replyArgs, sizeof(replyArgs) / sizeof(MsgArg));
    if (status == ER_OK) {
        QCC_DbgPrintf(("Successfully sent the reply"));
    } else {
        QCC_LogError(status, ("Error sending reply"));
    }
}

void ControllerService::SendMethodReplyWithResponseCodeAndID(const ajn::Message& msg, LSFResponseCode& responseCode, LSFString& lsfId)
{
    QCC_DbgPrintf(("%s: Method Reply for %s", __func__, msg->GetMemberName()));

    MsgArg replyArgs[2];

    replyArgs[0].Set("u", responseCode);
    replyArgs[1].Set("s", lsfId.c_str());

    QStatus status = ajn::BusObject::MethodReply(msg, replyArgs, sizeof(replyArgs) / sizeof(MsgArg));
    if (status == ER_OK) {
        QCC_DbgPrintf(("Successfully sent the reply"));
    } else {
        QCC_LogError(status, ("Error sending reply"));
    }
}

void ControllerService::SendMethodReplyWithUint32Value(const ajn::Message& msg, uint32_t& value)
{
    QCC_DbgPrintf(("%s: Method Reply for %s", __func__, msg->GetMemberName()));

    MsgArg replyArg;
    replyArg.Set("u", value);

    QStatus status = ajn::BusObject::MethodReply(msg, &replyArg, 1);
    if (status == ER_OK) {
        QCC_DbgPrintf(("Successfully sent the reply"));
    } else {
        QCC_LogError(status, ("Error sending reply"));
    }
}

void ControllerService::SendMethodReplyWithResponseCodeIDLanguageAndName(const ajn::Message& msg, LSFResponseCode& responseCode, LSFString& lsfId, LSFString& language, LSFString& name)
{
    QCC_DbgPrintf(("%s: Method Reply for %s", __func__, msg->GetMemberName()));

    MsgArg replyArgs[4];

    replyArgs[0].Set("u", responseCode);
    replyArgs[1].Set("s", lsfId.c_str());
    replyArgs[2].Set("s", language.c_str());
    replyArgs[3].Set("s", name.c_str());

    QStatus status = ajn::BusObject::MethodReply(msg, replyArgs, sizeof(replyArgs) / sizeof(MsgArg));
    if (status == ER_OK) {
        QCC_DbgPrintf(("Successfully sent the reply"));
    } else {
        QCC_LogError(status, ("Error sending reply"));
    }
}

void ControllerService::ScheduleFileReadWrite(Manager* manager)
{
    QCC_DbgTrace(("%s", __func__));
    fileWriterThread.SignalReadWrite();
}

uint32_t ControllerService::GetControllerServiceInterfaceVersion(void)
{
    QCC_DbgPrintf(("%s: controllerInterfaceVersion=%d", __func__, ControllerServiceInterfaceVersion));
    return ControllerServiceInterfaceVersion;
}

QStatus ControllerService::Get(const char*ifcName, const char*propName, MsgArg& val)
{
    QCC_DbgPrintf(("%s", __func__));
    QStatus status = ER_OK;
    // Check the requested property and return the value if it exists
    if (0 == strcmp("Version", propName)) {
        if (0 == strcmp(ifcName, ControllerServiceInterfaceName)) {
            status = val.Set("u", GetControllerServiceInterfaceVersion());
        } else if (0 == strcmp(ifcName, ControllerServiceLampInterfaceName)) {
            status = val.Set("u", lampManager.GetControllerServiceLampInterfaceVersion());
        } else if (0 == strcmp(ifcName, ControllerServiceLampGroupInterfaceName)) {
            status = val.Set("u", lampGroupManager.GetControllerServiceLampGroupInterfaceVersion());
        } else if (0 == strcmp(ifcName, ControllerServicePresetInterfaceName)) {
            status = val.Set("u", presetManager.GetControllerServicePresetInterfaceVersion());
        } else if (0 == strcmp(ifcName, ControllerServiceSceneInterfaceName)) {
            status = val.Set("u", sceneManager.GetControllerServiceSceneInterfaceVersion());
        } else if (0 == strcmp(ifcName, ControllerServiceMasterSceneInterfaceName)) {
            status = val.Set("u", masterSceneManager.GetControllerServiceMasterSceneInterfaceVersion());
        } else if (0 == strcmp(ifcName, LeaderElectionAndStateSyncInterfaceName)) {
            //TODO: Add support to return LSFTypes::LeaderElectionAndStateSyncInterfaceVersion
        } else {
            status = ER_BUS_OBJECT_NO_SUCH_INTERFACE;
        }
    } else {
        status = ER_BUS_NO_SUCH_PROPERTY;
    }

    return status;
}

QStatus ControllerService::SendBlobUpdate(LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp)
{
    QCC_DbgTrace(("%s:type=%d blob=%s checksum=%d timestamp=%llu", __func__, type, blob.c_str(), checksum, timestamp));
    serviceSessionMutex.Lock();
    SessionId session = serviceSession;
    serviceSessionMutex.Unlock();
    return elector.SendBlobUpdate(session, type, blob, checksum, timestamp);
}

void ControllerService::SendGetBlobReply(ajn::Message& message, LSFBlobType type, std::string blob, uint32_t checksum, uint64_t timestamp)
{
    QCC_DbgTrace(("%s:type=%d blob=%s checksum=%d timestamp=%llu", __func__, type, blob.c_str(), checksum, timestamp));
    elector.SendGetBlobReply(message, type, blob, checksum, timestamp);
}

bool ControllerService::IsRunning()
{
    isRunningLock.Lock();
    bool b = isRunning;
    isRunningLock.Unlock();
    return b;
}

QStatus ControllerService::IsConnectedToRouter()
{
    QCC_DbgPrintf(("%s", __func__));
    qcc::String daemonUniqueName = ":" + bus.GetGlobalGUIDShortString() + ".1";
    ajn::Message reply(bus);
    return bus.GetAllJoynProxyObj().MethodCall(
               org::freedesktop::DBus::Peer::InterfaceName,
               "Ping", NULL, 0, reply, 1000);
}

uint64_t ControllerService::GetRank()
{
    return internalPropertyStore.GetRank();
}

bool ControllerService::IsLeader()
{
    return internalPropertyStore.IsLeader();
}

void ControllerService::SetIsLeader(bool val)
{
    internalPropertyStore.SetIsLeader(val);
}

void ControllerService::AddObjDescriptionToAnnouncement(qcc::String path, qcc::String interface)
{
    QCC_DbgPrintf(("%s", __func__));
    std::vector<qcc::String> interfaces;
    interfaces.push_back(interface);
    aboutService->AddObjectDescription(path, interfaces);
    if (firstAnnouncementSent) {
        aboutService->Announce();
    }
}

void ControllerService::RemoveObjDescriptionFromAnnouncement(qcc::String path, qcc::String interface)
{
    QCC_DbgPrintf(("%s", __func__));
    std::vector<qcc::String> interfaces;
    interfaces.push_back(interface);
    aboutService->RemoveObjectDescription(path, interfaces);
    if (firstAnnouncementSent) {
        aboutService->Announce();
    }
}
