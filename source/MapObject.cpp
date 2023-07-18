#include "MapObject.h"

#include "Graphics/Texture2D.h"
#include "Graphics/BasicVertices.h"

#include "Map.h"
#include "Legend.h"

#include <glm/vec2.hpp>
#include <iostream>

MapObject::MapObject()
{
    
}

void MapObject::Init(const std::string& texturePath, Data::ObjectType type)
{
    m_ObjectType = type;

    // Only load one texture
    if (!m_Textures[(int)m_ObjectType])
        m_Textures[(int)m_ObjectType] = new Texture2D(texturePath);

    // Only create shader if it's the first 
    if (m_Shader.m_id == 0)
    {
        std::string vertexShaderSource = R"text(
            #version 330 core

            layout (location = 0) in vec3 position;
            layout (location = 1) in vec4 color;
            layout (location = 2) in vec2 texCoord;

            out vec2 passTextureCoord;

            uniform mat4 u_ProjectionMatrix = mat4(1.0);
            uniform mat4 u_ViewMatrix = mat4(1.0);

            void main()
            {
                gl_Position = u_ProjectionMatrix * u_ViewMatrix * vec4(position, 1.0);

                passTextureCoord = texCoord;
            }
        )text";

        std::string fragmentShaderSource = R"text(
            #version 330 core

            layout (location = 0) out vec4 color;
            in vec2 passTextureCoord;

            uniform sampler2D tex;

            void main()
            {
                color = texture(tex, passTextureCoord);

                if (color.a == 0.0)
                    discard;
            }
        )text";

        m_Shader = ShaderLoader::CreateShaderFromSource(vertexShaderSource, fragmentShaderSource);
    }

    int count = Data::m_Objects[m_ObjectType].size();

    m_Meshes[(int)m_ObjectType].m_Texture = m_Textures[(int)m_ObjectType];
    m_Meshes[(int)m_ObjectType].m_UseDynamicBuffer = true;
    m_Meshes[(int)m_ObjectType].m_DynamicBufferSize = sizeof(TextureVertex) * 4 * count;

    m_Meshes[(int)m_ObjectType].CreateEmptyBuffer();
}

void MapObject::AddToMesh()
{   
    auto& texture = m_Textures[(int)m_ObjectType];
    auto& mesh = m_Meshes[(int)m_ObjectType];

    glm::vec3 vertexPositions[4];
    BasicVertices::Quad::Construct(vertexPositions, m_Position, texture->m_Width * m_Scale, texture->m_Height * m_Scale);

    for (int i = 0; i < 4; i++)
    {
        TextureVertex vertex;
        vertex.position = vertexPositions[i];

        vertex.textureCoord = BasicVertices::Quad::TextureCoordinates[i];
        mesh.AddVertex(vertex);
    }

    for (int i = 0; i < 6; i++)
        mesh.AddIndex(BasicVertices::Quad::Indices[i] + mesh.GetVertices().size() - 4);
}

void MapObject::Update(bool clear)
{
    auto& texture = m_Textures[(int)m_ObjectType];
    auto& mesh = m_Meshes[(int)m_ObjectType];

    const float skyThreshold = 1000.0f;
    const float groundThreshold = -50.0f; 
    m_Scale = (0.85f / Map::m_Zoom) * 0.5f;

    float minScale = 0.125f;
    if (m_Scale < minScale)
        m_Scale = minScale;

    if (clear) mesh.Clear();

    float y = ((Data::Object*)m_ObjectData)->m_Position.y;

    if (Map::m_CurrentLayer == Map::Layers::Depths && y > groundThreshold)
        return;

    if (Map::m_CurrentLayer == Map::Layers::Surface && (y < groundThreshold || y > skyThreshold))
        return;

    if (Map::m_CurrentLayer == Map::Layers::Sky && y < skyThreshold)
        return;

    if (m_Found && !Map::m_Legend->m_Show[IconButton::ShowCompleted]) 
        return;
    
    // Culling 
    float margin = texture->m_Width + 10.0f;
    if (!Map::IsInView(m_Position, margin)) 
      return;

    // Set dynamic mesh
    AddToMesh();
}

void MapObject::Render()
{
    auto& texture = m_Textures[(int)m_ObjectType];
    auto& mesh = m_Meshes[(int)m_ObjectType];

    if (mesh.GetVertices().empty())
        return;

    mesh.Update();

    m_Shader.Bind();

    m_Shader.SetUniform("u_ProjectionMatrix", Map::m_ProjectionMatrix);
    m_Shader.SetUniform("u_ViewMatrix", Map::m_ViewMatrix);

    mesh.Render();

    m_Shader.Unbind();

    // Clear the mesh
    mesh.GetVertices().clear();
    mesh.GetIndices().clear();
}

bool MapObject::IsClicked(glm::vec2 position)
{
    auto& texture = m_Textures[(int)m_ObjectType];

    glm::vec2 worldPos = position / Map::m_Zoom + Map::m_CameraPosition;

    float largerHitbox = 1.25f;
    float width = texture->m_Width * m_Scale * largerHitbox;
    float height = texture->m_Height * m_Scale * largerHitbox; 

    if (worldPos.x > m_Position.x - width / 2 && worldPos.x < m_Position.x + width / 2)
    {
        if (worldPos.y > m_Position.y - height / 2 && worldPos.y < m_Position.y + height / 2)
        {
            return true;
        }
    }

    return false;
}

MapObject::~MapObject()
{
    auto& texture = m_Textures[(int)m_ObjectType];

    // Delete the texture (only once)
    if (texture) {
        delete texture;

        texture = nullptr;
    }

    // If the shader still exists
    if (m_Shader.m_id != 0)
        m_Shader.Delete();
}

Texture2D* MapObject::m_Textures[(int)Data::ObjectType::Count];
Shader MapObject::m_Shader;
Mesh<TextureVertex> MapObject::m_Meshes[(int)Data::ObjectType::Count];

float MapObject::m_Scale;