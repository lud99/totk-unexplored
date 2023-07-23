#include "MapObject.h"

#include "Graphics/Texture2D.h"
#include "Graphics/BasicVertices.h"

#include "Map.h"
#include "Legend.h"
#include "UI/LayerNavigation.h"

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
    m_Scale = (0.85f / Map::m_Zoom) * 0.5f;

    if (m_ObjectType == Data::ObjectType::Bubbul)
        m_Scale *= 0.5f;

    // For text
    if (m_ObjectType == Data::ObjectType::Location)
    {
        float minScale = 0.125f * 1.25f;
        if (m_Scale < minScale)
            m_Scale = minScale;

        if (clear)
        {
            Map::m_LocationsFont.BeginBatch();
            Map::m_LocationsFont2.BeginBatch();
        }

        if (!IsVisible(false))
            return;

        float margin = 100.0f;
        margin += margin * m_Scale;
        if (!Map::IsInView(m_Position, margin)) 
            return;

        glm::vec3 color = glm::vec3(190.0f, 177.0f, 112.0f) / glm::vec3(255.0f);
        //glm::vec3 colorOutline = glm::vec3(96.0f, 86.0f, 38.0f) / glm::vec3(255.0f);
        Map::m_LocationsFont.AddTextToBatch(m_ObjectData->m_DisplayName, m_Position, m_Scale * 0.5f, color, ALIGN_CENTER);
        //Map::m_LocationsFont2.AddTextToBatch(m_ObjectData->m_DisplayName, m_Position, m_Scale * 0.54f, colorOutline, ALIGN_CENTER);
    
        return;
    }

    float minScale = 0.125f * 0.25f;
    if (m_Scale < minScale)
        m_Scale = minScale;

    // For other objects
    if (clear) 
    {
        auto& mesh = m_Meshes[(int)m_ObjectType];
        mesh.Clear();
    }

    if (!IsVisible())
        return;

    // Set dynamic mesh
    AddToMesh();
}

void MapObject::Render()
{
    if (m_ObjectType == Data::ObjectType::Location)
    {
        //Map::m_LocationsFont2.m_ViewMatrix = &Map::m_ViewMatrix;
        //Map::m_LocationsFont2.RenderBatch();

        Map::m_LocationsFont.m_ViewMatrix = &Map::m_ViewMatrix;
        Map::m_LocationsFont.RenderBatch();



        return;
    }
        
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

bool MapObject::IsClicked(glm::vec2 position, bool ignoreVisibility)
{
    if (!IsVisible() && !ignoreVisibility) return false;

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

bool MapObject::IsVisible(bool culling)
{
    if (culling)
    {
        auto& texture = m_Textures[(int)m_ObjectType];
        float margin = texture->m_Width + 10.0f;
        if (!Map::IsInView(m_Position, margin)) 
            return false;
    }

    // Check if it should be shown
    if (!Map::m_Legend->m_Show[IconButton::ObjectTypeToButtonType(m_ObjectType)])
        return false;

    float y = m_ObjectData->m_Position.y;

    const float skyThreshold = 750.0f;
    const float groundThreshold = -50.0f; 

    Layer currentLayer = Map::m_LayerNavigation->GetLayer();

    if (currentLayer == Layer::Depths && y > groundThreshold)
        return false;

    if (currentLayer == Layer::Surface && (y < groundThreshold || y > skyThreshold))
        return false;

    if (currentLayer == Layer::Sky && y < skyThreshold)
        return false;

    // Cases for show levels
    // Only missing
    if (Map::m_Legend->m_ShowLevel == Legend::ShowLevel::Missing && m_Found)
        return false;

    // Only found
    if (Map::m_Legend->m_ShowLevel == Legend::ShowLevel::Completed && !m_Found)
        return false;

    // Missing and found
    if (Map::m_Legend->m_ShowLevel == Legend::ShowLevel::All)
        return true;

    return true;
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

const std::string MapObject::IconPaths[20] = {
    "romfs:/icons/korok_hidden.png",
    "romfs:/icons/korok_carry.png",
    "romfs:/icons/shrine.png",
    "romfs:/icons/lightroot.png",

    "romfs:/icons/cave.png",
    "romfs:/icons/bubbul.png",
    "romfs:/icons/well.png",
    "romfs:/icons/chasm.png",
    "romfs:/icons/location.png",

    "romfs:/icons/eye.png",
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

const std::string MapObject::Names[20] = {
    "Hidden Korok",
    "Lonely Korok",
    "Shrine",
    "Lightroot",

    "Cave",
    "Bubbul",
    "Well",
    "Chasm",
    "Location",

    "Hinox",
    "Talus",
    "Molduga",
    "Flux Construct",
    "Frox",
    "Gleeok",

    "Sage's Will",
    "Old Map",
    "Addison Sign",
    "Schema Stone",
    "Yiga Schematic"
};