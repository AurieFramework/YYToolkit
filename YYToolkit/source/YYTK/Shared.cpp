#include "Shared.hpp"
using namespace Aurie;

YYTK::RValue::RValue()
{
	this->m_Real = 0;
	this->m_Flags = 0;
	this->m_Kind = VALUE_UNSET;
}

YYTK::RValue::RValue(
	IN double Value
)
{
	this->m_Real = Value;
	this->m_Flags = 0;
	this->m_Kind = VALUE_REAL;
}

YYTK::RValue::RValue(
	IN int64_t Value
)
{
	this->m_i64 = Value;
	this->m_Flags = 0;
	this->m_Kind = VALUE_INT64;
}

YYTK::RValue::RValue(
	IN int32_t Value
)
{
	this->m_i32 = Value;
	this->m_Flags = 0;
	this->m_Kind = VALUE_INT32;
}

YYTK::RValue::RValue(
	IN CInstance* Object
)
{
	this->m_Object = Object;
	this->m_Flags = 0;
	this->m_Kind = VALUE_OBJECT;
}

YYTK::RValue::RValue(
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
