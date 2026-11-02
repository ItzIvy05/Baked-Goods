#include "AnimalWitness.h"
#include "ArrowImpacts.h"
#include "Aurora.h"
#include "FollowerLockpick.h"
#include "LingeringSubtitle.h"
#include "MurderReport.h"
#include "RaceMenuCommand.h"
#include "RegionWeather.h"

namespace
{
    constexpr std::array merged{
        "RegionWeatherFix.dll"sv,
        "LingeringSubtitleEngineFix.dll"sv,
        "AuroraFix.dll"sv
    };

    constexpr std::array unsupported{
        "StealthFixes.dll"sv
    };

    REX::INI::Bool<> regionWeatherFix{ "Features", "bRegionWeatherFix", true };
    REX::INI::Bool<> lingeringSubtitleFix{ "Features", "bLingeringSubtitleFix", true };
    REX::INI::Bool<> animalWitnessFix{ "Stealth Fixes", "bAnimalWitnessFix", true };
    REX::INI::Bool<> disableShowRaceMenu{ "Features", "bDisableShowRaceMenu", false };
    REX::INI::Bool<> auroraFix{ "Features", "bAuroraFix", true };
    REX::INI::Bool<> murderReportFix{ "Stealth Fixes", "bMurderReportFix", true };
    REX::INI::Bool<> followerLockpickFix{ "Stealth Fixes", "bFollowerLockpickFix", true };
    REX::INI::Bool<> arrowImpactFix{ "Stealth Fixes", "bArrowImpactFix", true };

    std::vector<std::string_view> FindLoaded(std::span<const std::string_view> a_dlls)
    {
        std::vector<std::string_view> loaded;
        for (const auto& dll : a_dlls) {
            if (REX::W32::GetModuleHandleA(dll.data())) {
                loaded.push_back(dll);
            }
        }
        return loaded;
    }

    std::string JoinNames(const std::vector<std::string_view>& a_dlls)
    {
        std::string names{ a_dlls.front() };
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

    void CheckLoadedDlls()
    {
        std::string message;
        const auto loadedMerged = FindLoaded(merged);
        if (loadedMerged.size() == 1) {
            message += std::format("{} is already part of Baked Goods, please disable it.\n", JoinNames(loadedMerged));
        } else if (loadedMerged.size() > 1) {
            message += std::format("{} are already part of Baked Goods, please disable them.\n", JoinNames(loadedMerged));
        }
        const auto loadedUnsupported = FindLoaded(unsupported);
        if (loadedUnsupported.size() == 1) {
            message += std::format("{} is not supported.\n", JoinNames(loadedUnsupported));
        } else if (loadedUnsupported.size() > 1) {
            message += std::format("{} are not supported.\n", JoinNames(loadedUnsupported));
        }
        if (!message.empty()) {
            REX::W32::MessageBoxA(nullptr, message.c_str(), "Baked Goods", 0);
            REX::W32::TerminateProcess(REX::W32::GetCurrentProcess(), EXIT_FAILURE);
        }
    }

    void OnMessage(SKSE::MessagingInterface::Message* a_message)
    {
        if (a_message->type == SKSE::MessagingInterface::kPostLoad) {
            CheckLoadedDlls();
            const auto ini = REX::INI::SettingStore::GetSingleton();
            ini->Init("Data/SKSE/Plugins/BakedGoods.ini", "Data/SKSE/Plugins/BakedGoods_Custom.ini");
            ini->Load();
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
            if (murderReportFix.GetValue()) {
                MurderReport::Install();
            }
            if (followerLockpickFix.GetValue()) {
                FollowerLockpick::Install();
            }
        }
        if (a_message->type == SKSE::MessagingInterface::kDataLoaded) {
            if (arrowImpactFix.GetValue()) {
                ArrowImpacts::Install();
            }
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
    SKSE::Init(skse, { .trampoline = true, .trampolineSize = 70 });
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}