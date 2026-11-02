#include "ArrowImpacts.h"

namespace
{
    constexpr std::array<RE::FormID, 1> gravel{
        0x1C151  // MaterialGravel
    };

    constexpr std::array<RE::FormID, 9> soft{
        0x12F38,  // MaterialDirt
        0x12F48,  // MaterialMud
        0x84782,  // MaterialCarriageWheel
        0x12F46,  // MaterialGrass
        0x5CF14,  // MaterialBook
        0x12F45,  // MaterialSnow
        0x12F3E,  // MaterialSand
        0x12F37,  // MaterialCloth
        0x363B0   // MaterialBasket
    };

    RE::BGSImpactData* Clone(RE::BGSImpactData* a_impact, RE::SOUND_LEVEL a_soundLevel)
    {
        const auto clone = RE::IFormFactory::Create<RE::BGSImpactData>();
        static_cast<RE::TESModel*>(clone)->CopyComponent(static_cast<RE::TESModel*>(a_impact));
        clone->data = a_impact->data;
        clone->data.soundLevel = a_soundLevel;
        clone->decalTextureSet = a_impact->decalTextureSet;
        clone->decalTextureSet2 = a_impact->decalTextureSet2;
        clone->sound1 = a_impact->sound1;
        clone->sound2 = a_impact->sound2;
        clone->hazard = a_impact->hazard;
        clone->dData = a_impact->dData;
        clone->formFlags |= RE::TESForm::RecordFlags::kInitialized;
        return clone;
    }

    void Remap(RE::BGSImpactDataSet* a_set, RE::BGSImpactData* a_from, RE::BGSImpactData* a_to, std::span<const RE::FormID> a_materials)
    {
        for (auto& [material, impact] : a_set->impactMap) {
            if (impact == a_from && std::ranges::contains(a_materials, material->GetFormID())) {
                impact = a_to;
            }
        }
    }
}

namespace ArrowImpacts
{
    void Install()
    {
        const auto dataHandler = RE::TESDataHandler::GetSingleton();
        const auto arrowDirt = dataHandler->LookupForm<RE::BGSImpactData>(0x193B1, "Skyrim.esm");     // WPNArrowVsDirtImpact
        const auto boltStick = dataHandler->LookupForm<RE::BGSImpactData>(0x00D216, "Dawnguard.esm");  // WPNBoltVsStickImpact
        Remap(dataHandler->LookupForm<RE::BGSImpactDataSet>(0x193B9, "Skyrim.esm"), arrowDirt, Clone(arrowDirt, RE::SOUND_LEVEL::kNormal), gravel);     // WPNzArrowImpactSet
        Remap(dataHandler->LookupForm<RE::BGSImpactDataSet>(0x00A311, "Dawnguard.esm"), boltStick, Clone(boltStick, RE::SOUND_LEVEL::kSilent), soft);  // WPNzBoltImpactSet
        arrowDirt->data.soundLevel = RE::SOUND_LEVEL::kSilent;
        dataHandler->LookupForm<RE::BGSImpactData>(0x193B3, "Skyrim.esm")->data.soundLevel = RE::SOUND_LEVEL::kLoud;     // WPNArrowBounceImpact
        dataHandler->LookupForm<RE::BGSImpactData>(0x4BB54, "Skyrim.esm")->data.soundLevel = RE::SOUND_LEVEL::kLoud;     // WPNArrowVsMetalImpact
        dataHandler->LookupForm<RE::BGSImpactData>(0x00D218, "Dawnguard.esm")->data.soundLevel = RE::SOUND_LEVEL::kLoud;  // WPNBoltVsBounceImpact
    }
}
