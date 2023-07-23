#pragma once

#include "Graphics/Texture2D.h"
#include "Graphics/Quad.h"
#include "Graphics/Mesh.hpp"
#include "Graphics/Shader.h" 
#include "Graphics/Font.h"
#include "Graphics/Quad.h"

#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

#include <switch.h>

#include "Data.h"

class ObjectInfo;
class MapObject;
class MapLocation;
class Legend;
class Dialog;
class LineRenderer;
class LayerNavigation;
namespace Map
{
    void Init();

    void UpdateMapObjects();
    void SetCameraPosition(glm::vec2 position);

    void Update();
    void Render();

    bool IsInView(glm::vec2 position, float margin);

    void LoadLayerImage();

    MapObject* GetObjectInCursor();

    void Destory();

    const float m_DefaultZoom = 0.125f * 4;
    const float m_CameraWidth = 1280.0f;
    const float m_CameraHeight = 720.0f;
    const float m_ScreenLeft = -m_CameraWidth / 2.0f;
    const float m_ScreenRight = m_CameraWidth / 2.0f;
    const float m_ScreenTop = m_CameraHeight / 2.0f;
    const float m_ScreenBottom = -m_CameraHeight / 2.0f;
    const float m_CameraLerpSpeed = 0.5f;

    extern TexturedQuad m_MapBackgrounds[3];

    extern Font m_Font, m_LocationsFont;
    extern LineRenderer* m_LineRenderer;

    extern float m_Zoom;

    extern glm::mat4 m_ProjectionMatrix;
    extern glm::mat4 m_ViewMatrix;

    extern glm::vec2 m_CameraPosition, m_TargetCameraPosition;
    extern glm::vec2 m_PrevCameraPosition;

    extern glm::vec2 m_TargetCursorPosition;

    //extern float 

    extern int m_PrevTouchCount;
    extern glm::vec2 m_PrevTouchPosition;
    extern glm::vec2 m_StartDragPos;
    extern bool m_IsDragging;
    extern bool m_ShouldExit;
    extern bool m_LoadMasterMode;

    extern PadState* m_Pad;

    extern std::unordered_map<Data::ObjectType, std::vector<MapObject>> m_MapObjects;

    extern TexturedQuad m_Cursor;

    extern Legend* m_Legend;
    extern ObjectInfo* m_ObjectInfo;
    extern Dialog* m_NoSavefileDialog;
    extern Dialog* m_GameRunningDialog;
    
    extern LayerNavigation* m_LayerNavigation;
};