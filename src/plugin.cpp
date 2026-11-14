#include "ActionNoise.h"
#include "ActorValueCache.h"
#include "AnimalWitness.h"
#include "ArrowImpacts.h"
#include "Aurora.h"
#include "DetectionDistance.h"
#include "FollowerLockpick.h"
#include "KeyboardData.h"
#include "LingeringSubtitle.h"
#include "MurderReport.h"
#include "Music.h"
#include "QueuedCell.h"
#include "RaceMenuCommand.h"
#include "RegionWeather.h"
#include "SneakSpellLight.h"
#include "WarpToLocation.h"
#include "WornEnchantment.h"

namespace {
    constexpr std::array merged{
        "RegionWeatherFix.dll"sv,
        "LingeringSubtitleEngineFix.dll"sv,
        "AuroraFix.dll"sv
    };

    REX::INI::Bool<> regionWeatherFix{"Features", "bRegionWeatherFix", true};
    REX::INI::Bool<> lingeringSubtitleFix{"Features", "bLingeringSubtitleFix", true};
    REX::INI::Bool<> animalWitnessFix{"Stealth Fixes", "bAnimalWitnessFix", true};
    REX::INI::Bool<> disableShowRaceMenu{"Features", "bDisableShowRaceMenu", false};
    REX::INI::Bool<> auroraFix{"Features", "bAuroraFix", true};
    REX::INI::Bool<> musicFix{"Features", "bMusicFix", true};
    REX::INI::Bool<> wornEnchantmentFix{"Features", "bWornEnchantmentFix", true};
    REX::INI::Bool<> murderReportFix{"Stealth Fixes", "bMurderReportFix", true};
    REX::INI::Bool<> followerLockpickFix{"Stealth Fixes", "bFollowerLockpickFix", true};
    REX::INI::Bool<> arrowImpactFix{"Stealth Fixes", "bArrowImpactFix", true};
    REX::INI::Bool<> detectionDistanceFix{"Stealth Fixes", "bDetectionDistanceFix", true};
    REX::INI::Bool<> actionNoiseFix{"Stealth Fixes", "bActionNoiseFix", true};
    REX::INI::Bool<> sneakSpellLightFix{"Stealth Fixes", "bSneakSpellLightFix", true};
    REX::INI::Bool<> queued3DCellFix{"Crash Fixes", "bQueued3DCellFix", true};
    REX::INI::Bool<> keyboardDataFix{"Crash Fixes", "bKeyboardDataFix", true};
    REX::INI::Bool<> actorValueCacheFix{"Crash Fixes", "bActorValueCacheFix", true};
    REX::INI::Bool<> warpToLocationFix{"Crash Fixes", "bWarpToLocationFix", true};

    struct StealthFixesClash {
        const REX::INI::Bool<>& ours;
        const char* section;
        const char* key;
    };

    const std::array<StealthFixesClash, 6> stealthFixesClashes{{
        {animalWitnessFix, "Crime", "bNoAnimalWitnesses"},
        {murderReportFix, "Crime", "bGuardAlarmNearby"},
        {followerLockpickFix, "Trespass", "bFollowers"},
        {arrowImpactFix, "Impacts", "bEnabled"},
        {detectionDistanceFix, "Combat", "bDetectionPriority"},
        {actionNoiseFix, "Formula", "bActionNoiseFix"}}
    };

    std::vector<std::string_view> FindLoaded(std::span<const std::string_view> a_dlls) {
        std::vector<std::string_view> loaded;
        for (const auto& dll : a_dlls) {
            if (REX::W32::GetModuleHandleA(dll.data())) {
                loaded.push_back(dll);
            }
        }
        return loaded;
    }

    std::string JoinNames(const std::vector<std::string_view>& a_dlls) {
        std::string names{a_dlls.front()};
        for (std::size_t i = 1; i < a_dlls.size(); ++i) {
            if (i == a_dlls.size() - 1) {
                names += " and ";
            } else {
                names += ", ";
            }
            names += a_dlls[i];
        }
        return names;
    }

    void CheckLoadedDlls() {
        std::string message;
        const auto loadedMerged = FindLoaded(merged);
        if (loadedMerged.size() == 1) {
            message += std::format("{} is already part of Baked Goods, please disable it.\n", JoinNames(loadedMerged));
        } else if (loadedMerged.size() > 1) {
            message += std::format("{} are already part of Baked Goods, please disable them.\n", JoinNames(loadedMerged));
        }
        if (!message.empty()) {
            REX::W32::MessageBoxA(nullptr, message.c_str(), "Baked Goods", 0);
            REX::W32::TerminateProcess(REX::W32::GetCurrentProcess(), EXIT_FAILURE);
        }
    }

    bool StealthFixesEnabled(const std::vector<std::string>& a_files, const char* a_section, const char* a_key) {
        bool enabled = true;
        for (const auto& file : a_files) {
            char buffer[128]{};
            std::string_view value{buffer, REX::W32::GetPrivateProfileStringA(a_section, a_key, "", buffer, static_cast<std::uint32_t>(std::size(buffer)), file.c_str())};
            for (auto& c : buffer) {
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
            value = value.substr(0, value.find_first_of(";#"));
            value = value.substr(std::min(value.find_first_not_of(" \t"), value.size()));
            value = value.substr(0, value.find_last_not_of(" \t") + 1);
            long number = 0;
            if (value == "true" || value == "yes" || value == "on") {
                enabled = true;
            } else if (value == "false" || value == "no" || value == "off") {
                enabled = false;
            } else if (const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), number); error == std::errc{} && end != value.data()) {
                enabled = number != 0;
            }
        }
        return enabled;
    }

    void CheckStealthFixes() {
        if (!REX::W32::GetModuleHandleA("StealthFixes.dll")) {
            return;
        }
        const std::filesystem::path presets{".\\Data\\SKSE\\Plugins\\StealthFixesCustomINI"};
        std::vector<std::string> files{".\\Data\\SKSE\\Plugins\\StealthFixes.ini"};
        if (std::filesystem::is_directory(presets)) {
            for (const auto& entry : std::filesystem::directory_iterator(presets)) {
                if (entry.is_regular_file() && _stricmp(entry.path().extension().string().c_str(), ".ini") == 0) {
                    files.push_back(entry.path().string());
                }
            }
        }
        std::sort(files.begin() + 1, files.end());
        files.push_back(".\\Data\\SKSE\\Plugins\\StealthFixes_User.ini");
        std::string message;
        for (const auto& clash : stealthFixesClashes) {
            if (clash.ours.GetValue() && StealthFixesEnabled(files, clash.section, clash.key)) {
                message += std::format("[{}] {}\n", clash.section, clash.key);
            }
        }
        if (!message.empty()) {
            message = "Baked Goods already fixes these. Turn them off in Stealth Fixes:\n" + message;
            REX::W32::MessageBoxA(nullptr, message.c_str(), "Baked Goods", 0);
            REX::W32::TerminateProcess(REX::W32::GetCurrentProcess(), EXIT_FAILURE);
        }
    }

    void OnMessage(SKSE::MessagingInterface::Message* a_message) {
        if (a_message->type == SKSE::MessagingInterface::kPostLoad) {
            CheckLoadedDlls();
            const auto ini = REX::INI::SettingStore::GetSingleton();
            ini->Init("Data/SKSE/Plugins/BakedGoods.ini", "Data/SKSE/Plugins/BakedGoods_Custom.ini");
            ini->Load();
            CheckStealthFixes();
            if (regionWeatherFix.GetValue()) {
                RegionWeather::Install();
            }
            if (lingeringSubtitleFix.GetValue()) {
                LingeringSubtitle::Install();
            }
            if (animalWitnessFix.GetValue()) {
                AnimalWitness::Install();
            }
            if (disableShowRaceMenu.GetValue()) {
                RaceMenuCommand::Install();
            }
            if (auroraFix.GetValue()) {
                Aurora::Install();
            }
            if (musicFix.GetValue() && !REX::W32::GetModuleHandleA("CombatMusicFixNG.dll") && !REX::W32::GetModuleHandleA("MusicQueueFix.dll")) {
                Music::Install();
            }
            if (wornEnchantmentFix.GetValue()) {
                WornEnchantment::Install();
            }
            if (murderReportFix.GetValue()) {
                MurderReport::Install();
            }
            if (followerLockpickFix.GetValue()) {
                FollowerLockpick::Install();
            }
            if (detectionDistanceFix.GetValue()) {
                DetectionDistance::Install();
            }
            if (actionNoiseFix.GetValue()) {
                ActionNoise::Install();
            }
            if (sneakSpellLightFix.GetValue()) {
                SneakSpellLight::Install();
            }
            if (queued3DCellFix.GetValue()) {
                QueuedCell::Install();
            }
            if (keyboardDataFix.GetValue()) {
                KeyboardData::Install();
            }
            if (actorValueCacheFix.GetValue()) {
                ActorValueCache::Install();
            }
            if (warpToLocationFix.GetValue()) {
                WarpToLocation::Install();
            }
        }
        if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {
            if (arrowImpactFix.GetValue()) {
                ArrowImpacts::Install();
            }
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse, {.trampoline = true, .trampolineSize = 168});
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}