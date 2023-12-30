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

	size_t YYTKInterfaceImpl::YkDetermineFunctionEntrySize()
	{
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

		// The section base is returned relative to the module base
		rdata_offset += reinterpret_cast<uint64_t>(GetModuleHandleA(nullptr));

		char* rdata_section_start = reinterpret_cast<char*>(rdata_offset);
		char* rdata_section_end = reinterpret_cast<char*>(rdata_offset + rdata_size);

		if (potential_reference >= rdata_section_start && potential_reference <= rdata_section_end)
			return sizeof(RFunctionStringRef);

		return sizeof(RFunctionStringFull);
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
			// Get the runner interface by reading assembly
			last_status = GmpGetRunnerInterface(
				m_RunnerInterface
			);

			// If we didn't get that, there's no chance in hell we're doing anything with the runner
			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			// Use the runner interface to find the (currently empty) the_functions array in memory
			last_status = GmpFindFunctionsArray(
				m_RunnerInterface,
				&m_FunctionsArray
			);

			// Make sure we got that, otherwise we still can't do anything
			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			last_status = Hooks::HkPreinitialize();

			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			// Get the array of builtins
			last_status = GmpGetBuiltinInformation(
				this->m_BuiltinCount,
				this->m_BuiltinArray
			);

			// Make sure we got that. While this isn't critical to YYTK,
			// it makes mod development way easier.
			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			CmWriteOutput(CM_LIGHTAQUA, "YYTK Next - Early initialization complete.");
			CmWriteOutput(CM_GRAY, "- m_FunctionsArray at 0x%p", m_FunctionsArray);
			CmWriteOutput(CM_GRAY, "- m_BuiltinCount at 0x%p", m_BuiltinCount);
			CmWriteOutput(CM_GRAY, "- m_BuiltinArray at 0x%p", m_BuiltinArray);

			m_FirstInitComplete = true;
			return AURIE_SUCCESS;
		}
		
		if (!m_SecondInitComplete)
		{
			// First up, we need to determine the RFunction entry size 
			// now that it's populated by the game.
			m_FunctionEntrySize = this->YkDetermineFunctionEntrySize();

			TRoutine array_equals = nullptr;
			last_status = this->GetNamedRoutinePointer(
				"array_equals",
				reinterpret_cast<PVOID*>(&array_equals)
			);

			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			GmpFindRVArrayOffset(
				array_equals,
				&m_RValueArrayOffset
			);

			TRoutine copy_static = nullptr;
			last_status = this->GetNamedRoutinePointer(
				"@@CopyStatic@@",
				reinterpret_cast<PVOID*>(&copy_static)
			);

			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			last_status = GmpFindScriptData(
				m_RunnerInterface,
				copy_static,
				&m_GetScriptData
			);

			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			RValue os_info_ds_map;
			last_status = CallBuiltinEx(
				os_info_ds_map,
				"os_get_info",
				nullptr,
				nullptr,
				{}
			);

			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			// Pull everything needed from the DS List
			// We need to pass the pointer to the interface into the RValue initializer
			// here, because Aurie didn't yet put our interface in its array, therefore
			// the hidden ObGetInterface calls within the RValue would fail.
			RValue dx_device, dx_context, dx_swapchain, window_handle;
			last_status = CallBuiltinEx(
				dx_device,
				"ds_map_find_value",
				nullptr,
				nullptr,
				{ os_info_ds_map, RValue("video_d3d11_device", this) }
			);

			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			last_status = CallBuiltinEx(
				dx_context,
				"ds_map_find_value",
				nullptr,
				nullptr,
				{ os_info_ds_map, RValue("video_d3d11_context", this) }
			);

			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			last_status = CallBuiltinEx(
				dx_swapchain,
				"ds_map_find_value",
				nullptr,
				nullptr,
				{ os_info_ds_map, RValue("video_d3d11_swapchain", this) }
			);

			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			last_status = CallBuiltinEx(
				window_handle,
				"window_handle",
				nullptr,
				nullptr,
				{ }
			);

			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			m_EngineDevice = reinterpret_cast<ID3D11Device*>(dx_device.m_Pointer);
			m_EngineDeviceContext = reinterpret_cast<ID3D11DeviceContext*>(dx_context.m_Pointer);
			m_EngineSwapchain = reinterpret_cast<IDXGISwapChain*>(dx_swapchain.m_Pointer);
			m_WindowHandle = reinterpret_cast<HWND>(window_handle.m_Pointer);

			assert(m_EngineDevice != nullptr);
			assert(m_EngineDeviceContext != nullptr);
			assert(m_EngineSwapchain != nullptr);

			last_status = Hooks::HkInitialize(
				m_WindowHandle,
				m_EngineSwapchain
			);

			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			CmWriteOutput(CM_LIGHTAQUA, "YYTK Next - Late initialization complete.");

			CmWriteOutput(
				CM_GRAY, 
				"- RFunction Entry Type: %s", 
				m_FunctionEntrySize == sizeof(RFunctionStringRef) ? "Referential" : "Embedded"
			);

			CmWriteOutput(CM_GRAY, "- m_GetScriptData at 0x%p", m_GetScriptData);
			CmWriteOutput(CM_GRAY, "- m_EngineDevice at 0x%p", m_EngineDevice);
			CmWriteOutput(CM_GRAY, "- m_EngineDeviceContext at 0x%p", m_EngineDeviceContext);
			CmWriteOutput(CM_GRAY, "- m_EngineSwapchain at 0x%p", m_EngineSwapchain);
			CmWriteOutput(CM_GRAY, "- m_RValueArrayOffset at 0x%llx", m_RValueArrayOffset);

			m_SecondInitComplete = true;
			return AURIE_SUCCESS;
		}

		// Simple stub if neither condition is met
		return Aurie::AURIE_SUCCESS;
	}

	void YYTKInterfaceImpl::Destroy() 
	{
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
		if (!m_RunnerInterface.StructGetMember)
			return AURIE_MODULE_INTERNAL_ERROR;

		if (Instance.m_Kind != VALUE_OBJECT)
			return AURIE_INVALID_PARAMETER;

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

		int instance_variable_count = m_RunnerInterface.StructGetKeys(
			&Instance,
			nullptr,
			nullptr
		);

		assert(instance_variable_count > 0);

		// Get the names of all 
		std::vector<const char*> instance_variable_names(instance_variable_count);
		m_RunnerInterface.StructGetKeys(
			&Instance, 
			instance_variable_names.data(),
			&instance_variable_count
		);

		for (int i = 0; i < instance_variable_count; i++)
		{
			const char* variable_name = instance_variable_names.at(i);

			if (EnumFunction(variable_name, m_RunnerInterface.StructGetMember(&Instance, variable_name)))
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
		for (size_t i = 0; i < *m_BuiltinCount; i++)
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
		if (Index >= *m_BuiltinCount)
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
		size_t array_length = static_cast<size_t>(CallBuiltin("array_length", { Value }).AsReal());
		if (ArrayIndex >= array_length)
			return AURIE_INVALID_PARAMETER;

		RValue* actual_array = *reinterpret_cast<RValue**>(
			&reinterpret_cast<char*>(Value.m_Pointer)[m_RValueArrayOffset]
		);

		IndexedValue = &(actual_array[ArrayIndex]);
		return AURIE_SUCCESS;
	}
}
