#pragma once
#include "../../../Enums/Enums.hpp"
#include <string>

struct YYObjectBase;
struct RefString;
struct CInstance;

template <typename T>
struct CDynamicArrayRef;
struct RefDynamicArrayOfRValue;
#pragma pack(push, 4)
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
			YYObjectBase* Object;
			CInstance* Instance;
			RefString* String;
			CDynamicArrayRef<RValue>* EmbeddedArray;
			RefDynamicArrayOfRValue* RefArray;
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

	YYRValue(const long long& Value) noexcept(true);

	// YYRValue V = "Hello, World!";
	YYRValue(const char* Value) noexcept(true);

	// YYRValue V = std::string("Hello, std::string!");
	YYRValue(const std::string& Value) noexcept(true);

	// Copy constructor
	YYRValue(const YYRValue& Value) noexcept(true);

	YYRValue(const RValue& Value) noexcept(true);

	// static_cast<int>(V);	
	operator int() const noexcept(true);

	// static_cast<double>(V);
	operator double() const noexcept(true);

	// static_cast<float>(V);
	operator float() const noexcept(true);

	// static_cast<bool>(V);
	operator bool() const noexcept(true);

	// static_cast<const char*>(V);
	operator const char* () const noexcept(true);

	// static_cast<std::string>(V);
	operator std::string() const noexcept(true);

	// static_cast<RefString*>(V);
	operator RefString*() const noexcept(true);

	operator YYObjectBase* () const noexcept(true);

	// V += 30.0;
	YYRValue& operator +=(const double& Value);

	// V -= 30.0;
	YYRValue& operator -=(const double& Value);

	// V *= 30.0;
	YYRValue& operator *=(const double& Value);

	// V /= 30.0;
	YYRValue& operator /=(const double& Value);

	// You really shouldn't use this unless you know what you're doing.
	template <typename T>
	inline T& As() const { return *((T*)this); }
};
#pragma pack(pop)