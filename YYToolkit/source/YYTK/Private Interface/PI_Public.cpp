#include "PI.hpp"
using namespace Aurie;

namespace YYTK
{
	double YYTKPrivateInterfaceImpl::RV_ToDouble(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();
		
		return g_ModuleInterface.GetRunnerInterface().REAL_RValue(Value);
	}

	int32_t YYTKPrivateInterfaceImpl::RV_ToInt32(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		return g_ModuleInterface.GetRunnerInterface().INT32_RValue(Value);
	}

	int64_t YYTKPrivateInterfaceImpl::RV_ToInt64(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		return g_ModuleInterface.GetRunnerInterface().INT64_RValue(Value);

	}

	PVOID YYTKPrivateInterfaceImpl::RV_ToPointer(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		return g_ModuleInterface.GetRunnerInterface().PTR_RValue(Value);
	}

	bool YYTKPrivateInterfaceImpl::RV_ToBoolean(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		return g_ModuleInterface.GetRunnerInterface().BOOL_RValue(Value);
	}

	const char* YYTKPrivateInterfaceImpl::RV_GetKindName(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		return g_ModuleInterface.GetRunnerInterface().KIND_NAME_RValue(Value);
	}

	YYObjectBase* YYTKPrivateInterfaceImpl::RV_ToObject(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		return static_cast<YYObjectBase*>(RV_ToPointer(Value));
	}

	CInstance* YYTKPrivateInterfaceImpl::RV_ToInstance(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		return static_cast<CInstance*>(RV_ToPointer(Value));
	}

	const char* YYTKPrivateInterfaceImpl::RV_ToCString(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		return g_ModuleInterface.GetRunnerInterface().YYGetString(Value, 0);
	}

	std::string YYTKPrivateInterfaceImpl::RV_ToString(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		return RV_ToCString(Value);
	}

	std::u8string YYTKPrivateInterfaceImpl::RV_ToU8String(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		return reinterpret_cast<const char8_t*>(RV_ToCString(Value));
	}

	std::map<std::string, RValue> YYTKPrivateInterfaceImpl::RV_ToMap(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		std::map<std::string, RValue> result;

		g_ModuleInterface.EnumInstanceMembers(
			*Value,
			[&result](IN const char* MemberName, IN OUT RValue* Value) -> bool
			{
				result[MemberName] = *Value;
				return false;
			}
		);

		return result;
	}

	std::map<std::string, RValue*> YYTKPrivateInterfaceImpl::RV_ToRefMap(
		IN RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		std::map<std::string, RValue*> result;

		g_ModuleInterface.EnumInstanceMembers(
			*Value,
			[&result](IN const char* MemberName, IN OUT RValue* Value) -> bool
			{
				result[MemberName] = Value;
				return false;
			}
		);

		return result;
	}

	std::vector<RValue> YYTKPrivateInterfaceImpl::RV_ToVector(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		// GetArraySize and GetArrayEntry cannot guarantee that our
		// RValue stays intact (due to engine functions being called), 
		// and as such require us to copy the current RValue.
		RValue current_value_copy = *Value;

		AurieStatus last_status = AURIE_SUCCESS;

		size_t array_size = 0;
		last_status = g_ModuleInterface.GetArraySize(current_value_copy, array_size);

		if (!AurieSuccess(last_status))
			return {};

		std::vector<RValue> result;
		for (size_t i = 0; i < array_size; i++)
		{
			RValue* element = nullptr;

			last_status = g_ModuleInterface.GetArrayEntry(
				current_value_copy,
				i,
				element
			);

			if (AurieSuccess(last_status) && element)
				result.push_back(*element);
		}

		return result;
	}

	std::vector<RValue*> YYTKPrivateInterfaceImpl::RV_ToRefVector(
		IN RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		AurieStatus last_status = AURIE_SUCCESS;

		size_t array_size = 0;
		last_status = g_ModuleInterface.GetArraySize(*Value, array_size);

		if (!AurieSuccess(last_status))
			return {};

		std::vector<RValue*> result;
		for (size_t i = 0; i < array_size; i++)
		{
			RValue* element = nullptr;

			last_status = g_ModuleInterface.GetArrayEntry(
				*Value,
				i,
				element
			);

			if (AurieSuccess(last_status) && element)
				result.push_back(element);
		}

		return result;
	}

	int32_t YYTKPrivateInterfaceImpl::RV_GetMemberCount(
		IN const RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		int32_t member_count = 0;

		// member_count will not be modified if the function fails.
		g_ModuleInterface.GetInstanceMemberCount(
			Value,
			member_count
		);

		return member_count;
	}

	RValue* YYTKPrivateInterfaceImpl::RV_ToCArray(
		IN RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		RValue* array_start = nullptr;

		g_ModuleInterface.GetArrayEntry(
			*Value,
			0,
			array_start
		);

		return array_start;
	}

	RValue YYTKPrivateInterfaceImpl::RV_IndexByNumber(
		IN const RValue* Value,
		IN size_t Index
	)
	{
		YkWaitForLowerLevelInit();

		// GetArrayEntry cannot guarantee that our RValue will not be modified.
		RValue value_copy = *Value;
		
		RValue* element = nullptr;
		AurieStatus last_status = g_ModuleInterface.GetArrayEntry(
			value_copy,
			Index,
			element
		);

		if (AurieSuccess(last_status))
			return *element;

		return {};
	}

	RValue* YYTKPrivateInterfaceImpl::RV_IndexByNumberRef(
		IN RValue* Value, 
		IN size_t Index
	)
	{
		YkWaitForLowerLevelInit();

		RValue* element = nullptr;
		g_ModuleInterface.GetArrayEntry(
			*Value,
			Index,
			element
		);

		return element;
	}

	RValue YYTKPrivateInterfaceImpl::RV_IndexByName(
		IN const RValue* Value,
		IN std::string_view Index
	)
	{
		YkWaitForLowerLevelInit();

		RValue* instance_member = nullptr;
		AurieStatus last_status = g_ModuleInterface.GetInstanceMember(
			*Value,
			Index.data(),
			instance_member
		);

		// Prevents access violations, null references are undefined behavior in the C++ standard
		if (!AurieSuccess(last_status) || !instance_member)
		{
			DbgPrintEx(
				LOG_SEVERITY_ERROR,
				"Trying to access inaccessible instance member '%s' (%s)!",
				Index.data(),
				AurieStatusToString(last_status)
			);

			return Value;
		}

		return *instance_member;
	}

	RValue* YYTKPrivateInterfaceImpl::RV_IndexByNameRef(
		IN RValue* Value,
		IN std::string_view Index
	)
	{
		YkWaitForLowerLevelInit();

		RValue* instance_member = nullptr;
		AurieStatus last_status = g_ModuleInterface.GetInstanceMember(
			*Value,
			Index.data(),
			instance_member
		);

		// Prevents access violations, null references are undefined behavior in the C++ standard
		if (!AurieSuccess(last_status) || !instance_member)
		{
			DbgPrintEx(
				LOG_SEVERITY_ERROR,
				"Trying to access inaccessible instance member '%s' (%s)!",
				Index.data(),
				AurieStatusToString(last_status)
			);

			return Value;
		}

		return instance_member;
	}

	bool YYTKPrivateInterfaceImpl::RV_IsUndefined(
		IN const RValue* Value
	)
	{
		return Value->m_Kind == VALUE_UNDEFINED;
	}

	bool YYTKPrivateInterfaceImpl::RV_IsUnset(
		IN const RValue* Value
	)
	{
		return Value->m_Kind == VALUE_UNSET;
	}

	bool YYTKPrivateInterfaceImpl::RV_IsStruct(
		IN const RValue* Value
	)
	{
		return Value->m_Kind == VALUE_OBJECT;
	}

	bool YYTKPrivateInterfaceImpl::RV_IsNumberCompatible(
		IN const RValue* Value
	)
	{
		return Value->m_Kind == VALUE_REAL || Value->m_Kind == VALUE_INT32 || Value->m_Kind == VALUE_INT64 || Value->m_Kind == VALUE_BOOL;
	}

	bool YYTKPrivateInterfaceImpl::RV_IsString(
		IN const RValue* Value
	)
	{
		return Value->m_Kind == VALUE_STRING;
	}

	bool YYTKPrivateInterfaceImpl::RV_IsArray(
		IN const RValue* Value
	)
	{
		return Value->m_Kind == VALUE_ARRAY;
	}

	void YYTKPrivateInterfaceImpl::RV_CreateEmpty(
		IN RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		Value->m_Real = 0;
		Value->m_Flags = 0;
		Value->m_Kind = VALUE_UNDEFINED;
	}

	void YYTKPrivateInterfaceImpl::RV_CreateFromDouble(
		IN RValue* Value, 
		IN double Contents
	)
	{
		YkWaitForLowerLevelInit();

		RV_CreateEmpty(Value);
		Value->m_Real = Contents;
		Value->m_Kind = VALUE_REAL;
	}

	void YYTKPrivateInterfaceImpl::RV_CreateFromInteger(
		IN RValue* Value, 
		IN int64_t Contents
	)
	{
		YkWaitForLowerLevelInit();

		RV_CreateEmpty(Value);
		Value->m_i64 = Contents;
		Value->m_Kind = VALUE_INT64;
	}

	void YYTKPrivateInterfaceImpl::RV_CreateFromPointer(
		IN RValue* Value, 
		IN void* Contents
	)
	{
		YkWaitForLowerLevelInit();

		RV_CreateEmpty(Value);
		Value->m_Kind = VALUE_PTR;
		Value->m_Pointer = Contents;
	}

	void YYTKPrivateInterfaceImpl::RV_CreateFromObjectPointer(
		IN RValue* Value,
		IN void* Contents
	)
	{
		YkWaitForLowerLevelInit();

		RV_CreateEmpty(Value);

		Value->m_Kind = VALUE_OBJECT;
		Value->m_Pointer = Contents;
	}

	void YYTKPrivateInterfaceImpl::RV_CreateFromVector(
		IN RValue* Value, 
		IN const std::vector<RValue>& Contents
	)
	{
		YkWaitForLowerLevelInit();

		RV_CreateEmpty(Value);

		// Create a dummy array with the size of Values.size(), and initialize all members to 0
		std::vector<double> dummy_array(Contents.size(), 0.0);

		// Initialize this RValue as an array
		g_ModuleInterface.GetRunnerInterface().YYCreateArray(
			Value,
			static_cast<int>(dummy_array.size()),
			dummy_array.data()
		);

		// Use direct object manipulation to set the actual values
		for (size_t index = 0; index < Contents.size(); index++)
		{
			RValue* member_value = nullptr;
			AurieStatus last_status = g_ModuleInterface.GetArrayEntry(
				*Value,
				index,
				member_value
			);

			// Make sure we got a valid pointer
			if (!AurieSuccess(last_status))
				continue;

			*member_value = Contents[index];
		}
	}

	void YYTKPrivateInterfaceImpl::RV_CreateFromAnsiString(
		IN RValue* Value,
		IN const std::string_view Contents
	)
	{
		YkWaitForLowerLevelInit();

		RV_CreateEmpty(Value);
		g_ModuleInterface.StringToRValue(
			Contents,
			*Value
		);
	}

	void YYTKPrivateInterfaceImpl::RV_CreateFromU8String(
		IN RValue* Value, 
		IN const std::u8string_view Contents
	)
	{
		YkWaitForLowerLevelInit();

		RV_CreateEmpty(Value);
		g_ModuleInterface.StringToRValue(
			reinterpret_cast<const char*>(Contents.data()),
			*Value
		);
	}

	void YYTKPrivateInterfaceImpl::RV_CreateFromBoolean(
		IN RValue* Value, 
		IN bool Contents
	)
	{
		YkWaitForLowerLevelInit();
		Value->m_Real = static_cast<double>(Contents);
		Value->m_Flags = 0;
		Value->m_Kind = VALUE_BOOL;

	}

	void YYTKPrivateInterfaceImpl::RV_CreateFromMap(
		IN RValue* Value, 
		IN const std::map<std::string, RValue>& Contents
	)
	{
		YkWaitForLowerLevelInit();

		// Initialize this RValue to unset.
		RV_CreateEmpty(Value);

		// Create an empty struct here.
		g_ModuleInterface.GetRunnerInterface().StructCreate(
			Value
		);

		for (auto [key, value] : Contents)
		{
			// "value" gets copied by StructAddRValue.
			g_ModuleInterface.GetRunnerInterface().StructAddRValue(
				Value,
				key.c_str(),
				&value
			);
		}
	}

	void YYTKPrivateInterfaceImpl::RV_Copy(
		IN RValue* Destination, 
		IN const RValue* Source
	)
	{
		YkWaitForLowerLevelInit();

		g_ModuleInterface.GetRunnerInterface().COPY_RValue(
			Destination,
			Source
		);
	}

	void YYTKPrivateInterfaceImpl::RV_Free(
		IN RValue* Value
	)
	{
		YkWaitForLowerLevelInit();

		g_ModuleInterface.GetRunnerInterface().FREE_RValue(
			Value
		);

		RV_CreateEmpty(Value);
	}

	const char* YYTKPrivateInterfaceImpl::CCode_GetName(
		IN const CCode* Object
	)
	{
		YkWaitForLowerLevelInit();

		return Object->m_Name;
	}

	const char* YYTKPrivateInterfaceImpl::CScript_GetName(
		IN const CScript* Object
	)
	{
		YkWaitForLowerLevelInit();

		return Object->m_Name;
	}

	CRoomInternal* YYTKPrivateInterfaceImpl::CRoom_GetInternalData(
		IN CRoom* Object
	)
	{
		YkWaitForLowerLevelInit();

		size_t bg_color_idx = 0;
		AurieStatus last_status = g_ModuleInterface.GetBuiltinVariableIndex(
			"background_color",
			bg_color_idx
		);

		// This lookup will fail in newer runners where backgrounds were removed
		if (!AurieSuccess(last_status))
		{
			// Note: We have to craft the pointer manually here, since
			// bool alignment prevents us from just having a struct (it'd get aligned to sizeof(PVOID)).

			// Don't ask why it's from m_Color and not from m_ShowColor, it doesn't make sense
			// and I can't figure out why it works - it just does.
			return reinterpret_cast<CRoomInternal*>(&Object->m_Color);
		}

		return &Object->WithBackgrounds.Internals;
	}

	bool YYTKPrivateInterfaceImpl::YYObjectBase_Add(
		IN YYObjectBase* Object,
		IN const char* Name, 
		IN const RValue& Value,
		IN int Flags
	)
	{
		UNREFERENCED_PARAMETER(Flags);
		YkWaitForLowerLevelInit();

		// Get the slot ID - this calls FindAlloc_Slot_From_Name
		int32_t variable_hash = 0;
		if (!AurieSuccess(g_ModuleInterface.GetVariableSlot(Object, Name, variable_hash)))
			return false;

		// Get the RValue reference
		RValue& rv = Object->InternalGetYYVarRef(variable_hash);

		// Copy the RValue from our stuff into the struct
		RV_Copy(&rv, &Value);

		return true;
	}

	RValue* YYTKPrivateInterfaceImpl::YYObjectBase_FindOrAllocateValue(
		IN YYObjectBase* Object, 
		IN const char* Name
	)
	{
		YkWaitForLowerLevelInit();

		// Get the slot ID - this calls FindAlloc_Slot_From_Name
		int32_t variable_hash = 0;
		if (!AurieSuccess(g_ModuleInterface.GetVariableSlot(Object, Name, variable_hash)))
			return nullptr;

		return &Object->InternalGetYYVarRef(variable_hash);
	}

	CInstanceInternal* YYTKPrivateInterfaceImpl::CInstance_GetInternalData(
		IN CInstance* Instance
	)
	{
		YkWaitForLowerLevelInit();

		RValue self_id_builtin;
		g_ModuleInterface.GetBuiltin(
			"id",
			Instance,
			NULL_INDEX,
			self_id_builtin
		);

		int32_t self_id = self_id_builtin.ToInt32();

		if (Instance->MembersOnly.Members.m_ID == self_id)
			return &Instance->MembersOnly.Members;

		if (Instance->SequenceInstanceOnly.Members.m_ID == self_id)
			return &Instance->SequenceInstanceOnly.Members;

		if (Instance->WithSkeletonMask.Members.m_ID == self_id)
			return &Instance->WithSkeletonMask.Members;

		DbgPrintEx(
			LOG_SEVERITY_ERROR,
			"Failed to determine CInstance member offset! Report this to GitHub and include the game name!"
		);

		return &Instance->SequenceInstanceOnly.Members;
	}

	CInstance* YYTKPrivateInterfaceImpl::CInstance_FromID(
		IN int32_t InstanceID
	)
	{
		YkWaitForLowerLevelInit();

		CInstance* buffer = nullptr;
		AurieStatus last_status = AURIE_SUCCESS;

		last_status = g_ModuleInterface.GetInstanceObject(
			InstanceID,
			buffer
		);

		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(
				LOG_SEVERITY_ERROR,
				"Cannot find CInstance for ID %d!",
				InstanceID
			);
		}

		return buffer;
	}
}
