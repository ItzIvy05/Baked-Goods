#include "LingeringSubtitle.h"

namespace
{
    using RemoveSubtitle_t = void(RE::SubtitleManager*, RE::TESObjectREFR*);
    REL::Relocation<RemoveSubtitle_t> RemoveSubtitle{ RELOCATION_ID(51754, 52627) };  // SubtitleManager::RemoveSubtitle

    // engine ends the line on demotion but leaves the subtitle for a low process update that comes hours later
    struct MovetoLow
    {
        static bool thunk(RE::Character* a_this)
        {
            const auto result = func(a_this);
            RemoveSubtitle(RE::SubtitleManager::GetSingleton(), a_this);
            return result;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct MovetoMiddleLow
    {
        static bool thunk(RE::Character* a_this)
        {
            const auto result = func(a_this);
            RemoveSubtitle(RE::SubtitleManager::GetSingleton(), a_this);
            return result;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    // the voice timer hands an expired line here, which only clears the subtitle while a voice is still playing
    struct StopCurrentDialogue
    {
        static void thunk(RE::Character* a_this)
        {
            func(a_this);
            RemoveSubtitle(RE::SubtitleManager::GetSingleton(), a_this);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    // a scene that ends mid line never finishes a non-actor speaker's line, so its subtitle stays
    struct EndDialogue
    {
        static void thunk(RE::BGSSceneActionDialogue* a_this, RE::BGSScene* a_scene)
        {
            const bool unfinished = (a_this->unk14 & 1) != 0 && (a_this->unk14 & 2) == 0;
            func(a_this, a_scene);
            if (!unfinished) {
                return;
            }
            const auto ref = a_scene->parentQuest->GetAliasedRef(a_this->actorID).get();
            if (!ref || ref->Is(RE::FormType::ActorCharacter)) {
                return;
            }
            const auto info = ref->extraList.GetByType<RE::ExtraSayToTopicInfo>();
            if (info && !info->voicePaused) {
                info->voicePaused = true;
                ref->UpdateSoundCallBack(false);
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };
}

namespace LingeringSubtitle
{
    void Install()
    {
        REL::Relocation<std::uintptr_t> vtbl{ RE::Character::VTABLE[0] };
        MovetoLow::func = vtbl.write_vfunc(0xF2, MovetoLow::thunk);  // Character::MovetoLow
        MovetoMiddleLow::func = vtbl.write_vfunc(0xF3, MovetoMiddleLow::thunk);  // Character::MovetoMiddleLow
        StopCurrentDialogue::func = vtbl.write_vfunc(0x4F, StopCurrentDialogue::thunk);  // Character::StopCurrentDialogue
        REL::Relocation<std::uintptr_t> dialogueVtbl{ RE::VTABLE_BGSSceneActionDialogue[0] };
        EndDialogue::func = dialogueVtbl.write_vfunc(0x11, EndDialogue::thunk);  // BGSSceneActionDialogue::End
    }
}
