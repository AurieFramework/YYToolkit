#include "Interface.hpp"
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
			FunctionName = function_entry.m_Name;
			FunctionRoutine = function_entry.m_Routine;
			ArgumentCount = function_entry.m_ArgumentCount;
		}
		else if (m_FunctionEntrySize == sizeof(RFunctionStringFull))
		{
			RFunctionStringFull& function_entry = functions_array->GetIndexFull(Index);
			
			// Properly null-terminate the string
			char string_buffer[70] = { 0 };
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
			this->PrintError(
				__FILE__,
				__LINE__,
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
			{ os_info_ds_map, RValue("video_d3d11_device", this) }
		);

		// This is not checking the return value of ds_map_find_value,
		// instead checking if we even called the function successfully.
		if (!AurieSuccess(last_status))
		{
			this->PrintError(
				__FILE__,
				__LINE__,
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
			{ os_info_ds_map, RValue("video_d3d11_swapchain", this) }
		);

		// This is not checking the return value of ds_map_find_value,
		// instead checking if we even called the function successfully.
		if (!AurieSuccess(last_status))
		{
			this->PrintError(
				__FILE__,
				__LINE__,
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

	size_t YYTKInterfaceImpl::YkDetermineFunctionEntrySize()
	{
		if (!m_FunctionsArray)
			return 0;

		RFunction* first_entry = *m_FunctionsArray;
		if (!first_entry)
			return 0;

		// This is either an ASCII string that I just interpreted as a pointer,
		// or it's a real pointer into somewhere in the main executable's .rdata section.
		// If it's the latter, we know sizeof(RFunction) == 24.
		const char* potential_reference = first_entry->ReferentialEntry.m_Name;
		if (!potential_reference)
			return 0;

		AurieStatus last_status = AURIE_SUCCESS;

		// Get the offset and size of the .rdata section
		uint64_t rdata_offset = 0;
		size_t rdata_size = 0;
		last_status = Internal::PpiGetModuleSectionBounds(
			GetModuleHandleA(nullptr),
			".rdata",
			rdata_offset,
			rdata_size
		);

		if (!AurieSuccess(last_status))
			return 0;

		// Get the offset and size of the .text section
		uint64_t text_offset = 0;
		size_t text_size = 0;
		last_status = Internal::PpiGetModuleSectionBounds(
			GetModuleHandleA(nullptr),
			".text",
			text_offset,
			text_size
		);

		if (!AurieSuccess(last_status))
			return 0;

		// The section base is returned relative to the module base
		text_offset += reinterpret_cast<uint64_t>(GetModuleHandleA(nullptr));
		rdata_offset += reinterpret_cast<uint64_t>(GetModuleHandleA(nullptr));

		char* rdata_section_start = reinterpret_cast<char*>(rdata_offset);
		char* rdata_section_end = reinterpret_cast<char*>(rdata_offset + rdata_size);

		// The string should be somewhere in the .rdata section
		// If it's not, it's definitely not RFunctionStringRef (or the structure is corrupt)
		if (potential_reference >= rdata_section_start && potential_reference <= rdata_section_end)
			return sizeof(RFunctionStringRef);

		char* text_section_start = reinterpret_cast<char*>(text_offset);
		char* text_section_end = reinterpret_cast<char*>(text_offset + text_size);
		char* routine = reinterpret_cast<char*>(first_entry->FullEntry.m_Routine);

		// The routine should be somewhere in the .text section
		// If it's not, it's definitely not RFunctionStringFull (or the structure is corrupt)
		
		if (routine >= text_section_start && routine <= text_section_end)
			return sizeof(RFunctionStringFull);

		// Unknown size - possibly wrong runner architecture?
		return 0;
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

	AurieStatus YYTKInterfaceImpl::Create()
	{
		AurieStatus last_status = AURIE_SUCCESS;

		// The first time this method is called, it's called by the Aurie Framework
		// when the interface is created via ObCreateInterface.
		// During this stage, the process entrypoint is still suspended, which
		// means the internal game structures aren't initialized yet. 
		//
		// The second time this method is called, it's called in YYTK's own module operation callback.
		// During this second call, GameMaker internal structures are already initialized,
		// and we're therefore able to use the newly set-up structures for our gain.

		if (!m_FirstInitComplete)
		{
			// Create the runner interface event
			m_RunnerInterfacePopulatedEvent = CreateEventA(
				nullptr,
				TRUE,
				FALSE,
				nullptr
			);

			// Get the runner interface by reading assembly
			last_status = GmpGetRunnerInterface(
				m_RunnerInterface
			);

			// If we didn't get that, there's no chance in hell we're doing anything with the runner...
			// Until v3.4, where a new method is introduced!
			if (!AurieSuccess(last_status))
			{
				PVOID rip = nullptr;
				last_status = GmpBreakpointInterfaceCreation(
					&rip,
					GmpHandleInterfaceCreationBP
				);

				if (!AurieSuccess(last_status) || !rip)
				{
					this->PrintError(
						__FILE__,
						__LINE__,
						"Failed to find runner interface! (%s)",
						AurieStatusToString(last_status)
					);

					return last_status;
				}
				else
				{
					m_IsUsingVeh = true;
					this->Print(
						CM_LIGHTAQUA,
						"Game code analysis finished. Breakpoint set on RIP 0x%p.",
						rip
					);
				}
			}

			last_status = Hooks::HkPreinitialize();

			if (!AurieSuccess(last_status))
			{
				this->PrintError(
					__FILE__,
					__LINE__,
					"Failed to initialize stage 1 hooks! (%s)",
					AurieStatusToString(last_status)
				);

				return last_status;
			}

			CmWriteOutput(CM_LIGHTAQUA, "YYTK Next - Early initialization complete.");

			m_FirstInitComplete = true;
			return AURIE_SUCCESS;
		}
		
		if (!m_SecondInitComplete)
		{
			// If we're using VEH, we have to wait until the runner interface is populated
			// by the exception handler.
			// We do this by using the event, which is signaled by the exception handler.
			if (m_IsUsingVeh)
			{
				this->Print(
					CM_LIGHTAQUA,
					"Please wait while the game creates a runner interface."
				);

				WaitForSingleObject(
					m_RunnerInterfacePopulatedEvent,
					INFINITE
				);

				this->Print(
					CM_LIGHTAQUA,
					"Runner interface created, proceeding with initialization."
				);
			}

			// Now we have to figure out if the runner is YYC or VM
			// We can either do that by calling "code_is_compiled",
			// or by checking if the YYC-only version of GmpFindFunctionsArray succeeds.

			// First we assume the runner is YYC, and look for the functions array
			last_status = YYC::GmpFindFunctionsArray(
				m_RunnerInterface,
				&m_FunctionsArray
			);

			// Before calling anything, we need to know the size of one RFunction entry
			// This might actually fail if the game is VM, so we can check for that too.
			m_FunctionEntrySize = this->YkDetermineFunctionEntrySize();

			// If we failed either getting the functions array, or determining the size,
			// the game is probably VM, and our initial YYC assumption is wrong.
			if (!AurieSuccess(last_status) || m_FunctionEntrySize == 0)
			{
				// Try to find the functions array again, this time using the VM method
				last_status = VM::GmpFindFunctionsArray(
					m_RunnerInterface,
					&m_FunctionsArray
				);

				// Determine the function entry size again
				m_FunctionEntrySize = this->YkDetermineFunctionEntrySize();

				// Check if we succeeded this time (VM)
				if (!AurieSuccess(last_status) || !m_FunctionEntrySize)
				{
					this->PrintError(
						__FILE__,
						__LINE__,
						"Failed to determine function array size! (%s)",
						AurieStatusToString(last_status)
					);

					return last_status;
				}
			}

			// Determine for sure if the runner is YYC
			RValue is_runner_yyc;
			last_status = this->CallBuiltinEx(
				is_runner_yyc,
				"code_is_compiled",
				nullptr,
				nullptr,
				{}
			);

			// Make sure we succeeded with that call
			if (!AurieSuccess(last_status))
			{
				this->PrintError(
					__FILE__,
					__LINE__,
					"Failed to determine runner edition! (%s)",
					AurieStatusToString(last_status)
				);

				return last_status;
			}

			// Set the flag for use later (right after this actually)
			if (is_runner_yyc.AsBool())
				m_IsYYCRunner = true;

			// Get the array of builtins
			// Also yes, now we need to put this if-else bullshit everywhere
			if (m_IsYYCRunner)
			{
				last_status = YYC::GmpGetBuiltinInformation(
					this->m_BuiltinCount,
					this->m_BuiltinArray
				);
			}
			else
			{
				last_status = VM::GmpGetBuiltinInformation(
					this->m_BuiltinCount,
					this->m_BuiltinArray
				);
			}

			// Make sure we got that. While this isn't critical to YYTK,
			// it makes mod development way easier.
			if (!AurieSuccess(last_status))
			{
				this->PrintError(
					__FILE__,
					__LINE__,
					"Failed to get built-in variable information! (%s)",
					AurieStatusToString(last_status)
				);

				return last_status;
			}

			TRoutine array_equals = nullptr;
			last_status = this->GetNamedRoutinePointer(
				"array_equals",
				reinterpret_cast<PVOID*>(&array_equals)
			);

			if (!AurieSuccess(last_status))
			{
				this->PrintError(
					__FILE__,
					__LINE__,
					"Failed to find array_equals! (%s)",
					AurieStatusToString(last_status)
				);

				return last_status;
			}

			// Find the offset required for direct array access (DynamicArrayOfRValue->m_Array)
			if (m_IsYYCRunner)
			{
				last_status = YYC::GmpFindRVArrayOffset(
					array_equals,
					&m_RValueArrayOffset
				);
			}
			else
			{
				last_status = VM::GmpFindRVArrayOffset(
					array_equals,
					m_RunnerInterface,
					&m_RValueArrayOffset
				);
			}

			if (!AurieSuccess(last_status))
			{
				this->PrintWarning(
					"Failed to find RValue array offset! (%s)",
					AurieStatusToString(last_status)
				);
			}

			// Find the Script_Data function
			TRoutine copy_static = nullptr;
			last_status = this->GetNamedRoutinePointer(
				"@@CopyStatic@@",
				reinterpret_cast<PVOID*>(&copy_static)
			);

			if (!AurieSuccess(last_status))
			{
				this->PrintError(
					__FILE__,
					__LINE__,
					"Failed to find @@CopyStatic@@ function! (%s)",
					AurieStatusToString(last_status)
				);

				return last_status;
			}

			last_status = GmpFindScriptData(
				m_RunnerInterface,
				copy_static,
				&m_GetScriptData
			);

			if (!AurieSuccess(last_status))
			{
				this->PrintError(
					__FILE__,
					__LINE__,
					"Failed to find script data! (%s)",
					AurieStatusToString(last_status)
				);

				return last_status;
			}

			// Find the Room_Data function
			TRoutine room_instance_clear = nullptr;
			last_status = this->GetNamedRoutinePointer(
				"room_instance_clear",
				reinterpret_cast<PVOID*>(&room_instance_clear)
			);

			if (!AurieSuccess(last_status))
			{
				this->PrintError(
					__FILE__,
					__LINE__,
					"Failed to find room_instance_clear function! (%s)",
					AurieStatusToString(last_status)
				);

				return last_status;
			}

			if (m_IsYYCRunner)
			{
				last_status = YYC::GmpFindRoomData(
					room_instance_clear,
					&m_GetRoomData
				);
			}
			else
			{
				last_status = VM::GmpFindRoomData(
					room_instance_clear,
					&m_GetRoomData
				);
			}

			if (!AurieSuccess(last_status))
			{
				this->PrintWarning(
					"Failed to find room data! (%s)",
					AurieStatusToString(last_status)
				);
			}

			// Find the Run_Room pointer
			size_t builtin_variable_index = 0;
			last_status = GetBuiltinVariableIndex(
				"background_color",
				builtin_variable_index
			);

			if (!AurieSuccess(last_status))
			{
				// We failed to get background_color (it seems to not exist in Fields of Mistria 2024.6 for example)
				last_status = GetBuiltinVariableIndex(
					"room_width",
					builtin_variable_index
				);

				// If even room_width fails, we bail.
				if (!AurieSuccess(last_status))
				{
					this->PrintError(
						__FILE__,
						__LINE__,
						"Failed to find built-in variables (background_color & room_width)! (%s)",
						AurieStatusToString(last_status)
					);

					return last_status;
				}
			}

			RVariableRoutine* builtin_variable_information = nullptr;
			last_status = GetBuiltinVariableInformation(
				builtin_variable_index,
				builtin_variable_information
			);

			if (!AurieSuccess(last_status))
			{
				this->PrintError(
					__FILE__,
					__LINE__,
					"Failed to get built-in variable information! (%s)",
					AurieStatusToString(last_status)
				);

				return last_status;
			}

			last_status = GmpFindCurrentRoomData(
				builtin_variable_information->m_SetVariable,
				&m_RunRoom
			);

			if (!AurieSuccess(last_status))
			{
				this->PrintError(
					__FILE__,
					__LINE__,
					"Failed to find current room data! (%s)",
					AurieStatusToString(last_status)
				);

				return last_status;
			}

			// Find window handle
			RValue window_handle;

			last_status = CallBuiltinEx(
				window_handle,
				"window_handle",
				nullptr,
				nullptr,
				{ }
			);

			if (!AurieSuccess(last_status))
			{
				this->PrintError(
					__FILE__,
					__LINE__,
					"Failed to get window_handle! (%s)",
					AurieStatusToString(last_status)
				);

				return last_status;
			}

			m_WindowHandle = reinterpret_cast<HWND>(window_handle.m_Pointer);

			// Fetch D3D11 info:
			// 
			// There will be always at least one iteration.
			// The maximum wait time is determined by the value of d3d11_try_limit 
			// prior to the loop.
			// 
			// The os_get_info function may be called up to d3d11_try_limit times, 
			// with the wait time (in milliseconds) being equal to d3d11_try_limit * 10.
			int d3d11_try_limit = 300;

			while (d3d11_try_limit > 0)
			{
				last_status = YkFetchD3D11Info(
					nullptr, // unused
					&m_EngineSwapchain
				);


				// If we succeeded, **and got the swapchain** we can simply break out without sleeping
				// Otherwise, wait 10ms and try again.
				if (AurieSuccess(last_status) && m_EngineSwapchain)
					break;

				Sleep(10);

				// Decrement our try counter, as to not get stuck in an infinite
				// loop, if for some reason the pointers never actually get set.
				d3d11_try_limit--;
			}

			// When we get here, either we ran out of our "try limit" or we broke out.
			// If we broke out, AurieSuccess(last_status) will pass.
			if (!AurieSuccess(last_status))
			{
				this->PrintError(
					__FILE__,
					__LINE__,
					"Failed to fetch D3D11 info! (%s)",
					AurieStatusToString(last_status)
				);

				return last_status;
			}

			assert(m_EngineSwapchain != nullptr);

			last_status = Hooks::HkInitialize(
				m_WindowHandle,
				m_EngineSwapchain
			);

			if (!AurieSuccess(last_status))
			{
				this->PrintError(
					__FILE__,
					__LINE__,
					"Failed to initialize stage 2 hooks! (%s)",
					AurieStatusToString(last_status)
				);

				return last_status;
			}

			last_status = GmpGetYYObjectBaseAdd(
				m_RunnerInterface,
				&m_AddToYYObjectBase
			);

			if (!AurieSuccess(last_status))
			{
				this->PrintWarning(
					"Failed to find YYObjectBase::Add! (%s)",
					AurieStatusToString(last_status)
				);

				// return last_status;
			}

			last_status = GmpGetFindAllocSlotFromName(
				m_AddToYYObjectBase,
				&m_FindAllocSlot
			);

			if (!AurieSuccess(last_status))
			{
				this->PrintWarning(
					"Failed to find FindAllocSlot! (%s)",
					AurieStatusToString(last_status)
				);

				// return last_status;
			}

			CmWriteOutput(CM_LIGHTAQUA, "YYTK Next - Late initialization complete.");

			CmWriteOutput(CM_GRAY, "- m_FunctionsArray at 0x%p", m_FunctionsArray);
			CmWriteOutput(CM_GRAY, "- m_BuiltinArray at 0x%p", m_BuiltinArray);
			CmWriteOutput(CM_GRAY, "- m_BuiltinCount at 0x%p", m_BuiltinCount);
			CmWriteOutput(CM_GRAY, "- m_GetScriptData at 0x%p", m_GetScriptData);
			CmWriteOutput(CM_GRAY, "- m_EngineSwapchain at 0x%p", m_EngineSwapchain);
			CmWriteOutput(CM_GRAY, "- m_RValueArrayOffset at 0x%llx", m_RValueArrayOffset);
			CmWriteOutput(CM_GRAY, "- m_GetRoomData at 0x%p", m_GetRoomData);
			CmWriteOutput(CM_GRAY, "- m_RunRoom at 0x%p", m_RunRoom);
			CmWriteOutput(CM_GRAY, "- m_AddToYYObjectBase at 0x%p", m_AddToYYObjectBase);
			CmWriteOutput(CM_GRAY, "- m_FindAllocSlot at 0x%p", m_FindAllocSlot);
			
			CmWriteOutput(
				CM_GRAY,
				"- RFunction Entry Type: %s",
				m_FunctionEntrySize == sizeof(RFunctionStringRef) ? "Referential" : "Embedded"
			);
			CmWriteOutput(
				CM_GRAY,
				"- Runner Edition: %s",
				m_IsYYCRunner ? "YYC" : "VM"
			);

			m_SecondInitComplete = true;
			return AURIE_SUCCESS;
		}

		// Simple stub if neither condition is met
		return Aurie::AURIE_SUCCESS;
	}

	void YYTKInterfaceImpl::Destroy() 
	{
		if (m_RunnerInterfacePopulatedEvent)
			CloseHandle(m_RunnerInterfacePopulatedEvent);

		Hooks::HkUninitialize(
			m_WindowHandle
		);
	}

	void YYTKInterfaceImpl::QueryVersion(
		OUT short& Major, 
		OUT short& Minor, 
		OUT short& Patch
	)
	{
		Major = YYTK_MAJOR;
		Minor = YYTK_MINOR;
		Patch = YYTK_PATCH;
	}

	AurieStatus YYTKInterfaceImpl::GetNamedRoutineIndex(
		IN const char* FunctionName, 
		OUT int* Index
	)
	{
		if (!m_RunnerInterface.Code_Function_Find)
			return AURIE_MODULE_INTERNAL_ERROR;

		if (!Index)
			return AURIE_INVALID_PARAMETER;

		// This function doesn't actually return false on error, because YoYo
		// If it can't find the function, it will just set the index to -1
		int index_intermediary = -1;
		m_RunnerInterface.Code_Function_Find(FunctionName, &index_intermediary);
		
		// See comment above
		if (index_intermediary == -1)
			return AURIE_OBJECT_NOT_FOUND;

		*Index = index_intermediary;
		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::GetNamedRoutinePointer(
		IN const char* FunctionName, 
		OUT PVOID* FunctionPointer
	)
	{
		// Make sure we have what we need
		if (!m_RunnerInterface.Code_Function_Find)
			return AURIE_MODULE_INTERNAL_ERROR;

		if (!m_FunctionsArray)
			return AURIE_MODULE_INTERNAL_ERROR;

		AurieStatus last_status = AURIE_SUCCESS;

		// Get the index for the function
		int function_index = -1;
		last_status = this->GetNamedRoutineIndex(
			FunctionName,
			&function_index
		);

		// Make sure we got one
		if (!AurieSuccess(last_status))
			return last_status;

		// Values greater or equal to 100k are reserved for scripts.
		// Values greater or equal to 500k are reserved for extension functions.
		// Until we can deal with those, just deny access.

		if (function_index >= 100000)
		{
			// If we don't have access to scripts, deny access to both scripts and Extension functions
			// If we do have access to scripts, we deny access only to Extension Functions
			if (function_index >= 500000 || !m_GetScriptData)
			{
				return AURIE_ACCESS_DENIED;
			}
			
			// Get the script
			*FunctionPointer = m_GetScriptData(function_index - 100000);
			assert(*FunctionPointer);

			return AURIE_SUCCESS;
		}

		// Previous check should've tripped if the value is -1
		assert(function_index > 0);

		std::string function_name;
		int32_t function_argument_count = 0;
		TRoutine function_routine = nullptr;

		this->YkExtractFunctionEntry(
			function_index,
			function_name,
			function_routine,
			function_argument_count
		);

		// Something's wrong, hard to say what...
		assert(function_routine != nullptr);

		// Function name should match? Most likely an issue with YYRunnerInterface?
		// Check Extension_PrePrepare in faulty runner
		assert(!_stricmp(function_name.c_str(), FunctionName));

		// Get the pointer to the function from the game array
		*FunctionPointer = function_routine;
		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::GetGlobalInstance(
		OUT CInstance** Instance
	)
	{
		AurieStatus last_status = AURIE_SUCCESS;

		if (!m_RunnerInterface.PTR_RValue)
			return AURIE_MODULE_INTERNAL_ERROR;

		if (!Instance)
			return AURIE_INVALID_PARAMETER;

		RValue global_scope;

		last_status = CallBuiltinEx(
			global_scope, 
			"@@GlobalScope@@",
			nullptr,
			nullptr,
			{}
		);

		if (!AurieSuccess(last_status))
			return last_status;

		*Instance = reinterpret_cast<CInstance*>(m_RunnerInterface.PTR_RValue(&global_scope));

		return AURIE_SUCCESS;
	}

	RValue YYTKInterfaceImpl::CallBuiltin(
		IN const char* FunctionName, 
		IN std::vector<RValue> Arguments
	)
	{
		CInstance* global_instance = nullptr;
		
		if (!AurieSuccess(GetGlobalInstance(&global_instance)))
			return {};

		// Previous check should've tripped
		assert(global_instance != nullptr);

		// Make sure to return an unset RValue if the lookup fails
		RValue result;
		if (!AurieSuccess(CallBuiltinEx(
			result,
			FunctionName,
			global_instance,
			global_instance,
			Arguments
		)))
		{
			return {};
		}

		return result;
	}

	AurieStatus YYTKInterfaceImpl::CallBuiltinEx(
		OUT RValue& Result,
		IN const char* FunctionName,
		IN CInstance* SelfInstance,
		IN CInstance* OtherInstance,
		IN std::vector<RValue> Arguments
	)
	{
		// Use the cached result if possible
		if (m_BuiltinFunctionCache.contains(FunctionName))
		{
			m_BuiltinFunctionCache.at(FunctionName)(
				&Result,
				SelfInstance,
				OtherInstance,
				static_cast<int>(Arguments.size()),
				Arguments.data()
			);

			return AURIE_SUCCESS;
		}

		// We don't have a cached value, so we try to fetch
		TRoutine function = nullptr;
		AurieStatus last_status = AURIE_SUCCESS;

		// Query for the function pointer
		last_status = this->GetNamedRoutinePointer(
			FunctionName,
			reinterpret_cast<PVOID*>(&function)
		);

		// Make sure we found a function
		if (!AurieSuccess(last_status))
			return last_status;

		// Previous check should've fired
		assert(function != nullptr);

		// Cache the result
		m_BuiltinFunctionCache.insert(
			std::make_pair(FunctionName, function)
		);

		function(
			&Result,
			SelfInstance,
			OtherInstance,
			static_cast<int>(Arguments.size()),
			Arguments.data()
		);

		return AURIE_SUCCESS;
	}

	void YYTKInterfaceImpl::Print(
		IN CmColor Color, 
		IN std::string_view Format, 
		IN ...
	)
	{
		// Parse the VA arguments
		va_list va_args;
		va_start(va_args, Format);
		std::string formatted_output = CmpParseVa(Format.data(), va_args);
		va_end(va_args);

		return CmWriteOutput(
			Color,
			formatted_output
		);
	}

	void YYTKInterfaceImpl::PrintInfo(
		IN std::string_view Format,
		IN ...
	)
	{
		// Parse the VA arguments
		va_list va_args;
		va_start(va_args, Format);
		std::string formatted_output = CmpParseVa(Format.data(), va_args);
		va_end(va_args);

		return CmWriteInfo(
			formatted_output
		);
	}

	void YYTKInterfaceImpl::PrintWarning(
		IN std::string_view Format,
		IN ...
	)
	{
		// Parse the VA arguments
		va_list va_args;
		va_start(va_args, Format);
		std::string formatted_output = CmpParseVa(Format.data(), va_args);
		va_end(va_args);

		return CmWriteWarning(
			formatted_output
		);
	}

	void YYTKInterfaceImpl::PrintError(
		IN std::string_view Filepath,
		IN const int Line, 
		IN std::string_view Format,
		IN ...
	)
	{
		// Parse the VA arguments
		va_list va_args;
		va_start(va_args, Format);
		std::string formatted_output = CmpParseVa(Format.data(), va_args);
		va_end(va_args);

		return CmWriteError(
			Filepath,
			Line,
			formatted_output
		);
	}

	AurieStatus YYTKInterfaceImpl::CreateCallback(
		IN AurieModule* Module, 
		IN EventTriggers Trigger, 
		IN PVOID Routine,
		IN int32_t Priority
	)
	{
		if (YkCallbackExists(Module, Routine))
			return AURIE_OBJECT_ALREADY_EXISTS;

		ModuleCallbackDescriptor callback_descriptor = YkCreateCallbackDescriptor(
			Module,
			Trigger,
			Routine,
			Priority
		);

		YkAddToCallbackList(callback_descriptor);

		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::RemoveCallback(
		IN AurieModule* Module, 
		IN PVOID Routine
	)
	{
		if (!YkCallbackExists(Module, Routine))
			return AURIE_OBJECT_NOT_FOUND;

		YkRemoveCallbackFromList(Module, Routine);
		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::GetInstanceMember(
		IN RValue Instance,
		IN const char* MemberName, 
		OUT RValue*& Member
	)
	{
		if (Instance.m_Kind != VALUE_OBJECT)
			return AURIE_INVALID_PARAMETER;

		// If we don't have the interface function, we use a fallback method
		if (!m_RunnerInterface.StructGetMember)
		{
			int32_t variable_hash = 0;
			AurieStatus last_status = AURIE_SUCCESS;

			last_status = this->GetVariableSlot(
				Instance,
				MemberName,
				variable_hash
			);

			if (!AurieSuccess(last_status))
				return last_status;

			Member = &Instance.m_Object->InternalGetYYVarRef(variable_hash);
			return AURIE_SUCCESS;
		}

		RValue* member_value = m_RunnerInterface.StructGetMember(&Instance, MemberName);

		if (!member_value)
			return AURIE_OBJECT_NOT_FOUND;

		Member = member_value;
		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::EnumInstanceMembers(
		IN RValue Instance, 
		IN std::function<bool(IN const char* MemberName, RValue* Value)> EnumFunction
	)
	{
		// Get the variable count in the instance, so we know what size vector to preallocate
		if (Instance.m_Kind != VALUE_OBJECT)
			return AURIE_INVALID_PARAMETER;

		// Determine the number of keys in the struct
		// Use a fallback method if StructGetKeys isn't available in the interface
		int instance_variable_count = -1;
		if (!m_RunnerInterface.StructGetKeys)
		{
			AurieStatus last_status = AURIE_SUCCESS;
			RValue name_count;

			// Ask the engine for the count - this function may not be present
			last_status = CallBuiltinEx(
				name_count,
				"variable_struct_names_count",
				nullptr,
				nullptr,
				{ Instance }
			);

			// Bail if the function isn't present
			if (!AurieSuccess(last_status))
				return last_status;

			instance_variable_count = static_cast<int>(name_count.AsReal());
		}
		else
		{
			// Query the count by passing nullptr for the keys array
			instance_variable_count = m_RunnerInterface.StructGetKeys(
				&Instance,
				nullptr,
				nullptr
			);
		}

		assert(instance_variable_count >= 0);

		// Create a vector with enough space to store all the instance variable names
		std::vector<const char*> instance_variable_names(instance_variable_count);
		
		// Use a fallback if StructGetKeys isn't available
		if (!m_RunnerInterface.StructGetKeys)
		{
			AurieStatus last_status = AURIE_SUCCESS;
			RValue name_array;

			// Ask the engine for the count - this function may not be present
			last_status = CallBuiltinEx(
				name_array,
				"variable_struct_get_names",
				nullptr,
				nullptr,
				{ Instance }
			);

			// Bail if the call failed
			if (!AurieSuccess(last_status))
				return last_status;

			// Loop through all elements of the array
			for (size_t i = 0; i < name_array.length(); i++)
			{
				// Use array_get to not rely on m_RValueArrayOffset
				RValue name;
				last_status = CallBuiltinEx(
					name,
					"array_get",
					nullptr,
					nullptr,
					{ name_array, static_cast<int64_t>(i) }
				);

				// If we can't get the name just bail
				if (!AurieSuccess(last_status))
					return last_status;

				// Assign the name into our vector
				instance_variable_names.at(i) = name.AsString().data();
			}
		}
		else
		{
			m_RunnerInterface.StructGetKeys(
				&Instance,
				instance_variable_names.data(),
				&instance_variable_count
			);
		}

		for (int i = 0; i < instance_variable_count; i++)
		{
			const char* variable_name = instance_variable_names.at(i);

			RValue* member_variable = nullptr;
			AurieStatus last_status = AURIE_SUCCESS;

			// Try to fetch the variable
			last_status = GetInstanceMember(Instance, variable_name, member_variable);

			// If we fail we bail
			if (!AurieSuccess(last_status))
				return last_status;

			// If EnumFunction returns true, we're good to go
			if (EnumFunction(variable_name, member_variable))
				return AURIE_SUCCESS;
		}

		return AURIE_OBJECT_NOT_FOUND;
	}

	AurieStatus YYTKInterfaceImpl::RValueToString(
		IN const RValue& Value,
		OUT std::string& String
	)
	{
		if (!m_RunnerInterface.YYGetString)
			return AURIE_MODULE_INTERNAL_ERROR;

		String = m_RunnerInterface.YYGetString(&Value, 0);
		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::StringToRValue(
		IN const std::string_view String,
		OUT RValue& Value
	)
	{
		if (!m_RunnerInterface.YYCreateString)
			return AURIE_MODULE_INTERNAL_ERROR;

		if (!m_RunnerInterface.YYStrDup)
			return AURIE_MODULE_INTERNAL_ERROR;

		// Duplicate the string using the runner. If you assign the string directly
		// to the RValue, which then goes out of use, the runner will try to free
		// memory that doesn't belong to it, which will cause a segfault.
		const char* duplicated_string = m_RunnerInterface.YYStrDup(String.data());
		
		// At no point should we try to create a null string?
		assert(duplicated_string != nullptr);

		m_RunnerInterface.YYCreateString(&Value, duplicated_string);

		return AURIE_SUCCESS;
	}

	const YYRunnerInterface& YYTKInterfaceImpl::GetRunnerInterface()
	{
		return m_RunnerInterface;
	}

	void YYTKInterfaceImpl::InvalidateAllCaches()
	{
		m_BuiltinFunctionCache.clear();
		m_BuiltinVariableCache.clear();
	}

	AurieStatus YYTKInterfaceImpl::GetScriptData(
		IN int Index, 
		OUT CScript*& Script
	)
	{
		if (!m_GetScriptData)
			return AURIE_MODULE_INTERNAL_ERROR;

		CScript* possible_script = nullptr;
		possible_script = m_GetScriptData(Index);

		// If we didn't retrieve a valid script, that means the index is invalid.
		if (!possible_script)
			return AURIE_OBJECT_NOT_FOUND;

		Script = possible_script;
		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::GetBuiltinVariableIndex(
		IN std::string_view Name, 
		OUT size_t& Index
	)
	{
		if (!m_BuiltinArray || !m_BuiltinCount)
			return AURIE_MODULE_INTERNAL_ERROR;

		// If the entry is already cached, we fetch from there
		if (m_BuiltinVariableCache.contains(Name.data()))
		{
			Index = m_BuiltinVariableCache.at(Name.data());
			return AURIE_SUCCESS;
		}

		// Loop all builtin entries
		for (int i = 0; i < *m_BuiltinCount; i++)
		{
			// If we have a match with the name, we cache the index and return
			if (!strcmp(Name.data(), m_BuiltinArray[i].m_Name))
			{
				m_BuiltinVariableCache[Name.data()] = i;
				Index = i;

				return AURIE_SUCCESS;
			}
		}

		// We didn't return yet? That must mean the name isn't in the array...
		return AURIE_OBJECT_NOT_FOUND;
	}

	AurieStatus YYTKInterfaceImpl::GetBuiltinVariableInformation(
		IN size_t Index,
		OUT RVariableRoutine*& VariableInformation
	)
	{
		if (!m_BuiltinArray || !m_BuiltinCount)
			return AURIE_MODULE_INTERNAL_ERROR;

		// Prevent ourselves from reading out-of-bounds
		if (static_cast<int>(Index) >= *m_BuiltinCount)
			return AURIE_INVALID_PARAMETER;

		VariableInformation = &m_BuiltinArray[Index];
		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::GetBuiltin(
		IN std::string_view Name,
		IN CInstance* TargetInstance,
		OPTIONAL IN int ArrayIndex,
		OUT RValue& Value
	)
	{
		if (!m_BuiltinArray || !m_BuiltinCount)
			return AURIE_MODULE_INTERNAL_ERROR;

		AurieStatus last_status = AURIE_SUCCESS;
		size_t variable_index = 0;

		// Get the index for the builtin variable
		// This function internally uses the builtin variable cache
		last_status = GetBuiltinVariableIndex(Name, variable_index);

		// Make sure we succeeded in that
		if (!AurieSuccess(last_status))
			return last_status;

		// Get the variable information
		RVariableRoutine* variable_information = nullptr;
		last_status = GetBuiltinVariableInformation(
			variable_index,
			variable_information
		);

		if (!AurieSuccess(last_status))
			return last_status;

		// Deny access to variables that can't be read
		if (!variable_information->m_GetVariable)
			return AURIE_ACCESS_DENIED;

		variable_information->m_GetVariable(
			TargetInstance,
			ArrayIndex,
			&Value
		);

		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::SetBuiltin(
		IN std::string_view Name, 
		IN CInstance* TargetInstance,
		OPTIONAL IN int ArrayIndex,
		IN RValue& Value
	)
	{
		if (!m_BuiltinArray || !m_BuiltinCount)
			return AURIE_MODULE_INTERNAL_ERROR;

		AurieStatus last_status = AURIE_SUCCESS;
		size_t variable_index = 0;

		// Get the index for the builtin variable
		// This function internally uses the builtin variable cache
		last_status = GetBuiltinVariableIndex(Name, variable_index);

		// Make sure we succeeded in that
		if (!AurieSuccess(last_status))
			return last_status;

		// Get the variable information
		RVariableRoutine* variable_information = nullptr;
		last_status = GetBuiltinVariableInformation(
			variable_index,
			variable_information
		);

		if (!AurieSuccess(last_status))
			return last_status;

		// We ignore the "can be set" element - only time we can't 
		// set the variable is when a setter literally doesn't exist.
		if (!variable_information->m_SetVariable)
			return AURIE_ACCESS_DENIED;

		variable_information->m_SetVariable(
			TargetInstance,
			ArrayIndex,
			&Value
		);

		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::GetArrayEntry(
		IN RValue& Value, 
		IN size_t ArrayIndex, 
		OUT RValue*& IndexedValue
	)
	{
		if (!m_RValueArrayOffset)
			return AURIE_MODULE_INTERNAL_ERROR;

		// Can't treat values that aren't arrays as arrays
		if (Value.m_Kind != VALUE_ARRAY)
			return AURIE_INVALID_PARAMETER;

		// Check the length of the array to deny out-of-bounds access
		size_t array_length = 0;

		AurieStatus last_status = GetArraySize(
			Value,
			array_length
		);

		// Make sure we got the array length
		if (!AurieSuccess(last_status))
			return last_status;
		
		// Prevent out-of-bounds access
		if (ArrayIndex >= array_length)
			return AURIE_INVALID_PARAMETER;

		RValue* actual_array = *reinterpret_cast<RValue**>(
			&reinterpret_cast<char*>(Value.m_Pointer)[m_RValueArrayOffset]
		);

		IndexedValue = &(actual_array[ArrayIndex]);
		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::GetArraySize(
		IN RValue& Value,
		OUT size_t& Size
	)
	{
		// Can't treat values that aren't arrays as arrays
		if (Value.m_Kind != VALUE_ARRAY)
			return AURIE_INVALID_PARAMETER;

		if (m_RunnerInterface.YYArrayGetLength)
		{
			int possible_array_size = m_RunnerInterface.YYArrayGetLength(&Value);

			// The runner returns -1 on a failure condition
			if (possible_array_size == -1)
				return AURIE_EXTERNAL_ERROR;

			Size = possible_array_size;
			return AURIE_SUCCESS;
		}

		RValue possible_size;
		AurieStatus last_status = AURIE_SUCCESS;

		last_status = CallBuiltinEx(
			possible_size,
			"array_length",
			nullptr,
			nullptr,
			{ Value }
		);

		if (!AurieSuccess(last_status))
			return last_status;

		Size = static_cast<size_t>(possible_size.AsReal());
		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::GetRoomData(
		IN int32_t RoomID,
		OUT CRoom*& Room
	)
	{
		if (!m_GetRoomData)
			return AURIE_MODULE_INTERNAL_ERROR;

		CRoom* potential_room = m_GetRoomData(RoomID);

		// The runner returns nullptr if the room doesn't exist
		if (!potential_room)
			return AURIE_OBJECT_NOT_FOUND;

		Room = potential_room;
		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::GetCurrentRoomData(
		OUT CRoom*& CurrentRoom
	)
	{
		if (!m_RunRoom)
			return AURIE_MODULE_INTERNAL_ERROR;

		CurrentRoom = *m_RunRoom;
		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::GetInstanceObject(
		IN int32_t InstanceID,
		OUT CInstance*& Instance
	)
	{
		/*
			In GameMaker, it is possible to access all instance's data from the ID.
			In GameMaker, it is possible to access all instance's IDs from the instance_id array.

			This function essentially replicates GV_InstanceId, but returns the actual instance.
		*/

		CRoom* current_room = nullptr;
		AurieStatus last_status = AURIE_SUCCESS;

		// Get the current room
		last_status = GetCurrentRoomData(
			current_room
		);

		if (!AurieSuccess(last_status))
			return last_status;

		// Loop all active instances in the room
		for (
			CInstance* inst = current_room->GetMembers().m_ActiveInstances.m_First; 
			inst != nullptr; 
			inst = inst->GetMembers().m_Flink
		)
		{
			// Check if the ID matches our target instance
			if (inst->GetMembers().m_ID != InstanceID)
				continue;
			
			// Return the pointer to it
			Instance = inst;
			return AURIE_SUCCESS;
		}

		return AURIE_OBJECT_NOT_FOUND;
	}

	AurieStatus YYTKInterfaceImpl::InvokeWithObject(
		IN const RValue& Object,
		IN std::function<void(CInstance* Self, CInstance* Other)> Method
	)
	{
		switch (Object.m_Kind)
		{
		case VALUE_STRING:
		{
			// We got an object name most probably
			RValue object_index = CallBuiltin(
				"asset_get_index",
				{ Object }
			);

			int64_t object_count = static_cast<int64_t>(CallBuiltin(
				"instance_number",
				{ object_index }
			).AsReal());

			// Return early if no objects exist
			if (object_count < 1)
				return AURIE_OBJECT_NOT_FOUND;

			for (int64_t i = 0; i < object_count; i++)
			{
				// Find the actual instance
				RValue instance = CallBuiltin(
					"instance_find",
					{
						object_index,
						i
					}
				);

				// If we already got a CInstance* from instance_find, we don't have to pre-process it
				if (instance.m_Kind == VALUE_OBJECT)
				{
					Method(instance.m_Object, instance.m_Object);
					continue;
				}

				// Get the instance ID from the instance
				int32_t instance_id = static_cast<int32_t>(instance.AsReal());

				// Skip inactive instances / instances that don't exist
				CInstance* object_instance = nullptr;
				if (!AurieSuccess(GetInstanceObject(
					instance_id,
					object_instance
				)))
				{
					continue;
				}

				Method(object_instance, object_instance);
			}

			return AURIE_SUCCESS;
		}
		case VALUE_INT32: // fallthrough
		case VALUE_INT64:
		case VALUE_REAL:
		{
			// We got an instance ID
			CInstance* instance = nullptr;
			AurieStatus last_status = GetInstanceObject(static_cast<int32_t>(Object.AsReal()), instance);

			// Return if the instance ID is invalid
			if (!AurieSuccess(last_status))
				return last_status;

			Method(instance, instance);
			
			return AURIE_SUCCESS;
		}
		}

		return AURIE_NOT_IMPLEMENTED;
	}

	AurieStatus YYTKInterfaceImpl::GetVariableSlot(
		IN const RValue& Object,
		IN const char* VariableName,
		OUT int32_t& Hash
	)
	{
		// TODO: Use variable_struct_get_hash if possible
		if (!m_FindAllocSlot)
			return AURIE_MODULE_INTERNAL_ERROR;

		Hash = m_FindAllocSlot(
			Object.m_Object,
			VariableName
		);

		return AURIE_SUCCESS;
	}
}
