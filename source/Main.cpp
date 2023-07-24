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
#include "UI/LayerNavigation.h"

#define SETTINGS_VERSION "2.1.0"

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
        file << SETTINGS_VERSION << "\n";
        file << Map::m_CameraPosition.x << "\n";
        file << Map::m_CameraPosition.y << "\n";
        file << Map::m_Zoom << "\n";
        file << (int)Map::m_Legend->m_IsOpen << "\n";
        file << (int)Map::m_LayerNavigation->GetLayer() << "\n";

        // Selected legend buttons
        file << Map::m_Legend->m_HighlightedButton << "\n";
        file << (int)Map::m_Legend->m_ShowLevel << "\n";
        file << Map::m_Legend->m_Page << "\n"; // Legend page
        for (int j = 0; j < (int)Map::m_Legend->m_Buttons.size(); j++)
        {
            for (int i = 0; i < (int)Map::m_Legend->m_Buttons[j].size(); i++)
            {
                auto button = Map::m_Legend->m_Buttons[j][i];
                if (button->m_Type == IconButton::ShowCompleted)
                    continue;

                file << button->m_IsToggled << "\n";
            }
        }

        Log("Saved settings");
    }
    else
        Log("Failed top open settings file (cleanUp())");
    file.close();

    // Save manually marked collectibles
    if (SavefileIO::Get().GameIsRunning)
    {
        std::ofstream koroksFile("sdmc:/switch/totk-unexplored/found.txt");
        if (koroksFile.is_open())
        {
            for (int i = 0; i < (int)Data::ObjectType::Count; i++)
            {
                Data::ObjectType type = Data::ObjectType(i);
                int objectCount = Data::m_Objects[type].size();
                for (int j = 0; j < objectCount; j++)
                {
                    koroksFile << Map::m_MapObjects[type][j].m_Found << "\n";
                }
            }

            Log("Saved manually marked collectibles");
        }
        else
            Log("Failed to open found file (cleanUp())");

        koroksFile.close();
    }
    else
    {
        // Else delete the file so only the actually found koroks are displayed
        if (remove("sdmc:/switch/totk-unexplored/found.txt") != 0)
            Log("Couldn't delete found.txt. It probably doesn't exist");
        else
            Log("foound.txt successfully deleted to avoid desync");
    }

    Map::Destory();
    Data::UnloadData();

    // Cleanup
    romfsExit();

    // Deinitialize EGL
    deinitEgl();

    // Deinitialize network
    deinitNxLink();
    socketExit();
}

void LoadSettings()
{
    // Load settings if they exist
    std::ifstream settingsFile("sdmc:/switch/totk-unexplored/settings.txt");
    if (settingsFile.is_open())
    {
        std::string line;

        std::getline(settingsFile, line);
        if (line != SETTINGS_VERSION)
        {
            // Settings saved in older version
            settingsFile.close();
            remove("sdmc:/switch/totk-unexplored/settings.txt");
            Log("Old settings detected, removing them");

            return;
        }

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

        Map::m_LayerNavigation->SetLayer((Layer)layer);

        // Selected legend buttons
        std::getline(settingsFile, line);
        Map::m_Legend->m_HighlightedButton = std::stoi(line); // highlighted button

        std::getline(settingsFile, line);
        Map::m_Legend->m_ShowLevel = (Legend::ShowLevel)std::stoi(line); // show level
        Map::m_Legend->m_Buttons[0].back()->Click(Map::m_Legend, true, (int)Map::m_Legend->m_ShowLevel); // Click the showCompleted button 

        std::getline(settingsFile, line);
        Map::m_Legend->m_Page = std::stoi(line); // Legend page
        for (int j = 0; j < (int)Map::m_Legend->m_Buttons.size(); j++)
        {
            for (int i = 0; i < (int)Map::m_Legend->m_Buttons[j].size(); i++)
            {
                auto button = Map::m_Legend->m_Buttons[j][i];
                if (button->m_Type == IconButton::ShowCompleted)
                    continue;

                std::getline(settingsFile, line);
                bool clicked = std::stoi(line);
                button->Click(Map::m_Legend, clicked);
            }
        }

        Map::m_Legend->UpdateSelectedButton();

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

    std::cout << "Before map init\n";

    Map::Init();
    Map::m_Pad = &pad;

    LoadSettings();

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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update
        Map::Update();

        // Render
        Map::Render();

        if (hasDoneFirstDraw && !hasLoadedSave)
        {
            Log("Will load save");
            Log("LoadGamesave() status:", SavefileIO::Get().LoadGamesave() ? "true" : "false");

            hasLoadedSave = true;

            std::cout << "Update map objects first time\n";
            Map::UpdateMapObjects();

            // Load manually checked koroks (but only if the game is running)
            // Override the checking from the savefile
            if (SavefileIO::Get().GameIsRunning)
            {
                std::ifstream foundFile("sdmc:/switch/totk-unexplored/found.txt");
                if (foundFile.is_open())
                {
                    for (int i = 0; i < (int)Data::ObjectType::Count; i++)
                    {
                        Data::ObjectType type = Data::ObjectType(i);
                        int objectCount = Data::m_Objects[type].size();
                        for (int j = 0; j < objectCount; j++)
                        {
                            std::string line;
                            std::getline(foundFile, line);
                            Map::m_MapObjects[type][j].m_Found = (bool)std::stoi(line);
                        }
                    }
                   
                    Log("Loaded manually marked collectibles");
                }
                else
                    Log("Failed to open found file (init())");

                foundFile.close();
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