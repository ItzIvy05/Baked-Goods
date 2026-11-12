#include "WornEnchantment.h"

namespace {
    struct DispelWornItemEnchantments {
        static void thunk(RE::Actor* a_actor) {
            func(a_actor);
            if (!a_actor->IsDead(false)) {
                a_actor->CastPermanentMagic(true, false, false, false);
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace WornEnchantment {
    void Install() {
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(50212, 51141), REL::VariantOffset(0x47B, 0x57D, 0x0)};  // ContainerMenu::TransferItem
        DispelWornItemEnchantments::func = target.write_call<5>(DispelWornItemEnchantments::thunk);
        REL::Relocation<std::uintptr_t> rebuild{RELOCATION_ID(24234, 418622), REL::VariantOffset(0xEE, 0xE6, 0x0)};  // UpdateWornGear, Character::Update after DispelWornItemEnchantments
        rebuild.write_fill(REL::NOP, 5);
    }
}