#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_ModuleInterface = nullptr;

RValue& HookModifyHealth( IN CInstance* Self, IN CInstance* Other, OUT RValue& Result, IN int ArgumentCount, IN RValue** Arguments) {
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_modify_health@Ari@Ari"));
	
	if (Arguments[0]->m_Real < 0) {
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[InfiniteHealth] - Prevented health from being reduced.");
		Arguments[0]->m_Real = 0;
	}

	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

EXPORTED AurieStatus ModuleInitialize(IN AurieModule* Module, IN const fs::path& ModulePath) {
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;
	
	last_status = ObGetInterface(
		"YYTK_Main", 
		(AurieInterfaceBase*&)(g_ModuleInterface)
	);

	if (!AurieSuccess(last_status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[InfiniteHealth] - Plugin loaded!");
	
	CScript* script_modify_health = nullptr;
	last_status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_modify_health@Ari@Ari",
		(PVOID*)&script_modify_health
	);

	if (!AurieSuccess(last_status))
	{
		g_ModuleInterface->Print(CM_RED, "[InfiniteHealth] - Failed to find script: gml_Script_modify_health@Ari@Ari !");
	}

	last_status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_modify_health@Ari@Ari",
		script_modify_health->m_Functions->m_ScriptFunction,
		HookModifyHealth,
		nullptr
	);

	if (!AurieSuccess(last_status))
	{
		g_ModuleInterface->Print(CM_RED, "[InfiniteHealth] - Failed to hook script: gml_Script_modify_health@Ari@Ari !");
	}

	return AURIE_SUCCESS;
}