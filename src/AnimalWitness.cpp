#include "AnimalWitness.h"

namespace
{
    // every crime alarm builds its witness list here, a null actor is skipped
    struct WitnessGetSmartPointer
    {
        static bool thunk(const RE::ActorHandle& a_handle, RE::NiPointer<RE::Actor>& a_actor)
        {
            func(a_handle, a_actor);
            if (a_actor && a_actor->IsAnimal()) {
                a_actor.reset();
            }
            return static_cast<bool>(a_actor);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace AnimalWitness
{
    void Install()
    {
        REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(37085, 38094), REL::VariantOffset(0x51, 0x5D, 0x0) };  // crime witness collector
        WitnessGetSmartPointer::func = target.write_call<5>(WitnessGetSmartPointer::thunk);
    }
}