#include "DetectionDistance.h"
#include "Util.h"

namespace DetectionDistance
{
    void Install()
    {
        REL::Relocation<std::uintptr_t> low{ RELOCATION_ID(504146, 372656), 0x8 };                                        // fCombatDetectionLowPriorityDistance
        REL::Relocation<std::uintptr_t> veryLow{ RELOCATION_ID(504148, 372659), 0x8 };                                    // fCombatDetectionVeryLowPriorityDistance
        REL::Relocation<std::uintptr_t> farTest{ RELOCATION_ID(43501, 44725), REL::VariantOffset(0x60D, 0x6B0, 0x0) };    // combat group detection priority
        REL::Relocation<std::uintptr_t> farPriority{ RELOCATION_ID(43501, 44725), REL::VariantOffset(0x61F, 0x6C3, 0x0) };
        REL::Relocation<std::uintptr_t> midTest{ RELOCATION_ID(43501, 44725), REL::VariantOffset(0x625, 0x6C9, 0x0) };
        REL::Relocation<std::uintptr_t> midPriority{ RELOCATION_ID(43501, 44725), REL::VariantOffset(0x632, 0x6D6, 0x0) };
        Util::SetDisplacement(farTest.address(), veryLow.address());
        Util::SetDisplacement(midTest.address(), low.address());
        farPriority.write(std::to_underlying(RE::DETECTION_PRIORITY::kVeryLow));
        midPriority.write(std::to_underlying(RE::DETECTION_PRIORITY::kLow));
    }
}
