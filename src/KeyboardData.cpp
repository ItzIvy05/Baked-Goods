#include "KeyboardData.h"

namespace {
    // a failed read leaves slots it never wrote
    struct GetDeviceData {
        static void thunk(RE::BSDirectInputManager* a_manager, REX::W32::IDirectInputDevice8A* a_device, std::uint32_t* a_count, REX::W32::DIDEVICEOBJECTDATA* a_data) {
            const auto capacity = *a_count;
            for (std::uint32_t i = 0; i < capacity; ++i) {
                a_data[i].ofs = std::numeric_limits<std::uint32_t>::max();
            }
            func(a_manager, a_device, a_count, a_data);
            const auto count = std::min(*a_count, capacity);
            std::uint32_t valid = 0;
            while (valid < count && a_data[valid].ofs < sizeof(RE::BSWin32KeyboardDevice::RUNTIME_DATA::curState)) {
                ++valid;
            }
            *a_count = valid;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace KeyboardData {
    void Install() {
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(67472, 68782), REL::VariantOffset(0xA1, 0xA1, 0x0)};  // BSWin32KeyboardDevice::Process
        GetDeviceData::func = target.write_call<5>(GetDeviceData::thunk);
    }
}