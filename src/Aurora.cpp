#include "Aurora.h"

namespace
{
    // same pick as the engine: the current weather's aurora, or the outgoing one while it fades out
    const char* WantedModel(RE::Sky* a_sky)
    {
        if (!a_sky->currentWeather->aurora.model.empty()) {
            return a_sky->currentWeather->aurora.model.c_str();
        }
        if (a_sky->lastWeather && !a_sky->lastWeather->aurora.model.empty()) {
            return a_sky->lastWeather->aurora.model.c_str();
        }
        return nullptr;
    }

    // the engine only swaps a loaded aurora when it fades to zero, a weather snap never fades so the old model stays
    struct UpdateAurora
    {
        static void thunk(RE::Sky* a_sky)
        {
            static const char* loaded = nullptr;
            const auto wanted = WantedModel(a_sky);
            if (!a_sky->auroraRoot) {
                if (!a_sky->auroraModel) {
                    loaded = wanted;
                }
            } else if (wanted && wanted != loaded) {
                a_sky->root->DetachChild(a_sky->auroraRoot.get());
                a_sky->auroraRoot.reset();
                loaded = wanted;
            }
            func(a_sky);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace Aurora
{
    void Install()
    {
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(25682, 26229), REL::VariantOffset(0x2C4, 0x5D9, 0x0) };  // Sky::Update
        UpdateAurora::func = target.write_call<5>(UpdateAurora::thunk);
    }
}
