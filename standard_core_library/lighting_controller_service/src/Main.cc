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

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <qcc/Debug.h>
#include <string>
#include <ControllerService.h>
#include <fstream>
#include <sstream>
#include <OEM_CS_Config.h>

#define QCC_MODULE "MAIN"

// if we're tracking the child process, we need to pass signals to it
static pid_t g_child_process = 0;
static volatile sig_atomic_t g_running = true;
static volatile sig_atomic_t isRunning = false;

static void SigIntHandler(int sig)
{
    g_running = false;
    if (g_child_process) {
        kill(g_child_process, SIGINT);
    }
}

static void SigTermHandler(int sig)
{
    g_running = false;
    if (g_child_process) {
        kill(g_child_process, SIGTERM);
    }
}

static std::string factoryConfigFile = "OEMConfig.ini";
static std::string configFile = "Config.ini";
static std::string lampGroupFile = "LampGroups.lsf";
static std::string presetFile = "Presets.lsf";
static std::string sceneFile = "Scenes.lsf";
static std::string masterSceneFile = "MasterScenes.lsf";
static std::string storeFile = "LightingControllerService";
static std::string factoryConfigFilePath = factoryConfigFile;
static std::string configFilePath = configFile;
static std::string lampGroupFilePath = lampGroupFile;
static std::string presetFilePath = presetFile;
static std::string sceneFilePath = sceneFile;
static std::string masterSceneFilePath = masterSceneFile;
static std::string storeFilePath = storeFile;
static std::string storeLocation;
static bool runForeground = false;
static bool overrideRank = false;
static uint64_t rank = 0;
static bool disableBackgroundLogging = true;

static void usage(int argc, char** argv)
{
    printf("Usage: %s -h ? -k <absolute_directory_path> -f -r\n\n", argv[0]);
    printf("Options:\n");
    printf("   -h                    = Print this help message\n");
    printf("   -?                    = Print this help message\n");
    printf("   -f                    = Run the Controller Service as a foreground process\n");
    printf("   -k <absolute_directory_path>   = The absolute path to a directory required to store the AllJoyn KeyStore, Persistent Store and read/write the Config FilePaths\n\n");
    printf("   -v                    = Print the version number and exit\n");
    printf("   -r                    = Override the rank for debugging purposes\n");
    printf("   -l                    = Enable background logging\n");
    printf("Default:\n");
    printf("    %s\n", argv[0]);
}

static void parseCommandLine(int argc, char** argv)
{
    /* Parse command line args */
    for (int i = 1; i < argc; ++i) {
        if (0 == strcmp("-h", argv[i]) || 0 == strcmp("-?", argv[i])) {
            usage(argc, argv);
            exit(0);
        } else if (0 == strcmp("-v", argv[i])) {
            printf("Version: %u\n", CONTROLLER_SERVICE_VERSION);
            exit(0);
        } else if (0 == strcmp("-k", argv[i])) {
            ++i;
            if (i == argc) {
                printf("option %s requires a parameter\n", argv[i - 1]);
                usage(argc, argv);
                exit(1);
            } else {
                storeLocation = argv[i];
                char* dirPath = getenv("HOME");
                if (dirPath == NULL) {
                    dirPath = const_cast<char*>("/");
                }
                static std::string absDirPath = std::string(dirPath) + "/" + storeLocation + "/";
                factoryConfigFilePath = absDirPath + factoryConfigFile;
                configFilePath = absDirPath + configFile;
                lampGroupFilePath = absDirPath + lampGroupFile;
                presetFilePath = absDirPath + presetFile;
                sceneFilePath = absDirPath + sceneFile;
                masterSceneFilePath = absDirPath + masterSceneFile;
                storeFilePath = storeLocation + "/" + storeFile;
            }
        } else if (0 == strcmp("-f", argv[i])) {
            runForeground = true;
        } else if (0 == strcmp("-l", argv[i])) {
            disableBackgroundLogging = false;
        } else if (0 == strcmp("-r", argv[i])) {
            ++i;
            if (i == argc) {
                printf("option %s requires a parameter\n", argv[i - 1]);
                usage(argc, argv);
                exit(1);
            } else {
                rank = strtouq(argv[i], NULL, 10);
                overrideRank = true;
            }
        } else {
            printf("Unknown option %s\n", argv[i]);
            usage(argc, argv);
            exit(-1);
        }
    }
}

void lsf_Sleep(uint32_t msec)
{
    usleep(1000 * msec);
}

void RunService(bool listenToInterrupts)
{
    QCC_DbgTrace(("%s", __func__));
    if (!storeLocation.empty()) {
        chdir(storeLocation.c_str());
    }

    if (listenToInterrupts) {
        signal(SIGINT, SigIntHandler);
        signal(SIGTERM, SigTermHandler);
    }

    lsf::ControllerServiceManager* controllerSvcManagerPtr =
        new lsf::ControllerServiceManager(factoryConfigFilePath, configFilePath, lampGroupFilePath, presetFilePath, sceneFilePath, masterSceneFilePath);

    if (controllerSvcManagerPtr == NULL) {
        QCC_LogError(ER_OUT_OF_MEMORY, ("%s: Failed to start the Controller Service Manager", __func__));
        exit(-1);
    }

    if (overrideRank) {
        controllerSvcManagerPtr->OverrideRank(rank);
    }

    QStatus status = controllerSvcManagerPtr->Start(storeLocation.empty() ? NULL : storeFilePath.c_str());

    isRunning = true;

    if (status == ER_OK) {
        while (g_running && controllerSvcManagerPtr->IsRunning()) {
            lsf_Sleep(TIMEOUT_MS_CONNECTED_TO_ROUTING_NODE);
        }
    }

    QCC_LogError(ER_FAIL, ("%s: Failed to talk to bus", __func__));

    if (controllerSvcManagerPtr) {
        controllerSvcManagerPtr->Stop();
        controllerSvcManagerPtr->Join();
        delete controllerSvcManagerPtr;
        controllerSvcManagerPtr = NULL;
        QCC_DbgPrintf(("%s: After delete controllerSvcManagerPtr", __func__));
    }

    isRunning = false;
}


void RunAndMonitor()
{
    // we are a background process!
    if (disableBackgroundLogging) {
        fclose(stdin);
        fclose(stdout);
        fclose(stderr);
    }

    signal(SIGINT, SigIntHandler);
    signal(SIGTERM, SigTermHandler);

    while (g_running) {
        pid_t pid = fork();

        if (pid == -1) {
            // failed to fork!
            exit(-1);
        } else if (pid == 0) {
            QCC_DbgPrintf(("%s: Starting Child", __func__));
            RunService(false);
            break;
        } else {
            g_child_process = pid;
            int status = 0;
            QCC_DbgPrintf(("%s: Child PID %d", __func__, pid));
            // wait for exit
            wait(&status);
            QCC_DbgPrintf(("%s: Exited child PID %d", __func__, pid));
            lsf_Sleep(1000);
        }
    }
}

int main(int argc, char** argv)
{
    QCC_DbgTrace(("%s", __func__));

    parseCommandLine(argc, argv);

    if (runForeground) {
        QCC_DbgPrintf(("%s: Running in foreground", __func__));
        RunService(true);
    } else {
        QCC_DbgPrintf(("%s: Running in background", __func__));
        QCC_LogError(ER_OK, ("%s: You are running Controller Service in the default background mode. To debug, start Controller Service with the -f option", __func__));
        pid_t pid = fork();

        if (pid == -1) {
            return -1;
        } else if (pid == 0) {
            QCC_DbgPrintf(("%s: Starting Monitor", __func__));
            RunAndMonitor();
            return 0;
        } else {
            // Unneeded parent process, just exit.
            QCC_DbgPrintf(("%s: Monitor PID %d", __func__, pid));
            QCC_DbgPrintf(("%s: Exiting Main", __func__));
            return 0;
        }
    }

    return 0;
}
