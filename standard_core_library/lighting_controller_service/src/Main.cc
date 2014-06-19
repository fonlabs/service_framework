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
#include <climits>
#include <signal.h>
#include <unistd.h>
#include <qcc/Debug.h>
#include <string>
#include <ControllerService.h>

#define QCC_MODULE "MAIN"

static volatile sig_atomic_t g_interrupt = false;

static void SigIntHandler(int sig)
{
    g_interrupt = true;
}

static std::string factoryConfigFile = "OEMConfig.ini";
static std::string configFile = "Config.ini";
static std::string lampGroupFile = "LampGroups.lmp";
static std::string presetFile = "Presets.lmp";
static std::string sceneFile = "Scenes.lmp";
static std::string masterSceneFile = "MasterScenes.lmp";
static std::string storeFileName = "LightingControllerService";
static std::string storeLocation;
bool storeLocationSpecified = false;

static void usage(int argc, char** argv)
{
    printf("Usage: %s -h ? -k <absolute_directory_path>\n\n", argv[0]);
    printf("Options:\n");
    printf("   -h                    = Print this help message\n");
    printf("   -?                    = Print this help message\n");
    printf("   -k <absolute_directory_path>   = The absolute path to a directory required to store the AllJoyn KeyStore, Persistent Store and read/write the Config files.\n\n");
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
        } else if (0 == strcmp("-k", argv[i])) {
            ++i;
            if (i == argc) {
                printf("option %s requires a parameter\n", argv[i - 1]);
                usage(argc, argv);
                exit(1);
            } else {
                storeLocationSpecified = true;
                storeLocation = argv[i];
            }
        } else {
            printf("Unknown option %s\n", argv[i]);
            usage(argc, argv);
            exit(1);
        }
    }
}

void lsf_Sleep(uint32_t msec)
{
    usleep(1000 * msec);
}

int main(int argc, char** argv)
{
    signal(SIGINT, SigIntHandler);

    parseCommandLine(argc, argv);

    lsf::ControllerServiceManager* controllerSvcManagerPtr = NULL;

    if (storeLocationSpecified) {
        char* dirPath = getenv("HOME");
        if (dirPath == NULL) {
            dirPath = const_cast<char*>("/");
        }

        static std::string absDirPath = std::string(dirPath) + "/" + storeLocation + "/";
        static std::string factoryConfigFilePath = absDirPath + factoryConfigFile;
        static std::string configFilePath = absDirPath + configFile;
        static std::string lampGroupFilePath = absDirPath + lampGroupFile;
        static std::string presetFilePath = absDirPath + presetFile;
        static std::string sceneFilePath = absDirPath + sceneFile;
        static std::string masterSceneFilePath = absDirPath + masterSceneFile;
        static std::string storeFilePath = storeLocation + "/" + storeFileName;
        controllerSvcManagerPtr = new lsf::ControllerServiceManager(factoryConfigFilePath, configFilePath, lampGroupFilePath, presetFilePath, sceneFilePath, masterSceneFilePath);
        if (controllerSvcManagerPtr) {
            controllerSvcManagerPtr->Start(storeFilePath.c_str());
        }
    } else {
        controllerSvcManagerPtr = new lsf::ControllerServiceManager(factoryConfigFile, configFile, lampGroupFile, presetFile, sceneFile, masterSceneFile);
        if (controllerSvcManagerPtr) {
            controllerSvcManagerPtr->Start(NULL);
        }
    }

    if (controllerSvcManagerPtr == NULL) {
        QCC_LogError(ER_OUT_OF_MEMORY, ("%s: Failed to start the Controller Service Manager", __FUNCTION__));
        g_interrupt = true;
    }

    while (g_interrupt == false) {
        lsf_Sleep(1000);
    }

    if (controllerSvcManagerPtr) {
        controllerSvcManagerPtr->Stop();
        delete controllerSvcManagerPtr;
    }
}
