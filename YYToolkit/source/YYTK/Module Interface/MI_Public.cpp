#include "MI.hpp"
using namespace Aurie;

namespace YYTK
{
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

		*Instance = global_scope.ToInstance();

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
				Result,
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
			Result,
			SelfInstance,
			OtherInstance,
			static_cast<int>(Arguments.size()),
			Arguments.data()
		);

		return AURIE_SUCCESS;
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

		RValue* member_value = nullptr;


		// If StructGetMember is available, use it.
		if (m_RunnerInterface.StructGetMember)
		{
			member_value = m_RunnerInterface.StructGetMember(&Instance, MemberName);
		}

		// If StructGetMember failed or isn't available, we try the other way:
		if (!member_value)
		{
			AurieStatus last_status = AURIE_SUCCESS;

			RValue variable_exists;
			int32_t variable_hash = 0;

			// Call variable_instance_exists to make sure the variable exists.
			// If we don't do this, GetVariableSlot will implicitely create it for us, leading to unexpected behavior.
			last_status = CallBuiltinEx(
				variable_exists,
				"variable_instance_exists",
				nullptr,
				nullptr,
				{ Instance, MemberName }
			);

			// If the method call failed, we can skip it (unsupported runner?)
			if (!AurieSuccess(last_status))
				return last_status;

			// If there variable doesn't exist, we don't wanna implicitely create it. Return AURIE_OBJECT_NOT_FOUND instead.
			if (!variable_exists.ToBoolean())
				return AURIE_OBJECT_NOT_FOUND;

			last_status = this->GetVariableSlot(
				Instance,
				MemberName,
				variable_hash
			);

			// If GetVariableSlot fails (which it only does if the internal function is unavailable)
			// we return the last status.
			if (!AurieSuccess(last_status))
				return last_status;

			// Otherwise we try to get the internal value and write it to InternalGetYYVarRef
			member_value = &Instance.m_Object->InternalGetYYVarRef(variable_hash);
		}

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

			instance_variable_count = name_count.ToInt32();
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

			auto names = name_array.ToRefVector();

			// Each element of instance_variable_names is simply 
			// the string representation of the corresponding
			// element inside names.
			for (auto& name : names)
				instance_variable_names.push_back(name->ToCString());
		}
		else
		{
			m_RunnerInterface.StructGetKeys(
				&Instance,
				instance_variable_names.data(),
				&instance_variable_count
			);
		}

		for (const char* variable_name : instance_variable_names)
		{
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

		m_RunnerInterface.YYCreateString(&Value, String.data());

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

		Size = possible_size.ToInt64();
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

			int64_t object_count = CallBuiltin(
				"instance_number",
				{ object_index }
			).ToInt64();

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
					Method(instance.ToInstance(), instance.ToInstance());
					continue;
				}

				// Get the instance ID from the instance
				int32_t instance_id = instance.ToInt32();

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
			AurieStatus last_status = GetInstanceObject(Object.ToInt32(), instance);

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

	AurieStatus YYTKInterfaceImpl::GetInstanceMemberCount(
		IN RValue Object,
		OUT int32_t& Count
	)
	{
		AurieStatus last_status = AURIE_SUCCESS;

		// We need object RValues
		if (Object.m_Kind != VALUE_OBJECT)
			return AURIE_INVALID_PARAMETER;

		// This function may not be present.
		RValue result;
		last_status = CallBuiltinEx(
			result,
			"variable_instance_names_count",
			nullptr,
			nullptr,
			{ Object }
		);

		if (!AurieSuccess(last_status))
		{
			// variable_instance_names_count isn't present.
			// Try to use StructGetKeys. If that fails, we bail.

			if (!m_RunnerInterface.StructGetKeys)
				return AURIE_UNAVAILABLE;
			
			// StructGetKeys returns the count if the last two args are nullptr.
			Count = m_RunnerInterface.StructGetKeys(
				&Object,
				nullptr,
				nullptr
			);

			return AURIE_SUCCESS;
		}

		Count = result.ToInt32();
		return AURIE_SUCCESS;
	}

	RValue YYTKInterfaceImpl::CallGameScript(
		IN std::string_view ScriptName, 
		IN const std::vector<RValue>& Arguments
	)
	{
		AurieStatus last_status = AURIE_SUCCESS;
		CInstance* global_instance = nullptr;

		// Get the global instance, which is the context we'll use
		// for the call to the script.
		last_status = this->GetGlobalInstance(
			&global_instance
		);

		if (!AurieSuccess(last_status))
			return last_status;

		// Call the script.
		RValue result;
		last_status = this->CallGameScriptEx(
			result,
			ScriptName,
			global_instance,
			global_instance,
			Arguments
		);

		return result;
	}

	AurieStatus YYTKInterfaceImpl::CallGameScriptEx(
		OUT RValue& Result,
		IN std::string_view ScriptName,
		IN CInstance* SelfInstance,
		IN CInstance* OtherInstance,
		IN const std::vector<RValue>& Arguments
	)
	{
		AurieStatus last_status = AURIE_SUCCESS;

		// We need to be safe here, as the caller might pass
		// in a name of a built-in function.
		//
		// Since there is no OBJECT_TYPE-like mechanism in YYToolkit, 
		// we need to check it via the ID. Scripts have an ID > 100'000.
		int function_index = -1;
		last_status = this->GetNamedRoutineIndex(
			ScriptName.data(),
			&function_index
		);

		if (!AurieSuccess(last_status))
			return AURIE_OBJECT_NOT_FOUND;

		// Reject IDs under 100'000 (built-in functions)
		// and IDs that are above or equal to 500'000 (extension functions).
		if (function_index < 100'000 || function_index >= 500'000)
			return AURIE_INVALID_PARAMETER;

		// Get the actual script object
		CScript* script_object = nullptr;
		last_status = this->GetScriptData(
			function_index - 100'000,
			script_object
		);

		// If we failed getting it, we return the status code
		// returned by GetScriptData.
		if (!AurieSuccess(last_status))
			return last_status;

		// Is there a function linked to this script?
		if (!script_object->m_Functions)
			return AURIE_ACCESS_DENIED;

		// Is the function linked to this script valid?
		if (!script_object->m_Functions->m_ScriptFunction)
			return AURIE_ACCESS_DENIED;

		// Create a vector with a pre-allocated array
		std::vector<const RValue*> rvalue_pointers;
		rvalue_pointers.reserve(Arguments.size());

		// Push all the arguments back in there
		for (const auto& arg : Arguments)
			rvalue_pointers.push_back(&arg);

		// Call the actual script
		script_object->m_Functions->m_ScriptFunction(
			SelfInstance,
			OtherInstance,
			Result,
			static_cast<int>(rvalue_pointers.size()),
			const_cast<RValue**>(rvalue_pointers.data())
		);

		return AURIE_SUCCESS;
	}

	bool YYTKInterfaceImpl::IsInstanceOfObject(
		IN const RValue& Instance, 
		IN std::string_view ObjectName
	)
	{
		// Instance should refer to an object
		if (Instance.m_Kind != VALUE_OBJECT)
			return false;

		// Extract the object from the RValue
		YYObjectBase* object = Instance.ToObject();

		// Figure out if the object qualifies as an instance
		if (!object || object->m_ObjectKind != OBJECT_KIND_CINSTANCE)
			return false;

		// If the instance has no parent object
		if (!Instance.ToInstance()->m_Object)
			return false;

		// If the parent object is nameless
		if (!Instance.ToInstance()->m_Object->m_Name)
			return false;

		// Compare the names
		return _stricmp(ObjectName.data(), Instance.ToInstance()->m_Object->m_Name) == 0;
	}

	AurieStatus YYTKInterfaceImpl::GetMethodParameterCount(
		IN std::string_view MethodName,
		OUT int32_t& Count
	)
	{
		int32_t index = 0;
		AurieStatus last_status = AURIE_SUCCESS;

		// Get the index of the method.
		last_status = GetNamedRoutineIndex(MethodName.data(), &index);

		// Make sure we succeeded (ie. method exists)
		if (!AurieSuccess(last_status))
			return last_status;

		// Reserved for scripts.
		if (index >= 100'000)
			return AURIE_ACCESS_DENIED;

		// Extract the argument count.
		std::string function_name; TRoutine function_routine = nullptr;
		YkExtractFunctionEntry(index, function_name, function_routine, Count);

		return AURIE_SUCCESS;
	}
}
