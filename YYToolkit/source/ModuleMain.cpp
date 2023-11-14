#include <Aurie/shared.hpp>
#include "YYTK/Tool.hpp"
using namespace Aurie;

void ModuleCallbackRoutine(
	IN const AurieModule* const AffectedModule,
	IN const AurieModuleOperationType OperationType,
	IN const bool IsFutureCall
)
{
	// We're only interested in future calls (ie. calls that didn't yet happen)
	if (!IsFutureCall)
		return;

	// We're waiting for ModuleInitialize calls
	if (OperationType != AURIE_OPERATION_INITIALIZE)
		return;

	// Call the interface's Create function again, to make sure 
	YYTK::g_ModuleInterface.Create();

	// Disable any future callbacks
	Internal::ObpSetModuleOperationCallback(g_ArSelfModule, nullptr);
}

EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	YYTK::CmpCreateConsole();

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
		&YYTK::g_ModuleInterface,
		"YYTK_Main"
	);

	int index = 0;
	AurieStatus status = YYTK::g_ModuleInterface.GetNamedRoutineIndex(
		"@@GlobalScope@@",
		&index
	);

	YYTK::CmWriteOutput(
		YYTK::CM_LIGHTAQUA, 
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
	if (!YYTK::g_ModuleInterface.m_FirstInitComplete)
		return AURIE_MODULE_INITIALIZATION_FAILED;

	if (!YYTK::g_ModuleInterface.m_SecondInitComplete)
		return AURIE_MODULE_INITIALIZATION_FAILED;

	int index = 0;
	AurieStatus status = YYTK::g_ModuleInterface.GetNamedRoutineIndex(
		"gml_Script_input_player_verify",
		&index
	);

	YYTK::CmWriteOutput(
		YYTK::CM_LIGHTAQUA,
		"[Initialize] GetNamedRoutineIndex() returns status %d and function ID %d!",
		status,
		index
	);

	YYTK::YYRunnerInterface& runner_interface = YYTK::g_ModuleInterface.m_RunnerInterface;
	
	YYTK::CInstance* instance = nullptr;
	YYTK::g_ModuleInterface.GetGlobalInstance(&instance);


	YYTK::RValue global_instance; 
	global_instance.m_Pointer = instance; 
	global_instance.m_Kind = YYTK::VALUE_OBJECT;

	int num_keys = runner_interface.StructGetKeys(
		&global_instance,
		nullptr,
		nullptr
	);
	
	std::vector<const char*> keys(num_keys);
	runner_interface.StructGetKeys(&global_instance, keys.data(), &num_keys);

	for (int i = 0; i < num_keys; i++)
	{
		YYTK::RValue* member = runner_interface.StructGetMember(&global_instance, keys[i]);

		YYTK::CmWriteOutput(YYTK::CM_LIGHTPURPLE, "%s", runner_interface.YYGetString(member, 0));
	}

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleUnload(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	return AURIE_SUCCESS;
}
