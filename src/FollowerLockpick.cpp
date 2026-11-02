#include "FollowerLockpick.h"

namespace
{
    struct IsAnOwner
    {
        static bool thunk(RE::TESObjectREFR* a_ref, RE::Actor* a_actor, bool a_useFaction, bool a_requiresOwner)
        {
            if (func(a_ref, a_actor, a_useFaction, a_requiresOwner)) {
                return true;
            }
            std::uint32_t losCount = 1;
            return RE::ProcessLists::GetSingleton()->RequestHighestDetectionLevelAgainstActor(a_actor, losCount) <= 0;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace FollowerLockpick
{
    void Install()
    {
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36496, 37496), REL::VariantOffset(0x1AB, 0x1CE, 0x0) };  // commanded actor favor
        IsAnOwner::func = target.write_call<5>(IsAnOwner::thunk);
    }
}
