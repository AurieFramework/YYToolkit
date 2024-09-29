#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static const int INVALID_ITEM_ID = -1;
static const int BLACKBERRY_JAM_ITEM_ID = 113;
static const int BLUEBERRY_JAM_ITEM_ID = 116;
static const int MARMALADE_ITEM_ID = 798;
static const int ROSEHIP_JAM_ITEM_ID = 1047;

static const int MINES_LOCATION_ID = 46;
static const int BEACH_LOCATION_ID = 8;
static const int TOWN_LOCATION_ID = 56;
static const int FARM_LOCATION_ID = 25;

static const std::tuple<int, int> MINES_TELEPORT_POINT = std::make_tuple(216, 198);
static const std::tuple<int, int> BEACH_TELEPORT_POINT = std::make_tuple(1722, 505);
static const std::tuple<int, int> TOWN_TELEPORT_POINT = std::make_tuple(1097, 1323);
static const std::tuple<int, int> FARM_TELEPORT_POINT = std::make_tuple(809, 306);

static YYTKInterface* g_ModuleInterface = nullptr;
static int held_item_id = INVALID_ITEM_ID;
static int telepop_destination_id = INVALID_ITEM_ID;
static bool teleport_ari = false;
static bool reposition_ari = false;
static bool room_loaded = false;

void ObjectCallback(
	IN FWCodeEvent& CodeEvent
)
{
	auto& [self, other, code, argc, argv] = CodeEvent.Arguments();

	if (!self)
		return;

	if (!self->m_Object)
		return;

	if (!strstr(self->m_Object->m_Name, "obj_ari"))
		return;

	if (teleport_ari)
	{
		CScript* gml_script_goto_location_id = nullptr;
		g_ModuleInterface->GetNamedRoutinePointer(
			"gml_Script_goto_location_id",
			(PVOID*)&gml_script_goto_location_id
		);

		RValue retval;
		RValue* location_id = new RValue(telepop_destination_id);
		gml_script_goto_location_id->m_Functions->m_ScriptFunction(
			self,
			other,
			retval,
			1,
			{ &location_id }
		);
		delete location_id;

		teleport_ari = false;
		reposition_ari = true;

		g_ModuleInterface->Print(CM_LIGHTGREEN, "[Telepop] - Ari teleported to Town!");
	}
	else if (reposition_ari && room_loaded) {
		RValue x;
		RValue y;
		
		if (telepop_destination_id == MINES_LOCATION_ID) {
			x = std::get<0>(MINES_TELEPORT_POINT);
			y = std::get<1>(MINES_TELEPORT_POINT);
		}

		if (telepop_destination_id == BEACH_LOCATION_ID) {
			x = std::get<0>(BEACH_TELEPORT_POINT);
			y = std::get<1>(BEACH_TELEPORT_POINT);
		}

		if (telepop_destination_id == TOWN_LOCATION_ID) {
			x = std::get<0>(TOWN_TELEPORT_POINT);
			y = std::get<1>(TOWN_TELEPORT_POINT);
		}

		if (telepop_destination_id == FARM_LOCATION_ID) {
			x = std::get<0>(FARM_TELEPORT_POINT);
			y = std::get<1>(FARM_TELEPORT_POINT);
		}

		g_ModuleInterface->SetBuiltin("x", self, NULL_INDEX, x);
		g_ModuleInterface->SetBuiltin("y", self, NULL_INDEX, y);

		held_item_id = INVALID_ITEM_ID;
		telepop_destination_id = INVALID_ITEM_ID;

		reposition_ari = false;
		room_loaded = false;
	}
}

RValue& GmlScriptHeldItemCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_held_item@Ari@Ari"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	if (Result.m_Kind != VALUE_UNDEFINED)
	{
		if (held_item_id != Result.at("item_id").m_i64 && !teleport_ari)
		{
			held_item_id = Result.at("item_id").m_i64;
		}
	}

	return Result;
}

RValue& GmlScriptModifyStaminaCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	if (held_item_id == BLACKBERRY_JAM_ITEM_ID) {
		teleport_ari = true;
		telepop_destination_id = MINES_LOCATION_ID;
		held_item_id = INVALID_ITEM_ID;
	}

	if (held_item_id == BLUEBERRY_JAM_ITEM_ID) {
		teleport_ari = true;
		telepop_destination_id = BEACH_LOCATION_ID;
		held_item_id = INVALID_ITEM_ID;
	}

	if (held_item_id == MARMALADE_ITEM_ID) {
		teleport_ari = true;
		telepop_destination_id = TOWN_LOCATION_ID;
		held_item_id = INVALID_ITEM_ID;
	}

	if (held_item_id == ROSEHIP_JAM_ITEM_ID) {
		teleport_ari = true;
		telepop_destination_id = FARM_LOCATION_ID;
		held_item_id = INVALID_ITEM_ID;
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_modify_stamina@Ari@Ari"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptShowRoomTitleCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	if (reposition_ari) {
		room_loaded = true;
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_show_room_title"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptSetupMainScreenCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	held_item_id = INVALID_ITEM_ID;
	telepop_destination_id = INVALID_ITEM_ID;
	teleport_ari = false;
	room_loaded = false;

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_setup_main_screen@TitleMenu@TitleMenu"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

void CreateHookGmlScriptHeldItem(AurieStatus& status)
{
	CScript* gml_script_held_item = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_held_item@Ari@Ari",
		(PVOID*)&gml_script_held_item
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[Telepop] - Failed to get script (gml_Script_held_item@Ari@Ari)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_held_item@Ari@Ari",
		gml_script_held_item->m_Functions->m_ScriptFunction,
		GmlScriptHeldItemCallback,
		nullptr
	);


	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[Telepop] - Failed to hook script (gml_Script_held_item@Ari@Ari)!");
	}
}

void CreateHookGmlScriptModifyStamina(AurieStatus& status)
{
	CScript* gml_script_modify_stamina = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_modify_stamina@Ari@Ari",
		(PVOID*)&gml_script_modify_stamina
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Telepop] - Failed to get script (gml_Script_modify_stamina@Ari@Ari)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_modify_stamina@Ari@Ari",
		gml_script_modify_stamina->m_Functions->m_ScriptFunction,
		GmlScriptModifyStaminaCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Telepop] - Failed to hook script (gml_Script_modify_stamina@Ari@Ari)!");
	}
}

void CreateHookGmlScriptShowRoomTitle(AurieStatus& status)
{
	CScript* gml_script_modify_stamina = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_show_room_title",
		(PVOID*)&gml_script_modify_stamina
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Telepop] - Failed to get script (gml_Script_show_room_title)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_show_room_title",
		gml_script_modify_stamina->m_Functions->m_ScriptFunction,
		GmlScriptShowRoomTitleCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Telepop] - Failed to hook script (gml_Script_show_room_title)!");
	}
}

void CreateHookGmlScriptSetupMainScreen(AurieStatus& status)
{
	CScript* gml_script_setup_main_screen = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_setup_main_screen@TitleMenu@TitleMenu",
		(PVOID*)&gml_script_setup_main_screen
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to get script (gml_Script_setup_main_screen@TitleMenu@TitleMenu)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_setup_main_screen@TitleMenu@TitleMenu",
		gml_script_setup_main_screen->m_Functions->m_ScriptFunction,
		GmlScriptSetupMainScreenCallback,
		nullptr
	);


	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to hook script (gml_Script_setup_main_screen@TitleMenu@TitleMenu)!");
	}
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus status = AURIE_SUCCESS;
	
	status = ObGetInterface(
		"YYTK_Main", 
		(AurieInterfaceBase*&)(g_ModuleInterface)
	);

	if (!AurieSuccess(status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	g_ModuleInterface->Print(CM_LIGHTYELLOW, "[Telepop] - Plugin starting...");
	
	g_ModuleInterface->CreateCallback(
		g_ArSelfModule,
		EVENT_OBJECT_CALL,
		ObjectCallback,
		0
	);

	CreateHookGmlScriptHeldItem(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Telepop] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptModifyStamina(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Telepop] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptShowRoomTitle(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Telepop] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptSetupMainScreen(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Telepop] - Exiting due to failure on start!");
		return status;
	}

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[Telepop] - Plugin started!");
	return AURIE_SUCCESS;
}