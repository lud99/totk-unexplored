#include "LayerNavigation.h"

#include <switch.h>

#include <iostream>

#include "../Map.h"
#include "../Legend.h"

LayerNavigation::LayerNavigation()
{
    std::string icons[3] = { "romfs:/icons/depths.png", "romfs:/icons/surface.png", "romfs:/icons/sky.png" };
    
    float paddingX = 20.0f;

    for (int i = 0; i < 3; i++)
    {
        
        m_Buttons[i].Create(icons[i]);
        m_Buttons[i].m_Shader.Delete();

        std::string vertexShaderSource = R"text(
            #version 330 core

            layout (location = 0) in vec3 position;
            layout (location = 1) in vec4 color;
            layout (location = 2) in vec2 texCoord;

            out vec2 passTextureCoord;

            uniform mat4 u_ProjectionMatrix = mat4(1.0);
            uniform mat4 u_ViewMatrix = mat4(1.0);
            uniform mat4 u_ModelMatrix = mat4(1.0);
            uniform float u_Scale;

            void main()
            {
                gl_Position = u_ProjectionMatrix * u_ViewMatrix * u_ModelMatrix * vec4(position * u_Scale, 1.0);

                passTextureCoord = texCoord;
            }
        )text";

        std::string fragmentShaderSource = R"text(
            #version 330 core

            layout (location = 0) out vec4 color;
            in vec2 passTextureCoord;

            uniform sampler2D tex;
            uniform vec4 u_MultColor;

            void main()
            {
                color = texture(tex, passTextureCoord) * u_MultColor;

                if (color.a == 0.0)
                    discard;
            }
        )text";

        m_Buttons[i].m_Shader = ShaderLoader::CreateShaderFromSource(vertexShaderSource, fragmentShaderSource);

        m_Buttons[i].m_Position.x = Map::m_ScreenRight - m_Buttons[i].m_Texture->m_Width / 2 - paddingX;
        
        m_Buttons[i].m_Scale = 1.0f;

        m_Buttons[i].m_ProjectionMatrix = &Map::m_ProjectionMatrix;
        m_Buttons[i].m_ViewMatrix = nullptr;
    }

    m_Buttons[0].m_Position.y = -80;
    m_Buttons[1].m_Position.y = 0;
    m_Buttons[2].m_Position.y = 80;

    // Icon
    m_UpIcon.Create("romfs:/icons/dpad-up.png");
    m_UpIcon.m_Position.x = m_Buttons[0].m_Position.x;

    m_UpIcon.m_ProjectionMatrix = &Map::m_ProjectionMatrix;
    m_UpIcon.m_ViewMatrix = nullptr;

}

void LayerNavigation::SetLayer(Layer layer)
{
    m_CurrentLayer = layer;

    // if (m_CurrentLayer != m_PreviousLayer)
    //     Map::m_TargetCursorPosition = glm::vec2(0.0f, 0.0f);

    m_PreviousLayer = m_CurrentLayer;
}

Layer LayerNavigation::GetLayer()
{
    return m_CurrentLayer;
}

void LayerNavigation::Up()
{
    int newLayer = (int)m_CurrentLayer + 1;
    if (newLayer <= (int)Layer::Sky)
    {
        SetLayer((Layer)newLayer);
        Map::m_TargetCursorPosition = glm::vec2(0.0f, 0.0f);
    }
        
}
void LayerNavigation::Down()
{
    int newLayer = (int)m_CurrentLayer - 1;
    if (newLayer >= (int)Layer::Depths)
    {
        SetLayer((Layer)newLayer);
        Map::m_TargetCursorPosition = glm::vec2(0.0f, 0.0f);
    }
}

void LayerNavigation::Update()
{
    u64 buttonsPressed = padGetButtonsDown(Map::m_Pad);

    if (!Map::m_Legend->m_IsOpen)
    {
        if (buttonsPressed & HidNpadButton_Up)
            Up();
        if (buttonsPressed & HidNpadButton_Down)
            Down();
    }

    HidTouchScreenState state={0};
    if (hidGetTouchScreenStates(&state, 1))
    {
        // A new touch
        if (state.count != m_PrevTouchCount)
        {   
            m_PrevTouchCount = state.count;

            if (state.count == 1)
            {
                // Convert to more suitable coords
                glm::vec2 touchPosition = glm::vec2(state.touches[0].x - Map::m_CameraWidth / 2, -(state.touches[0].y - Map::m_CameraHeight / 2)); 

                for (int i = (int)Layer::Depths; i < (int)Layer::Count; i++)
                {
                    if (ButtonIsClicked(m_Buttons[i], touchPosition))
                        SetLayer((Layer)i);
                }
            }
        }
    }
}

void LayerNavigation::Render()
{
    for (int i = 0; i < 3; i++)
    {
        auto& button = m_Buttons[i];
        if (i == (int)GetLayer())
        {
            button.m_Shader.Bind();
            button.m_Shader.SetUniform("u_MultColor", glm::vec4(201.0f, 178.0f, 255.0f, 255.0f) / glm::vec4(255.0f));
        } else
        {
            button.m_Shader.Bind();
            button.m_Shader.SetUniform("u_MultColor", glm::vec4(1.0f));
        }

        button.Render(); 

        button.m_Shader.Bind();
        button.m_Shader.SetUniform("u_MultColor", glm::vec4(1.0f));
    }

    if (Map::m_Legend->m_IsOpen) return;

    // Up
    m_UpIcon.m_Position.y = m_Buttons[2].m_Position.y + 50.0f;
    m_UpIcon.m_Scale = 1.0f;

    m_UpIcon.Render();

    // Down
    m_UpIcon.m_Position.y = m_Buttons[0].m_Position.y - 50.0f;
    m_UpIcon.m_Scale = -1.0f;

    m_UpIcon.Render();
}

void LayerNavigation::UpdateSelectedButton()
{

}

bool LayerNavigation::ButtonIsClicked(TexturedQuad& button, glm::vec2 touchPosition)
{
    float width = button.m_Texture->m_Width * button.m_Scale;
    float height = button.m_Texture->m_Height * button.m_Scale;

    float left = button.m_Position.x - width / 2;
    float right = button.m_Position.x + width / 2;
    float top = button.m_Position.y + height / 2;
    float bottom = button.m_Position.y - height / 2;

    if (touchPosition.x > left && touchPosition.x < right)
    {
        if (touchPosition.y > bottom && touchPosition.y < top)
           return true;
    }

    return false;
}

bool LayerNavigation::IsPositionOn(glm::vec2 position)
{
    for (int i = 0; i < 3; i++)
    {
        auto& button = m_Buttons[i];
        if (ButtonIsClicked(button, position))
            return true;
    }

    return false;
}

LayerNavigation::~LayerNavigation()
{

}