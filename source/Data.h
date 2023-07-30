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
        uint64_t m_CompletionGuid;

        glm::vec3 m_Position;

        std::string m_DisplayName;

        Object();
        Object(uint32_t completionHash, glm::vec3 position, std::string displayName = "");
    };

    struct KorokPath
    {
        uint32_t m_BelongsToCompletionHash;
        std::vector<glm::vec3> m_Points;

        KorokPath(uint32_t belongsToCompletionHash, std::vector<glm::vec3> points);

        KorokPath() {};
    };

    struct Korok : public Object
    {
        bool m_IsPair = false;

        Data::KorokPath* m_Path = nullptr;

        std::string m_KorokType;
        //std::string m_internalHash;

        Korok() {};

        Korok(uint32_t completionHash, glm::vec3 position, bool isPair, std::string korokType);/* :
            completionHash(hash), internalName(internalName), x(x), y(y), zeldaDungeonId(zeldaDungeonId) {};*/
    };

    struct KorokInfo
    {
        std::string text;
        std::string image;

        KorokInfo(std::string text , std::string image) : 
            text(text), image(image) {};
    };

    enum class ObjectType
    {
        HiddenKorok,
        CarryKorok,
        Shrine,
        Lightroot,
        
        Cave,
        Bubbul,
        Well,
        Chasm,
        Location,
        
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
    std::vector<Object*> GetObjectsByHash(ObjectType type, uint32_t hash);
    //Object* GetObjectByHash(ObjectType type, uint32_t hash), std::set<int>;
    bool ObjectExists(ObjectType type, uint32_t hash);

    Object* GetObjectByGuid(ObjectType type, uint64_t guid);
    bool ObjectExistsByGuid(ObjectType type, uint64_t guid);

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