#include "YYRValue.hpp"

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
	this->I32 = Value; // A bool is really just a 0 or a 1, so I can freely cast it to an integer.
}

YYRValue::YYRValue(const char* Value) noexcept(true)
{
	this->Kind = VALUE_STRING;
	this->Flags = 0;
	// TODO: Create the string
}

YYRValue::YYRValue(const std::string& Value) noexcept(true)
{
	this->Kind = VALUE_STRING;
	this->Flags = 0;
	// TODO: Create the string
}

YYRValue::operator double() const noexcept(true)
{
	switch (Kind)
	{
	case VALUE_REAL:
		return Real;
	case VALUE_BOOL: /* Fallthrough */
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
	return "";
}

YYRValue::operator std::string() const noexcept(true)
{
	return "";
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
