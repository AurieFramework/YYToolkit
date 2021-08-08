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
	YYRValue() noexcept(true);

	// YYRValue V = 30.0;
	YYRValue(const double& Value) noexcept(true);

	// YYRValue V = 30.0f;
	YYRValue(const float& Value) noexcept(true);

	// YYRValue V = true;
	YYRValue(const bool& Value) noexcept(true);

	// YYRValue V = "Hello, World!";
	YYRValue(const char* Value) noexcept(true);

	// YYRValue V = std::string("Hello, std::string!");
	YYRValue(const std::string& Value) noexcept(true);

	operator double() const noexcept(true);

	operator float() const noexcept(true);

	operator bool() const noexcept(true);

	operator const char* () const noexcept(true);

	operator std::string() const noexcept(true);

	YYRValue& operator +=(const double& Value);

	YYRValue& operator -=(const double& Value);

	YYRValue& operator *=(const double& Value);

	YYRValue& operator /=(const double& Value);
};