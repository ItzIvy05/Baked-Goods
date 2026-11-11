#include "ActorValueCache.h"

namespace {
    REL::Relocation<RE::BSReadWriteLock*> storageLock{RELOCATION_ID(517485, 404014)};  // actor value storage lock

    // the clear unlocks before it empties the keys
    struct ClearBaseValues {
        static void thunk(RE::ActorValueStorage::LocalMap<float>* a_baseValues) {
            RE::BSWriteLockGuard locker{*storageLock};
            func(a_baseValues);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace ActorValueCache {
    void Install() {
        REL::Relocation<std::uintptr_t> target{RELOCATION_ID(37529, 38478), REL::VariantOffset(0x2D, 0x2D, 0x0)};  // base value cache clear
        ClearBaseValues::func = target.write_call<5>(ClearBaseValues::thunk);
    }
}