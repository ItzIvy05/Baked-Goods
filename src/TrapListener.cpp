#include "TrapListener.h"

namespace
{
    using RemoveTrapEntry_t = void(void*, void*);
    REL::Relocation<RemoveTrapEntry_t> RemoveTrapEntry{ RELOCATION_ID(25190, 25718) };  // TESTrapListener::RemoveTrapEntry

    // trap phantoms leave the world without their trap entry, so it outlives the freed collidable
    struct PhantomRemoved
    {
        static void thunk(RE::hkpPhantomListener* a_this, RE::hkpPhantom* a_phantom)
        {
            func(a_this, a_phantom);
            const auto layer = a_phantom->collidable.GetCollisionLayer();
            if (layer != RE::COL_LAYER::kTrap && layer != RE::COL_LAYER::kCloudTrap) {
                return;
            }
            const auto listener = SKSE::stl::adjust_pointer<void>(a_this, -0x28);
            auto entry = *SKSE::stl::adjust_pointer<void*>(listener, 0x8);
            while (entry && *SKSE::stl::adjust_pointer<const RE::hkpCollidable*>(entry, 0x18) != &a_phantom->collidable) {
                entry = *SKSE::stl::adjust_pointer<void*>(entry, 0x10);
            }
            if (entry) {
                RemoveTrapEntry(listener, entry);
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace TrapListener
{
    void Install()
    {
        REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_TESTrapListener[2] };
        PhantomRemoved::func = vtbl.write_vfunc(0x2, PhantomRemoved::thunk);  // TESTrapListener::PhantomRemovedCallback
    }
}
