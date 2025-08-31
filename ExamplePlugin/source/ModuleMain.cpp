// Source code for the Time of Mistria mod.
// Made for YYToolkit v4.0.1

#include <YYToolkit/YYTK_Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static TRoutine g_OriginalFunction = nullptr;

void Hook(
	RValue* Result,
	CInstance* Self,
	CInstance* Other,
	int        ArgumentCount,
	RValue*    Arguments
)
{
	DbgPrintEx(LOG_SEVERITY_DEBUG, "instance_exists occurred.");
	return g_OriginalFunction(*Result, Self, Other, ArgumentCount, Arguments);
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	TRoutine game_function = nullptr;

	// Get a pointer to the target function using YYToolkit's interface
	GetInterface()->GetNamedRoutinePointer(
		"instance_exists",
		reinterpret_cast<PVOID*>(&game_function)
	);

	DbgPrintEx(LOG_SEVERITY_DEBUG, "Got instance_exists at %p", game_function);

	// Create the hook
	AurieStatus hook_status = MmCreateHook(
		g_ArSelfModule,
		"My Hook",
		game_function,
		Hook,
		reinterpret_cast<PVOID*>(&g_OriginalFunction)
	);

	DbgPrintEx(LOG_SEVERITY_DEBUG, "We got status %s for the hook", AurieStatusToString(hook_status));

	return AURIE_SUCCESS;
}