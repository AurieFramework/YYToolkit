#include "YYRValue.hpp"
#include "../CDynamicArray/CDynamicArray.hpp"
#include "../RefThing/RefThing.hpp"

YYRValue::YYRValue() noexcept(true)
{
	// Just set it to unset and zero out the whole 8-byte space.
	// Check it on https://godbolt.org/, it's true!
	this->Kind = VALUE_UNSET;
	this->Flags = 0;
	this->Real = 0.0;
}

YYRValue::YYRValue(const double& Value) noexcept(true)
{
	this->Kind = VALUE_REAL;
	this->Flags = 0;
	this->Real = Value;
}

YYRValue::YYRValue(const float& Value) noexcept(true)
{
	this->Kind = VALUE_REAL;
	this->Flags = 0;
	this->Real = static_cast<double>(Value);
}

YYRValue::YYRValue(const bool& Value) noexcept(true)
{
	this->Kind = VALUE_BOOL;
	this->Flags = 0;
	this->Real = static_cast<double>(Value); // A bool is really just a 0 or a 1, so I can freely cast it to an integer.
}

YYRValue::YYRValue(const long long& Value) noexcept(true)
{
	this->Kind = VALUE_INT64;
	this->Flags = 0;
	this->I64 = Value;
}

YYRValue::YYRValue(const char* Value) noexcept(true)
{
	this->Kind = VALUE_STRING;
	this->Flags = 0;
	this->String = RefString::Alloc(Value, strlen(Value) + 1);
}

YYRValue::YYRValue(const std::string& Value) noexcept(true)
{
	this->Kind = VALUE_STRING;
	this->Flags = 0;
	this->String = RefString::Alloc(Value.c_str(), Value.length() + 1);
}

YYRValue::YYRValue(const YYRValue& Value) noexcept(true)
{
	this->Kind = Value.Kind;
	this->Flags = 0;

	switch (Kind)
	{
	case VALUE_REAL: /* Fallthrough */
	case VALUE_BOOL:
		this->Real = Value.Real;
		break;
	case VALUE_PTR: /* Fallthrough */
	case VALUE_OBJECT:
		this->Object = Value.Object;
		break;
	case VALUE_INT32:
	case VALUE_INT64:
		this->I64 = Value.I64;
		break;
	case VALUE_ARRAY:
		this->EmbeddedArray = CDynamicArrayRef<RValue>::Assign(Value.EmbeddedArray);
		break;
	case VALUE_STRING:
		this->String = RefString::Assign(Value.String);
		break;
	default:
		memcpy(this, &Value, sizeof(double)); // Copy the entire union and be done with it.
	}
}

YYRValue::YYRValue(const RValue& Value) noexcept(true)
{
	const YYRValue* _val = reinterpret_cast<const YYRValue*>(&Value);
	*this = YYRValue(_val);
}

YYRValue::operator int() const noexcept(true)
{
	return static_cast<int>(operator double());
}

YYRValue::operator double() const noexcept(true)
{
	switch (Kind)
	{
	case VALUE_REAL:
	case VALUE_BOOL: /* Fallthrough */
		return Real;
	case VALUE_INT32:
	case VALUE_INT64:
		return static_cast<double>(I64);
	default:
		return 0.0;
	}
}

YYRValue::operator float() const noexcept(true)
{
	return static_cast<float>(operator double());
}

YYRValue::operator bool() const noexcept(true)
{
	return operator double() > 0.5;
}

YYRValue::operator const char* () const noexcept(true)
{
	if (Kind == VALUE_STRING)
	{
		if (String)
		{
			return String->Get();
		}
	}

	return nullptr;
}

YYRValue::operator std::string() const noexcept(true)
{
	if (Kind == VALUE_STRING)
	{
		if (String)
		{
			const char* pString = String->Get();
			
			if (pString)
				return std::string(pString);
		}
	}

	return "";
}

YYRValue::operator RefString* () const noexcept(true)
{
	if (Kind == VALUE_STRING)
		return String;
	// else
	return nullptr;
}

YYRValue::operator YYObjectBase* () const noexcept(true)
{
	if (Kind == VALUE_OBJECT)
		return Object;

	return nullptr;
}

YYRValue& YYRValue::operator+=(const double& Value)
{
	switch (Kind)
	{
	case VALUE_REAL: /* Fallthrough */
	case VALUE_INT32:
	case VALUE_INT64:
	case VALUE_BOOL:
		*this = static_cast<double>(*this); // Convert this YYRValue to a value holding a double
		this->Real += Value;
		break; // No throwing today!
	default:
		throw "Trying to add to an unsupported type!";
	}

	return *this;
}

YYRValue& YYRValue::operator-=(const double& Value)
{
	switch (Kind)
	{
	case VALUE_REAL: /* Fallthrough */
	case VALUE_INT32:
	case VALUE_INT64:
	case VALUE_BOOL:
		*this = static_cast<double>(*this); // Convert this YYRValue to a value holding a double
		this->Real -= Value;
		break; // No throwing today!
	default:
		throw "Trying to subtract from an unsupported type!";
	}

	return *this;
}

YYRValue& YYRValue::operator*=(const double& Value)
{
	switch (Kind)
	{
	case VALUE_REAL: /* Fallthrough */
	case VALUE_INT32:
	case VALUE_INT64:
	case VALUE_BOOL:
		*this = static_cast<double>(*this); // Convert this YYRValue to a value holding a double
		this->Real *= Value;
		break; // No throwing today!
	default:
		throw "Trying to multiply an unsupported type!";
	}

	return *this;
}

YYRValue& YYRValue::operator/=(const double& Value)
{
	switch (Kind)
	{
	case VALUE_REAL: /* Fallthrough */
	case VALUE_INT32:
	case VALUE_INT64:
	case VALUE_BOOL:
		*this = static_cast<double>(*this); // Convert this YYRValue to a value holding a double
		this->Real /= Value;
		break; // No throwing today!
	default:
		throw "Trying to divide an unsupported type!";
	}

	return *this;
}