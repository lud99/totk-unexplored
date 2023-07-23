#pragma once

#include "Graphics/Quad.h"

#include "Data.h"
#include "MapObject.h"

class ObjectInfo
{
public:
    ObjectInfo();

    void Update();
    void Render(glm::mat4 projMat = glm::mat4(1.0f), glm::mat4 viewMat = glm::mat4(1.0f));
    
    void SetObject(MapObject* mapObject);
    void SetOpen(bool open);

    void SetPosition(glm::vec2 position);
    glm::vec2 GetPosition();

    ~ObjectInfo();

public:
    const int Width = 350.0f;
    const int m_Margin = 30.0f;

    Quad m_Background;
    TexturedQuad m_Icon;

    std::string m_Text;
    Data::ObjectType m_Type;
    MapObject* m_MapObject;

    bool m_IsOpen = false;
};