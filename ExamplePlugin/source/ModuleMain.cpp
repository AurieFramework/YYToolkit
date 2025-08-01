// Source code for the Time of Mistria mod.
// Made for YYToolkit v4.0.1

#include <YYToolkit/YYTK_Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_ModuleInterface = nullptr;

// As a percentage. 100 = default time passage
static int16_t g_TimeScalingFactor = 100;

RValue& UpdateClock(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	const auto original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "ClockUpdate"));

	// If we can't get the global instance, return early.
	CInstance* global_instance = nullptr;
	if (!AurieSuccess(g_ModuleInterface->GetGlobalInstance(&global_instance)))
	{
		return original(
			Self,
			Other,
			Result,
			ArgumentCount,
			Arguments
		);
	}

	// Get the clock object
	RValue& time = *global_instance->GetRefMember("__clock")->GetRefMember("time");

	// Save the current time from the clock
	int64_t old_time_value = time.ToInt64();
	{
		original(
			Self,
			Other,
			Result,
			ArgumentCount,
			Arguments
		);
	}
	// Time may have incremented - get the current time again.
	int64_t new_time_value = time.ToInt64();

	// Measure the time difference.
	int64_t difference = new_time_value - old_time_value;

	// If time incremented
	if (difference != 0)
	{
		// Scale the difference by g_TimeScalingFactor percent
		time = old_time_value + (difference / 100.0 * g_TimeScalingFactor);
	}

	// On PAGEUP:
	if (g_ModuleInterface->CallBuiltin("keyboard_check_pressed", { VK_PRIOR }).ToBoolean())
	{
		// Attempt to get a number from the user.
		RValue integer_result = g_ModuleInterface->CallBuiltin(
			"get_integer",
			{
				"Please input the time scaling factor as a percentage.\r\n"
				"(Ex.: 50 = time runs at half-speed, 100 = time runs normally)\r\n"
				"Minimum = 0%, Maximum = 500%",
				g_TimeScalingFactor
			}
		);

		if (integer_result.m_Kind == VALUE_UNDEFINED || integer_result.m_Kind == VALUE_UNSET)
			return Result;

		if (integer_result.ToDouble() > 500.0 || integer_result.ToDouble() < 0.0)
		{
			g_ModuleInterface->GetRunnerInterface().YYError(
				"\r\nInvalid time scaling factor!\r\n"
				"There's some limits imposed so you don't break your game.\r\n"
				"They're there for a reason!\r\n"
			);
		}

		g_TimeScalingFactor = static_cast<int16_t>(integer_result.ToInt32());
	}

	return Result;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;

	// Gets a handle to the interface exposed by YYTK
	// You can keep this pointer for future use, as it will not change unless YYTK is unloaded.
	last_status = ObGetInterface(
		"YYTK_Main",
		(AurieInterfaceBase*&)(g_ModuleInterface)
	);

	// If we can't get the interface, we fail loading.
	if (!AurieSuccess(last_status))
	{
		g_ModuleInterface->PrintError(
			__FILE__,
			__LINE__,
			"[Time of Mistria v1.0.1] Failed to get YYTK_Main interface! Reason: %s. Is YYToolkit installed?",
			AurieStatusToString(last_status)
		);

		return last_status;
	}

	CScript* clock_update_script = nullptr;
	last_status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_update@Clock@Clock",
		reinterpret_cast<PVOID*>(&clock_update_script)
	);

	if (!AurieSuccess(last_status))
	{
		g_ModuleInterface->PrintError(
			__FILE__,
			__LINE__,
			"[Time of Mistria v1.0.1] Failed to find gml_Script_update@Clock@Clock! Reason: %s",
			AurieStatusToString(last_status)
		);

		return last_status;
	}

	last_status = MmCreateHook(
		g_ArSelfModule,
		"ClockUpdate",
		clock_update_script->m_Functions->m_ScriptFunction,
		UpdateClock,
		nullptr
	);

	if (!AurieSuccess(last_status))
	{
		g_ModuleInterface->PrintError(
			__FILE__,
			__LINE__,
			"[Time of Mistria v1.0.1] Failed to set a hook on gml_Script_update@Clock@Clock! Reason: %s",
			AurieStatusToString(last_status)
		);

		return last_status;
	}

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[Time of Mistria v1.0.1] Press PAGE UP to change the time scaling factor!");

	return AURIE_SUCCESS;
}