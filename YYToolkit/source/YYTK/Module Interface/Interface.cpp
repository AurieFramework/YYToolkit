#include "Interface.hpp"

namespace YYTK
{

	void YYTKInterfaceImpl::InternalExtractFunctionEntry(
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

	size_t YYTKInterfaceImpl::InternalDetermineFunctionEntrySize()
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
			last_status = YYTK::GmpGetRunnerInterface(
				m_RunnerInterface
			);

			// If we didn't get that, there's no chance in hell we're doing anything with the runner
			if (!AurieSuccess(last_status))
				return Aurie::AURIE_MODULE_INTERNAL_ERROR;

			// Use the runner interface to find the (currently empty) the_functions array in memory
			last_status = YYTK::GmpFindFunctionsArray(
				m_RunnerInterface,
				&m_FunctionsArray
			);

			// Make sure we got that, otherwise we still can't do anything
			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			YYTK::CmWriteOutput(CM_LIGHTBLUE, "Welcome to YYTK Next!");
			YYTK::CmWriteOutput(CM_LIGHTBLUE, "m_Functions at %p!", m_FunctionsArray);

			m_FirstInitComplete = true;
			return AURIE_SUCCESS;
		}
		
		if (!m_SecondInitComplete)
		{
			// First up, we need to determine the RFunction entry size 
			// now that it's populated by the game.
			m_FunctionEntrySize = this->InternalDetermineFunctionEntrySize();

			TRoutine copy_static = nullptr;
			last_status = this->GetNamedRoutinePointer(
				"@@CopyStatic@@",
				&copy_static
			);

			if (!AurieSuccess(last_status))
				return Aurie::AURIE_MODULE_INTERNAL_ERROR;

			last_status = YYTK::GmpFindScriptData(
				m_RunnerInterface,
				copy_static,
				&m_GetScriptData
			);

			if (!AurieSuccess(last_status))
				return Aurie::AURIE_MODULE_INTERNAL_ERROR;

			m_SecondInitComplete = true;
			return AURIE_SUCCESS;
		}

		// Simple stub if neither condition is met
		return Aurie::AURIE_SUCCESS;
	}

	void YYTKInterfaceImpl::Destroy()
	{

	}

	void YYTKInterfaceImpl::QueryVersion(
		OUT short& Major, 
		OUT short& Minor, 
		OUT short& Patch
	)
	{
		Major = 0;
		Minor = 0;
		Patch = 0;
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
		OUT TRoutine* FunctionPointer
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

		// Values greater or equal to 100k are reserved for scripts.
		// Values greater or equal to 500k are reserved for extension functions.
		// Until we can deal with those, just deny access.
		if (function_index >= 100000)
			return AURIE_ACCESS_DENIED;

		// Make sure we got one
		if (!AurieSuccess(last_status))
			return last_status;

		// Previous check should've tripped if the value is -1
		assert(function_index > 0);

		std::string function_name;
		int32_t function_argument_count = 0;
		TRoutine function_routine = nullptr;

		this->InternalExtractFunctionEntry(
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
		if (!m_RunnerInterface.YYGetPtr)
			return AURIE_MODULE_INTERNAL_ERROR;

		if (!Instance)
			return AURIE_INVALID_PARAMETER;

		RValue global_scope = CallBuiltin("@@GlobalScope@@", {});
		*Instance = reinterpret_cast<CInstance*>(m_RunnerInterface.YYGetPtr(&global_scope, 0));

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
		if (m_FunctionCache.contains(FunctionName))
		{
			m_FunctionCache.at(FunctionName)(
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
			&function
		);

		// Make sure we found a function
		if (!AurieSuccess(last_status))
			return last_status;

		// Previous check should've fired
		assert(function != nullptr);

		// Cache the result
		m_FunctionCache.insert(
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
		IN const char* Format, 
		IN ...
	)
	{

	}

	void YYTKInterfaceImpl::PrintInfo(
		IN const char* Format, 
		IN ...
	)
	{

	}

	void YYTKInterfaceImpl::PrintWarning(
		IN const char* Format, 
		IN ...
	)
	{

	}

	void YYTKInterfaceImpl::PrintError(
		IN const char* Function, 
		IN const int Line, 
		IN const char* Format, 
		IN ...
	)
	{

	}

	AurieStatus YYTKInterfaceImpl::CreateCallback(
		IN const Aurie::AurieModule* Module, 
		IN EventTriggers Trigger, 
		IN PVOID Routine
	)
	{
		return AurieStatus();
	}

	AurieStatus YYTKInterfaceImpl::RemoveCallback(
		IN const Aurie::AurieModule* Module, 
		IN PVOID Routine
	)
	{
		return AurieStatus();
	}

	AurieStatus YYTKInterfaceImpl::InvalidateAllCaches()
	{
		m_FunctionCache.clear();
		return AURIE_SUCCESS;
	}
}