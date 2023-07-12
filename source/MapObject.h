#pragma once

#include "Graphics/Mesh.hpp"
#include "Graphics/Shader.h" 
#include "Graphics/Texture2D.h"
#include "Graphics/BasicVertices.h"

#include "Map.h"
#include "Legend.h"

#include <glm/vec2.hpp>
#include <iostream>

class Texture2D;

class MapObject
{
public:
    static const int ObjectTypesCount = 10;

    static Texture2D* m_Textures[ObjectTypesCount];
    static Shader m_Shader;
    static Mesh<TextureVertex> m_Meshes[ObjectTypesCount];

    Data::Object* m_ObjectData;

    int m_UniqueType;

    glm::vec2 m_Position = glm::vec2(0.0f, 0.0f); // The position on screen
    bool m_Found = false;

    static float m_Scale;

public:
    MapObject();

    static void Init(const std::string& path, int count);

    void AddToMesh();

    void Update(bool clear = false);
    static void Render();

    bool IsClicked(glm::vec2 position);

    ~MapObject();
};

Texture2D* MapObject::m_Textures[MapOb];
Shader MapObject::m_Shader;
Mesh<TextureVertex> MapObject::m_Mesh;

bool MapObject<T>::m_ShowAnyway = false;
float MapObject<T>::m_Scale;