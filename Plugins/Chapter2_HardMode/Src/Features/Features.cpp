#include "Features.hpp"

YYRValue Features::CallBuiltinWrapper(YYTKPlugin* pPlugin, CInstance* Instance, const char* Name, const std::vector<YYRValue>& rvArgs)
{
    if (!Instance)
    {
        auto GlobalCallFn = pPlugin->GetCoreExport<YYTKStatus(*)(const char*, int, YYRValue&, const YYRValue*)>("Global_CallBuiltin");

        YYRValue Result;
        GlobalCallFn(Name, rvArgs.size(), Result, rvArgs.data());

        return Result;
    }

    // Modify the args to be const*, so the compiler doesn't complain
    auto InstCallFn = pPlugin->GetCoreExport<YYTKStatus(*)(CInstance*, CInstance*, YYRValue&, int, const char*, const YYRValue*)>("CallBuiltinFunction");

    YYRValue Result;
    InstCallFn(Instance, Instance, Result, rvArgs.size(), Name, rvArgs.data());

    return Result;
}

void Features::RemoveSavePoints(YYTKPlugin* Plugin, CInstance* Self)
{
    YYRValue global_CurrentRoom = CallBuiltinWrapper(Plugin, Self, "variable_global_get", { "currentroom" });

    // Whitelisted save points
    switch (static_cast<int>(global_CurrentRoom))
    {
    case 3: // Queen's Mansion - Rooftop
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

void Features::ChangeEnemyStats(YYTKPlugin* Plugin, CInstance* Self, double KromerMul, double HPMul, double ATKMul)
{
    YYRValue Monster = CallBuiltinWrapper(Plugin, nullptr, "variable_global_get", { "monster" });
    YYRValue MonsterHP = CallBuiltinWrapper(Plugin, nullptr, "variable_global_get", { "monsterhp" });
    YYRValue MonsterMaxHP = CallBuiltinWrapper(Plugin, nullptr, "variable_global_get", { "monstermaxhp" });
    YYRValue MonsterKromer = CallBuiltinWrapper(Plugin, nullptr, "variable_global_get", { "monstergold" });
    YYRValue MonsterATK = CallBuiltinWrapper(Plugin, nullptr, "variable_global_get", { "monsterat" });

    if (Monster.As<RValue>().Kind != VALUE_ARRAY)
        return;
    else if (MonsterHP.As<RValue>().Kind != VALUE_ARRAY)
        return;
    else if (MonsterMaxHP.As<RValue>().Kind != VALUE_ARRAY)
        return;
    else if (MonsterKromer.As<RValue>().Kind != VALUE_ARRAY)
        return;
    else if (MonsterATK.As<RValue>().Kind != VALUE_ARRAY)
        return;

    for (int n = 0; n < 3; n++)
    {
        if (Monster.As<RValue>().RefArray->m_Array[n].Real < 0.5)
            continue;

        if (MonsterHP.As<RValue>().RefArray->m_Array[n].Kind == VALUE_REAL)
            MonsterHP.As<RValue>().RefArray->m_Array[n].Real *= HPMul;

        if (MonsterMaxHP.As<RValue>().RefArray->m_Array[n].Kind == VALUE_REAL)
            MonsterMaxHP.As<RValue>().RefArray->m_Array[n].Real *= HPMul;

        if (MonsterKromer.As<RValue>().RefArray->m_Array[n].Kind == VALUE_REAL)
            MonsterKromer.As<RValue>().RefArray->m_Array[n].Real *= KromerMul;

        if (MonsterATK.As<RValue>().RefArray->m_Array[n].Kind == VALUE_REAL)
            MonsterATK.As<RValue>().RefArray->m_Array[n].Real *= ATKMul;
    }
}

bool Features::IsSnowGraveRoute(YYTKPlugin* Plugin)
{
    YYRValue Flags = CallBuiltinWrapper(Plugin, nullptr, "variable_global_get", { "flag" });
    
    RValue& rvFlag = Flags.As<RValue>();

    if (rvFlag.Kind != VALUE_ARRAY)
        return false;
    
    return (rvFlag.RefArray->m_Array[916].Real < 0.5) && (rvFlag.RefArray->m_Array[915].Real > 0);
}

