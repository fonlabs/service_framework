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

#define QCC_MODULE "MAIN"

// if we're tracking the child process, we need to pass signals to it
static pid_t g_child_process = 0;


static volatile sig_atomic_t g_running = true;
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

static std::string obsConfigFile = "";
static std::string factoryConfigFile = "OEMConfig.ini";
static std::string configFile = "Config.ini";
static std::string lampGroupFile = "LampGroups.lsf";
static std::string presetFile = "Presets.lsf";
static std::string sceneFile = "Scenes.lsf";
static std::string masterSceneFile = "MasterScenes.lsf";
static std::string storeFileName = "LightingControllerService";
static std::string storeLocation;
static bool runDaemon = false;

static void usage(int argc, char** argv)
{
    printf("Usage: %s -h ? -k <absolute_directory_path> -d -o <Full path to the Onboarding config file required on OpenWRT>\n\n", argv[0]);
    printf("Options:\n");
    printf("   -h                    = Print this help message\n");
    printf("   -?                    = Print this help message\n");
    printf("   -d                    = Run the Controller Service as a background daemon\n");
    printf("   -k <absolute_directory_path>   = The absolute path to a directory required to store the AllJoyn KeyStore, Persistent Store and read/write the Config files.\n\n");
    printf("   -o <file>             = Full path to the Onboarding config file (needed for DeviceId)\n");
    printf("Default:\n");
    printf("    %s\n", argv[0]);
}


static void parseCommandLine(int argc, char** argv)
{
    int opt;

    while ((opt = getopt(argc, argv, "h?k:do:")) != -1) {
        switch (opt) {
        case 'h':
        case '?':
            usage(argc, argv);
            exit(0);
            break;

        case 'd':
            runDaemon = true;
            break;

        case 'k':
            storeLocation = optarg;
            break;

        case 'o':
            obsConfigFile = optarg;
            break;

        default:
            usage(argc, argv);
            exit(-1);
            break;
        }
    }
}

void lsf_Sleep(uint32_t msec)
{
    usleep(1000 * msec);
}


void RunService()
{
    QCC_DbgTrace(("%s", __func__));
    if (!storeLocation.empty()) {
        chdir(storeLocation.c_str());
    }

    lsf::ControllerServiceManager* controllerSvcManagerPtr =
        new lsf::ControllerServiceManager(obsConfigFile, factoryConfigFile, configFile, lampGroupFile, presetFile, sceneFile, masterSceneFile);
    controllerSvcManagerPtr->Start(storeLocation.empty() ? NULL : storeLocation.c_str());

    if (controllerSvcManagerPtr == NULL) {
        QCC_LogError(ER_OUT_OF_MEMORY, ("%s: Failed to start the Controller Service Manager", __func__));
        exit(-1);
    }

    while (g_running && controllerSvcManagerPtr->IsRunning()) {
        lsf_Sleep(1000);
    }

    if (controllerSvcManagerPtr) {
        controllerSvcManagerPtr->Stop();
        delete controllerSvcManagerPtr;
    }
}


void RunAndMonitor()
{
    // we are a background process!
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);

    while (g_running) {
        pid_t pid = fork();

        if (pid == -1) {
            // failed to fork!
            exit(-1);
        } else if (pid == 0) {
            RunService();
        } else {
            g_child_process = pid;
            int status = 0;
            // wait for exit
            wait(&status);
        }
    }
}


int main(int argc, char** argv)
{
    QCC_DbgTrace(("%s", __func__));
    signal(SIGINT, SigIntHandler);
    signal(SIGTERM, SigTermHandler);

    parseCommandLine(argc, argv);

#ifdef _OPEN_WRT_
    if (obsConfigFile.empty()) {
        printf("OBS Config File must be specified\n");
        return -1;
    }
#endif

    if (runDaemon) {
        pid_t pid = fork();

        if (pid == -1) {
            return -1;
        } else if (pid == 0) {
            RunAndMonitor();
        } else {
            // Unneeded parent process, just exit.
            return 0;
        }
    } else {
        RunService();
    }
}
