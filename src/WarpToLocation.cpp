#include "WarpToLocation.h"

namespace {
    // the handler reads the process without a null check
    struct HandleWarp {
        static std::uint32_t thunk(void* a_wrapper, RE::Actor* a_actor, void* a_message) {
            if (!a_actor->GetActorRuntimeData().currentProcess) {
                return 1;
            }
            return func(a_wrapper, a_actor, a_message);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace WarpToLocation {
    void Install() {
        REL::Relocation<std::uintptr_t> vtbl{RE::VTABLE_ActorMovementMessageMap_16___MessageHandlerWrapper_MovementMessageWarpToLocation_[0]};
        HandleWarp::func = vtbl.write_vfunc(0x1, HandleWarp::thunk);  // MessageHandlerWrapper::Handle
    }
}