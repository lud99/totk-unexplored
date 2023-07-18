#include "Map.h"

#include <algorithm>
#include <switch.h>

#include "Graphics/BasicVertices.h"
#include "Graphics/LineRenderer.h"
#include "Graphics/Quad.h"
#include "MapLocation.h"
#include "Legend.h"
#include "Dialog.h"
#include "MapObject.h"
#include "KorokDialog.h"
#include "Log.h"

#include "SavefileIO.h" 

constexpr float MapScale = 0.125;

IconButton::ButtonTypes ObjectTypeToButtonType(Data::ObjectType objectType)
{
    if (objectType == Data::ObjectType::HiddenKorok || objectType == Data::ObjectType::CarryKorok) 
        return IconButton::ButtonTypes::Koroks; 

    return IconButton::ButtonTypes((int)objectType - 1);
}

void Map::Init()
{
    //Data::LoadPaths();

    Data::LoadFromJSON("romfs:/map_data.json");

    m_ProjectionMatrix = glm::ortho(-m_CameraWidth / 2, m_CameraWidth / 2, -m_CameraHeight / 2, m_CameraHeight / 2, -1.0f, 1.0f);

    std::string mapImagePaths[3] = { "romfs:/map/depths-small.jpg", "romfs:/map/surface-small.jpg", "romfs:/map/sky-small.jpg" };
    for (int i = 0; i < 3; i++)
    {
        m_MapBackgrounds[i].Create(mapImagePaths[i]);
        m_MapBackgrounds[i].m_ProjectionMatrix = &m_ProjectionMatrix;
        m_MapBackgrounds[i].m_ViewMatrix = &m_ViewMatrix; 
    }


    // Load font
    m_Font.Load("romfs:/arial.ttf"); 
    m_Font.m_ProjectionMatrix = &m_ProjectionMatrix;
    m_Font.m_ViewMatrix = &m_ViewMatrix;

    m_LineRenderer = new LineRenderer();

    m_KorokDialog = new KorokDialog();

    // Create UI
    m_Legend = new Legend();
    m_NoSavefileDialog = new Dialog(glm::vec2(0.0f, 0.0f), 700.0f, 400.0f, Dialog::InvalidSavefile);
    m_GameRunningDialog = new Dialog(glm::vec2(0.0f, 0.0f), 700.0f, 400.0f, Dialog::GameIsRunning);
    // m_MasterModeDialog = new Dialog(glm::vec2(0.0f, 0.0f), 700.0f, 400.0f, Dialog::MasterModeChoose);

    // m_MasterModeIcon.Create("romfs:/mastermodeicon.png");
    // m_MasterModeIcon.m_Position = glm::vec2(m_ScreenLeft + 45.0f, m_ScreenBottom + 40.0f);
    // m_MasterModeIcon.m_Scale = 0.1f;
    // m_MasterModeIcon.m_ProjectionMatrix = &m_ProjectionMatrix;
    // m_MasterModeIcon.m_ViewMatrix = nullptr;

    // Icons for map objects
    std::string mapObjectIcons[] = 
    {
        "romfs:/icons/korok_hidden.png",
        "romfs:/icons/korok_carry.png",
        "romfs:/icons/shrine.png",
        "romfs:/icons/lightroot.png",
        "romfs:/icons/bubbul.png",

        "romfs:/icons/cave.png",
        "romfs:/icons/well.png",
        "romfs:/icons/chasm.png",
        "romfs:/icons/location.png",

        "romfs:/icons/hinox.png",
        "romfs:/icons/talus.png",
        "romfs:/icons/molduga.png",
        "romfs:/icons/flux_construct.png",
        "romfs:/icons/frox.png",
        "romfs:/icons/gleeok.png",

        "romfs:/icons/sages_will.png",
        "romfs:/icons/old_map.png",
        "romfs:/icons/addison_sign.png", 
        "romfs:/icons/schema_stone.png",
        "romfs:/icons/yiga_schematic.png"
    };

    // Initialize all map objects
    for (int i = 0; i < (int)Data::ObjectType::Count; i++)
    {
        Data::ObjectType type = (Data::ObjectType)i;
        //std::cout << "a" << i << "\n";
        int objectCount = Data::m_Objects[type].size();
        //std::cout << "b" << objectCount << "\n";

        if (objectCount == 0) continue;

        m_MapObjects[type].resize(objectCount);

        for (int j = 0; j < objectCount; j++)
        {
            //m_MapObjects[type].push_back(MapObject());
            //std::cout << "c " << j << "\n";

            m_MapObjects[type][j].Init(mapObjectIcons[i], type);
        }

        //std::cout << mapObjectIcons[i] << ", " << m_MapObjects[type].size() << "\n";

        // Run once for each object type
        
    }

    // Create locations
    //m_Locations = new MapLocation[Data::LocationsCount];

    std::cout << "c\n";
    UpdateMapObjects();
}

void Map::UpdateMapObjects()
{

    SavefileIO& save = SavefileIO::Get();

    if (!save.LoadedSavefile)
        return;

    auto TransformPosition = [](Data::Object* o) {
        return glm::vec2(o->m_Position.x, -o->m_Position.z) * MapScale;
    };

    // Iterate all object types
    for (int i = 0; i < (int)Data::ObjectType::Count; i++)
    {
        Data::ObjectType type = (Data::ObjectType)i;
        int objectCount = Data::m_Objects[type].size();

        std::cout << "update objs " << objectCount << "\n";
        
        // Iterate all map objects for that type
        for (int j = 0; j < objectCount; j++)
        {
            auto& mapObject = m_MapObjects[type][j];
            auto dataObject = Data::m_Objects[type][j];

            mapObject.m_Position = TransformPosition(dataObject);
            mapObject.m_ObjectData = dataObject;

            // Check if the object has been found
            mapObject.m_Found = std::find(
                save.loadedData.found[type].begin(), 
                save.loadedData.found[type].end(), 
                dataObject) != save.loadedData.found[type].end();
        }
    }

    // for (int i = 0; i < Data::LocationsCount; i++) // Locations
    // {
    //     m_Locations[i].m_Position = glm::vec2(Data::Locations[i].x, -Data::Locations[i].y) * MapScale;
    //     m_Locations[i].m_LocationData = &Data::Locations[i];

    //     // Check if the korok has been found (if the found vector contains it)
    //     m_Locations[i].m_Found = std::find(
    //         save.loadedData.visitedLocations.begin(), 
    //         save.loadedData.visitedLocations.end(), 
    //         &Data::Locations[i]) != save.loadedData.visitedLocations.end();
    // }

    Log("Updated map objects");
}

void Map::Update()
{
    if (m_Pad == nullptr) return;

    u64 buttonsPressed = padGetButtonsDown(m_Pad);
    u64 buttonsDown = padGetButtons(m_Pad);

    float zoomAmount = 0.015f;
    float dragAmont = 0.85f;
    float analogStickMovementSpeed = 10.0f;
    float minZoom = 0.1f;
        
    // Handle zooming like BotW
    HidAnalogStickState analog_stick_r = padGetStickPos(m_Pad, 1);

    // Get the stick position between -1.0f and 1.0f, instead of -32767 and 32767
    glm::vec2 stickRPosition = glm::vec2((float)analog_stick_r.x / (float)JOYSTICK_MAX, (float)analog_stick_r.y / (float)JOYSTICK_MAX);
   
    float deadzone = 0.1f;
    if (fabs(stickRPosition.y) >= deadzone)
        m_Zoom *= 1.0f + zoomAmount * stickRPosition.y;

    // Zoom with L and R
    if (buttonsDown & HidNpadButton_R) // Zoom in
        m_Zoom *= 1.0f + zoomAmount;

    if (buttonsDown & HidNpadButton_L) // Zoom out
        m_Zoom *= 1.0f - zoomAmount;

    // Reset zoom if pressing L or R stick
    if (buttonsPressed & HidNpadButton_StickL)
    {
        m_Zoom = m_DefaultZoom;
        m_CameraPosition = glm::vec2(0.0f, 0.0f);
    }

    if (m_Zoom < minZoom) m_Zoom = minZoom;

    // Open profile picker
    if (buttonsPressed & HidNpadButton_Minus)
    {
        if (SavefileIO::Get().LoadedSavefile)
        {
            m_LoadMasterMode = false;

            SavefileIO::Get().LoadGamesave(false, true);
            UpdateMapObjects();
        }
    }

    // Toggle legend
    if (buttonsPressed & HidNpadButton_X)
    {
        // Close the korok dialog first, then if it's not open close the legend
        if (m_KorokDialog->m_IsOpen)
            m_KorokDialog->SetOpen(false);
        else if (!m_NoSavefileDialog->m_IsOpen)
            m_Legend->m_IsOpen = !m_Legend->m_IsOpen;
    }

    // Toggle showing everything
    // if (buttonsDown & HidNpadButton_B)
    //     m_ShowAllObjects = true;
    // if (buttonsUp & HidNpadButton_B)
    //     m_ShowAllObjects = false;

    // Toggle master mode
    if (buttonsPressed & HidNpadButton_Y)
    {
        if (SavefileIO::Get().MostRecentMasterModeFile != -1)
        {
            m_LoadMasterMode = !m_LoadMasterMode;

            SavefileIO::Get().LoadGamesave(m_LoadMasterMode);
            UpdateMapObjects();
        }
    }

    if (buttonsPressed & HidNpadButton_B)
    {
        if (m_KorokDialog->m_IsOpen) 
        {
            int index = m_KorokDialog->m_KorokIndex;
            if (index < 800)
                m_MapObjects[Data::ObjectType::HiddenKorok][index].m_Found = true;
            if (index >= 800)
                m_MapObjects[Data::ObjectType::CarryKorok][index].m_Found = true;
            
            m_KorokDialog->SetOpen(false);
        }
    }

    // Analog stick camera movement
    // Read the sticks' position
    HidAnalogStickState analog_stick_l = padGetStickPos(m_Pad, 0);

    // Get the stick position between -1.0f and 1.0f, instead of -32767 and 32767
    glm::vec2 stickLPosition = glm::vec2((float)analog_stick_l.x / (float)JOYSTICK_MAX, (float)analog_stick_l.y / (float)JOYSTICK_MAX);
   
    float distanceToCenter = glm::distance(stickLPosition, glm::vec2(0.0f, 0.0f));
    if (distanceToCenter >= deadzone)
        m_CameraPosition += stickLPosition * (analogStickMovementSpeed / m_Zoom);

    m_ViewMatrix = glm::mat4(1.0f); // Reset (important)
    m_ViewMatrix = glm::scale(m_ViewMatrix, glm::vec3(m_Zoom, m_Zoom, 0.0f));
    m_ViewMatrix = glm::translate(m_ViewMatrix, glm::vec3(-m_CameraPosition, 1.0));

    // Dragging
    HidTouchScreenState state={0};
    if (hidGetTouchScreenStates(&state, 1)) {
        // Convert to more suitable coords
        glm::vec2 touchPosition = glm::vec2(state.touches[0].x - m_CameraWidth / 2, -(state.touches[0].y - m_CameraHeight / 2)); 

        // A new touch
        if (state.count != m_PrevTouchCount)
        {   
            m_PrevTouchCount = state.count;

            // Dont drag if finger is on the legend
            if (!(m_Legend->m_IsOpen && m_Legend->IsPositionOnLegend(touchPosition)) && 
                !(m_NoSavefileDialog->m_IsOpen && m_NoSavefileDialog->IsPositionOn(touchPosition)) &&
                !(m_GameRunningDialog->m_IsOpen && m_GameRunningDialog->IsPositionOn(touchPosition))/* &&
                !(m_MasterModeDialog->m_IsOpen && m_MasterModeDialog->IsPositionOn(touchPosition))*/)
            {
                // Check if the finger was pressed
                if (state.count == 1)
                {   
                    // Check if clicked korok
                    bool clicked = false;

                    // Hidden koroks
                    // auto& hiddenKoroks = Data::m_Objects[Data::ObjectType::HiddenKorok];
                    // for (int i = 0; i < hiddenKoroks.size(); i++)
                    // {
                    //     if ((!hiddenKoroks[i]->m_Found || m_Legend->m_Show[IconButton::ShowCompleted]) && hiddenKoroks[i]->IsClicked(touchPosition))
                    //     {
                    //         // Set the korok dialog
                    //         m_KorokDialog->SetSeed(hiddenKoroks[i]->m_ObjectData->m_ZeldaDungeonId, i);
                    //         m_KorokDialog->SetOpen(true);

                    //         m_Legend->m_IsOpen = false;

                    //         clicked = true;
                    //     }
                    // }     

                    // // Carry koroks    
                    // auto& carryKoroks = Data::m_Objects[Data::ObjectType::CarryKorok];           
                    // for (int i = 0; i < carryKoroks.size(); i++)
                    // {
                    //     if ((!carryKoroks[i].m_Found || m_Legend->m_Show[IconButton::ShowCompleted]) && carryKoroks[i].IsClicked(touchPosition))
                    //     {
                    //         // Set the korok dialog
                    //         m_KorokDialog->SetSeed(carryKoroks[i].m_ObjectData->m_ZeldaDungeonId, i);
                    //         m_KorokDialog->SetOpen(true);

                    //         m_Legend->m_IsOpen = false;

                    //         clicked = true;
                    //     }
                    // }

                    // Hide the korok info if no korok was clicked on
                    if (!clicked)
                    {
                        //m_KorokDialog->SetOpen(false);
                    }
                    
                    // Only drag if not clicking on korok
                    m_IsDragging = true;
                    m_PrevTouchPosition = touchPosition; // The origin of the drag
                }
            }
                
            // Check if the finger was released
            if (state.count == 0)
                m_IsDragging = false;
        }

        // Handle the camera dragging
        if (state.count >= 1 && m_IsDragging)
        {
            // Calculate how much the finger has moved this frame
            glm::vec2 delta = m_PrevTouchPosition - touchPosition;

            // Move the camera by the delta. Flip the direction of the y-coordinate and 
            // divide by the zoom to move the same amount irregardless of the zoom
            m_CameraPosition += (delta * dragAmont) / m_Zoom;

            // Set the touch pos to the most recent one, so we only check for the delta between each frame and not from when the drag started
            m_PrevTouchPosition = touchPosition;
        }
    }

    m_ViewMatrix = glm::mat4(1.0f); // Reset (important)
    m_ViewMatrix = glm::scale(m_ViewMatrix, glm::vec3(m_Zoom, m_Zoom, 0.0f));
    m_ViewMatrix = glm::translate(m_ViewMatrix, glm::vec3(-m_CameraPosition, 1.0));

    if (m_Legend->m_IsOpen)
    {
        m_Legend->Update();
    } else 
    {
        if (buttonsPressed & HidNpadButton_Up)
        {
            Layers newLayer = (Layers)(m_CurrentLayer + 1);
            if (newLayer <= 2)
                m_CurrentLayer = newLayer;
        }
        if (buttonsPressed & HidNpadButton_Down)
        {
            Layers newLayer = (Layers)(m_CurrentLayer - 1);
            if (newLayer >= 0)
                m_CurrentLayer = newLayer;
        }

        //if ((buttonsPressed & HidNpadButton_Up) || (buttonsPressed & HidNpadButton_Down))
            //LoadLayerImage();
    }


    if (m_NoSavefileDialog->m_IsOpen) 
        m_NoSavefileDialog->Update();
    if (m_GameRunningDialog->m_IsOpen)
        m_GameRunningDialog->Update();
    // if (m_MasterModeDialog->m_IsOpen)
    //     m_MasterModeDialog->Update();

    // Update objects
    if (SavefileIO::Get().LoadedSavefile)
    {
        // Clear the meshes during the first iteration of the loop (i == 0) == true
        for (int i = 0; i < (int)Data::ObjectType::Count; i++)
        {
            Data::ObjectType type = Data::ObjectType(i);
            int objectCount = Data::m_Objects[type].size();

            if (objectCount == 0) continue;
            
            for (int j = 0; j < objectCount; j++)
            {
                m_MapObjects[type][j].Update(j == 0);
            }
        }
    
        //for (int i = 0; i < Data::LocationsCount; i++)
            //m_Locations[i].Update();

        MapLocation::m_ShowAnyway = m_Legend->m_Show[IconButton::ShowCompleted];
    }

    m_PrevCameraPosition = m_CameraPosition;
}

void Map::Render()
{
    m_MapBackgrounds[m_CurrentLayer].Render();

    Map::m_Font.BeginBatch();

    if (SavefileIO::Get().LoadedSavefile)
    {
        if (m_Legend->m_Show[IconButton::ButtonTypes::Koroks]) 
        {   
            // Render korok paths
            // for (int k = 0; k < Data::KorokCount; k++)
            // {
            //     // This korok has no paths
            //     if (m_Koroks[k].m_ObjectData->path == nullptr)
            //         continue;

            //     // Don't render if found
            //     if (m_Koroks[k].m_Found && !m_Legend->m_Show[IconButton::ShowCompleted])
            //         continue;

            //     Data::KorokPath* path = m_Koroks[k].m_ObjectData->path;

            //     // 0 -> 1
            //     // 1 -> 2
            //     // 2 -> 3
            //     for (unsigned int p = 1; p < path->points.size(); p++)
            //     {
            //         glm::vec2 start = path->points[p - 1] * MapScale;
            //         start.y *= -1; // Flip the y coord
            //         glm::vec2 end = path->points[p] * MapScale;
            //         end.y *= -1;

            //         float width = (1.0f / m_Zoom) * 2.0f;
            //         if (m_Zoom >= 3.0f)
            //             width = 0.75f;

            //         m_LineRenderer->AddLine(start, end, width);
            //     }
            // }

            // m_LineRenderer->RenderLines(m_ProjectionMatrix, m_ViewMatrix);

            //MapObject<Data::HiddenKorok>::Render();
            //MapObject<Data::CarryKorok>::Render();
        }

        

        for (int i = 0; i < (int)Data::ObjectType::Count; i++)
        {
            Data::ObjectType type = (Data::ObjectType)i;

            if (Data::m_Objects[type].size() == 0) continue;

            // Run once for each object type if it is selected in the legend
            if (m_Legend->m_Show[ObjectTypeToButtonType(type)])
                m_MapObjects[type][0].Render();
        }

        /*if (m_Legend->m_Show[IconButton::ButtonTypes::Locations])
        {
            for (int i = 0; i < Data::LocationsCount; i++)
                m_Locations[i].Render();
        }*/
    }

    m_Font.RenderBatch();

    // Draw behind legend
    //if (m_LoadMasterMode)
       // m_MasterModeIcon.Render();

    m_Font.BeginBatch();
    if (m_Legend->m_IsOpen) 
        m_Legend->Render();

    if (m_NoSavefileDialog->m_IsOpen) 
        m_NoSavefileDialog->Render();
    if (m_GameRunningDialog->m_IsOpen)
        m_GameRunningDialog->Render();
    /*if (m_MasterModeDialog->m_IsOpen)
        m_MasterModeDialog->Render();*/


    if (!m_Legend->m_IsOpen && !m_KorokDialog->m_IsOpen && SavefileIO::Get().LoadedSavefile)
        m_Font.AddTextToBatch("Press X to open legend", glm::vec2(m_ScreenLeft + 20, m_ScreenTop - 30), 0.5f);  

    if (SavefileIO::Get().LoadedSavefile)
    {
        if (SavefileIO::Get().GameIsRunning)
        {
            m_Font.AddTextToBatch("Totk is running.", glm::vec2(m_ScreenRight - 20, m_ScreenTop - 30), 0.5f, glm::vec3(1.0f), ALIGN_RIGHT);
            m_Font.AddTextToBatch("Loaded older save", glm::vec2(m_ScreenRight - 20, m_ScreenTop - 60), 0.5f, glm::vec3(1.0f), ALIGN_RIGHT);
        }

        float bottomTextX = m_ScreenRight - 30;

        /*if (SavefileIO::MasterModeFileExists && !m_LoadMasterMode)
            m_Font.AddTextToBatch("Press Y to load master mode", glm::vec2(bottomTextX, m_ScreenBottom + 55), 0.5f, glm::vec3(1.0f), ALIGN_RIGHT);  
        else if (m_LoadMasterMode)
            m_Font.AddTextToBatch("Press Y to load normal mode", glm::vec2(bottomTextX, m_ScreenBottom + 55), 0.5f, glm::vec3(1.0f), ALIGN_RIGHT);  
    */
        m_Font.AddTextToBatch("L and R to zoom, (-) to change user, (+) to exit", 
            glm::vec2(bottomTextX, m_ScreenBottom + 20), 0.5f, glm::vec3(1.0f), ALIGN_RIGHT);  
    }

    m_KorokDialog->Render(m_ProjectionMatrix, m_ViewMatrix);

    glm::mat4 emptyViewMatrix(1.0);
    m_Font.m_ViewMatrix = &emptyViewMatrix; // Don't draw the text relative to the camera 

    m_Font.RenderBatch();

    m_Font.m_ViewMatrix = &m_ViewMatrix;
}

bool Map::IsInView(glm::vec2 position, float margin = 100.0f)
{
    // Calculate camera bounds
    float viewLeft = m_CameraPosition.x - (m_CameraWidth / 2) / m_Zoom - margin;
    float viewRight = m_CameraPosition.x + (m_CameraWidth / 2) / m_Zoom + margin;
    float viewBottom = m_CameraPosition.y - (m_CameraHeight / 2) / m_Zoom - margin;
    float viewTop = m_CameraPosition.y + (m_CameraHeight / 2) / m_Zoom + margin;

    // Check if the position would be outside of view (horizontal)
    if (position.x < viewLeft || position.x > viewRight)
        return false;

    // Check if the position would be outside of view (vertical)
    if (position.y < viewBottom || position.y > viewTop)
        return false;

    return true;
}

void Map::LoadLayerImage()
{
    // std::string mapImagePaths[3] = { "romfs:/map/depths-small.jpg", "romfs:/map/surface-small.jpg", "romfs:/map/sky-small.jpg" };

    // m_MapBackground.Create(mapImagePaths[m_CurrentLayer]);
}

void Map::Destory()
{
    //delete[] m_Locations;

    delete m_Legend;
    delete m_NoSavefileDialog;
    delete m_GameRunningDialog;
    //delete m_MasterModeDialog;
    delete m_KorokDialog;
}

Map::Layers Map::m_CurrentLayer = Map::Layers::Surface;

TexturedQuad Map::m_MapBackgrounds[3];
Font Map::m_Font;
LineRenderer* Map::m_LineRenderer;
//TexturedQuad Map::m_MasterModeIcon;

float Map::m_Zoom = Map::m_DefaultZoom;

glm::mat4 Map::m_ProjectionMatrix = glm::mat4(1.0f);
glm::mat4 Map::m_ViewMatrix = glm::mat4(1.0f);

glm::vec2 Map::m_CameraPosition = glm::vec2(0.0f, 0.0f);
glm::vec2 Map::m_PrevCameraPosition;

int Map::m_PrevTouchCount = 0;
glm::vec2 Map::m_PrevTouchPosition;
glm::vec2 Map::m_StartDragPos;
bool Map::m_IsDragging = false;
bool Map::m_ShouldExit = false;
bool Map::m_LoadMasterMode = false;

PadState* Map::m_Pad;

// MapObject<Data::HiddenKorok>* Map::m_HiddenKoroks;
// MapObject<Data::CarryKorok>* Map::m_CarryKoroks;
// MapObject<Data::Shrine>* Map::m_Shrines;
// MapObject<Data::Lightroot>* Map::m_Lightroots;

// MapObject<Data::Cave>* Map::m_Caves;
// MapObject<Data::Well>* Map::m_Wells;

// MapObject<Data::Hinox>* Map::m_Hinoxes;
// MapObject<Data::Talus>* Map::m_Taluses;
// MapObject<Data::Molduga>* Map::m_Moldugas;
// MapObject<Data::FluxConstruct>* Map::m_FluxConstructs;
// MapObject<Data::Frox>* Map::m_Froxes;
// MapObject<Data::Gleeok>* Map::m_Gleeoks;

std::unordered_map<Data::ObjectType, std::vector<MapObject>> Map::m_MapObjects;

MapLocation* Map::m_Locations;

Legend* Map::m_Legend;
KorokDialog* Map::m_KorokDialog;
Dialog* Map::m_NoSavefileDialog;
Dialog* Map::m_GameRunningDialog;
//Dialog* Map::m_MasterModeDialog;