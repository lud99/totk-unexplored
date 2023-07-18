#include <fstream>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <thread>
#include <chrono>
#include <dirent.h>

#include <switch.h>

#include "SavefileIO.h"
#include "Graphics/InitGL.h"
#include "Graphics/Font.h"
#include "Map.h"
#include "Accounts.h"
#include "Dialog.h"
#include "Legend.h"
#include "Log.h"
#include "MapObject.h"

bool openGLInitialized = false;
bool nxLinkInitialized = false;
int s_nxlinkSock = -1;

static void deinitNxLink()
{
    if (s_nxlinkSock >= 0)
    {
        close(s_nxlinkSock);
        socketExit();
        s_nxlinkSock = -1;
    }
}

void cleanUp()
{
    // Save settings
    if (!SavefileIO::Get().DirectoryExists("sdmc:/switch/totk-unexplored"))
        mkdir("sdmc:/switch/totk-unexplored", 0777);

    std::ofstream file("sdmc:/switch/totk-unexplored/settings.txt");
    if (file.is_open())
    {
        file << Map::m_CameraPosition.x << "\n";
        file << Map::m_CameraPosition.y << "\n";
        file << Map::m_Zoom << "\n";
        file << (int)Map::m_Legend->m_IsOpen << "\n";
        file << (int)Map::m_CurrentLayer << "\n";
        Log("Saved settings");
    }
    else
        Log("Failed top open settings file (cleanUp())");
    file.close();

    // Save marked koroks
    if (SavefileIO::Get().GameIsRunning)
    {
        std::ofstream koroksFile("sdmc:/switch/totk-unexplored/koroks.txt");
        if (koroksFile.is_open())
        {

            for (auto& entry : Map::m_MapObjects[Data::ObjectType::HiddenKorok])
                koroksFile << (int)entry.m_Found << "\n";
            for (auto& entry : Map::m_MapObjects[Data::ObjectType::CarryKorok])
                koroksFile << (int)entry.m_Found << "\n";

            Log("Saved manually marked koroks");
        }
        else
            Log("Failed to open koroks file (cleanUp())");

        koroksFile.close();
    }
    else
    {
        // Else delete the file so only the actually found koroks are displayed
        if (remove("sdmc:/switch/totk-unexplored/koroks.txt") != 0)
            Log("Couldn't delete koroks.txt. It probably doesn't exist");
        else
            Log("koroks.txt successfully deleted to avoid desync");
    }

    Map::Destory();

    // Cleanup
    romfsExit();

    // Deinitialize EGL
    deinitEgl();

    // Deinitialize network
    deinitNxLink();
    socketExit();
}

int main()
{
    LogInit();

    // To be able to run memory cleanup when the app closes
    if (R_FAILED(appletLockExit()))
        Log("appletLockExit() failed");

    // Setup NXLink
    socketInitializeDefault();
    s_nxlinkSock = nxlinkStdio();
    nxLinkInitialized = true;

    // Init romfs
    if (R_FAILED(romfsInit()))
        Log("romfsInit() failed");

    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // OpenGL
    // Initialize EGL on the default window
    openGLInitialized = initEgl(nwindowGetDefault());
    if (!openGLInitialized)
    {
        Log("OpenGL Failed to initialize");
        return 0;
    }

    // Load OpenGL routines using glad
    gladLoadGL();

    // OpenGL config
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    PadState pad;
    padInitializeDefault(&pad);

    Map::Init();
    Map::m_Pad = &pad;

    // Load settings if they exist
    std::ifstream settingsFile("sdmc:/switch/totk-unexplored/settings.txt");
    if (settingsFile.is_open())
    {
        std::string line;

        std::getline(settingsFile, line);
        Map::m_CameraPosition.x = std::stof(line);

        std::getline(settingsFile, line);
        Map::m_CameraPosition.y = std::stof(line);

        std::getline(settingsFile, line);
        Map::m_Zoom = std::stof(line);

        std::getline(settingsFile, line);
        Map::m_Legend->m_IsOpen = (bool)std::stoi(line);

        std::getline(settingsFile, line);
        int layer = std::stoi(line);
        if (layer < 0) layer = 0;
        if (layer > 2) layer = 2;
        Map::m_CurrentLayer = (Map::Layers)layer;

        if (Map::m_CameraPosition.x > 1000.0f)
            Map::m_CameraPosition.x = 1000.0f;
        if (Map::m_CameraPosition.x < -1000.0f)
            Map::m_CameraPosition.x = -1000.0f;
        if (Map::m_CameraPosition.y > 1000.0f)
            Map::m_CameraPosition.y = 1000.0f;
        if (Map::m_CameraPosition.y < -1000.0f)
            Map::m_CameraPosition.y = -1000.0f;
    }
    else
    {
        Log("Failed to open settings file");
    }

    settingsFile.close();

    bool hasDoneFirstDraw = false;
    bool hasLoadedSave = false;

    while (appletMainLoop())
    {
        // Scan the gamepad. This should be done once for each frame
        padUpdate(&pad);

        u64 buttonsDown = padGetButtonsDown(&pad);

        if (buttonsDown & HidNpadButton_Plus)
            break;

        if (Map::m_ShouldExit)
            break;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Update
        Map::Update();

        // Render
        Map::Render();

        if (hasDoneFirstDraw && !hasLoadedSave)
        {
            std::cout << "will load save\n";
            Log("LoadGamesave() status:", SavefileIO::Get().LoadGamesave() ? "true" : "false");

            hasLoadedSave = true;

            std::cout << "Update map objects first time\n";
            Map::UpdateMapObjects();

            // Load manually checked koroks (but only if the game is running)
            // Override the checking from the savefile
            if (SavefileIO::Get().GameIsRunning)
            {
                std::ifstream koroksFile("sdmc:/switch/totk-unexplored/koroks.txt");
                if (koroksFile.is_open())
                {
                    for (auto& entry : Map::m_MapObjects[Data::ObjectType::HiddenKorok])
                    {
                        std::string line;
                        std::getline(koroksFile, line);
                        entry.m_Found = (bool)std::stoi(line);
                    }
                    for (auto& entry : Map::m_MapObjects[Data::ObjectType::CarryKorok])
                    {
                        std::string line;
                        std::getline(koroksFile, line);
                        entry.m_Found = (bool)std::stoi(line);
                    }

                    Log("Loaded manually marked koroks");
                }
                else
                    Log("Failed to open koroks file (init())");

                koroksFile.close();
            }
        }

        eglSwapBuffers(s_display, s_surface);

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            Log("OpenGL error", (int)err);
        }

        hasDoneFirstDraw = true;
    }

    Log("Exiting...");

    cleanUp();

    appletUnlockExit(); // Exit the app

    return EXIT_SUCCESS;
}