#include "SneakSpellLight.h"

namespace {
    constexpr float fadeTime = 0.3f;

    RE::NiFloatKey* GetFadeKeys(RE::NiTimeController* a_controller) {
        const auto interpolator = netimmerse_cast<RE::NiFloatInterpolator*>(static_cast<RE::NiInterpController*>(a_controller)->GetInterpolator(0));
        if (!interpolator || !interpolator->floatData || interpolator->floatData->numKeys != 2 || interpolator->floatData->type != RE::NiFloatData::KeyType::kLink) {
            return nullptr;
        }
        return interpolator->floatData->keys;
    }

    RE::NiTimeController* CreateFade(RE::NiLight* a_light) {
        const auto dimmer = RE::NiLightDimmerController::Create();
        const auto interpolator = RE::NiFloatInterpolator::Create();
        const auto data = RE::NiFloatData::Create();
        data->ReplaceAnim(RE::NiLinFloatKey::CreateArray(2), 2, RE::NiFloatData::KeyType::kLink);
        interpolator->floatData.reset(data);
        dimmer->SetInterpolator(interpolator, 0);
        dimmer->flags.set(RE::NiTimeController::Flag::kAnimType_AppInit, RE::NiTimeController::Flag::kCycleType_Clamp, RE::NiTimeController::Flag::kComputeScaledTime);
        dimmer->frequency = 1.0f;
        dimmer->phase = 0.0f;
        dimmer->loKeyTime = 0.0f;
        dimmer->hiKeyTime = fadeTime;
        dimmer->SetTarget(a_light);
        return dimmer;
    }

    void FadeTo(RE::NiLight* a_light, float a_fade) {
        auto& fade = a_light->GetLightRuntimeData().fade;
        static REL::Relocation<const RE::NiRTTI*> rtti{RE::NiRTTI_NiLightDimmerController};
        auto controller = a_light->GetController(rtti.get());
        if (!controller) {
            if (fade == a_fade) {
                return;
            }
            controller = CreateFade(a_light);
        }
        const auto keys = GetFadeKeys(controller);
        if (!keys) {
            fade = a_fade;  // a dimmer from the light's own nif, leave it alone
            return;
        }
        keys[0].SetTime(0.0f);
        keys[0].SetValue(fade);
        keys[1].SetTime(fadeTime);
        keys[1].SetValue(a_fade);
        controller->Start(0.0f);
    }

    void SetHandLights(RE::Actor* a_actor, bool a_sneaking) {
        auto& runtime = a_actor->GetActorRuntimeData();
        for (const auto caster : runtime.magicCasters) {
            if (!caster || !caster->light || !caster->light->light) {
                continue;
            }
            const auto light = caster->light->light.get();
            if (a_sneaking) {
                FadeTo(light, 0.0f);
                continue;
            }
            const auto spell = runtime.selectedSpells[std::to_underlying(caster->castingSource)];
            if (!spell) {
                continue;
            }
            const auto effect = spell->GetAVEffect();
            if (effect && effect->data.light) {
                FadeTo(light, effect->data.light->fade);  // what GenDynamic gave it
            }
        }
    }

    struct SetSneaking {
        static bool thunk(RE::Actor* a_this, bool a_sneaking) {
            const auto result = func(a_this, a_sneaking);
            SetHandLights(a_this, a_sneaking);
            return result;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    // spell drawn while already sneaking
    struct GenDynamic {
        static RE::NiLight* thunk(RE::TESObjectLIGH* a_this, RE::TESObjectREFR* a_ref, RE::NiNode* a_node, char a_forceDynamic, char a_useLightRadius, char a_affectRefOnly) {
            const auto light = func(a_this, a_ref, a_node, a_forceDynamic, a_useLightRadius, a_affectRefOnly);
            if (light && a_ref) {
                const auto actor = a_ref->As<RE::Actor>();
                if (actor && actor->IsSneaking()) {
                    light->GetLightRuntimeData().fade = 0.0f;
                }
            }
            return light;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    // sneak cleared without SetSneaking
    struct RemoveMovementFlags {
        static bool thunk(RE::ActorState* a_this, std::uint16_t a_flags) {
            const auto result = func(a_this, a_flags);
            const auto actor = skyrim_cast<RE::Actor*>(a_this);
            if (actor) {
                SetHandLights(actor, false);
            }
            return result;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace SneakSpellLight {
    void Install() {
        auto toggleOffset = REL::VariantOffset(0x344, 0x36A, 0x0);
        if (REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_104)) {
            toggleOffset = REL::VariantOffset(0x344, 0x374, 0x0);
        }
        REL::Relocation<std::uintptr_t> toggle{RELOCATION_ID(38048, 39004), toggleOffset};                           // sneak action
        REL::Relocation<std::uintptr_t> idle{RELOCATION_ID(39022, 40089), REL::VariantOffset(0x43D, 0x457, 0x0)};    // furniture and idles
        REL::Relocation<std::uintptr_t> prison{RELOCATION_ID(39569, 40655), REL::VariantOffset(0x250, 0x256, 0x0)};  // PlayerCharacter::GoToPrison
        REL::Relocation<std::uintptr_t> fine{RELOCATION_ID(39573, 40659), REL::VariantOffset(0x19A, 0x166, 0x0)};    // PlayerCharacter::PayFine
        REL::Relocation<std::uintptr_t> controls{RELOCATION_ID(39652, 40739), REL::VariantOffset(0x4E, 0x4F, 0x0)};  // UserEventEnabledEvent
        SetSneaking::func = toggle.write_call<5>(SetSneaking::thunk);
        idle.write_call<5>(SetSneaking::thunk);
        prison.write_call<5>(SetSneaking::thunk);
        fine.write_call<5>(SetSneaking::thunk);
        controls.write_call<5>(SetSneaking::thunk);

        REL::Relocation<std::uintptr_t> create{RELOCATION_ID(33391, 34172), REL::VariantOffset(0x86, 0x86, 0x0)};  // hand light
        GenDynamic::func = create.write_call<5>(GenDynamic::thunk);

        auto refusedOffset = REL::VariantOffset(0x426, 0x478, 0x0);
        if (REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_104)) {
            refusedOffset = REL::VariantOffset(0x426, 0x482, 0x0);
        }
        REL::Relocation<std::uintptr_t> refused{RELOCATION_ID(38048, 39004), refusedOffset};                           // SneakStop refused by the graph
        REL::Relocation<std::uintptr_t> low{RELOCATION_ID(36614, 37622), REL::VariantOffset(0x176, 0x1BD, 0x0)};       // Actor::MoveToLow
        REL::Relocation<std::uintptr_t> furnitureA{RELOCATION_ID(38649, 39680), REL::VariantOffset(0x95, 0x7A, 0x0)};  // furniture and idles
        REL::Relocation<std::uintptr_t> furnitureB{RELOCATION_ID(38651, 39682), REL::VariantOffset(0x95, 0x7A, 0x0)};  // furniture and idles
        REL::Relocation<std::uintptr_t> package{RELOCATION_ID(38831, 39861), REL::VariantOffset(0x164, 0x1D1, 0x0)};   // packages
        RemoveMovementFlags::func = refused.write_call<5>(RemoveMovementFlags::thunk);
        low.write_call<5>(RemoveMovementFlags::thunk);
        furnitureA.write_call<5>(RemoveMovementFlags::thunk);
        furnitureB.write_call<5>(RemoveMovementFlags::thunk);
        package.write_call<5>(RemoveMovementFlags::thunk);

        if (REL::Module::IsSE()) {
            REL::Relocation<std::uintptr_t> race{REL::ID(36901), 0x1DF};  // Actor::SwitchRace
            race.write_call<5>(RemoveMovementFlags::thunk);
        }

        if (REL::Module::IsAE()) {
            REL::Relocation<std::uintptr_t> race{REL::ID(37925), 0x150};   // Actor::SwitchRace
            REL::Relocation<std::uintptr_t> attach{REL::ID(34151), 0xCD};  // hand light
            REL::Relocation<std::uintptr_t> art{REL::ID(34185), 0x407};    // ActorMagicCaster::CheckAttachCastingArt
            race.write_call<5>(SetSneaking::thunk);
            attach.write_call<5>(GenDynamic::thunk);
            art.write_call<5>(GenDynamic::thunk);
        }
    }
}