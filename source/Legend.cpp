#include "Legend.h"

#include <iostream>

#include <glm/gtc/matrix_inverse.hpp>

#include "Map.h"
#include "SavefileIO.h"

Legend::Legend()
{
    float left = -Map::m_CameraWidth / 2.0f;
    float top = Map::m_CameraHeight / 2.0f;
    float bottom = -Map::m_CameraHeight / 2.0f;

    m_Background.Create(
        glm::vec2(left, bottom),
        glm::vec2(left + m_Width, bottom),
        glm::vec2(left + m_Width, top),
        glm::vec2(left, top)
    );

    m_Background.m_ProjectionMatrix = &Map::m_ProjectionMatrix;

    m_Background.m_Color = glm::vec4(0.0f, 0.0f, 0.0f, 0.5f);

    float buttonPadding = 25.0f;
    float buttonVerticalPadding = 12.5f;

    float topOffset = 80.0f;
    float buttonHeight = 40.0f;//65.0f;

    m_Buttons.resize(m_NumberOfPages);

    for (int j = 0; j < m_NumberOfPages; j++)
    {
        auto& buttonsOnPage = m_Buttons[j];

        int end = std::min((int)IconButton::ButtonTypes::Count, (j + 1) * m_ButtonsPerPage);
        for (int i = j * m_ButtonsPerPage - (1 * j); i < end; i++)
        {
            // Dark magic
            float topY = top - (buttonHeight * ((i + j) % m_ButtonsPerPage)) - (topOffset + buttonVerticalPadding * ((i + j) % m_ButtonsPerPage));

            // Last iteration. Show the show completed buttom
            // Align the show complete button to the bottom
            if (i == end - 1)
            {
                topY = bottom + buttonHeight + buttonVerticalPadding * 3;
            }

            float posLeft = left + buttonPadding;
            float buttonWidth = m_Width - (buttonPadding * 2);

            // Last iteration. Show the show completed buttom
            IconButton* button = nullptr;
            if (i == end - 1)    
                i = (int)IconButton::ShowCompleted;

            std::cout << topY << "\n";

            button = new IconButton((IconButton::ButtonTypes)i, glm::vec2(posLeft, topY), buttonWidth, buttonHeight, 1.25f * 0.5f);
            
            button->m_Button.m_Color = IconButton::DefaultColor;

            buttonsOnPage.push_back(button);

            m_Show[i] = false;
        }
    }


    // Show koroks and shrines by default
    m_Buttons[0][0]->Click(this, true);
    m_Show[IconButton::Koroks] = true;
    m_Buttons[0][1]->Click(this, true);
    m_Show[IconButton::Shrines] = true;
    
    // Set first highlighted button
    UpdateSelectedButton();
}

void Legend::Update()
{
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

                // Iterate only the buttons on the current page
                for (unsigned int i = 0; i < m_Buttons[m_Page].size(); i++)
                {
                    IconButton* button = m_Buttons[m_Page][i];

                    float left = button->m_Position.x;
                    float right = button->m_Position.x + button->m_Width;
                    float top = button->m_Position.y;
                    float bottom = button->m_Position.y - button->m_Height;

                    if (touchPosition.x > left && touchPosition.x < right)
                    {
                        if (touchPosition.y > bottom && touchPosition.y < top)
                        {
                            button->Click(this);

                            m_HighlightedButton = i;
                            UpdateSelectedButton();
                            
                            break;
                        }
                    }
                }
            }
        }
    }

    u64 buttonsPressed = padGetButtonsDown(Map::m_Pad);
    if (buttonsPressed & HidNpadButton_Down)
    {
        if (m_HighlightedButton < (int)m_Buttons[m_Page].size() - 1)
        {
            m_HighlightedButton++;

            UpdateSelectedButton();
        }
    }
    if (buttonsPressed & HidNpadButton_Up)
    {
        if (m_HighlightedButton > 0)
        {
            m_HighlightedButton--;

            UpdateSelectedButton();
        }
    }
    if (buttonsPressed & HidNpadButton_Right)
    {
        if (m_Page < m_NumberOfPages)
        {
            m_Page++;

            // Clamp what button is selected
            m_HighlightedButton = std::min(m_HighlightedButton, (int)m_Buttons[m_Page].size() - 1);

            UpdateSelectedButton();
        }
    }
    if (buttonsPressed & HidNpadButton_Left)
    {
        if (m_Page > 0)
        {
            m_Page--;

            // Clamp what button is selected
            m_HighlightedButton = std::min(m_HighlightedButton, (int)m_Buttons[m_Page].size() - 1);

            UpdateSelectedButton();
        }
    }
    if (buttonsPressed & HidNpadButton_A)
    {
        // Bounds check
        if (m_HighlightedButton >= 0 && m_HighlightedButton < (int)m_Buttons[m_Page].size())
            m_Buttons[m_Page][m_HighlightedButton]->Click(this);
    }
}

void Legend::UpdateSelectedButton()
{
    for (int j = 0; j < m_NumberOfPages; j++)
    {
        for (unsigned int i = 0; i < m_Buttons[j].size(); i++) {
            if (m_Buttons[j][i]->m_Button.m_Color == IconButton::SelectedColor)
                m_Buttons[j][i]->m_Button.m_Color = IconButton::DefaultColor;

            m_Buttons[j][i]->m_IsSelected = false;
        }
    }

    m_Buttons[m_Page][m_HighlightedButton]->m_IsSelected = true;
}

void Legend::Render()
{
    std::cout << "Button: " << m_HighlightedButton << ", page: " << m_Page << "\n";

    m_Background.Render();

    glm::mat4 empty(1.0);
    Map::m_Font.m_ViewMatrix = &empty;

    float viewLeft = -Map::m_CameraWidth / 2;
    float viewTop = Map::m_CameraHeight / 2;
    Map::m_Font.AddTextToBatch("Legend", glm::vec2(viewLeft + 25.0f, viewTop - 55.0f), 0.75f, glm::vec3(1.0));
    Map::m_Font.AddTextToBatch("X to close", glm::vec2(viewLeft + m_Width - 26.0f, viewTop - 55.0f), 0.5f, glm::vec3(1.0), ALIGN_RIGHT);

    for (unsigned int i = 0; i < m_Buttons[m_Page].size(); i++)
    {
        m_Buttons[m_Page][i]->Render();
    }

    // Render the Show Complete button. it should alwa
    //m_Buttons.back().back()->Render();

    Map::m_Font.m_ViewMatrix = &Map::m_ViewMatrix;
}

bool Legend::IsPositionOnLegend(glm::vec2 position)
{
    float left = -Map::m_CameraWidth / 2;
    if (position.x > left && position.x < left + m_Width) {
        if (position.y > -Map::m_CameraHeight / 2 && position.y < Map::m_CameraWidth / 2) {
            return true;
        }
    }

    return false;
}

Legend::~Legend()
{
    for (int j = 0; j < m_NumberOfPages; j++)
    {
        for (unsigned int i = 0; i < m_Buttons.size(); i++)
            delete m_Buttons[j][i];
    }
}

IconButton::IconButton()
{

}

IconButton::IconButton(ButtonTypes type, glm::vec2 position, float width, float height, float iconScale)
{
    m_Width = width;
    m_Height = height;
    m_Position = position;
    m_Type = type;

    std::string iconPaths[] = 
    {
        "romfs:/icons/korok_hidden.png",
        "romfs:/icons/shrine.png",
        "romfs:/icons/lightroot.png",
        "romfs:/icons/bubbul.png",

        "romfs:/icons/cave.png",
        "romfs:/icons/well.png",
        "romfs:/icons/chasm.png",
        "romfs:/icons/location.png",

        "romfs:/icons/hinox.png",
        "romfs:/icons/talus.png",
        "romfs:/icons/molduga.png",
        "romfs:/icons/flux_construct.png",
        "romfs:/icons/frox.png",
        "romfs:/icons/gleeok.png",

        "romfs:/icons/sages_will.png",
        "romfs:/icons/old_map.png",
        "romfs:/icons/addison_sign.png", 
        "romfs:/icons/schema_stone.png",
        "romfs:/icons/yiga_schematic.png",
        ""
    };

    std::string texts[] = 
    {
        "Koroks",
        "Shrines",
        "Lightroots",
        "Bubbuls",

        "Caves",
        "Wells",
        "Chasms",
        "Locations",

        "Hinoxes",
        "Taluses",
        "Moldugas",
        "Flux Constructs",
        "Froxes",
        "Gleeoks",

        "Sage's Wills",
        "Old Maps",
        "Addison Signs",
        "Schema Stones",
        "Yiga Schematics",
        "Show Completed"
    };

    m_Text = texts[(int)m_Type];

    m_Button.Create(
        glm::vec2(position.x, position.y - height),
        glm::vec2(position.x + width, position.y - height),
        glm::vec2(position.x + width, position.y),
        glm::vec2(position.x, position.y)
    );

    m_Button.m_ProjectionMatrix = &Map::m_ProjectionMatrix;

    m_Border.Create(
        "romfs:/buttonborder.png",
        glm::vec2(position.x, position.y - height),
        glm::vec2(position.x + width, position.y - height),
        glm::vec2(position.x + width, position.y),
        glm::vec2(position.x, position.y)
    );
    m_Border.m_ProjectionMatrix = &Map::m_ProjectionMatrix;

    std::string iconPath = iconPaths[(int)m_Type];
    if (iconPath != "")
        m_Icon.Create(iconPath);

    m_Icon.m_Scale = iconScale;

    float iconLeftMargin = 35.0f;
    m_Icon.m_Position = glm::vec2(position.x + iconLeftMargin, position.y - height / 2.0f);

    m_Icon.m_ProjectionMatrix = &Map::m_ProjectionMatrix;
}

void IconButton::Render()
{
    m_Button.Render();
    if (m_IsSelected) m_Border.Render();

    m_Icon.Render();

    float mainTextMargin = 35.0f;
    glm::vec2 mainTextPosition(
        m_Icon.m_Position.x + mainTextMargin, 
        m_Icon.m_Position.y - m_Height / 8.0f
    );

    float countTextMargin = 20.0f;
    glm::vec2 countTextPosition(
        m_Position.x + m_Width - countTextMargin, 
        mainTextPosition.y
    );

    SavefileIO& save = SavefileIO::Get();

    std::string countString = "";

    Map::m_Font.AddTextToBatch(m_Text, mainTextPosition, 0.45f, glm::vec3(1.0));

    if (!save.LoadedSavefile) return;

    switch (m_Type)
    {
    case Koroks:
        countString = std::to_string(save.loadedData.found[Data::ObjectType::HiddenKorok].size() + save.loadedData.found[Data::ObjectType::CarryKorok].size()) + 
            "/" + std::to_string(Data::m_Objects[Data::ObjectType::HiddenKorok].size() + Data::m_Objects[Data::ObjectType::CarryKorok].size());
        break;

    case ShowCompleted:
    case Count:
        break;
   
    default:
        countString = std::to_string(save.loadedData.found[ButtonTypeToObjectType(m_Type)].size()) + "/" + std::to_string(Data::m_Objects[ButtonTypeToObjectType(m_Type)].size());
        break;
    
    /*case Locations:
        countString = std::to_string(save.visitedLocations.size()) + "/" + std::to_string(Data::LocationsCount);
        break;*/
    }

    Map::m_Font.AddTextToBatch(countString, countTextPosition, 0.45f, glm::vec3(1.0), ALIGN_RIGHT);
}

bool IconButton::Click(Legend* legend)
{
    m_IsToggled = !m_IsToggled;

    if (m_IsToggled)
        m_Button.m_Color = IconButton::HighlightedColor;
    else
        m_Button.m_Color = IconButton::DefaultColor;

    legend->m_Show[m_Type] = m_IsToggled;

    return m_IsToggled;
}

bool IconButton::Click(Legend* legend, bool toggled)
{
    m_IsToggled = toggled;

    if (m_IsToggled)
        m_Button.m_Color = IconButton::HighlightedColor;
    else
        m_Button.m_Color = IconButton::DefaultColor;

    legend->m_Show[m_Type] = m_IsToggled;

    return m_IsToggled;
}

Data::ObjectType IconButton::ButtonTypeToObjectType(ButtonTypes buttonType)
{
    assert(buttonType != ButtonTypes::Koroks);

    return Data::ObjectType((int)buttonType + 1);
}

IconButton::~IconButton()
{
    
}

constexpr glm::vec4 IconButton::HighlightedColor;
constexpr glm::vec4 IconButton::DefaultColor;
constexpr glm::vec4 IconButton::SelectedColor;