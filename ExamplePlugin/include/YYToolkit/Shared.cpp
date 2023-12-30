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
		ObGetInterface(
			"YYTK_Main",
			reinterpret_cast<AurieInterfaceBase*&>(module_interface)
		);
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
		// Bool argument has incorrect type!
		assert(false);
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
		// Real argument has incorrect type!
		assert(false);
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
	if (!AurieSuccess(GetYYTKInterface()->GetArrayEntry(
		*this,
		Index,
		result
	)))
	{
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

RValue& CInstance::operator[](
	IN std::string_view Element
	)
{
	return RValue(this).at(Element);
}

RValue& YYTK::CInstance::at(
	IN std::string_view Element
)
{
	return RValue(this).at(Element);
}