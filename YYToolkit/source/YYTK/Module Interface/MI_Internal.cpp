#include "MI.hpp"
using namespace Aurie;

namespace YYTK
{
	void YYTKInterfaceImpl::YkExtractFunctionEntry(
		IN size_t Index,
		OUT std::string& FunctionName,
		OUT TRoutine& FunctionRoutine,
		OUT int32_t& ArgumentCount
	)
	{
		RFunction* functions_array = *m_FunctionsArray;

		if (m_FunctionEntrySize == sizeof(RFunctionStringRef))
		{
			RFunctionStringRef& function_entry = functions_array->GetIndexReferential(Index);

			// Save stuff into the variables
			if (function_entry.m_Name)
				FunctionName = function_entry.m_Name;

			FunctionRoutine = function_entry.m_Routine;
			ArgumentCount = function_entry.m_ArgumentCount;
		}
		else if (m_FunctionEntrySize == sizeof(RFunctionStringFull))
		{
			RFunctionStringFull& function_entry = functions_array->GetIndexFull(Index);

			// Properly null-terminate the string
			char string_buffer[70] = { 0 };
			if (function_entry.m_Name)
				strncpy_s(string_buffer, function_entry.m_Name, 64);

			// You know the rest...
			FunctionName = string_buffer;
			FunctionRoutine = function_entry.m_Routine;
			ArgumentCount = function_entry.m_ArgumentCount;
		}
	}

	AurieStatus YYTKInterfaceImpl::YkFetchD3D11Info(
		OUT ID3D11Device** DeviceObject,
		OUT IDXGISwapChain** Swapchain
	)
	{
		AurieStatus last_status = AURIE_SUCCESS;

		// Call os_get_info, which gets us the necessary info
		RValue os_info_ds_map;
		last_status = CallBuiltinEx(
			os_info_ds_map,
			"os_get_info",
			nullptr,
			nullptr,
			{}
		);

		// This is not checking the return value of os_get_info,
		// instead checking if we even called the function successfully.
		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(
				LOG_SEVERITY_CRITICAL,
				"Failed to call os_get_info function! (%s)",
				AurieStatusToString(last_status)
			);

			return last_status;
		}

		// Pull everything needed from the DS List
		// We need to pass the pointer to the interface into the RValue initializer
		// here, because Aurie didn't yet put our interface in its array (we're being called from stage II init), 
		// therefore the hidden ObGetInterface calls within the RValue would fail.
		RValue dx_swapchain, dx_device;

		last_status = CallBuiltinEx(
			dx_device,
			"ds_map_find_value",
			nullptr,
			nullptr,
			{ os_info_ds_map, "video_d3d11_device" }
		);

		// This is not checking the return value of ds_map_find_value,
		// instead checking if we even called the function successfully.
		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(
				LOG_SEVERITY_CRITICAL,
				"Failed to get video_d3d11_device! (%s)",
				AurieStatusToString(last_status)
			);

			return AURIE_OBJECT_NOT_FOUND;
		}

		last_status = CallBuiltinEx(
			dx_swapchain,
			"ds_map_find_value",
			nullptr,
			nullptr,
			{ os_info_ds_map, "video_d3d11_swapchain" }
		);

		// This is not checking the return value of ds_map_find_value,
		// instead checking if we even called the function successfully.
		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(
				LOG_SEVERITY_CRITICAL,
				"Failed to get video_d3d11_swapchain! (%s)",
				AurieStatusToString(last_status)
			);

			return AURIE_OBJECT_NOT_FOUND;
		}

		if (DeviceObject)
			*DeviceObject = static_cast<ID3D11Device*>(dx_device.m_Pointer);

		if (Swapchain)
			*Swapchain = static_cast<IDXGISwapChain*>(dx_swapchain.m_Pointer);

		return AURIE_SUCCESS;
	}

	ModuleCallbackDescriptor* YYTKInterfaceImpl::YkFindDescriptor(
		IN const ModuleCallbackDescriptor& Descriptor
	)
	{
		auto iterator = std::find_if(
			m_RegisteredCallbacks.begin(),
			m_RegisteredCallbacks.end(),
			[Descriptor](const ModuleCallbackDescriptor& Element) -> bool
			{
				return Descriptor.OwnerModule == Element.OwnerModule &&
					Descriptor.Routine == Element.Routine &&
					Descriptor.Trigger == Element.Trigger &&
					Descriptor.Priority == Element.Priority;
			}
		);

		if (iterator == std::end(m_RegisteredCallbacks))
			return nullptr;

		return &(*iterator);
	}

	void YYTKInterfaceImpl::YkRemoveCallbackFromList(
		IN Aurie::AurieModule* Module,
		IN PVOID Routine
	)
	{
		if (!YkCallbackExists(Module, Routine))
			return;

		std::erase_if(
			m_RegisteredCallbacks,
			[Routine](const ModuleCallbackDescriptor& Descriptor) -> bool
			{
				return Descriptor.Routine == Routine;
			}
		);
	}

	bool YYTKInterfaceImpl::YkCallbackExists(
		IN Aurie::AurieModule* Module,
		IN PVOID Routine
	)
	{
		return std::find_if(
			m_RegisteredCallbacks.begin(),
			m_RegisteredCallbacks.end(),
			[Module, Routine](const ModuleCallbackDescriptor& Descriptor) -> bool
			{
				return Descriptor.Routine == Routine && Descriptor.OwnerModule == Module;
			}
		) != std::end(m_RegisteredCallbacks);
	}

	ModuleCallbackDescriptor YYTKInterfaceImpl::YkCreateCallbackDescriptor(
		IN Aurie::AurieModule* Module,
		IN EventTriggers Trigger,
		IN PVOID Routine,
		IN int32_t Priority
	)
	{
		ModuleCallbackDescriptor descriptor = {};
		descriptor.OwnerModule = Module;
		descriptor.Trigger = Trigger;
		descriptor.Routine = Routine;
		descriptor.Priority = Priority;

		return descriptor;
	}

	ModuleCallbackDescriptor* YYTKInterfaceImpl::YkAddToCallbackList(
		IN ModuleCallbackDescriptor& Descriptor
	)
	{
		m_RegisteredCallbacks.emplace_back(Descriptor);

		// Make sure the descriptors are sorted by priority, so that when
		// YkDispatchCallbacks runs, they're sorted
		std::sort(
			m_RegisteredCallbacks.begin(),
			m_RegisteredCallbacks.end(),
			[](const ModuleCallbackDescriptor& First, const ModuleCallbackDescriptor& Second) -> bool
			{
				return First.Priority > Second.Priority;
			}
		);

		return YkFindDescriptor(Descriptor);
	}
}