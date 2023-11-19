#include <Aurie/shared.hpp>
#include "YYTK/Tool.hpp"
using namespace Aurie;
using namespace YYTK;

void ModuleCallbackRoutine(
	IN AurieModule* AffectedModule,
	IN AurieModuleOperationType OperationType,
	IN bool IsFutureCall
)
{
	UNREFERENCED_PARAMETER(AffectedModule);

	// We're only interested in future calls (ie. calls that didn't yet happen)
	if (!IsFutureCall)
		return;

	// If we didn't run through the 2nd stage interface init yet, 
	if (OperationType == AURIE_OPERATION_INITIALIZE)
	{
		if (!g_ModuleInterface.m_SecondInitComplete)
			g_ModuleInterface.Create();
	}

	// Before any plugin unloading happens, we want to remove the callbacks for that plugin (if it has any)
	else if (OperationType == AURIE_OPERATION_UNLOAD)
	{
		std::erase_if(
			g_ModuleInterface.m_RegisteredCallbacks,
			[AffectedModule](const ModuleCallbackDescriptor& Descriptor)
			{
				return Descriptor.OwnerModule == AffectedModule;
			}
		);
	}
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
