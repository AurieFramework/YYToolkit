#include "Features.hpp"

YYRValue Features::CallBuiltinWrapper(YYTKPlugin* pPlugin, CInstance* Instance, const char* Name, const std::vector<YYRValue>& rvArgs)
{
    // Modify the args to be const*, so the compiler doesn't complain
    auto CallBuiltin = pPlugin->GetCoreExport<YYTKStatus(*)(CInstance*, CInstance*, YYRValue&, int, const char*, const YYRValue*)>("CallBuiltinFunction");

    YYRValue Result;
    CallBuiltin(Instance, Instance, Result, rvArgs.size(), Name, rvArgs.data());

    return Result;
}

void Features::RemoveSavePoints(YYTKPlugin* Plugin, CInstance* Self)
{
    YYRValue global_CurrentRoom = CallBuiltinWrapper(Plugin, Self, "variable_global_get", { "currentroom" });

    switch (static_cast<int>(global_CurrentRoom))
    {
        // Whitelisted save points
    case 71: // My Castle Town
    case 84: // Dark World?
    case 87: // Cyber Field - Entrance
    case 98: // Cyber Field - Music Shop
    case 130: // Cyber City - Music Shop
    case 142: // Cyber City - Heights
    case 166: // Queen's Mansion - Entrance
    case 180: // Queen's Mansion - Basement
    case 205: // Queen's Mansion - 4F
        break;

    default: // Destroy the savepoint.
        CallBuiltinWrapper(Plugin, Self, "instance_destroy", {});
        break;
    }
}
