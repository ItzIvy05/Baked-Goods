#include "Music.h"

namespace
{
    RE::BSSpinLock& GetMusicLock()
    {
        static REL::Relocation<RE::BSSpinLock*> lock{ RELOCATION_ID(525734, 412215) };  // BSMusicManager lock
        return *lock;
    }

    // one queue entry per music type, so one remove ends it
    struct ProcessEvent
    {
        static RE::BSEventNotifyControl thunk(RE::BSMusicManager* a_this, const RE::BSMusicEvent* a_event, RE::BSTEventSource<RE::BSMusicEvent>* a_source)
        {
            if (a_event->musicType && a_event->msgType == RE::BSMusicEvent::MUSIC_MESSAGE_TYPE::kAdd) {
                RE::BSSpinLockGuard locker{ GetMusicLock() };
                if (a_event->musicType->flags.none(RE::BSIMusicType::MST::kRemovalQueued) && std::ranges::contains(a_this->musicQueue, a_event->musicType)) {
                    return RE::BSEventNotifyControl::kContinue;
                }
            }
            return func(a_this, a_event, a_source);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    // duck or finish everything still playing, not just the last one
    struct DoClearDucking
    {
        static void thunk(RE::BSIMusicType* a_this)
        {
            func(a_this);
            for (const auto other : RE::BSMusicManager::GetSingleton()->musicQueue) {
                if (other && other != a_this && other->typeStatus == RE::BSIMusicType::MUSIC_STATUS::kPlaying) {
                    if (a_this->flags.any(RE::BSIMusicType::MST::kPlaysOver)) {
                        other->DoApplyDuckingAttenuation(a_this->ducksOtherMusicBy);
                    } else {
                        other->DoFinish(a_this->flags.any(RE::BSIMusicType::MST::kAbruptTransition));
                    }
                }
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace Music
{
    void Install()
    {
        REL::Relocation<std::uintptr_t> manager{ RE::VTABLE_BSMusicManager[0] };
        ProcessEvent::func = manager.write_vfunc(0x1, ProcessEvent::thunk);  // BSMusicManager::ProcessEvent
        REL::Relocation<std::uintptr_t> type{ RE::VTABLE_BGSMusicType[1] };
        DoClearDucking::func = type.write_vfunc(0x5, DoClearDucking::thunk);  // BGSMusicType::DoClearDucking
    }
}
