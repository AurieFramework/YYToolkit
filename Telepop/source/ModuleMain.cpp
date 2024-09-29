#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_ModuleInterface = nullptr;
static int held_item_id = -1;
static bool teleport_ari = false;

// -----------------
// 8 = beach (1722, 505)
// 25 = farm (809, 306)
// 46 = mines (216, 198)
// 56 = town (1097, 1323)
static int teleportation_id = 56;
static bool reposition_ari = false;
static bool room_loaded = false;
static int frame_counter = 0;
static int ari_x = 0;
static int ari_y = 0;
// ------------------

void FrameCallback(FWFrame& FrameContext)
{
	UNREFERENCED_PARAMETER(FrameContext);

	if (GetAsyncKeyState(VK_PAUSE) & 1)
	{
		RValue integer_result = g_ModuleInterface->CallBuiltin(
			"get_integer",
			{
				"Please input a teleportation ID.",
				teleportation_id
			}
		);

		if (integer_result.m_Kind != VALUE_UNDEFINED && integer_result.m_Kind != VALUE_UNSET)
		{
			teleportation_id = static_cast<int16_t>(integer_result.AsReal());
		}
	}
	if (GetAsyncKeyState(VK_INSERT) & 1)
	{
		RValue x_coordinate = g_ModuleInterface->CallBuiltin(
			"get_integer",
			{
				"Please input an X coordinate.",
				ari_x
			}
		);

		if (x_coordinate.m_Kind != VALUE_UNDEFINED && x_coordinate.m_Kind != VALUE_UNSET)
		{
			ari_x = static_cast<int16_t>(x_coordinate.AsReal());
		}

		RValue y_coordinate = g_ModuleInterface->CallBuiltin(
			"get_integer",
			{
				"Please input an Y coordinate.",
				ari_y
			}
		);

		if (y_coordinate.m_Kind != VALUE_UNDEFINED && y_coordinate.m_Kind != VALUE_UNSET)
		{
			ari_y = static_cast<int16_t>(y_coordinate.AsReal());
		}
	}

	frame_counter++;
}

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

	// ------------------------
	RValue x;
	g_ModuleInterface->GetBuiltin(
		"x",
		self,
		NULL_INDEX,
		x
	);
	RValue y;
	g_ModuleInterface->GetBuiltin(
		"y",
		self,
		NULL_INDEX,
		y
	);
	if (frame_counter % 300 == 0) {
		g_ModuleInterface->Print(CM_AQUA, "[Telepop] - X,Y COORDS: %f, %f", x.m_Real, y.m_Real);
	}
	// ------------------------

	if (teleport_ari)
	{
		CScript* gml_script_item_id_to_name = nullptr;
		g_ModuleInterface->GetNamedRoutinePointer(
			"gml_Script_goto_location_id",
			(PVOID*)&gml_script_item_id_to_name
		);

		RValue retval;
		RValue* location_id = new RValue(teleportation_id);
		gml_script_item_id_to_name->m_Functions->m_ScriptFunction(
			self,
			other,
			retval,
			1,
			{ &location_id }
		);

		delete location_id;
		teleport_ari = false;
		held_item_id = -1;

		g_ModuleInterface->Print(CM_LIGHTGREEN, "[Telepop] - Ari teleported to Town!");

		// --------------------
		reposition_ari = true;
	}
	else if (reposition_ari && room_loaded) {
		RValue goto_x = ari_x;
		RValue goto_y = ari_y;
		g_ModuleInterface->SetBuiltin("x", self, NULL_INDEX, goto_x);
		g_ModuleInterface->SetBuiltin("y", self, NULL_INDEX, goto_y);

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
	if (held_item_id == 1047) {
		teleport_ari = true;
		held_item_id = -1;
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
		Module,
		EVENT_FRAME,
		FrameCallback,
		0
	);

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

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[Telepop] - Plugin started!");
	return AURIE_SUCCESS;
}