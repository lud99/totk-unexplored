#include "ObjectInfo.h"

#include "Data.h"
#include "Map.h"
#include "SavefileIO.h"
#include "Log.h"
#include "MapObject.h"

glm::vec3 ToInGameCoordinates(glm::vec3 realCoordinates)
{
	return { realCoordinates.x, -realCoordinates.z, realCoordinates.y - 105.0f };
}

std::string PadIntWithZeros(int value, int digits = 4)
{
    bool isNegative = value < 0;
    std::string s = std::to_string(std::abs(value));

    int numberOfZeroes = digits - s.length();

    for (int i = 0; i < numberOfZeroes; i++)
    {
        s = "0" + s;
    }

    if (isNegative) s = "-" + s;

    return s;
}

std::string ObjectPositionToString(Data::Object* object)
{
    glm::vec3 coords = ToInGameCoordinates(object->m_Position);

    std::string result = "";
    result += PadIntWithZeros((int)coords.x) + "  ";
    result += PadIntWithZeros((int)coords.y) + "  ";
    result += PadIntWithZeros((int)coords.z);
    return result;
}

ObjectInfo::ObjectInfo()
{
    m_Background.Create(
        glm::vec2(Map::m_ScreenLeft, Map::m_ScreenBottom),
        glm::vec2(Map::m_ScreenLeft + Width, Map::m_ScreenBottom),
        glm::vec2(Map::m_ScreenLeft + Width, Map::m_ScreenTop),
        glm::vec2(Map::m_ScreenLeft, Map::m_ScreenTop)
    );
    m_Background.m_Color = glm::vec4(0.0f, 0.0f, 0.0f, 0.6f);

    m_Icon.Create("");
    m_Icon.m_Position = glm::vec2(Map::m_ScreenLeft + Width / 2 - m_Icon.m_Texture->m_Width / 2, Map::m_ScreenTop - 50.0f);

    m_Icon.m_ProjectionMatrix = &Map::m_ProjectionMatrix;
}

void ObjectInfo::Render(glm::mat4 projMat, glm::mat4 viewMat)
{
    if (!m_IsOpen) 
        return; 

    m_Background.m_ProjectionMatrix = &projMat;
    m_Background.m_ViewMatrix = nullptr;

    m_Background.Render();

    m_Icon.Render();
    
    // Place the text just below the image
    float textY = Map::m_ScreenTop - 110.0f;

    // Render text
    glm::vec2 textStartPos(Map::m_ScreenLeft + m_Margin, textY);
    glm::vec2 textPos = textStartPos;

    float padding = 5;

    m_Icon.m_Position = glm::vec2(Map::m_ScreenLeft + Width - m_Icon.m_Texture->m_Width - padding, textY + m_Icon.m_Texture->m_Height / 4);
    m_Icon.m_Scale = 0.8f;

    std::string title = MapObject::Names[(int)m_Type]; 
    std::string name = m_Object->m_DisplayName;
    std::string position = ObjectPositionToString(m_Object); 

    // Title (what the object is)
    glm::vec2 textSize = Map::m_Font.AddTextToBatch(title, textPos, 0.75f, glm::vec3(1.0f), ALIGN_LEFT, Width - m_Margin * 2 - padding);
    textPos.y -= textSize.y + 25;

    // The name of it
    if (m_Type == Data::ObjectType::Bubbul)
        name = "Bubbul in " + name; 

    if (name != "")
    {
        textSize = Map::m_Font.AddTextToBatch(name, textPos, 0.4f, glm::vec3(1.0f), ALIGN_LEFT, Width - m_Margin * 2 - padding);
        textPos.y -= textSize.y + 50;
    } else 
    {
        textPos.y -= 20;
    }
        
    // In game position
    textSize = Map::m_Font.AddTextToBatch("Position", textPos, 0.5f, glm::vec3(1.0f), ALIGN_LEFT, Width - m_Margin * 2);
    textPos.y -= textSize.y + 25;
    textSize = Map::m_Font.AddTextToBatch(position, textPos, 0.4f, glm::vec3(1.0f), ALIGN_LEFT, Width - m_Margin * 2);
    textPos.y -= textSize.y + 50;

    // Other info
    std::string otherInfo = "";
    if (m_Type == Data::ObjectType::Cave)
    {
        otherInfo = "This cave has n entrances";
    } else if (m_Type == Data::ObjectType::HiddenKorok || m_Type == Data::ObjectType::CarryKorok)
    {
        Data::Korok* korok = (Data::Korok*) m_Object;
        otherInfo = korok->m_KorokType;
        if (korok->m_Path)
        {
            textSize = Map::m_Font.AddTextToBatch("To find this korok, start at the empty part of the path", textPos, 0.5f, glm::vec3(1.0f), ALIGN_LEFT, Width - m_Margin * 2);
            textPos.y -= textSize.y + 50;
        }
    }

    textSize = Map::m_Font.AddTextToBatch(otherInfo, textPos, 0.5f, glm::vec3(1.0f), ALIGN_LEFT, Width - m_Margin * 2);
    textPos.y -= textSize.y + 50;

    //Map::m_Font.AddTextToBatch(m_Text, textPos, 0.5f, glm::vec3(1.0f), ALIGN_LEFT, Width - m_Margin * 2);
    Map::m_Font.AddTextToBatch("X to close", glm::vec2(Map::m_ScreenLeft + Width - 20.0f, Map::m_ScreenTop - 35.0f), 0.45f, glm::vec3(1.0f), ALIGN_RIGHT);
    
    if (m_Type == Data::ObjectType::HiddenKorok || m_Type == Data::ObjectType::CarryKorok)
        Map::m_Font.AddTextToBatch("B to mark as found", glm::vec2(Map::m_ScreenLeft + 15, Map::m_ScreenTop - 35.0f), 0.45f, glm::vec3(1.0f), ALIGN_LEFT);
}

void ObjectInfo::SetOpen(bool open)
{
    m_IsOpen = open;
}

void ObjectInfo::SetObject(Data::ObjectType type, Data::Object* object)
{
    m_Type = type;
    m_Object = object;

    m_Icon.Create(MapObject::IconPaths[(int)m_Type]);
}

void ObjectInfo::SetPosition(glm::vec2 position)
{
    m_Background.m_Position = position;
}

glm::vec2 ObjectInfo::GetPosition()
{
    return m_Background.m_Position;
}

ObjectInfo::~ObjectInfo()
{
    
}