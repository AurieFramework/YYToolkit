#include <Aurie/shared.hpp>
#include "YYTK/Tool.hpp"
using namespace Aurie;
using namespace YYTK;

EXPORTED void ModuleOperationCallback(
	IN AurieModule* AffectedModule,
	IN AurieModuleOperationType OperationType,
	OPTIONAL IN OUT AurieOperationInfo* OperationInfo
)
{
	// If we didn't get a valid operation info struct, we bail
	if (!OperationInfo)
		return;

	// We're only interested in future calls (ie. calls that didn't yet happen)
	if (!OperationInfo->IsFutureCall)
		return;

	switch (OperationType)
	{
	case AURIE_OPERATION_INITIALIZE:
		{
			if (!g_ModuleInterface.m_SecondInitComplete)
				g_ModuleInterface.Create();
			break;
		}
	case AURIE_OPERATION_UNLOAD:
		{
			// Before any plugin unloading happens, we want to remove the callbacks for that plugin (if it has any)
			std::erase_if(
				g_ModuleInterface.m_RegisteredCallbacks,
				[AffectedModule](const ModuleCallbackDescriptor& Descriptor)
				{
					return Descriptor.OwnerModule == AffectedModule;
				}
			);
		}
	}
}

EXPORTED AurieStatus ModulePreinitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus last_status = AURIE_SUCCESS;

	last_status = ObCreateInterface(
		Module,
		&g_ModuleInterface,
		"YYTK_Main"
	);

	DbgPrintEx(
		LOG_SEVERITY_TRACE,
		"[%s:%d] YYTK_Main interface created with status %s",
		__FILE__, 
		__LINE__, 
		AurieStatusToString(last_status)
	);

	if (!AurieSuccess(last_status))
		return last_status;

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