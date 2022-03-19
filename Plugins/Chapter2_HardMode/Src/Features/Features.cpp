#include "Features.hpp"

YYRValue Features::CallBuiltinWrapper(YYTKPlugin* pPlugin, CInstance* Instance, const char* Name, const std::vector<YYRValue>& rvArgs)
{
    auto InstCallFn = pPlugin->GetCoreExport<bool(*)(YYRValue&, const std::string&, CInstance*, CInstance*, const std::vector<YYRValue>&)>("CallBuiltin");

    YYRValue Result;
    InstCallFn(Result, Name, Instance, Instance, rvArgs);

    return Result;
}

void Features::RemoveSavePoints(YYTKPlugin* Plugin, CInstance* Self)
{
    YYRValue global_CurrentRoom = CallBuiltinWrapper(Plugin, Self, "variable_global_get", { "currentroom" });

    // Whitelisted save points
    switch (static_cast<int>(global_CurrentRoom))
    {
    case 84: // intro_connector (DR 1.0x)
    case 85: // intro_connector (DR 1.10)
    case 166: // dw_mansion_entrace (DR 1.0x)
    case 167: // dw_mansion_entrace (DR 1.10)
        break;
    default: // Destroy the savepoint.
        YYRValue rvSavePointID = CallBuiltinWrapper(Plugin, nullptr, "asset_get_index", { "obj_savepoint" });
        CallBuiltinWrapper(Plugin, Self, "instance_deactivate_object", { rvSavePointID });
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

int Features::IsSnowGraveRoute(YYTKPlugin* Plugin)
{
    YYRValue Flags = CallBuiltinWrapper(Plugin, nullptr, "variable_global_get", { "flag" });
    
    RValue& rvFlag = Flags.As<RValue>();

    if (rvFlag.Kind != VALUE_ARRAY)
        return 0;
    
    // Direct transcription of scr_sideb_get_phase

    if (rvFlag.RefArray->m_Array[916].Real < 0.5)
    {
        int FlagValue = round(rvFlag.RefArray->m_Array[915].Real);

        if (FlagValue > 0 && FlagValue < 4)
            return 1;

        else if (FlagValue >= 4 && FlagValue < 7)
            return 2;

        else if (FlagValue >= 7 && FlagValue < 20)
            return 3;

        else if (FlagValue >= 20)
            return 4;
    }

    return 0;
}
