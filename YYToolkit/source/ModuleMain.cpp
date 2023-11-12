#include <Aurie/shared.hpp>
#include "YYTK/Tool.hpp"
using namespace Aurie;

EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	YYTK::Internal::CmpCreateConsole();

	AurieStatus last_status = AURIE_SUCCESS;
	fs::path plugin_folder;

	// Get the game folder 
	last_status = Aurie::Internal::MdpGetImageFolder(
		g_ArInitialImage,
		plugin_folder
	);

	if (!AurieSuccess(last_status))
		return last_status;

	// Craft the path %GAMEDIR%\\mods\\yytk
	plugin_folder = plugin_folder / "mods" / "yytk";

	Aurie::ObCreateInterface(
		Module,
		&YYTK::g_ModuleInterface,
		"YYTK_Main"
	);

	YYTK::g_ModuleInterface.Create();

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleUnload(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	return AURIE_SUCCESS;
}
