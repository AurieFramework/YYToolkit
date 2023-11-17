#include <Aurie/shared.hpp>
#include "YYTK/Tool.hpp"
using namespace Aurie;
using namespace YYTK;

void ModuleCallbackRoutine(
	IN const AurieModule* const AffectedModule,
	IN const AurieModuleOperationType OperationType,
	IN const bool IsFutureCall
)
{
	UNREFERENCED_PARAMETER(AffectedModule);

	// We're only interested in future calls (ie. calls that didn't yet happen)
	if (!IsFutureCall)
		return;

	// We're waiting for ModuleInitialize calls
	if (OperationType != AURIE_OPERATION_INITIALIZE)
		return;

	// Call the interface's Create function again, to make sure 
	g_ModuleInterface.Create();

	// Disable any future callbacks
	Internal::ObpSetModuleOperationCallback(g_ArSelfModule, nullptr);
}

EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(ModulePath);

	CmpCreateConsole();

	AurieStatus last_status = AURIE_SUCCESS;
	fs::path plugin_folder;

	// Get the game folder 
	last_status = Internal::MdpGetImageFolder(
		g_ArInitialImage,
		plugin_folder
	);

	if (!AurieSuccess(last_status))
		return last_status;

	// Craft the path %GAMEDIR%\\mods\\yytk
	plugin_folder = plugin_folder / "mods" / "yytk";

	ObCreateInterface(
		Module,
		&g_ModuleInterface,
		"YYTK_Main"
	);

	int index = 0;
	AurieStatus status = g_ModuleInterface.GetNamedRoutineIndex(
		"@@GlobalScope@@",
		&index
	);

	CmWriteOutput(
		CM_LIGHTAQUA, 
		"[Preload] GetNamedRoutineIndex(\"@@GlobalScope@@\") returns status %d and function ID %d!",
		status, 
		index
	);
	
	Internal::ObpSetModuleOperationCallback(
		g_ArSelfModule,
		ModuleCallbackRoutine
	);

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	if (!g_ModuleInterface.m_FirstInitComplete)
		return AURIE_MODULE_INITIALIZATION_FAILED;

	if (!g_ModuleInterface.m_SecondInitComplete)
		return AURIE_MODULE_INITIALIZATION_FAILED;

	int index = 0;
	AurieStatus status = g_ModuleInterface.GetNamedRoutineIndex(
		"gml_Script_input_player_verify",
		&index
	);

	CmWriteOutput(
		CM_LIGHTAQUA,
		"[Initialize] GetNamedRoutineIndex() returns status %d and function ID %d!",
		status,
		index
	);

	RValue test("hi", &g_ModuleInterface);
	assert(test.m_Kind == VALUE_STRING);

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleUnload(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	return AURIE_SUCCESS;
}
