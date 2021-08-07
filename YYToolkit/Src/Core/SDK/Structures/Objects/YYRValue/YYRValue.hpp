#pragma once
#include "../../../Enums/Enums.hpp"
#include <string>

// Base class with no overloading, just a pure RValue.
struct RValue
{
	union
	{
		double Real;
		int I32;
		long long I64;

		// Pointers
		union
		{
			struct YYObjectBase* Object;
			struct RefString* String;
			void* Pointer;
		};
	};

	int Flags;
	int Kind;
};

// Struct with overloads, no direct access to variables.
struct YYRValue : protected RValue
{
	// Default constructor
	YYRValue() noexcept(true)
	{
		// Just set it to unset and zero out the whole 8-byte space.
		// Check it on https://godbolt.org/, it's true!
		this->Kind = VALUE_UNSET;
		this->Flags = 0;
		this->Real = 0.0;
	}

	// YYRValue V = 30.0;
	YYRValue(const double& Value) noexcept(true)
	{
		this->Kind = VALUE_REAL;
		this->Flags = 0;
		this->Real = Value;
	}

	// YYRValue V = 30.0f;
	YYRValue(const float& Value) noexcept(true)
	{
		this->Kind = VALUE_REAL;
		this->Flags = 0;
		this->Real = static_cast<double>(Value);
	}

	// YYRValue V = true;
	YYRValue(const bool& Value) noexcept(true)
	{
		this->Kind = VALUE_BOOL;
		this->Flags = 0;
		this->I32 = Value; // A bool is really just a 0 or a 1, so I can freely cast it to an integer.
	}

	// YYRValue V = "Hello, World!";
	YYRValue(const char* Value) noexcept(true)
	{
		this->Kind = VALUE_STRING;
		this->Flags = 0;
		// TODO: Create the string
	}

	// YYRValue V = std::string("Hello, std::string!");
	YYRValue(const std::string& Value) noexcept(true)
	{
		this->Kind = VALUE_STRING;
		this->Flags = 0;
		// TODO: Create the string
	}

	operator double() const noexcept(true)
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

	operator float() const noexcept(true)
	{
		return static_cast<float>(operator double());
	}

	operator bool() const noexcept(true)
	{
		switch (Kind)
		{
		case VALUE_BOOL: /* Fallthrough */
		case VALUE_INT32:
		case VALUE_INT64:
			return static_cast<bool>(I32);
		case VALUE_REAL:
			return Real > 0.5;
		default:
			return false;
		}
	}

	operator const char*() const noexcept(true)
	{
		// TODO: This.
	}

	operator std::string() const noexcept(true)
	{
		// TODO: Also this.
	}

	YYRValue& operator +=(const double& Value)
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

	YYRValue& operator -=(const double& Value)
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
			throw "Trying to add to an unsupported type!";
		}

		return *this;
	}

	YYRValue& operator *=(const double& Value)
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
			throw "Trying to add to an unsupported type!";
		}

		return *this;
	}

	YYRValue& operator /=(const double& Value)
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
			throw "Trying to add to an unsupported type!";
		}

		return *this;
	}
};