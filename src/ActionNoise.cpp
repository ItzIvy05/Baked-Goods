#include "ActionNoise.h"

namespace ActionNoise
{
    void Install()
    {
        REL::Relocation<std::uintptr_t> actor{ RELOCATION_ID(25812, 26365) };  // actor detection formula
        REL::Relocation<std::uintptr_t> noise{ RELOCATION_ID(25813, 26366) };  // noise detection formula
        REL::safe_write(actor.address() + REL::Relocate(0x2E0, 0x29D), std::to_array<std::uint8_t>({ 0x10, 0x2D }));                                     // movss xmm5, fSneakActionMult
        REL::safe_write(actor.address() + REL::Relocate(0x2EA, 0x2BA), std::to_array<std::uint8_t>({ 0x40, 0x8B }));                                     // mov eax, sound
        REL::safe_write(actor.address() + REL::Relocate(0x2F2, 0x2CA), std::to_array<std::uint8_t>({ 0xF3, 0x0F, 0x2A }));                               // cvtsi2ss xmm6, eax
        REL::safe_write(actor.address() + REL::Relocate(0x2F7, 0x2D7), std::to_array<std::uint8_t>({ 0x59, 0xF5 }));                                     // mulps xmm6, xmm5
        REL::safe_write(noise.address() + REL::Relocate(0x104, 0xD7), std::to_array<std::uint8_t>({ 0x59, REL::Relocate<std::uint8_t>(0x1D, 0x15) }));  // mulss attenuation, fSneakActionMult
        REL::safe_write(noise.address() + REL::Relocate(0x112, 0xE5), std::to_array<std::uint8_t>({ 0x40, 0x8B }));                                     // mov eax, sound
    }
}
