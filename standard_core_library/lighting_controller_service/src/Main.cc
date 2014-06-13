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
#include <Main.h>
#include <qcc/Debug.h>

#define QCC_MODULE "MAIN"

static volatile sig_atomic_t g_interrupt = false;

lsf::ControllerServiceManager* controllerSvcManagerPtr = NULL;

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
static std::string keyStoreLocation;
bool keyStoreLocationSpecified = false;

static void usage(int argc, char** argv)
{
    printf("Usage: %s -h ? -f <OEM Config File> -c <config> -l <file_name>  -p <file_name>  -s <file_name>  -m <file_name> -k <directory_name>\n\n", argv[0]);
    printf("Options:\n");
    printf("   -h                    = Print this help message\n");
    printf("   -?                    = Print this help message\n");
    printf("   -f <OEM Config File>  = The file containing the default OEM configuration\n");
    printf("   -c <config>           = The file where the configuration will be saved\n");
    printf("   -l <file_name>        = The file where the Lamp Groups will be saved\n");
    printf("   -p <file_name>        = The file where the Presets will be saved\n");
    printf("   -s <file_name>        = The file where the Scenes will be saved\n");
    printf("   -m <file_name>        = The file where the Master Scenes will be saved\n\n");
    printf("   -k <directory_name>   = The location to store the AllJoyn KeyStore. On Android this should be set to the value returned by Context.getFileStreamPath(\"alljoyn_keystore\").getAbsolutePath()\n\n");
    printf("Default:\n");
    printf("    %s -f %s -c %s -l %s -p %s -s %s -m %s -k %s\n",
           argv[0],
           factoryConfigFile.c_str(), configFile.c_str(),
           lampGroupFile.c_str(), presetFile.c_str(), sceneFile.c_str(), masterSceneFile.c_str(), keyStoreLocation.c_str());
}


static void parseCommandLine(int argc, char** argv)
{
    int c;
    while ((c = getopt(argc, argv, "f:c:l:p:s:m:h?k:")) != -1) {
        switch (c) {
        case 'f':
            factoryConfigFile = optarg;
            break;

        case 'c':
            configFile = optarg;
            break;

        case 'l':
            lampGroupFile = optarg;
            break;

        case 'p':
            presetFile = optarg;
            break;

        case 's':
            sceneFile = optarg;
            break;

        case 'm':
            masterSceneFile = optarg;
            break;

        case 'k':
            keyStoreLocationSpecified = true;
            keyStoreLocation = optarg;
            break;

        default:
            usage(argc, argv);
            exit(-1);

        case 'h':
        case '?':
            usage(argc, argv);
            exit(0);
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

    lsf::ControllerServiceManager controllerSvcManager(factoryConfigFile, configFile, lampGroupFile, presetFile, sceneFile, masterSceneFile);

    controllerSvcManagerPtr = &controllerSvcManager;

    if (keyStoreLocationSpecified) {
        controllerSvcManager.Start(keyStoreLocation.c_str());
    } else {
        controllerSvcManager.Start(NULL);
    }

    while (g_interrupt == false) {
        lsf_Sleep(1000);
    }

    controllerSvcManager.Stop();

    controllerSvcManagerPtr = NULL;
}
