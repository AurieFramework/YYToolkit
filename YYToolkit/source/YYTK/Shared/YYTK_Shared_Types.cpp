#include "YYTK_Shared_Types.hpp"
#include "YYTK_Shared_Interface.hpp"

using namespace YYTK;
using namespace Aurie;

double YYTK::RValue::ToDouble() const
{
	return GetInterface()->GetRunnerInterface().REAL_RValue(this);
}

int32_t YYTK::RValue::ToInt32() const
{
	return GetInterface()->GetRunnerInterface().INT32_RValue(this);
}

int64_t YYTK::RValue::ToInt64() const
{
	return GetInterface()->GetRunnerInterface().INT64_RValue(this);
}

bool YYTK::RValue::ToBoolean() const
{
	return GetInterface()->GetRunnerInterface().BOOL_RValue(this);
}

std::string YYTK::RValue::GetKindName() const
{
	return GetInterface()->GetRunnerInterface().KIND_NAME_RValue(this);
}

YYObjectBase* YYTK::RValue::ToObject() const
{
	return ToPointer<YYObjectBase*>();
}

CInstance* YYTK::RValue::ToInstance() const
{
	return ToPointer<CInstance*>();
}

const char* YYTK::RValue::ToCString() const
{
	return GetInterface()->GetRunnerInterface().YYGetString(this, 0);
}

std::string YYTK::RValue::ToString() const
{
	return ToCString();
}

std::u8string YYTK::RValue::ToUTF8String() const
{
	return reinterpret_cast<const char8_t*>(GetInterface()->GetRunnerInterface().YYGetString(this, 0));
}

std::map<std::string, RValue*> YYTK::RValue::ToRefMap()
{
	std::map<std::string, RValue*> result;

	GetInterface()->EnumInstanceMembers(
		*this,
		[&result](IN const char* MemberName, IN OUT RValue* Value) -> bool
		{
			result[MemberName] = Value;
			return false;
		}
	);

	return result;
}

std::map<std::string, RValue> YYTK::RValue::ToMap() const
{
	std::map<std::string, RValue> result;

	GetInterface()->EnumInstanceMembers(
		*this,
		[&result](IN const char* MemberName, IN OUT RValue* Value) -> bool
		{
			result[MemberName] = *Value;
			return false;
		}
	);

	return result;
}

std::vector<RValue*> YYTK::RValue::ToRefVector()
{
	AurieStatus last_status = AURIE_SUCCESS;

	size_t array_size = 0;
	last_status = GetInterface()->GetArraySize(*this, array_size);

	if (!AurieSuccess(last_status))
		return {};

	std::vector<RValue*> result;
	for (size_t i = 0; i < array_size; i++)
	{
		RValue* element = nullptr;

		last_status = GetInterface()->GetArrayEntry(
			*this,
			i,
			element
		);

		if (AurieSuccess(last_status) && element)
			result.push_back(element);
	}

	return result;
}

std::vector<RValue> YYTK::RValue::ToVector() const
{
	// GetArraySize and GetArrayEntry cannot guarantee that our
	// RValue stays intact (due to engine functions being called), 
	// and as such require us to copy the current RValue.
	RValue current_value_copy = *this;

	AurieStatus last_status = AURIE_SUCCESS;

	size_t array_size = 0;
	last_status = GetInterface()->GetArraySize(current_value_copy, array_size);

	if (!AurieSuccess(last_status))
		return {};

	std::vector<RValue> result;
	for (size_t i = 0; i < array_size; i++)
	{
		RValue* element = nullptr;

		last_status = GetInterface()->GetArrayEntry(
			current_value_copy,
			i,
			element
		);

		if (AurieSuccess(last_status) && element)
			result.push_back(*element);
	}

	return result;
}

RValue* YYTK::RValue::GetRefMember(
	IN const char* MemberName
)
{
	return this->ToInstance()->GetRefMember(MemberName);
}

RValue* YYTK::RValue::GetRefMember(
	IN const std::string& MemberName
)
{
	return this->ToInstance()->GetRefMember(MemberName);
}

RValue YYTK::RValue::GetMember(
	IN const char* MemberName
) const
{
	return this->ToInstance()->GetMember(MemberName);
}

RValue YYTK::RValue::GetMember(
	IN const std::string& MemberName
) const
{
	return this->ToInstance()->GetMember(MemberName);
}

int32_t YYTK::RValue::GetMemberCount() const
{
	return this->ToInstance()->GetMemberCount();
}

RValue* YYTK::RValue::ToArray()
{
	RValue* array_start = nullptr;

	GetInterface()->GetArrayEntry(
		*this,
		0,
		array_start
	);

	return array_start;
}

void* YYTK::RValue::ToPointer() const
{
	return GetInterface()->GetRunnerInterface().PTR_RValue(this);
}

YYTK::RValue::RValue()
{
	this->m_Real = 0;
	this->m_Flags = 0;
	this->m_Kind = VALUE_UNDEFINED;
}

YYTK::RValue::~RValue()
{
	this->__Free();
}

YYTK::RValue::RValue(
	IN const std::vector<RValue>& Values
)
{
	// Initialize to undefined
	*this = RValue();

	if (!GetInterface()->GetRunnerInterface().YYCreateArray)
		return;

	// Create a dummy array with the size of Values.size(), and initialize all members to 0
	std::vector<double> dummy_array(Values.size(), 0.0);

	// Initialize this RValue as an array
	GetInterface()->GetRunnerInterface().YYCreateArray(
		this,
		static_cast<int>(dummy_array.size()),
		dummy_array.data()
	);

	// Use direct object manipulation to set the actual values
	for (size_t index = 0; index < Values.size(); index++)
	{
		RValue* member_value = nullptr;
		AurieStatus last_status = GetInterface()->GetArrayEntry(
			*this,
			index,
			member_value
		);

		// Make sure we got a valid pointer
		if (!AurieSuccess(last_status))
			continue;

		*member_value = std::data(Values)[index];
	}
}

YYTK::RValue::RValue(
	IN void* Pointer
)
{
	*this = RValue();
	this->m_Kind = VALUE_PTR;
	this->m_Pointer = Pointer;
}

RValue::RValue(
	IN std::string_view Value
)
{
	// Initialize it to just empty stuff
	*this = RValue();

	// We can ignore this, because if it fails, we're just initialized to UNSET
	GetInterface()->StringToRValue(
		Value,
		*this
	);
}

YYTK::RValue::RValue(
	IN std::u8string_view Value
)
{
	// Initialize it to just empty stuff
	*this = RValue();

	// We can ignore this, because if it fails, we're just initialized to UNSET
	GetInterface()->StringToRValue(
		reinterpret_cast<const char*>(Value.data()),
		*this
	);
}

YYTK::RValue::RValue(
	IN const char* Value
)
{
	*this = RValue(std::string_view(Value));
}

YYTK::RValue::RValue(
	IN const char8_t* Value
)
{
	*this = RValue(std::u8string(Value));
}

YYTK::RValue::RValue(
	IN bool Value
)
{
	this->m_Real = static_cast<double>(Value);
	this->m_Flags = 0;
	this->m_Kind = VALUE_BOOL;
}

YYTK::RValue::RValue(
	IN const RValue& Other
)
{
	*this = RValue();

	GetInterface()->GetRunnerInterface().COPY_RValue(
		this,
		&Other
	);
}

RValue& YYTK::RValue::operator=(
	IN const RValue& Other
	)
{
	this->__Free();

	GetInterface()->GetRunnerInterface().COPY_RValue(
		this,
		&Other
	);

	return *this;
}

YYTK::RValue::RValue(
	IN const std::map<std::string, RValue>& Values
)
{
	// Initialize this RValue to unset.
	*this = RValue();

	// Create an empty struct here.
	GetInterface()->GetRunnerInterface().StructCreate(
		this
	);

	for (auto [key, value] : Values)
	{
		// "value" gets copied by StructAddRValue.
		GetInterface()->GetRunnerInterface().StructAddRValue(
			this,
			key.c_str(),
			&value
		);
	}
}

RValue& YYTK::RValue::operator[](
	IN size_t Index
	)
{
	return *this->ToRefVector().at(Index);
}

RValue YYTK::RValue::operator[](
	IN size_t Index
	) const
{
	return this->ToVector().at(Index);
}

RValue& RValue::operator[](
	IN std::string_view Element
	)
{
	if (!GetInterface())
		return *this;

	RValue* instance_member = nullptr;
	AurieStatus last_status = GetInterface()->GetInstanceMember(
		*this,
		Element.data(),
		instance_member
	);

	// Prevents access violations, null references are undefined behavior in the C++ standard
	if (!AurieSuccess(last_status) || !instance_member)
	{
		GetInterface()->PrintError(
			__FILE__,
			__LINE__,
			"Trying to access inaccessible instance member '%s' (%s)!",
			Element.data(),
			AurieStatusToString(last_status)
		);

		return *this;
	}

	return *instance_member;
}

const RValue& YYTK::RValue::operator[](
	IN std::string_view MemberName
	) const
{
	if (!GetInterface())
		return *this;

	RValue* instance_member = nullptr;
	AurieStatus last_status = GetInterface()->GetInstanceMember(
		*this,
		MemberName.data(),
		instance_member
	);

	// Prevents access violations, null references are undefined behavior in the C++ standard
	if (!AurieSuccess(last_status) || !instance_member)
	{
		GetInterface()->PrintError(
			__FILE__,
			__LINE__,
			"Trying to access inaccessible instance member '%s' (%s)!",
			MemberName.data(),
			AurieStatusToString(last_status)
		);

		return *this;
	}

	return *instance_member;
}

bool YYTK::RValue::ContainsValue(
	IN std::string_view MemberName
) const
{
	RValue* member = this->ToInstance()->GetRefMember(
		MemberName.data()
	);

	return member != nullptr;
}

YYTK::RValue::operator bool()
{
	return this->ToBoolean();
}

YYTK::RValue::operator double()
{
	return this->ToDouble();
}

YYTK::RValue::operator std::string()
{
	return this->ToString();
}

YYTK::RValue::operator std::u8string()
{
	return this->ToUTF8String();
}

YYTK::RValue::operator int32_t()
{
	return this->ToInt32();
}

YYTK::RValue::operator int64_t()
{
	return this->ToInt64();
}

void YYTK::RValue::__Free()
{
	GetInterface()->GetRunnerInterface().FREE_RValue(this);

	this->m_i64 = 0;
	this->m_Flags = 0;
	this->m_Kind = VALUE_UNDEFINED;
}

#if YYTK_DEFINE_INTERNAL
CInstanceInternal& YYTK::CInstance::GetMembers()
{
	YYTKInterface* module_interface = GetInterface();

	// SequenceInstanceOnly is used in most new games that v3 is targetting
	if (!module_interface)
		return this->SequenceInstanceOnly.Members;

	RValue self_id_builtin;
	module_interface->GetBuiltin(
		"id",
		this,
		NULL_INDEX,
		self_id_builtin
	);

	int32_t self_id = self_id_builtin.ToInt32();

	if (this->MembersOnly.Members.m_ID == self_id)
		return this->MembersOnly.Members;

	if (this->SequenceInstanceOnly.Members.m_ID == self_id)
		return this->SequenceInstanceOnly.Members;

	if (this->WithSkeletonMask.Members.m_ID == self_id)
		return this->WithSkeletonMask.Members;

	module_interface->PrintError(
		__FILE__,
		__LINE__,
		"Failed to determine CInstance member offset! Report this to GitHub and include the game name!"
	);

	return this->SequenceInstanceOnly.Members;
}

bool YYObjectBase::Add(
	IN const char* Name,
	IN const RValue& Value,
	IN int Flags
)
{
	// Get the module interface
	YYTKInterface* module_interface = GetInterface();
	if (!module_interface)
		return false;

	// Check if we have the needed function
	if (!module_interface->GetRunnerInterface().COPY_RValue)
		return false;

	// Get the slot ID - this calls FindAlloc_Slot_From_Name
	int32_t variable_hash = 0;
	if (!AurieSuccess(module_interface->GetVariableSlot(this, Name, variable_hash)))
		return false;

	// Get the RValue reference
	RValue& rv = this->InternalGetYYVarRef(variable_hash);

	// Copy the RValue from our stuff into the struct
	module_interface->GetRunnerInterface().COPY_RValue(&rv, &Value);
	rv.m_Flags = Flags; // Make the behavior consistent with the actual func

	return true;
}

bool YYObjectBase::IsExtensible()
{
	return this->m_Flags & 1;
}

RValue* YYObjectBase::FindOrAllocValue(
	IN const char* Name
)
{
	// Get the interface
	YYTKInterface* module_interface = GetInterface();
	if (!module_interface)
		return nullptr;

	// Get the slot ID - this calls FindAlloc_Slot_From_Name
	int32_t variable_hash = 0;
	if (!AurieSuccess(module_interface->GetVariableSlot(this, Name, variable_hash)))
		return nullptr;

	return &this->InternalGetYYVarRef(variable_hash);
}

CRoomInternal& YYTK::CRoom::GetMembers()
{
	YYTKInterface* module_interface = GetInterface();

	// Return the more likely thing.
	if (!module_interface)
		return this->WithBackgrounds.Internals;

	size_t bg_color_idx = 0;
	AurieStatus last_status = module_interface->GetBuiltinVariableIndex(
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
		return *reinterpret_cast<CRoomInternal*>(&this->m_Color);
	}

	return this->WithBackgrounds.Internals;
}

#endif // YYTK_DEFINE_INTERNAL

RValue YYTK::CInstance::ToRValue() const
{
	return RValue(this);
}

RValue* YYTK::CInstance::GetRefMember(
	IN const char* MemberName
)
{
	RValue* member_value = nullptr;

	AurieStatus last_status = AURIE_SUCCESS;
	last_status = GetInterface()->GetInstanceMember(
		this,
		MemberName,
		member_value
	);

	if (!AurieSuccess(last_status))
	{
		GetInterface()->GetRunnerInterface().YYError(
			"[YYToolkit Code Error]\r\n"
			"Trying to fetch invalid member '%s' from instance!\r\n"
			"Status code returned: %s",
			MemberName,
			AurieStatusToString(last_status)
		);

		return nullptr;
	}

	return member_value;
}

RValue* YYTK::CInstance::GetRefMember(
	IN const std::string& MemberName
)
{
	RValue* member_value = nullptr;

	AurieStatus last_status = AURIE_SUCCESS;
	last_status = GetInterface()->GetInstanceMember(
		this,
		MemberName.c_str(),
		member_value
	);

	if (!AurieSuccess(last_status))
	{
		GetInterface()->GetRunnerInterface().YYError(
			"[YYToolkit Code Error]\r\n"
			"Trying to fetch invalid member '%s' from instance!\r\n"
			"Status code returned: %s",
			MemberName.c_str(),
			AurieStatusToString(last_status)
		);

		return nullptr;
	}

	return member_value;
}

const RValue* YYTK::CInstance::GetRefMember(
	IN const char* MemberName
) const
{
	RValue* member_value = nullptr;

	AurieStatus last_status = AURIE_SUCCESS;
	last_status = GetInterface()->GetInstanceMember(
		this,
		MemberName,
		member_value
	);

	if (!AurieSuccess(last_status))
	{
		GetInterface()->GetRunnerInterface().YYError(
			"[YYToolkit Code Error]\r\n"
			"Trying to fetch invalid member '%s' from instance!\r\n"
			"Status code returned: %s",
			MemberName,
			AurieStatusToString(last_status)
		);

		return nullptr;
	}

	return member_value;
}

const RValue* YYTK::CInstance::GetRefMember(
	IN const std::string& MemberName
) const
{
	RValue* member_value = nullptr;

	AurieStatus last_status = AURIE_SUCCESS;
	last_status = GetInterface()->GetInstanceMember(
		this,
		MemberName.c_str(),
		member_value
	);

	if (!AurieSuccess(last_status))
	{
		GetInterface()->GetRunnerInterface().YYError(
			"[YYToolkit Code Error]\r\n"
			"Trying to fetch invalid member '%s' from instance!\r\n"
			"Status code returned: %s",
			MemberName.c_str(),
			AurieStatusToString(last_status)
		);

		return nullptr;
	}

	return member_value;
}

RValue YYTK::CInstance::GetMember(
	IN const char* MemberName
) const
{
	RValue* member_value = nullptr;

	AurieStatus last_status = AURIE_SUCCESS;
	last_status = GetInterface()->GetInstanceMember(
		this,
		MemberName,
		member_value
	);

	if (!AurieSuccess(last_status))
	{
		GetInterface()->GetRunnerInterface().YYError(
			"[YYToolkit Code Error]\r\n"
			"Trying to fetch invalid member '%s' from instance!\r\n"
			"Status code returned: %s",
			MemberName,
			AurieStatusToString(last_status)
		);

		return RValue();
	}

	return *member_value;
}

RValue YYTK::CInstance::GetMember(
	IN const std::string& MemberName
) const
{
	RValue* member_value = nullptr;

	AurieStatus last_status = AURIE_SUCCESS;
	last_status = GetInterface()->GetInstanceMember(
		this,
		MemberName.c_str(),
		member_value
	);

	if (!AurieSuccess(last_status))
	{
		GetInterface()->GetRunnerInterface().YYError(
			"[YYToolkit Code Error]\r\n"
			"Trying to fetch invalid member '%s' from instance!\r\n"
			"Status code returned: %s",
			MemberName.c_str(),
			AurieStatusToString(last_status)
		);

		return RValue();
	}

	return *member_value;
}

int32_t YYTK::CInstance::GetMemberCount() const
{
	int32_t member_count = 0;

	// member_count will not be modified if the function fails.
	GetInterface()->GetInstanceMemberCount(
		this,
		member_count
	);

	return member_count;
}

CInstance* YYTK::CInstance::FromInstanceID(
	IN int32_t InstanceID
)
{
	CInstance* buffer = nullptr;
	AurieStatus last_status = AURIE_SUCCESS;

	last_status = GetInterface()->GetInstanceObject(
		InstanceID,
		buffer
	);

	if (!AurieSuccess(last_status))
		return nullptr;

	return buffer;
}
