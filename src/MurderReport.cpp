#include "MurderReport.h"

namespace MurderReport
{
    void Install()
    {
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(36431, 37426), REL::VariantOffset(0xEF, 0xEF, 0x0) };  // murder alarm
        target.write_fill(REL::NOP, 3);  // ProcessLists::AnyGuardInAlarm result
    }
}
