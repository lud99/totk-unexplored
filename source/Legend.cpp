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

    m_Arrow.Create("romfs:/icons/arrow.png");
    m_Arrow.m_Position.y = -m_Arrow.m_Texture->m_Height / 2;
    m_Arrow.m_ProjectionMatrix = &Map::m_ProjectionMatrix;

    float buttonPadding = 40.0f;//25.0f;
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
    
    m_Buttons[0].back()->Click(this, true, (int)ShowLevel::Missing);

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
        
    HidAnalogStickState analog_stick_r = padGetStickPos(Map::m_Pad, 1);

    // Get the stick position between -1.0f and 1.0f, instead of -32767 and 32767
    glm::vec2 stickRPosition = glm::vec2((float)analog_stick_r.x / (float)JOYSTICK_MAX, (float)analog_stick_r.y / (float)JOYSTICK_MAX);
   
    float deadzone = 0.4f;
    if (fabs(stickRPosition.x) >= deadzone)
    {
        if (stickRPosition.x > 0)  
        {
            if (m_Page < m_NumberOfPages - 1)
            {
                m_Page++;

                // Clamp what button is selected
                m_HighlightedButton = std::min(m_HighlightedButton, (int)m_Buttons[m_Page].size() - 1);

                UpdateSelectedButton();
            }
        } 

        if (stickRPosition.x < 0)  
        {
            if (m_Page > 0)
            {
                m_Page--;

                // Clamp what button is selected
                m_HighlightedButton = std::min(m_HighlightedButton, (int)m_Buttons[m_Page].size() - 1);

                UpdateSelectedButton();
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
        if (m_Page < m_NumberOfPages - 1)
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

    if (buttonsPressed & HidNpadButton_Y)
    {
        m_Buttons[m_Page].back()->Click(this);
    }


    // Position and flip arrow according to page
    float arrowPadding = 3.0f;
    if (m_Page == 0)
    {
        m_Arrow.m_Position.x = Map::m_ScreenLeft + m_Width - m_Arrow.m_Texture->m_Width - arrowPadding;
        m_Arrow.m_Scale = -std::abs(m_Arrow.m_Scale);
    }
        
    if (m_Page == 1)
    {
        m_Arrow.m_Position.x = Map::m_ScreenLeft + 10 + m_Arrow.m_Texture->m_Width / 2 + arrowPadding;
        m_Arrow.m_Scale = std::abs(m_Arrow.m_Scale);
    }
}

void Legend::UpdateSelectedButton()
{
    for (int j = 0; j < m_NumberOfPages; j++)
    {
        for (unsigned int i = 0; i < m_Buttons[j].size(); i++) {
            if (m_Buttons[j][i]->m_Button.m_Color == IconButton::SelectedColor)
                m_Buttons[j][i]->m_Button.m_Color = IconButton::DefaultColor;

            //if (m_Buttons[j][i]->m_Button.m_Color == IconButton::HighlightedColor)
              //  m_Show[(int)m_Buttons[j][i]->m_Type] = true;

            m_Buttons[j][i]->m_IsSelected = false;
        }
    }

    m_Buttons[m_Page][m_HighlightedButton]->m_IsSelected = true;
}

void Legend::Render()
{
    m_Background.Render();

    glm::mat4 empty(1.0);
    Map::m_Font.m_ViewMatrix = &empty;

    float viewLeft = -Map::m_CameraWidth / 2;
    float viewTop = Map::m_CameraHeight / 2;
    Map::m_Font.AddTextToBatch("Legend", glm::vec2(viewLeft + 25.0f, viewTop - 55.0f), 0.75f, glm::vec3(1.0));
    Map::m_Font.AddTextToBatch("B to close", glm::vec2(viewLeft + m_Width - 26.0f, viewTop - 55.0f), 0.5f, glm::vec3(1.0), ALIGN_RIGHT);

    for (unsigned int i = 0; i < m_Buttons[m_Page].size(); i++)
    {
        m_Buttons[m_Page][i]->Render();
    }

    m_Arrow.Render();

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

IconButton::ButtonTypes IconButton::ObjectTypeToButtonType(Data::ObjectType objectType)
{
    if (objectType == Data::ObjectType::HiddenKorok || objectType == Data::ObjectType::CarryKorok) 
        return IconButton::ButtonTypes::Koroks; 

    return IconButton::ButtonTypes((int)objectType - 1);
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
        "romfs:/icons/yiga_schematic.png",
        "romfs:/icons/Y.png"
    };

    std::string texts[] = 
    {
        "Koroks",
        "Shrines",
        "Lightroots",

        "Caves",
        "Bubbuls",
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

    float iconLeftMargin = 25.0f;
    m_Icon.m_Position = glm::vec2(position.x + iconLeftMargin, position.y - height / 2.0f);

    m_Icon.m_ProjectionMatrix = &Map::m_ProjectionMatrix;
}

void IconButton::Render()
{
    m_Button.Render();
    if (m_IsSelected) m_Border.Render();

    m_Icon.Render();

    std::string showCompletedTexts[3] = { "Show missing", "Show completed", "Show all" };
    if (m_Type == IconButton::ShowCompleted)
        m_Text = showCompletedTexts[m_State];
        
    float mainTextMargin = 25.0f;
    glm::vec2 mainTextPosition(
        m_Icon.m_Position.x + mainTextMargin, 
        m_Icon.m_Position.y - m_Height / 8.0f
    );

    float countTextMargin = 15.0f;
    glm::vec2 countTextPosition(
        m_Position.x + m_Width - countTextMargin, 
        mainTextPosition.y
    );

    SavefileIO& save = SavefileIO::Get();

    std::string countString = "";

    float fontSize = 0.425f; 

    Map::m_Font.AddTextToBatch(m_Text, mainTextPosition, fontSize, glm::vec3(1.0));

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
    }

    Map::m_Font.AddTextToBatch(countString, countTextPosition, fontSize, glm::vec3(1.0), ALIGN_RIGHT);
}

bool IconButton::Click(Legend* legend)
{
    return Click(legend, !m_IsToggled);
}

bool IconButton::Click(Legend* legend, bool toggled, int state)
{
    // Special case for Show Completed button for having multiple states
    if (m_Type == IconButton::ShowCompleted)
    {
        m_IsToggled = true;

        if (state == -1)
            m_State++;
        else
            m_State = state;

        m_State = m_State % 3;
        legend->m_ShowLevel = (Legend::ShowLevel)m_State;
    } else
    {
        m_IsToggled = toggled;

        legend->m_Show[m_Type] = m_IsToggled;
    }

    if (m_IsToggled)
        m_Button.m_Color = IconButton::HighlightedColor;
    else
        m_Button.m_Color = IconButton::DefaultColor;

    // If this button is show completed
    if (m_Type == IconButton::ShowCompleted)
    {
        // Propegate that change to all the other show completed buttons
        for (int i = 0; i < legend->m_NumberOfPages; i++)
        {
            auto showCompletedButton = legend->m_Buttons[i].back();

            if (showCompletedButton == this) continue;

            // If it states is different from this, then click it. That should prevent infinite recursion
            if (showCompletedButton->m_IsToggled != m_IsToggled || showCompletedButton->m_State != m_State)
                showCompletedButton->Click(legend, m_IsToggled, m_State);
        }
    }

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