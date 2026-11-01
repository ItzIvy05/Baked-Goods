#include "RaceMenuCommand.h"

namespace
{
    bool Execute(const RE::SCRIPT_PARAMETER*, RE::SCRIPT_FUNCTION::ScriptData*, RE::TESObjectREFR*, RE::TESObjectREFR*, RE::Script*, RE::ScriptLocals*, double&, std::uint32_t&)
    {
        RE::ConsoleLog::GetSingleton()->Print("Use the Face Sculptor in Riften");
        return true;
    }
}

namespace RaceMenuCommand
{
    void Install()
    {
        RE::SCRIPT_FUNCTION::LocateScriptCommand("ShowRaceMenu"sv)->executeFunction = Execute;
    }
}
