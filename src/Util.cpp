#include "Util.h"

namespace Util
{
    void SetDisplacement(std::uintptr_t a_instruction, std::uintptr_t a_target)
    {
        REL::safe_write(a_instruction + 4, static_cast<std::int32_t>(a_target - (a_instruction + 8)));
    }
}
