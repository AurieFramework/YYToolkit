#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_ModuleInterface = nullptr;

void FrameCallback(FWFrame& FrameContext)
{
	UNREFERENCED_PARAMETER(FrameContext);

	static uint32_t frame_counter = 0;

	if (frame_counter % 30 == 0)
		g_ModuleInterface->PrintWarning("[Example Plugin] - 30 frames have passed! Framecount: %u", frame_counter);

	frame_counter++;
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
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
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[Example Plugin] - Hello from PluginEntry!");

	last_status = g_ModuleInterface->CreateCallback(
		Module,
		EVENT_FRAME,
		FrameCallback,
		0
	);

	if (!AurieSuccess(last_status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[Example Plugin] - Failed to register callback!");
	}

	return AURIE_SUCCESS;
}