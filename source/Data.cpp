#include "Data.h"

#include <json.hpp>

#include <iostream>
#include <map>
#include <vector>
#include <fstream>

#include "Log.h"

using namespace Data;

namespace Data
{
    Object::Object() {};
    Object::Object(uint32_t completionHash, glm::vec3 position, std::string displayName)
    {
        m_CompletionHash = completionHash;
        m_Position = position;

        m_DisplayName = displayName;
    }

    KorokPath::KorokPath(uint32_t belongsToCompletionHash, std::vector<glm::vec3> points)
    {
        m_BelongsToCompletionHash = m_BelongsToCompletionHash;
        m_Points = points; 
    }

    Korok::Korok(uint32_t completionHash, glm::vec3 position, bool isPair, std::string korokType)
    {
        m_CompletionHash = completionHash;
        m_Position = position;

        m_IsPair = isPair;
        m_KorokType = korokType;
    }

    /*Cave::Cave(uint32_t completionHash, glm::vec3 position, const std::string& displayName, uint32_t bubbulDefeatedHash)
    {
        m_CompletionHash = completionHash;
        m_Position = position;

        m_DisplayName = displayName;

        m_BubbulDefeatedHash = bubbulDefeatedHash;
    }*/
}

void Data::LoadFromJSON(const std::string& filepath)
{
    Log("loading");
    using namespace nlohmann;

    std::ifstream file(filepath);
    std::stringstream strStream;
    strStream << file.rdbuf();
    std::string fileContents = strStream.str();

    file.close();

    json fileData = json::parse(fileContents);

    auto ParseJSONPosition = [] (json& position) -> glm::vec3 {
        return glm::vec3((float)position["x"], (float)position["y"], (float)position["z"]);
    };  

    auto vec3ToVec2 = [] (glm::vec3 position) -> glm::vec2 {
        return glm::vec2(position.x, position.z);
    };  


    Log("shrines");
    // Shrines
    for (auto& entry : fileData["shrines"])
    {
        Object* obj = new Object(
            uint32_t(entry["completion_hash"]), 
            ParseJSONPosition(entry["position"]),
            std::string(entry["display_name"])
        );

        m_Objects[ObjectType::Shrine].push_back(obj);
    }

    // Lightroots
    for (auto& entry : fileData["lightroots"])
    {
        Object* obj = new Object(
            uint32_t(entry["completion_hash"]), 
            ParseJSONPosition(entry["position"]),
            std::string(entry["display_name"])
        );

        m_Objects[ObjectType::Lightroot].push_back(obj);
    }

    // Single koroks
    for (auto& entry : fileData["hidden_koroks"])
    {
        Korok* obj = new Korok(
            uint32_t(entry["completion_hash"]), 
            ParseJSONPosition(entry["position"]), 
            false, // single korok
            std::string(entry["type"])
        );

        if (entry.contains("path"))
        {
            // Korok with Start -> End
            if (entry["path"].contains("start") && entry["path"].contains("end")) 
            {
                glm::vec3 start = ParseJSONPosition(entry["path"]["start"]);
                glm::vec3 end = ParseJSONPosition(entry["path"]["end"]);

                obj->m_Path = new KorokPath(obj->m_CompletionHash, { start, end });
            }
            //if (entry["path"].contains("flowers"))
        }

        m_Objects[ObjectType::HiddenKorok].push_back((Object*)obj);
    }
    
    // Carry koroks
    for (auto& entry : fileData["carry_koroks"])
    {
        Korok* obj = new Korok(
            uint32_t(entry["completion_hash"]), 
            ParseJSONPosition(entry["position"]), 
            true, // carry korok
            std::string(entry["type"])
        );

        if (entry.contains("path"))
        {
            // Korok with Start -> End
            if (entry["path"].contains("start") && entry["path"].contains("end")) 
            {
                glm::vec3 start = ParseJSONPosition(entry["path"]["start"]);
                glm::vec3 end = ParseJSONPosition(entry["path"]["end"]);

                obj->m_Path = new KorokPath(obj->m_CompletionHash, { start, end });
            }
            //if (entry["path"].contains("flowers"))
        }

        m_Objects[ObjectType::CarryKorok].push_back((Object*)obj);
    }

    // Bubbuls
    for (auto& entry : fileData["bubbuls"])
    {
        Object* obj = new Object(
            uint32_t(entry["completion_hash"]), 
            ParseJSONPosition(entry["position"]),
            std::string(entry["display_name"]) 
        );

        m_Objects[ObjectType::Bubbul].push_back(obj);
    }

    Log("locations");
    // Locations
    for (auto& entry : fileData["locations"])
    {
        Object* obj = new Object(
            uint32_t(entry["completion_hash"]), 
            ParseJSONPosition(entry["position"]),
            std::string(entry["display_name"])
        );

        m_Objects[ObjectType::Location].push_back(obj);
    }

    // Caves
    Log("caves");
    for (auto& entry : fileData["caves"])
    {
        // Skip duplicates
        //if (ObjectExists(ObjectType::Cave, uint32_t(entry["completion_hash"])))
          //  continue;

        Object* obj = new Object(
            uint32_t(entry["completion_hash"]), 
            ParseJSONPosition(entry["position"]),
            std::string(entry["display_name"])
        );

        m_Objects[ObjectType::Cave].push_back(obj);
    }

    // Wells
    Log("well");
    for (auto& entry : fileData["wells"])
    {
        Object* obj = new Object(
            uint32_t(entry["completion_hash"]), 
            ParseJSONPosition(entry["position"]),
            std::string(entry["display_name"])
        );

        m_Objects[ObjectType::Well].push_back(obj);   
    }

    // Chasms
    Log("chasm");
    for (auto& entry : fileData["chasms"])
    {
        Object* obj = new Object(
            uint32_t(entry["completion_hash"]), 
            ParseJSONPosition(entry["position"]),
            std::string(entry["display_name"])
        );

        m_Objects[ObjectType::Chasm].push_back(obj);
    }

    // Bosses
    std::string bosses[] = { "hinoxes", "taluses", "moldugas", "flux_constructs", "froxes", "gleeoks" };
    for (int i = 0; i < 6; i++)
    {
        for (auto& entry : fileData[bosses[i]])
        {
            Object* obj = new Object(
                uint32_t(entry["completion_hash"]), 
                ParseJSONPosition(entry["position"])
            );  

            // :)
            m_Objects[ObjectType((int)ObjectType::Hinox + i)].push_back(obj);
        }
    }

    // Old maps
    Log("old mpa");
    for (auto& entry : fileData["old_maps"])
    {
        Object* obj = new Object(
            uint32_t(entry["completion_hash"]), 
            ParseJSONPosition(entry["position"])
        );

        m_Objects[ObjectType::OldMap].push_back(obj);
    }

    // Sages wills
    Log("sagesw");
    for (auto& entry : fileData["sages_wills"])
    {
        Object* obj = new Object(
            0,
            ParseJSONPosition(entry["position"])
        );

        obj->m_CompletionGuid = std::stoul(std::string(entry["guid_hash"]));

        m_Objects[ObjectType::SagesWill].push_back(obj);
    }

    // Addison signposts
    Log("addison");
    for (auto& entry : fileData["addison_signposts"])
    {
        Object* obj = new Object(
            0,
            ParseJSONPosition(entry["position"])
        );

        obj->m_CompletionGuid = std::stoul(std::string(entry["guid_hash"]));

        m_Objects[ObjectType::AddisonSign].push_back(obj);
    }

    // Schema stones
    Log("schema");
    for (auto& entry : fileData["schema_stones"])
    {
        Object* obj = new Object(
            uint32_t(entry["completion_hash"]), 
            ParseJSONPosition(entry["position"]),
            std::string(entry["display_name"])
        );

        m_Objects[ObjectType::SchemaStone].push_back(obj);
    }
    // Yiga schematics
    for (auto& entry : fileData["yiga_schematics"])
    {
        Object* obj = new Object(
            uint32_t(entry["completion_hash"]), 
            ParseJSONPosition(entry["position"]),
            std::string(entry["display_name"])
        );

        m_Objects[ObjectType::YigaSchematic].push_back(obj);
    }

    // Logging
    for (int i = 0; i < (int)ObjectType::Count; i++)
    {
        std::cout << "Object " << i << ": " <<  m_Objects[ObjectType(i)].size() << " entries\n";
    }
}

void Data::UnloadData()
{
    for (int i = 0; i < (int)ObjectType::Count; i++)
    {
        for (auto entry : m_Objects[ObjectType(i)])
        {
            // Delete korok paths if they exists
            ObjectType type = ObjectType(i);
            if (type == ObjectType::HiddenKorok || type == ObjectType::CarryKorok)
            {
                Data::Korok* korok = (Data::Korok*)entry;
                if (korok->m_Path)
                    delete korok->m_Path;
            }

            if (entry != nullptr)
                delete entry;
        }
    }
}

Data::Object* Data::GetObjectByHash(ObjectType type, uint32_t hash)
{
    for (auto entry : m_Objects[type])
    {
        if (entry->m_CompletionHash == hash)
            return entry;        
    }

    return nullptr;
}

Data::Object* Data::GetObjectByGuid(ObjectType type, uint64_t guid)
{
    for (auto entry : m_Objects[type])
    {
        if (entry->m_CompletionGuid == guid)
            return entry;        
    }

    return nullptr;
}

bool Data::ObjectExists(ObjectType type, uint32_t hash)
{
    return GetObjectByHash(type, hash) != nullptr;
}

bool Data::ObjectExistsByGuid(ObjectType type, uint64_t guid)
{
    return GetObjectByGuid(type, guid) != nullptr;
}

std::unordered_map<Data::ObjectType, std::vector<Data::Object*>> Data::m_Objects;

/*
Data::Korok Data::Koroks;

Data::ObjectWithName Data::Shrines;
Data::ObjectWithName Data::Lightroots;

Data::Cave Data::Caves; 
Data::ObjectWithName Data::Wells; 

Data::Object Data::Hinoxes;
Data::Object Data::Taluses;
Data::Object Data::Moldugas;
Data::Object Data::FluxConstructs;
Data::Object Data::Froxes;
Data::Object Data::Gleeoks;*/