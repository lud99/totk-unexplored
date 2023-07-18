#pragma once

#include "Graphics/Mesh.hpp"
#include "Graphics/Shader.h" 

#include "Data.h"

#include <glm/vec2.hpp>
#include <iostream>

class Texture2D;

class MapObject
{
public:
    static Texture2D* m_Textures[(int)Data::ObjectType::Count];
    static Shader m_Shader;
    static Mesh<TextureVertex> m_Meshes[(int)Data::ObjectType::Count];

    Data::Object* m_ObjectData;

    Data::ObjectType m_ObjectType;

    glm::vec2 m_Position = glm::vec2(0.0f, 0.0f); // The position on screen
    bool m_Found = false;

    static float m_Scale;

public:
    MapObject();

    void Init(const std::string& texturePath, Data::ObjectType type);

    void AddToMesh();

    void Update(bool clear = false);
    void Render();

    bool IsClicked(glm::vec2 position);

    ~MapObject();
};