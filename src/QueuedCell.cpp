#include "QueuedCell.h"

namespace {
    using IsCellLoaded_t = bool(RE::TES*, RE::TESObjectCELL*, bool);
    REL::Relocation<IsCellLoaded_t> IsCellLoaded{RELOCATION_ID(13163, 13308)};  // cell attach gate

    // Re ask the the question if parentCell is ready again after the gate.
    struct AttachQueued3D {
        static void thunk(RE::TES* a_tes, RE::TESObjectREFR* a_ref, RE::TESObjectCELL* a_cell, RE::TESModelDB::TESQueuedHandles* a_task, bool a_unk, RE::NiAVObject* a_3D) {
            if (IsCellLoaded(a_tes, a_cell, false)) {
                func(a_tes, a_ref, a_cell, a_task, a_unk, a_3D);
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace QueuedCell {
    void Install() {
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(12880, 13023), REL::VariantOffset(0x179, 0x1AB, 0x0)};  // queued reference attach 3D
        AttachQueued3D::func = target.write_call<5>(AttachQueued3D::thunk);
    }
}