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

    // the ref can be in an unloaded cell by now, only the decrements check loadedData
    struct IncrementQueuedRefCount {
        static void thunk(RE::TESObjectCELL* a_cell) {
            if (a_cell->GetRuntimeData().loadedData) {
                func(a_cell);
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct IncrementCriticalQueuedRefCount {
        static void thunk(RE::TESObjectCELL* a_cell) {
            if (a_cell->GetRuntimeData().loadedData) {
                func(a_cell);
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct IncrementQueuedDistantRefCount {
        static void thunk(RE::TESObjectCELL* a_cell, RE::TESObjectREFR* a_ref) {
            if (a_cell->GetRuntimeData().loadedData) {
                func(a_cell, a_ref);
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace QueuedCell {
    void Install() {
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(12880, 13023), REL::VariantOffset(0x179, 0x1AB, 0x0)};    // queued reference attach 3D
        REL::Relocation<std::uintptr_t> queued{RELOCATION_ID(12910, 13057), REL::VariantOffset(0x3AD, 0x347, 0x0)};    // queue reference load, queuedRefCount++
        REL::Relocation<std::uintptr_t> critical{RELOCATION_ID(12910, 13057), REL::VariantOffset(0x3C2, 0x35C, 0x0)};  // queue reference load, criticalQueuedRefCount++
        REL::Relocation<std::uintptr_t> distant{RELOCATION_ID(12910, 13057), REL::VariantOffset(0x3DA, 0x374, 0x0)};   // queue reference load, queuedDistantRefCount++
        AttachQueued3D::func = target.write_call<5>(AttachQueued3D::thunk);
        IncrementQueuedRefCount::func = queued.write_call<5>(IncrementQueuedRefCount::thunk);
        IncrementCriticalQueuedRefCount::func = critical.write_call<5>(IncrementCriticalQueuedRefCount::thunk);
        IncrementQueuedDistantRefCount::func = distant.write_call<5>(IncrementQueuedDistantRefCount::thunk);
    }
}
