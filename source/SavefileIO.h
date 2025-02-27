#pragma once

#include <vector>
#include <cstdint>
#include <switch.h>
#include <thread>

#include "Data.h"

class SavefileIO
{
public:
    static SavefileIO& Get();

    bool LoadGamesave(bool loadMasterMode = false, bool chooseProfile = false);

    uint32_t ReadU32(unsigned char* buffer, uint32_t offset);
    uint64_t ReadU64(unsigned char* buffer, uint64_t offset);

    int MountSavefile(bool openProfilePicker = false);
    bool UnmountSavefile();

    bool LoadBackup(bool masterMode = false);

    uint32_t GetHashTableEnd(unsigned char* buffer, int fileSize);

    uint32_t GetSavefilePlaytime(const std::string& filepath);
    int GetMostRecentSavefile(const std::string& dir, bool masterMode = false); // Needs slash at end of dir

    void CopySavefiles();
    s32 CopyFile(const std::string &srcPath, const std::string &dstPath);

    bool ParseFile(const char* filepath);

    bool DirectoryExists(const std::string& filepath);
    bool FileExists(const std::string& filepath);

private:
    SavefileIO() {};

public:
    struct LoadedData
    {
        std::unordered_map<Data::ObjectType, std::vector<Data::Object*>> found;
        std::unordered_map<Data::ObjectType, std::vector<Data::Object*>> missing;

        // std::vector<Data::Korok*> foundKoroks;
        // std::vector<Data::Korok*> missingKoroks;
        // std::vector<Data::ObjectWithName*> foundShrines;
        // std::vector<Data::ObjectWithName*> missingShrines;
        // std::vector<Data::ObjectWithName*> foundLightroots;
        // std::vector<Data::ObjectWithName*> missingLightroots;
        // std::vector<Data::Cave*> foundCaves;
        // std::vector<Data::Cave*> missingCaves;
        // std::vector<Data::ObjectWithName*> foundWells;
        // std::vector<Data::ObjectWithName*> missingWells;
        // //std::vector<Data::Location*> visitedLocations;
        // //std::vector<Data::Location*> unexploredLocations;
        // std::vector<Data::Object*> defeatedHinoxes;
        // std::vector<Data::Object*> undefeatedHinoxes;
        // std::vector<Data::Object*> defeatedTaluses;
        // std::vector<Data::Object*> undefeatedTaluses;
        // std::vector<Data::Object*> defeatedMoldugas;
        // std::vector<Data::Object*> undefeatedMoldugas;
        // std::vector<Data::Object*> defeatedFluxConstructs;
        // std::vector<Data::Object*> undefeatedFluxConstructs;
        // std::vector<Data::Object*> defeatedFroxs;
        // std::vector<Data::Object*> undefeatedFroxs;
        // std::vector<Data::Object*> defeatedGleeoks;
        // std::vector<Data::Object*> undefeatedGleeoks;
    };

    LoadedData loadedData;

    u64 AccountUid1;
    u64 AccountUid2;

    int MostRecentNormalModeFile;
    int MostRecentMasterModeFile;

    bool LoadedSavefile;
    bool GameIsRunning;
    bool NoSavefileForUser;
    bool MasterModeFileExists;
    bool MasterModeFileLoaded;
    bool HasDLC;

    int MasterModeSlot;
};