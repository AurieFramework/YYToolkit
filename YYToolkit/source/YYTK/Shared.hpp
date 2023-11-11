// File: Shared.hpp
// 
// Defines stuff shared between the tool and its plugins.
// Structs are meant to be opaque here, unless YYTK_INCLUDE_PRIVATE is defined.
// YYTK_INCLUDE_PRIVATE is set to 1 in the main YYTK project through Visual Studio's Project Properties.

#ifndef YYTK_SHARED_H_
#define YYTK_SHARED_H_

#include <cstdint>
#include <string>

#ifndef YYTKAPI
#define YYTKAPI extern "C" __declspec(dllexport)
#endif // YYTKAPI

#ifndef UTEXT
#define UTEXT(x) ((const unsigned char*)(x))
#endif // UTEXT

namespace YYTK
{
	struct IBuffer;
	struct CInstance;
	struct YYObjectBase;

	typedef void* HYYMUTEX;
	typedef void* HSPRITEASYNC;

	struct HTTP_REQ_CONTEXT;

	enum CmColor : uint8_t
	{
		CM_BLACK = 0,
		CM_BLUE,
		CM_GREEN,
		CM_AQUA,
		CM_RED,
		CM_PURPLE,
		CM_YELLOW,
		CM_WHITE,
		CM_GRAY,
		CM_LIGHTBLUE,
		CM_LIGHTGREEN,
		CM_LIGHTAQUA,
		CM_LIGHTRED,
		CM_LIGHTPURPLE,
		CM_LIGHTYELLOW,
		CM_BRIGHTWHITE
	};

	enum RValueType : unsigned int
	{
		VALUE_REAL = 0,				// Real value
		VALUE_STRING,				// String value
		VALUE_ARRAY,				// Array value
		VALUE_PTR,					// Ptr value
		VALUE_VEC3,					// Vec3 (x,y,z) value (within the RValue)
		VALUE_UNDEFINED,			// Undefined value
		VALUE_OBJECT,				// YYObjectBase* value 
		VALUE_INT32,				// Int32 value
		VALUE_VEC4,					// Vec4 (x,y,z,w) value (allocated from pool)
		VALUE_VEC44,				// Vec44 (matrix) value (allocated from pool)
		VALUE_INT64,				// Int64 value
		VALUE_ACCESSOR,				// Actually an accessor
		VALUE_NULL,					// JS Null
		VALUE_BOOL,					// Bool value
		VALUE_ITERATOR,				// JS For-in Iterator
		VALUE_REF,					// Reference value (uses the ptr to point at a RefBase structure)
		VALUE_UNSET = 0x0ffffff		// Unset value (never initialized)
	};

	// A copy of AurieStatus, so plugins don't have to have the Aurie shared headers.
	enum YYTKStatus : uint32_t
	{
		// The operation completed successfully.
		YYTK_SUCCESS = 0,
		// An invalid architecture was specified.
		YYTK_INVALID_ARCH,
		// An error occured in an external function call.
		YYTK_EXTERNAL_ERROR,
		// The requested file was not found.
		YYTK_FILE_NOT_FOUND,
		// The requested access to the object was denied.
		YYTK_ACCESS_DENIED,
		// An object with the same identifier / priority is already registered.
		YYTK_OBJECT_ALREADY_EXISTS,
		// One or more parameters were invalid.
		YYTK_INVALID_PARAMETER,
		// Insufficient memory is available.
		YYTK_INSUFFICIENT_MEMORY,
		// An invalid signature was detected.
		YYTK_INVALID_SIGNATURE,
		// The requested operation is not implemented.
		YYTK_NOT_IMPLEMENTED,
		// An internal error occured in the module.
		YYTK_MODULE_INTERNAL_ERROR,
		// The module failed to resolve dependencies.
		YYTK_MODULE_DEPENDENCY_NOT_RESOLVED,
		// The module failed to initialize.
		YYTK_MODULE_INITIALIZATION_FAILED,
		// The target file header, directory, or RVA could not be found or is invalid.
		YYTK_FILE_PART_NOT_FOUND,
		// The object was not found.
		YYTK_OBJECT_NOT_FOUND
	};

	enum EventType : uint32_t
	{
		EVT_CODE_EXECUTE = (1 << 0),					// The event represents a Code_Execute() call.
		EVT_YYERROR = (1 << 1),							// The event represents a YYError() call.
		EVT_ENDSCENE = (1 << 2),						// The event represents an LPDIRECT3DDEVICE9::EndScene() call.
		EVT_PRESENT = (1 << 3),							// The event represents an IDXGISwapChain::Present() call.
		EVT_RESIZEBUFFERS = (1 << 4),					// The event represents an IDXGISwapChain::ResizeBuffers() call.
		EVT_WNDPROC = (1 << 5),							// The event represents a window procedure call.
		EVT_DOCALLSCRIPT = (1 << 6),					// The event represents a DoCallScript() call.
	};

	enum InstanceKeywords : int
	{
		VAR_SELF = -1,
		VAR_OTHER = -2,
		VAR_ALL = -3,
		VAR_NOONE = -4,
		VAR_GLOBAL = -5,
		VAR_BUILTIN = -6,
		VAR_LOCAL = -7,
		VAR_STACKTOP = -9,
		VAR_ARGUMENT = -15,
	};

	// https://github.com/YoYoGames/GMEXT-Steamworks/blob/main/source/Steamworks_vs/Steamworks/Ref.h
	template <typename T> 
	struct RefFactory
	{
		static T Alloc(T Thing, int Size) 
		{ 
			return Thing;
		}

		static T Create(T Thing, int& _size)
		{ 
			_size = 0; 
			return Thing;
		}

		static T Destroy(T Thing)
		{
			return Thing;
		}
	};

	// https://github.com/YoYoGames/GMEXT-Steamworks/blob/main/source/Steamworks_vs/Steamworks/Ref.h
	template <typename T>
	struct RefThingInternal
	{
		T Thing;
		int RefCount;
		int Size;

		RefThingInternal(T Thing)
		{
			this->Thing = RefFactory<T>::Create(Thing, this->Size);
			this->RefCount = 0;

			this->Reference();
		}

		RefThingInternal(int MaxSize)
		{
			this->Thing = RefFactory<T>::Alloc(this->Thing, MaxSize);
			this->Size = MaxSize;
			this->RefCount = 0;

			this->Reference();
		} 

		~RefThingInternal()
		{
			this->Dereference();
		}

		void Reference()
		{
			RefCount++;
		}

		void Dereference()
		{
			RefCount--;
			if (RefCount == 0)
			{
				delete this;
			}
		}

		static RefThingInternal<T>* Assign(RefThingInternal<T>* Other) 
		{ 
			if (Other)
				Other->Reference();

			return Other;
		}

		static RefThingInternal<T>* Remove(RefThingInternal<T>* Other)
		{ 
			if (Other)
				Other->Dereference();

			return nullptr; 
		}
	};

	template <typename T>
	struct RefThing
	{
		RefThingInternal<T>* Thing;

		RefThing() 
		{
			Thing = nullptr;
		}

		RefThing(T Thing)
		{
			this->Thing = new RefThingInternal<T>(Thing);
		}

		RefThing(const RefThingInternal<T>& Other)
		{
			this->Thing = Other.Thing;
			
			if (this->Thing)
				this->Thing->Reference();
		}

		~RefThing()
		{
			if (this->Thing)
				this->Thing->Dereference();
		}
	};

	using RefString = RefThingInternal<const char*>;

	// https://github.com/YoYoGames/GMEXT-Steamworks/blob/main/source/Steamworks_vs/Steamworks/YYRValue.h
#pragma pack(push, 4)
	struct RValue
	{
		union
		{
			int32_t i32;
			int64_t i64;
			double real;

			union
			{
				RefString* string;
				PVOID pointer;
			};
		};

		unsigned int flags;
		RValueType kind;
	};
#pragma pack(pop)
}

#endif // YYTK_SHARED_H_