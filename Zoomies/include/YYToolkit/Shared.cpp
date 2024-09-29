#include "Shared.hpp"
#include <cassert>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* GetYYTKInterface()
{
	static YYTKInterface* module_interface = nullptr;

	// Try getting the interface
	// If we error, we return nullptr.
	if (!module_interface)
	{
		AurieStatus last_status = ObGetInterface(
			"YYTK_Main",
			reinterpret_cast<AurieInterfaceBase*&>(module_interface)
		);

		if (!AurieSuccess(last_status))
			printf("[%s : %d] FATAL: Failed to get YYTK Interface (%s)!\n", __FILE__, __LINE__, AurieStatusToString(last_status));
	}

	return module_interface;
}

RValue::RValue()
{
	this->m_Real = 0;
	this->m_Flags = 0;
	this->m_Kind = VALUE_UNDEFINED;
}

YYTK::RValue::RValue(
	IN std::initializer_list<RValue> Values
)
{
	// Initialize to undefined
	*this = RValue();

	if (!GetYYTKInterface())
		return;

	if (!GetYYTKInterface()->GetRunnerInterface().YYCreateArray)
		return;

	// Create a dummy array with the size of Values.size(), and initialize all members to 0
	std::vector<double> dummy_array(Values.size(), 0.0);

	// Initialize this RValue as an array
	GetYYTKInterface()->GetRunnerInterface().YYCreateArray(
		this,
		static_cast<int>(dummy_array.size()),
		dummy_array.data()
	);

	// Use direct object manipulation to set the actual values
	for (size_t index = 0; index < Values.size(); index++)
	{
		RValue* member_value = nullptr;
		AurieStatus last_status = GetYYTKInterface()->GetArrayEntry(
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

RValue::RValue(
	IN bool Value
)
{
	this->m_Real = static_cast<double>(Value);
	this->m_Flags = 0;
	this->m_Kind = VALUE_BOOL;
}

RValue::RValue(
	IN double Value
)
{
	this->m_Real = Value;
	this->m_Flags = 0;
	this->m_Kind = VALUE_REAL;
}

RValue::RValue(
	IN int64_t Value
)
{
	this->m_i64 = Value;
	this->m_Flags = 0;
	this->m_Kind = VALUE_INT64;
}

RValue::RValue(
	IN int32_t Value
)
{
	this->m_i32 = Value;
	this->m_Flags = 0;
	this->m_Kind = VALUE_INT32;
}

RValue::RValue(
	IN CInstance* Object
)
{
	this->m_Object = Object;
	this->m_Flags = 0;
	this->m_Kind = VALUE_OBJECT;
}

YYTK::RValue::RValue(
	IN const char* Value
)
{
	// Init to empty
	*this = std::string_view(Value);
}

#if YYTK_CPP_VERSION > 202002L
YYTK::RValue::RValue(
	IN const char8_t* Value
)
{
	*this = std::u8string_view(Value);
}
#endif // YYTK_CPP_VERSION

RValue::RValue(
	IN std::string_view Value
)
{
	// Initialize it to just empty stuff
	*this = RValue();

	// Let's not crash on invalid interfaces provided
	if (!GetYYTKInterface())
		return;

	// We can ignore this, because if it fails, we're just initialized to UNSET
	GetYYTKInterface()->StringToRValue(
		Value,
		*this
	);
}

#if YYTK_CPP_VERSION > 202002L
YYTK::RValue::RValue(
	IN std::u8string_view Value
)
{
	*this = std::string(Value.cbegin(), Value.cend());
}
#endif // YYTK_CPP_VERSION

YYTK::RValue::RValue(
	IN const std::string& Value
)
{
	*this = std::string_view(Value);
}

#if YYTK_CPP_VERSION > 202002L
YYTK::RValue::RValue(
	IN const std::u8string& Value
)
{
	*this = std::u8string_view(Value);
}
#endif // YYTK_CPP_VERSION

RValue::RValue(
	IN std::string_view Value,
	IN class YYTKInterface* Interface
)
{
	// Initialize it to just empty stuff
	*this = RValue();

	// Let's not crash on invalid interfaces provided
	if (!Interface)
		return;

	// We can ignore this, because if it fails, we're just initialized to UNSET
	Interface->StringToRValue(
		Value,
		*this
	);
}

bool RValue::AsBool() const
{
	switch (this->m_Kind)
	{
	case VALUE_REAL:
	case VALUE_BOOL:
		return this->m_Real > 0.5;
	case VALUE_PTR:
	case VALUE_OBJECT:
		return this->m_Pointer != nullptr;
	case VALUE_UNDEFINED:
		return false;
	case VALUE_INT32:
	case VALUE_REF:
		return this->m_i32 > 0;
	case VALUE_INT64:
		return this->m_i64 > 0;
	default:
		GetYYTKInterface()->PrintError(
			__FILE__,
			__LINE__,
			"Trying to get boolean value of invalid kind '%u'!",
			this->m_Kind
		);
	}

	return false;
}

double RValue::AsReal() const
{
	switch (this->m_Kind)
	{
	case VALUE_REAL:
	case VALUE_BOOL:
		return this->m_Real;
	case VALUE_INT32:
	case VALUE_REF:
		return static_cast<double>(this->m_i32);
	case VALUE_INT64:
		return static_cast<double>(this->m_i64);
	default:
		GetYYTKInterface()->PrintError(
			__FILE__,
			__LINE__,
			"Trying to get real value of invalid kind '%u'!",
			this->m_Kind
		);
	}

	return 0.0;
}

std::string_view RValue::AsString()
{
	// Let's not crash on invalid interfaces provided
	if (!GetYYTKInterface())
		return "";

	if (!GetYYTKInterface()->GetRunnerInterface().YYGetString)
		return "";

	// Reason I don't use RValueToString is because that duplicates the string
	return GetYYTKInterface()->GetRunnerInterface().YYGetString(this, 0);
}

std::string_view RValue::AsString(
	IN YYTKInterface* Interface
)
{
	// Let's not crash on invalid interfaces provided
	if (!Interface)
		return "";

	if (!Interface->GetRunnerInterface().YYGetString)
		return "";

	return Interface->GetRunnerInterface().YYGetString(this, 0);
}

RValue& RValue::operator[](
	IN size_t Index
)
{
	if (!GetYYTKInterface())
		return *this;

	RValue* result = nullptr;
	AurieStatus last_status = GetYYTKInterface()->GetArrayEntry(
		*this,
		Index,
		result
	);

	if (!AurieSuccess(last_status))
	{
		GetYYTKInterface()->PrintError(
			__FILE__,
			__LINE__,
			"Trying to index invalid array index '%lld' (%s)!",
			Index,
			AurieStatusToString(last_status)
		);

		return *this;
	}

	return *result;
}

RValue& RValue::operator[](
	IN std::string_view Element
)
{
	if (!GetYYTKInterface())
		return *this;

	RValue* instance_member = nullptr;
	AurieStatus last_status = GetYYTKInterface()->GetInstanceMember(
		*this,
		Element.data(),
		instance_member
	);

	// Prevents access violations, null references are undefined behavior in the C++ standard
	if (!AurieSuccess(last_status) || !instance_member)
	{
		GetYYTKInterface()->PrintError(
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

RValue& RValue::at(
	IN size_t Index
)
{
	return this->operator[](Index);
}

RValue& RValue::at(
	IN std::string_view Element
)
{
	return this->operator[](Element);
}

RValue* YYTK::RValue::data()
{
	if (!GetYYTKInterface())
		return this;
	
	RValue* data_base_address = this;

	// The "data_base_address" variable will remain a thisptr unless the function succeeds,
	// so we don't need to check the return value, as we will always return a valid pointer
	GetYYTKInterface()->GetArrayEntry(
		*this,
		0,
		data_base_address
	);

	return data_base_address;
}

size_t YYTK::RValue::length()
{
	// Non-array RValues always only have 1 element
	if (this->m_Kind != VALUE_ARRAY)
		return 1;

	YYTKInterface* module_interface = GetYYTKInterface();
	if (!module_interface)
		return 0;

	// We don't have to check return value, since if the function failed,
	// the value in current_size is preserved.
	size_t current_size = 0;
	module_interface->GetArraySize(
		*this,
		current_size
	);

	return current_size;
}

#if YYTK_DEFINE_INTERNAL
CInstanceInternal& YYTK::CInstance::GetMembers()
{
	YYTKInterface* module_interface = GetYYTKInterface();

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

	int32_t self_id = static_cast<int32_t>(self_id_builtin.AsReal());
	
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
#endif // YYTK_DEFINE_INTERNAL

RValue& CInstance::operator[](
	IN std::string_view Element
)
{
	return RValue(this).at(Element);
}

RValue& CInstance::at(
	IN std::string_view Element
)
{
	return RValue(this).at(Element);
}

#if YYTK_DEFINE_INTERNAL
bool YYObjectBase::Add(
	IN const char* Name,
	IN const RValue& Value, 
	IN int Flags
)
{
	// Get the module interface
	YYTKInterface* module_interface = GetYYTKInterface();
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
	YYTKInterface* module_interface = GetYYTKInterface();
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
	YYTKInterface* module_interface = GetYYTKInterface();

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