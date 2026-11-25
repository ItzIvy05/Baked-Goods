#include "LingeringSubtitle.h"

namespace {
    using RemoveSubtitle_t = void(RE::SubtitleManager*, RE::TESObjectREFR*);
    REL::Relocation<RemoveSubtitle_t> RemoveSubtitle{RELOCATION_ID(51754, 52627)};  // SubtitleManager::RemoveSubtitle

    void Remove(RE::SubtitleManager* a_this, RE::TESObjectREFR* a_speaker) {
        RemoveSubtitle(a_this, a_speaker);
        bool stale = false;
        {
            RE::BSSpinLockGuard locker{a_this->lock};
            stale = a_this->subtitles.empty() && a_this->currentSpeaker;
        }
        if (stale) {
            a_this->KillSubtitles();
        }
    }

    struct SayAddSubtitle {
        static void thunk(RE::SubtitleManager* a_this, RE::Actor* a_speaker, const char* a_text, bool a_forceDisplay) {
            func(a_this, a_speaker, a_text, a_forceDisplay);
            if (a_speaker->IsPlayerRef()) {
                return;
            }
            auto& runtime = a_speaker->GetActorRuntimeData();
            if (runtime.voiceTimer <= 0.0f) {
                const auto settings = RE::GameSettingCollection::GetSingleton();
                runtime.voiceTimer = static_cast<float>(std::strlen(a_text)) * settings->GetSetting("fAIDefaultSpeechMult")->GetFloat() + settings->GetSetting("fGameplayVoiceFilePadding")->GetFloat();
            }
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    // the high update skips actors the camera can't see, which also freezes their voice timer mid line
    struct SkipFullUpdate {
        static bool thunk(RE::Actor* a_actor) {
            const auto& runtime = a_actor->GetActorRuntimeData();
            if (runtime.voiceTimer > 0.0f || (runtime.soundCallBackSet >= 1 && runtime.soundCallBackSet <= 3)) {
                return false;
            }
            return func(a_actor);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    // engine ends the line on demotion but leaves the subtitle for a low process update that comes hours later
    struct MovetoLow {
        static bool thunk(RE::Character* a_this) {
            const auto result = func(a_this);
            Remove(RE::SubtitleManager::GetSingleton(), a_this);
            return result;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    struct MovetoMiddleLow {
        static bool thunk(RE::Character* a_this) {
            const auto result = func(a_this);
            Remove(RE::SubtitleManager::GetSingleton(), a_this);
            return result;
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    // the voice timer hands an expired line here, which only clears the subtitle while a voice is still playing
    struct StopCurrentDialogue {
        static void thunk(RE::Character* a_this) {
            func(a_this);
            Remove(RE::SubtitleManager::GetSingleton(), a_this);
        }
        static inline REL::Relocation<decltype(thunk)> func;
    };

    // a scene that ends mid line never finishes a non-actor speaker's line, so its subtitle stays
    struct EndDialogue {
        static void thunk(RE::BGSSceneActionDialogue* a_this, RE::BGSScene* a_scene) {
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

namespace LingeringSubtitle {
    void Install() {
        REL::Relocation<std::uintptr_t> vtbl{RE::Character::VTABLE[0]};
        MovetoLow::func = vtbl.write_vfunc(0xF2, MovetoLow::thunk);                      // Character::MovetoLow
        MovetoMiddleLow::func = vtbl.write_vfunc(0xF3, MovetoMiddleLow::thunk);          // Character::MovetoMiddleLow
        StopCurrentDialogue::func = vtbl.write_vfunc(0x4F, StopCurrentDialogue::thunk);  // Character::StopCurrentDialogue
        REL::Relocation<std::uintptr_t> dialogueVtbl{RE::VTABLE_BGSSceneActionDialogue[0]};
        EndDialogue::func = dialogueVtbl.write_vfunc(0x11, EndDialogue::thunk);                                   // BGSSceneActionDialogue::End
        REL::Relocation<std::uintptr_t> say{RELOCATION_ID(36542, 37543), REL::VariantOffset(0x2BF, 0x350, 0x0)};  // Actor::Say
        SayAddSubtitle::func = say.write_call<5>(SayAddSubtitle::thunk);
        REL::Relocation<std::uintptr_t> highUpdate{RELOCATION_ID(40356, 41369), REL::VariantOffset(0x101, 0x105, 0x0)};  // ProcessLists high actor update
        SkipFullUpdate::func = highUpdate.write_call<5>(SkipFullUpdate::thunk);
        auto& trampoline = SKSE::GetTrampoline();
        trampoline.write_call<5>(REL::Relocation<std::uintptr_t>{RELOCATION_ID(19119, 19521), REL::VariantOffset(0xA5, 0xA5, 0x0)}.address(), Remove);    // TESObjectREFR::UpdateSoundCallBack
        trampoline.write_call<5>(REL::Relocation<std::uintptr_t>{RELOCATION_ID(19119, 19521), REL::VariantOffset(0x153, 0x153, 0x0)}.address(), Remove);  // TESObjectREFR::UpdateSoundCallBack
        trampoline.write_call<5>(REL::Relocation<std::uintptr_t>{RELOCATION_ID(19119, 19521), REL::VariantOffset(0x1E4, 0x1E4, 0x0)}.address(), Remove);  // TESObjectREFR::UpdateSoundCallBack
        trampoline.write_call<5>(REL::Relocation<std::uintptr_t>{RELOCATION_ID(36543, 37544), REL::VariantOffset(0x173, 0x173, 0x0)}.address(), Remove);  // Actor::UpdateSoundCallback
        trampoline.write_call<5>(REL::Relocation<std::uintptr_t>{RELOCATION_ID(36543, 37544), REL::VariantOffset(0x492, 0x492, 0x0)}.address(), Remove);  // Actor::UpdateSoundCallback
        trampoline.write_call<5>(REL::Relocation<std::uintptr_t>{RELOCATION_ID(36543, 37544), REL::VariantOffset(0x602, 0x602, 0x0)}.address(), Remove);  // Actor::UpdateSoundCallback
        trampoline.write_call<5>(REL::Relocation<std::uintptr_t>{RELOCATION_ID(36728, 37741), REL::VariantOffset(0x145, 0x145, 0x0)}.address(), Remove);  // Actor::PauseCurrentDialogue
        trampoline.write_call<5>(REL::Relocation<std::uintptr_t>{RELOCATION_ID(36728, 37741), REL::VariantOffset(0x4D5, 0x4D9, 0x0)}.address(), Remove);  // Actor::PauseCurrentDialogue
    }
}