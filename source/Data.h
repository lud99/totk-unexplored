#pragma once

#include <unordered_map>
#include <cstdint>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <map>

namespace Data
{
    struct Object
    {
        uint32_t m_CompletionHash;

        glm::vec3 m_Position;

        Object();
        Object(uint32_t completionHash, glm::vec3 position);
    };

    struct ObjectWithName : public Object
    {
        std::string m_DisplayName;

        ObjectWithName();
        ObjectWithName(uint32_t completionHash, glm::vec3 position, std::string displayName);
    };

    struct KorokPath
    {
        std::string internalName;
        std::vector<glm::vec2> points;

        KorokPath(const std::string& internalName, std::vector<glm::vec2>& points) :
            internalName(internalName), points(points) {}

        KorokPath() {};
    };

    struct Korok : public Object
    {
        int m_ZeldaDungeonId;

        bool m_IsPair = false;

        //Data::KorokPath* path = nullptr;

        Korok() {};

        Korok(uint32_t completionHash, glm::vec3 position, int zeldaDungeonId, bool isPair);/* :
            completionHash(hash), internalName(internalName), x(x), y(y), zeldaDungeonId(zeldaDungeonId) {};*/
    };

    struct Cave : public ObjectWithName
    {
        uint32_t m_BubbulDefeatedHash;

        Cave() {};

        Cave(uint32_t completionHash, glm::vec3 position, const std::string& displayName, uint32_t bubbulDefeatedHash);/* :
            hash(hash), x(x), y(y) {};*/
    };

    // From https://github.com/MrCheeze/botw-tools/blob/master/gamedata/s32_data_0.xml
    // and https://objmap.zeldamods.org/#/map/z3,0,0

    // struct DLCShrine
    // {
    //     uint32_t hash;

    //     float x = 0;
    //     float y = 0;

    //     DLCShrine(uint32_t hash, float x, float y) : hash(hash), x(x), y(y) {};
    // };

    // For location names and their positions, https://github.com/MrCheeze/botw-waypoint-map/blob/gh-pages/map_locations.js
    // For the location hashes, https://github.com/marcrobledo/savegame-editors/blob/master/zelda-botw/zelda-botw.locations.js

    // struct Location
    // {
    //     uint32_t hash;

    //     std::string displayName;

    //     float x = 0;
    //     float y = 0;

    //     Location(uint32_t hash, const std::string& displayName, float x, float y) :
    //         hash(hash), displayName(displayName), x(x), y(y) {};
    // };

    struct KorokInfo
    {
        std::string text;
        std::string image;

        KorokInfo(std::string text , std::string image) : 
            text(text), image(image) {};
    };

    struct HiddenKorok : public Korok { public: using Korok::Korok; };
    struct CarryKorok : public Korok { public: using Korok::Korok; };
    struct Shrine : public ObjectWithName { public: using ObjectWithName::ObjectWithName; };
    struct Lightroot : public ObjectWithName { public: using ObjectWithName::ObjectWithName; };
    struct Well : public ObjectWithName { public: using ObjectWithName::ObjectWithName; };

    struct Hinox : public Object { public: using Object::Object; };
    struct Talus : public Object { public: using Object::Object; };
    struct Molduga : public Object { public: using Object::Object; };
    struct FluxConstruct : public Object { public: using Object::Object; };
    struct Frox : public Object { public: using Object::Object; };
    struct Gleeok : public Object { public: using Object::Object; };

    enum class ObjectType
    {
        HiddenKorok,
        CarryKorok,
        Shrine,
        Lightroot,
        Bubbul,
        
        Cave,
        Well,
        Chasms,
        Locations,
        
        Hinox,
        Talus,
        Molduga,
        FluxConstruct,
        Frox,
        Gleeok,

        SagesWill,
        OldMap,
        AddisonSign,
        SchemaStone,
        YigaSchematic,

        Count,
    };

    extern std::unordered_map<ObjectType, std::vector<Object*>> m_Objects;

    Object* GetObjectByHash(ObjectType type, uint32_t hash);
    bool ObjectExists(ObjectType type, uint32_t hash);

    // Create koroks

    // const int HiddenKorokCount = 800;
    // extern HiddenKorok HiddenKoroks[HiddenKorokCount];
    // const int CarryKorokCount = 100;
    // extern CarryKorok CarryKoroks[CarryKorokCount];

    // //extern std::map<int, KorokInfo> KorokInfos;

    // HiddenKorok* HiddenKorokExists(uint32_t hash);
    // CarryKorok* CarryKorokExists(uint32_t hash);

    // // const int KorokPathsCount = 97;
    // // extern KorokPath KorokPaths[KorokPathsCount];

    // void LoadPaths();

    void LoadFromJSON(const std::string& filepath);
    void UnloadData();

    // // Create shrines

    // const int ShrineCount = 152;
    // const int LightrootCount = 120;
    // extern Shrine Shrines[ShrineCount];
    // extern Lightroot Lightroots[LightrootCount];

    // Shrine* ShrineExists(uint32_t hash);
    // Lightroot* LightrootExists(uint32_t hash);

    // const int CaveCount = 148;
    // extern Cave Caves[CaveCount]; 

    // Cave* CaveExists(uint32_t hash);

    // const int WellCount = 58;
    // extern Well Wells[WellCount]; 

    // Well* WellExists(uint32_t hash);

    // //const int DLCShrineCount = 16;
    // //extern DLCShrine DLCShrines[DLCShrineCount];

    // //DLCShrine* DLCShrineExists(uint32_t hash);

    // // Create locations (coordinates are rounded)
    // //const int LocationsCount = 187;
    // //extern Location Locations[LocationsCount];

    // //Location* LocationExists(uint32_t hash);

    // const int HinoxCount = 69;
    // const int TalusCount = 87;
    // const int MoldugaCount = 4;
    // const int FluxConstructCount = 35;
    // const int FroxCount = 40;
    // const int GleeokCount = 14;

    // extern Hinox Hinoxes[HinoxCount];
    // extern Talus Taluses[TalusCount];
    // extern Molduga Moldugas[MoldugaCount];
    // extern FluxConstruct FluxConstructs[FluxConstructCount];
    // extern Frox Froxes[FroxCount];
    // extern Gleeok Gleeoks[GleeokCount];

    // Hinox* HinoxExists(uint32_t hash);
    // Talus* TalusExists(uint32_t hash);
    // Molduga* MoldugaExists(uint32_t hash);
    // FluxConstruct* FluxConstructExists(uint32_t hash);
    // Frox* FroxExists(uint32_t hash);
    // Gleeok* GleeokExists(uint32_t hash);
};