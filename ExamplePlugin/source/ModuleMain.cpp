// Source code for the Time of Mistria mod.
// Made for YYToolkit v4.0.1

#include <YYToolkit/YYTK_Shared.hpp>
using namespace Aurie;
using namespace YYTK;

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(Module);
	UNREFERENCED_PARAMETER(ModulePath);

	// We do a bit of trolling.
	YYTK::GetPrivateInterface()->YkSetRuntimeFlags(1);
	DbgPrint("[ExamplePlugin] Runtime flag bit 1 set.");

	return AURIE_SUCCESS;
}