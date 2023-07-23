#pragma once

#include "Graphics/Quad.h"

#include "Data.h"

class Legend;

class IconButton
{
public:
    enum ButtonTypes {
        Koroks = 0,
        Shrines,
        Lightroots,
        
        Caves,
        Bubbuls,
        Wells,
        Chasms,
        Locations,

        Hinoxes,
        Taluses,
        Moldugas,
        FluxConstructs,
        Froxes,
        Gleeoks,

        SagesWills,
        OldMaps,
        AddisonSigns,
        SchemaStones,
        YigaSchematics,
        ShowCompleted,
        Count
    };

public:
    IconButton();

    // Position is top-left corner
    IconButton(ButtonTypes type, glm::vec2 position, float width, float height, float iconScale = 1.0f);

    static IconButton::ButtonTypes ObjectTypeToButtonType(Data::ObjectType objectType);

    void Render();

    bool Click(Legend* legend);
    bool Click(Legend* legend, bool toggled, int state = -1);

    Data::ObjectType ButtonTypeToObjectType(ButtonTypes buttonType);

    ~IconButton();

public:
    Quad m_Button;
    TexturedQuad m_Border;
    TexturedQuad m_Icon;
    std::string m_Text;

    bool m_IsSelected = false;
    bool m_IsToggled = false;

    int m_State = 0;

    ButtonTypes m_Type = ButtonTypes::Koroks;

    static constexpr glm::vec4 HighlightedColor = glm::vec4(64.0f, 113.0f, 145.0f, 0.8f);
    static constexpr glm::vec4 DefaultColor = glm::vec4(0.0f, 0.0f, 0.0f, 0.9f);
    static constexpr glm::vec4 SelectedColor = glm::vec4(85.0f, 158.0f, 100.0f, 0.6f);

    glm::vec2 m_Position;
    float m_Width = 0.0f;
    float m_Height = 0.0f;

    glm::vec4 m_BackgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
};

class Legend
{
public:
    Legend();

    void Update();
    void Render();

    bool IsPositionOnLegend(glm::vec2 position);
    void UpdateSelectedButton();

    ~Legend();

public:
    Quad m_Background;
    TexturedQuad m_Arrow;

    std::vector<std::vector<IconButton*>> m_Buttons;

    float m_Width = 350.0f;

    int m_PrevTouchCount = 0;

    int m_HighlightedButton = 0;
    int m_Page = 0;

    enum ShowLevel {
        Missing, 
        Completed, 
        All
    };

    ShowLevel m_ShowLevel = ShowLevel::Missing;
    
    const int m_ButtonsPerPage = 11;
    int m_NumberOfPages = 2;//(int)((IconButton::ButtonTypes::Count + 1) / m_ButtonsPerPage);

    bool m_IsOpen = true;

    bool m_Show[IconButton::ButtonTypes::Count];
};