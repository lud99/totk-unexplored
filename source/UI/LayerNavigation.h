#pragma once

#include "../Graphics/Quad.h"

enum class Layer 
{
    Depths,
    Surface,
    Sky,
    Count
};

class LayerNavigation
{
public:
    LayerNavigation();

    void SetLayer(Layer layer);
    Layer GetLayer();

    void Up();
    void Down();

    void Update();
    void Render();

    void UpdateSelectedButton();

    // TextureQuad has its position at the center of the texture
    bool ButtonIsClicked(TexturedQuad& button, glm::vec2 touchPosition);

    bool IsPositionOn(glm::vec2 position);

    ~LayerNavigation();

private:
    Layer m_CurrentLayer = Layer::Surface;
    Layer m_PreviousLayer = m_CurrentLayer;

    TexturedQuad m_Buttons[3];
    TexturedQuad m_UpIcon;

    int m_PrevTouchCount = 0;
};