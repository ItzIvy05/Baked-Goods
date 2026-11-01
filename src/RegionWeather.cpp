#include "RegionWeather.h"

namespace
{
    std::atomic<bool> teleported{ false };

    bool NeedsResync(RE::Sky* a_sky)
    {
        static REL::Relocation<bool (*)(RE::TESRegion*, RE::TESWeather*)> hasWeather{ RELOCATION_ID(16204, 16450) };  // TESRegion::HasWeather
        if (!a_sky->region || !a_sky->currentWeather || a_sky->overrideWeather || !a_sky->region->dataList || hasWeather(a_sky->region, a_sky->currentWeather)) {
            return false;
        }
        for (const auto& data : a_sky->region->dataList->regionDataList) {
            if (data && data->GetType() == RE::TESRegionData::Type::kWeather) {
                return data->IsLoaded();
            }
        }
        return false;
    }

    // doors, coc, fast travel and MoveTo all teleport the player through here
    struct TeleportSetParentCell
    {
        static void thunk(RE::TESObjectREFR* a_this, RE::TESObjectCELL* a_cell)
        {
            func(a_this, a_cell);
            teleported = true;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    // the new region is committed before a running blend swallows the switch
    struct SkyUpdateWeather
    {
        static void thunk(RE::Sky* a_sky)
        {
            func(a_sky);
            if (teleported.exchange(false) && NeedsResync(a_sky)) {
                a_sky->region = nullptr;
                a_sky->flags.set(RE::Sky::Flags::kFastTravel);
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace RegionWeather
{
    void Install()
    {
        REL::Relocation<std::uintptr_t> teleport{ RELOCATION_ID(39657, 40744), REL::VariantOffset(0x134, 0x131, 0x0) };  // PlayerCharacter teleport
        TeleportSetParentCell::func = teleport.write_call<5>(TeleportSetParentCell::thunk);
        REL::Relocation<std::uintptr_t> skyUpdate{ RELOCATION_ID(25682, 26229), REL::VariantOffset(0x29C, 0x3E6, 0x0) };  // Sky::Update
        SkyUpdateWeather::func = skyUpdate.write_call<5>(SkyUpdateWeather::thunk);
    }
}
