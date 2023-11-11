#include "YYTK/Tool.hpp"
using namespace Aurie;

// Called in an Early Launch environment, only Aurie functions are available.
// The process entry point hasn't been called yet.
EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	YYTK::Internal::CmpCreateConsole();

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

	YYTK::YYRunnerInterface my_interface;
	YYTK::YYTKStatus status = YYTK::Internal::GmpGetRunnerInterface(my_interface);

	YYTK::CmWriteError(
		__FILE__,
		__LINE__,
		"YYTKStatus %d", status
	);

	return AURIE_SUCCESS;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	return AURIE_SUCCESS;
}