#include "Shared.hpp"
#include <cassert>
using namespace Aurie;
using namespace YYTK;

RValue::RValue()
{
	this->m_Real = 0;
	this->m_Flags = 0;
	this->m_Kind = VALUE_UNSET;
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

RValue::RValue(
	IN std::string_view Value,
	IN YYTKInterface* Interface
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
