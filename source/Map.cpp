#include "Map.h"

#include <algorithm>
#include <switch.h>

#include "Graphics/BasicVertices.h"
#include "Graphics/LineRenderer.h"
#include "Graphics/Quad.h"
#include "Legend.h"
#include "Dialog.h"
#include "MapObject.h"
#include "KorokDialog.h"
#include "ObjectInfo.h"
#include "./UI/LayerNavigation.h"
#include "Log.h"

#include "SavefileIO.h" 

constexpr float MapScale = 0.125;

glm::vec2 TransformPositionTo2DMap(glm::vec3 position) 
{
    return glm::vec2(position.x, -position.z) * MapScale;
}

void Map::Init()
{
    //Data::LoadPaths();

    Log("LoadFromJSON ");
    Data::LoadFromJSON("romfs:/map_data.json");

    m_ProjectionMatrix = glm::ortho(-m_CameraWidth / 2, m_CameraWidth / 2, -m_CameraHeight / 2, m_CameraHeight / 2, -1.0f, 1.0f);

    // Load font
    m_Font.Load("romfs:/Roboto-Regular.ttf"/*"romfs:/arial.ttf"*/); 
    m_Font.m_ProjectionMatrix = &m_ProjectionMatrix;
    m_Font.m_ViewMatrix = &m_ViewMatrix;

    m_LocationsFont.Load("romfs:/Roboto-Medium.ttf"); 
    m_LocationsFont.m_ProjectionMatrix = &m_ProjectionMatrix;
    m_LocationsFont.m_ViewMatrix = &m_ViewMatrix;

    m_LineRenderer = new LineRenderer();

    m_KorokDialog = new KorokDialog();
    m_ObjectInfo = new ObjectInfo();

    // Create UI
    m_Legend = new Legend();
    m_NoSavefileDialog = new Dialog(glm::vec2(0.0f, 0.0f), 700.0f, 400.0f, Dialog::InvalidSavefile);
    m_GameRunningDialog = new Dialog(glm::vec2(0.0f, 0.0f), 700.0f, 400.0f, Dialog::GameIsRunning);

    m_LayerNavigation = new LayerNavigation();

    m_Cursor.Create("romfs:/icons/cursor.png");
    m_Cursor.m_Position = glm::vec2(0 - m_Cursor.m_Texture->m_Width / 2, 0 - m_Cursor.m_Texture->m_Height / 2);
    m_Cursor.m_ProjectionMatrix = &m_ProjectionMatrix;

    LoadLayerImage();

    // Initialize all map objects
    for (int i = 0; i < (int)Data::ObjectType::Count; i++)
    {
        Data::ObjectType type = (Data::ObjectType)i;

        int objectCount = Data::m_Objects[type].size();
        if (objectCount == 0) continue;

        m_MapObjects[type].resize(objectCount);

        for (int j = 0; j < objectCount; j++)
        {
            m_MapObjects[type][j].Init(MapObject::IconPaths[i], type);
        }
    }

    UpdateMapObjects();
}

void Map::UpdateMapObjects()
{
    SavefileIO& save = SavefileIO::Get();

    if (!save.LoadedSavefile)
        return;

    // Iterate all object types
    for (int i = 0; i < (int)Data::ObjectType::Count; i++)
    {
        Data::ObjectType type = (Data::ObjectType)i;
        int objectCount = Data::m_Objects[type].size();
        
        // Iterate all map objects for that type
        for (int j = 0; j < objectCount; j++)
        {
            auto& mapObject = m_MapObjects[type][j];
            auto dataObject = Data::m_Objects[type][j];

            mapObject.m_Position = TransformPositionTo2DMap(dataObject->m_Position);
            mapObject.m_ObjectData = dataObject;

            // Check if the object has been found
            mapObject.m_Found = std::find(
                save.loadedData.found[type].begin(), 
                save.loadedData.found[type].end(), 
                dataObject) != save.loadedData.found[type].end();
        }
    }

    Log("Updated map objects");
}

void Map::Update()
{
    if (m_Pad == nullptr) return;

    // Load map texture if it hasn't been loaded
    if (!m_MapBackgrounds[(int)m_LayerNavigation->GetLayer()].m_Texture)
        LoadLayerImage();

    u64 buttonsPressed = padGetButtonsDown(m_Pad);
    u64 buttonsDown = padGetButtons(m_Pad);

    float zoomAmount = 0.02f;
    float dragAmont = 0.85f;
    float analogStickMovementSpeed = 10.0f;
    float minZoom = 0.1f;
        
    // Handle zooming like BotW
    HidAnalogStickState analog_stick_r = padGetStickPos(m_Pad, 1);

    // Get the stick position between -1.0f and 1.0f, instead of -32767 and 32767
    glm::vec2 stickRPosition = glm::vec2((float)analog_stick_r.x / (float)JOYSTICK_MAX, (float)analog_stick_r.y / (float)JOYSTICK_MAX);
   
    float deadzone = 0.3f;
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
        else if (m_ObjectInfo->m_IsOpen)
            m_ObjectInfo->SetOpen(false);
        else if (!m_NoSavefileDialog->m_IsOpen)
            m_Legend->m_IsOpen = !m_Legend->m_IsOpen;
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

                    // Objects
                    for (int i = 0; i < (int)Data::ObjectType::Count; i++)
                    {
                        Data::ObjectType type = Data::ObjectType(i);

                        if (type == Data::ObjectType::Location) //if (type == Data::ObjectType::HiddenKorok || type == Data::ObjectType::CarryKorok)
                            continue;

                        int objectCount = Data::m_Objects[type].size();                        
                        for (int j = 0; j < objectCount; j++)
                        {
                            MapObject& mapObject = m_MapObjects[type][j];
                            if (mapObject.IsClicked(touchPosition))
                            {
                                m_ObjectInfo->SetObject(type, mapObject.m_ObjectData);
                                m_ObjectInfo->SetOpen(true);

                                m_Legend->m_IsOpen = false;

                                clicked = true;
                            }
                        }
                    }

                    // Hide the object info if no object was clicked on
                    if (!clicked)
                    {
                        //m_ObjectInfo->SetOpen(false);
                    }
                    
                    // Only drag if not clicking on object
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
        m_Legend->Update();

    if (m_NoSavefileDialog->m_IsOpen) 
        m_NoSavefileDialog->Update();
    if (m_GameRunningDialog->m_IsOpen)
        m_GameRunningDialog->Update();

    m_LayerNavigation->Update(); 

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
    }

    m_PrevCameraPosition = m_CameraPosition;
}

void Map::Render()
{
    m_MapBackgrounds[(int)m_LayerNavigation->GetLayer()].Render();

    if (SavefileIO::Get().LoadedSavefile)
    {
        if (m_Legend->m_Show[IconButton::ButtonTypes::Koroks]) 
        {   
            // Render korok paths
            m_LineRenderer->Clear();
            for (int i = (int)Data::ObjectType::HiddenKorok; i < (int)Data::ObjectType::CarryKorok + 1; i++)
            {
                Data::ObjectType type = (Data::ObjectType)i;
                for (int k = 0; k < m_MapObjects[type].size(); k++)
                {
                    MapObject& korokObject = m_MapObjects[type][k];
                    Data::Korok* korokData = (Data::Korok*)m_MapObjects[type][k].m_ObjectData;
                    if (!korokData->m_Path)
                        continue;

                    auto& pointsOnPath = korokData->m_Path->m_Points;

                    // Don't render if found
                    if (korokObject.m_Found || !korokObject.IsVisible())
                        continue;

                    // 0 -> 1
                    // 1 -> 2
                    // 2 -> 3
                    for (unsigned int p = 1; p < pointsOnPath.size(); p++)
                    {
                        glm::vec2 start = TransformPositionTo2DMap(pointsOnPath[p - 1]);
                        glm::vec2 end = TransformPositionTo2DMap(pointsOnPath[p]);

                        float width = (1.0f / m_Zoom) * 1.0f;
                        if (m_Zoom >= 3.0f)
                            width = 0.25f;

                        m_LineRenderer->AddLine(start, end, width);
                    }
                }   
            }

            m_LineRenderer->RenderLines(m_ProjectionMatrix, m_ViewMatrix);
        } 
 

        // Render line connecting cave entrances
        std::map<std::string, int> caveCounts;
        std::map<std::string, std::vector<MapObject*>> cavesByName;
        if (m_Legend->m_Show[IconButton::Caves])
        {
            m_LineRenderer->Clear();
            for (int i = 0; i < m_MapObjects[Data::ObjectType::Cave].size(); i++)
            {
                std::string name = m_MapObjects[Data::ObjectType::Cave][i].m_ObjectData->m_DisplayName;

                if (m_MapObjects[Data::ObjectType::Cave][i].m_Found || !m_MapObjects[Data::ObjectType::Cave][i].IsVisible()) 
                    continue;

                if (caveCounts.count(name) == 0) caveCounts[name] = 0;
                caveCounts[name]++;

                cavesByName[name].push_back(&m_MapObjects[Data::ObjectType::Cave][i]);
            }

            for (auto& entry : caveCounts)
            {
                std::string name = entry.first;
                int count = entry.second;

                float lineWidth = (1.0f / m_Zoom) * 1.5f;
                if (m_Zoom >= 3.0f)
                    lineWidth = 0.75f;

                // Multiple entrances to the same cave
                if (count > 1)
                {
                    glm::vec3 color = glm::vec3(40.0f, 27.0f, 4.0f) / glm::vec3(255.0f);

                    auto& caves = cavesByName[name];
                    if (count == 2)
                    {
                        // Draw line straight between the two entrances
                        m_LineRenderer->AddLine(caves[0]->m_Position, caves[1]->m_Position, lineWidth, glm::vec4(color, 1.0f));
                    }
                }
            }

            //m_LineRenderer->AddLine(start, end, width);
            m_LineRenderer->RenderLines(m_ProjectionMatrix, m_ViewMatrix);  
        }

        for (int i = 0; i < (int)Data::ObjectType::Count; i++)
        {
            Data::ObjectType type = (Data::ObjectType)i;

            if (Data::m_Objects[type].size() == 0) continue;

            // Run once for each object type if it is selected in the legend
            if (m_Legend->m_Show[IconButton::ObjectTypeToButtonType(type)])
                m_MapObjects[type][0].Render();
        }
    }

    m_Cursor.Render();

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
    
    m_LayerNavigation->Render();


    if (!m_Legend->m_IsOpen && !m_KorokDialog->m_IsOpen && !m_ObjectInfo->m_IsOpen && SavefileIO::Get().LoadedSavefile)
        m_Font.AddTextToBatch("Press X to open legend", glm::vec2(m_ScreenLeft + 20, m_ScreenTop - 30), 0.5f);  

    if (SavefileIO::Get().LoadedSavefile)
    {
        if (SavefileIO::Get().GameIsRunning)
        {
            m_Font.AddTextToBatch("Totk is running.", glm::vec2(m_ScreenRight - 20, m_ScreenTop - 30), 0.5f, glm::vec3(1.0f), ALIGN_RIGHT);
            m_Font.AddTextToBatch("Loaded older save", glm::vec2(m_ScreenRight - 20, m_ScreenTop - 60), 0.5f, glm::vec3(1.0f), ALIGN_RIGHT);
        }

        float bottomTextX = m_ScreenRight - 30;

        m_Font.AddTextToBatch("L and R to zoom, (-) to change user, (+) to exit", 
            glm::vec2(bottomTextX, m_ScreenBottom + 20), 0.5f, glm::vec3(1.0f), ALIGN_RIGHT);  
    }

    m_KorokDialog->Render(m_ProjectionMatrix, m_ViewMatrix);
    m_ObjectInfo->Render(m_ProjectionMatrix, m_ViewMatrix);

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
    std::string mapImagePaths[3] = { "romfs:/map/depths-small.png", "romfs:/map/surface-small.png", "romfs:/map/sky-small.png" };

    int currentLayer = (int)m_LayerNavigation->GetLayer();

    m_MapBackgrounds[currentLayer].Create(mapImagePaths[currentLayer]);
    m_MapBackgrounds[currentLayer].m_ProjectionMatrix = &m_ProjectionMatrix;
    m_MapBackgrounds[currentLayer].m_ViewMatrix = &m_ViewMatrix; 
}

MapObject* Map::GetObjectInCursor()
{
    
}

void Map::Destory()
{
    //delete[] m_Locations;

    delete m_Legend;
    delete m_NoSavefileDialog;
    delete m_GameRunningDialog;

    delete m_LayerNavigation;

    delete m_KorokDialog;
    delete m_ObjectInfo;
}

TexturedQuad Map::m_MapBackgrounds[3];
Font Map::m_Font;
Font Map::m_LocationsFont;
Font Map::m_LocationsFont2;
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

std::unordered_map<Data::ObjectType, std::vector<MapObject>> Map::m_MapObjects;

TexturedQuad Map::m_Cursor;

Legend* Map::m_Legend;
KorokDialog* Map::m_KorokDialog;
ObjectInfo* Map::m_ObjectInfo;
Dialog* Map::m_NoSavefileDialog;
Dialog* Map::m_GameRunningDialog;
LayerNavigation* Map::m_LayerNavigation;