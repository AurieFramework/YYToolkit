#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static const double INVALID_COORD = -1.0;
static const double DEFAULT_MOVE_MODIFIER = 1.0;

static YYTKInterface* g_ModuleInterface = nullptr;
static bool ari_is_moving_in_line_along_x = false;
static bool ari_is_moving_in_line_along_y = false;
static bool ari_is_standing_still = false;
static bool coordinate_is_increasing = false;
static bool ari_pivoted = false;
static double ari_x = INVALID_COORD;
static double ari_y = INVALID_COORD;
static double ari_move_modifier = DEFAULT_MOVE_MODIFIER;

bool AriIsMovingInALine() {
	return ari_is_moving_in_line_along_x || ari_is_moving_in_line_along_y;
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

	RValue x;
	g_ModuleInterface->GetBuiltin("x", self, NULL_INDEX, x);

	RValue y;
	g_ModuleInterface->GetBuiltin("y", self, NULL_INDEX, y);

	if (ari_x == x.m_Real && ari_y == y.m_Real) { // x-coord unchanged AND y-coord unchanged
		ari_is_standing_still = true;
	}
	else {
		ari_is_standing_still = false;
	}

	if (ari_x != x.m_Real && ari_y != y.m_Real) { // x-coord changed AND y-coord changed
		ari_is_moving_in_line_along_x = false;
		ari_is_moving_in_line_along_y = false;
	}

	if (!AriIsMovingInALine()) {
		if (ari_x != x.m_Real && ari_y == y.m_Real) { // x-coord changed AND y-coord unchanged
			if (x.m_Real > ari_x) {
				coordinate_is_increasing = true;
			}
			else {
				coordinate_is_increasing = false;
			}
			ari_is_moving_in_line_along_x = true;
		}
		if (ari_x == x.m_Real && ari_y != y.m_Real) { // x-coord changed AND y-coord unchanged
			if (y.m_Real > ari_y) {
				coordinate_is_increasing = true;
			}
			else {
				coordinate_is_increasing = false;
			}
			ari_is_moving_in_line_along_y = true;
		}
	}
	else {
		if (ari_is_moving_in_line_along_x) {
			if (ari_y != y.m_Real) { // y-coord changed
				ari_is_moving_in_line_along_x = false;
			}
			else {
				if (!ari_is_standing_still) {
					if (coordinate_is_increasing) {
						if (x.m_Real < ari_x) {
							ari_pivoted = true;
							coordinate_is_increasing = !coordinate_is_increasing;
						}
					}
					else {
						if (x.m_Real > ari_x) {
							ari_pivoted = true;
							coordinate_is_increasing = !coordinate_is_increasing;
						}
					}
				}
			}
		}
		if (ari_is_moving_in_line_along_y) {
			if (ari_x != x.m_Real) { // x-coord changed
				ari_is_moving_in_line_along_y = false;
			}
			else {
				if (!ari_is_standing_still) {
					if (coordinate_is_increasing) {
						if (y.m_Real < ari_y) {
							ari_pivoted = true;
							coordinate_is_increasing = !coordinate_is_increasing;
						}
					}
					else {
						if (y.m_Real > ari_y) {
							ari_pivoted = true;
							coordinate_is_increasing = !coordinate_is_increasing;
						}
					}
				}
			}
		}
	}

	ari_x = x.m_Real;
	ari_y = y.m_Real;

	CInstance* global_instance = nullptr;
	g_ModuleInterface->GetGlobalInstance(&global_instance);

	CScript* script_object = nullptr;
	g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_get_move_speed@Ari@Ari",
		(PVOID*)&script_object
	);

	RValue result;
	script_object->m_Functions->m_ScriptFunction(
		global_instance->at("__ari").m_Object,
		self,
		result,
		0,
		nullptr
	);
}

RValue& GmlScriptGetMoveSpeedCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_get_move_speed@Ari@Ari"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	if (!ari_is_standing_still) {
		if (AriIsMovingInALine()) {
			if (ari_pivoted) {
				ari_move_modifier = 1.0;
				ari_pivoted = false;
			}
			else {
				ari_move_modifier += 0.02;
				if (ari_move_modifier > 3.0) {
					ari_move_modifier = 3.0;
				}
			}
		}
		else {
			ari_move_modifier = 1.0;
		}
	}

	Result.m_Real *= ari_move_modifier;
	return Result; // 2.0 is default run speed, max run speed boost from gear is 20% => 2.4
}

void CreateHookGmlScriptGetMoveSpeed(AurieStatus& status)
{
	CScript* gml_script_get_move_speed = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_get_move_speed@Ari@Ari",
		(PVOID*)&gml_script_get_move_speed
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Telepop] - Failed to get script (gml_Script_get_move_speed@Ari@Ari)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_get_move_speed@Ari@Ari",
		gml_script_get_move_speed->m_Functions->m_ScriptFunction,
		GmlScriptGetMoveSpeedCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Telepop] - Failed to hook script (gml_Script_get_move_speed@Ari@Ari)!");
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

	CreateHookGmlScriptGetMoveSpeed(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[Telepop] - Exiting due to failure on start!");
		return status;
	}

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[Telepop] - Plugin started!");
	return AURIE_SUCCESS;
}