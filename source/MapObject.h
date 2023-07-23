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
    glm::vec3 m_Color = glm::vec3(1.0f, 1.0f, 1.0f);
    bool m_Found = false;

    static float m_Scale;

    static const std::string IconPaths[20];
    static const std::string Names[20];
public:
    MapObject();

    void Init(const std::string& texturePath, Data::ObjectType type);

    void AddToMesh();

    void Update(bool clear = false);
    void Render();

    bool IsClicked(glm::vec2 position, bool ignoreVisibility = false);
    bool IsVisible(bool culling = true);

    ~MapObject();
};