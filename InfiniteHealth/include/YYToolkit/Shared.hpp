// File: Shared.hpp
// 
// Defines stuff shared between the tool and its plugins.
// Structs are meant to be opaque here, unless YYTK_INCLUDE_PRIVATE is defined.
// YYTK_INCLUDE_PRIVATE is set to 1 in the main YYTK project through Visual Studio's Project Properties.

#ifndef YYTK_SHARED_H_
#define YYTK_SHARED_H_

#define YYTK_MAJOR 3
#define YYTK_MINOR 4
#define YYTK_PATCH 3

#ifndef YYTK_CPP_VERSION
#ifndef _MSVC_LANG
#define YYTK_CPP_VERSION __cplusplus
#else
#define YYTK_CPP_VERSION _MSVC_LANG
#endif // _MSVC_LANG
#endif // YYTK_CPP_VERSION

#include <Aurie/shared.hpp>
#include <FunctionWrapper/FunctionWrapper.hpp>
#include <d3d11.h>
#include <functional>

#include <cstdint>
#include <string>

#ifndef UTEXT
#define UTEXT(x) ((const unsigned char*)(x))
#endif // UTEXT

#ifndef NULL_INDEX
#define NULL_INDEX INT_MIN
#endif

namespace YYTK
{
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

	enum RValueType : uint32_t
	{
		VALUE_REAL = 0,				// Real value
		VALUE_STRING = 1,			// String value
		VALUE_ARRAY = 2,			// Array value
		VALUE_PTR = 3,				// Ptr value
		VALUE_VEC3 = 4,				// Vec3 (x,y,z) value (within the RValue)
		VALUE_UNDEFINED = 5,		// Undefined value
		VALUE_OBJECT = 6,			// YYObjectBase* value 
		VALUE_INT32 = 7,			// Int32 value
		VALUE_VEC4 = 8,				// Vec4 (x,y,z,w) value (allocated from pool)
		VALUE_VEC44 = 9,			// Vec44 (matrix) value (allocated from pool)
		VALUE_INT64 = 10,			// Int64 value
		VALUE_ACCESSOR = 11,		// Actually an accessor
		VALUE_NULL = 12,			// JS Null
		VALUE_BOOL = 13,			// Bool value
		VALUE_ITERATOR = 14,		// JS For-in Iterator
		VALUE_REF = 15,				// Reference value (uses the ptr to point at a RefBase structure)
		VALUE_UNSET = 0x0ffffff		// Unset value (never initialized)
	};

	// These cannot be bitwise-operated on anymore
	enum EventTriggers : uint32_t
	{
		EVENT_OBJECT_CALL = 1,	// The event represents a Code_Execute() call.
		EVENT_FRAME = 2,		// The event represents an IDXGISwapChain::Present() call.
		EVENT_RESIZE = 3,		// The event represents an IDXGISwapChain::ResizeBuffers() call.
		EVENT_UNUSED = 4,		// This value is unused.
		EVENT_WNDPROC = 5		// The event represents a WndProc() call.
	};

	enum InstanceKeywords : int32_t
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

	struct IBuffer;
	struct CInstance;
	struct YYObjectBase;
	struct YYRunnerInterface;
	class YYTKInterface;
	struct RValue;
	struct RVariableRoutine;
	struct CRoom;
	struct CBackGM;
	struct CViewGM;
	struct CPhysicsWorld;
	struct RTile;
	struct YYRoomTiles;
	struct YYRoomInstances;
	struct CLayer;
	struct CLayerEffectInfo;
	struct CLayerElementBase;
	struct CLayerInstanceElement;
	struct CLayerSpriteElement;
	struct CInstanceBase;
	struct CWeakRef;
	struct CObjectGM;
	struct CPhysicsObject;
	struct CSkeletonInstance;
	struct CPhysicsDataGM;
	struct GCObjectContainer;
	struct YYRECT;
	struct CInstanceInternal;

	template <typename TKey, typename TValue, int TInitialMask>
	struct CHashMap;

	struct YYGMLException
	{
		char m_Object[16];
	};

	using TRoutine = void(*)(
		OUT RValue* Result,
		IN CInstance* Self,
		IN CInstance* Other,
		IN int ArgumentCount,
		IN RValue* Arguments
		);

	using PFUNC_YYGML = void(*)(
		IN CInstance* Self,
		IN CInstance* Other
		);

	using PFUNC_RAW = void(*)();

	using PFUNC_YYGMLScript = RValue & (*)(
		IN CInstance* Self,
		IN CInstance* Other,
		OUT RValue& Result,
		IN int ArgumentCount,
		IN RValue** Arguments // Array of RValue pointers
		);

	using PFN_YYObjectBaseAdd = void(__thiscall*)(
		IN YYObjectBase* This,
		IN const char* Name,
		IN const RValue& Value,
		IN int Flags
		);

	using PFN_FindAllocSlot = int(*)(
		OPTIONAL IN YYObjectBase* Object,
		IN const char* Name
	);

#pragma pack(push, 4)
	struct RValue
	{
		union
		{
			int32_t m_i32;
			int64_t m_i64;
			double m_Real;

			CInstance* m_Object;
			PVOID m_Pointer;
		};

		unsigned int m_Flags;
		RValueType m_Kind;

		// Constructors
		RValue();

		RValue(
			IN std::initializer_list<RValue> Values
		);

		RValue(
			IN bool Value
		);

		RValue(
			IN double Value
		);

		RValue(
			IN int64_t Value
		);

		RValue(
			IN int32_t Value
		);

		RValue(
			IN CInstance* Object
		);

		RValue(
			IN const char* Value
		);

#if YYTK_CPP_VERSION > 202002L
		RValue(
			IN const char8_t* Value
		);
#endif // YYTK_CPP_VERSION
		RValue(
			IN std::string_view Value
		);

#if YYTK_CPP_VERSION > 202002L
		RValue(
			IN std::u8string_view Value
		);
#endif // YYTK_CPP_VERSION
		RValue(
			IN const std::string& Value
		);

#if YYTK_CPP_VERSION > 202002L
		RValue(
			IN const std::u8string& Value
		);
#endif // YYTK_CPP_VERSION
		RValue(
			IN std::string_view Value,
			IN YYTKInterface* Interface
		);

		// Custom getters
		bool AsBool() const;

		double AsReal() const;

		std::string_view AsString();

		std::string_view AsString(
			IN YYTKInterface* Interface
		);

		// Overloaded operators
		RValue& operator[](
			IN size_t Index
			);

		RValue& operator[](
			IN std::string_view Element
			);

		// STL-like access
		RValue& at(
			IN size_t Index
		);

		RValue& at(
			IN std::string_view Element
		);

		RValue* data();

		size_t length();
	};
#pragma pack(pop)

	struct RToken
	{
		int m_Kind;
		unsigned int m_Type;
		int m_Ind;
		int m_Ind2;
		RValue m_Value;
		int m_ItemNumber;
		RToken* m_Items;
		int m_Position;
	};

	struct YYGMLFuncs
	{
		const char* m_Name;
		union
		{
			PFUNC_YYGMLScript m_ScriptFunction;
			PFUNC_YYGML m_CodeFunction;
			PFUNC_RAW m_RawFunction;
		};
		PVOID m_FunctionVariables; // YYVAR
	};

	struct CCode
	{
		int (**_vptr$CCode)(void);
		CCode* m_Next;
		int m_Kind;
		int m_Compiled;
		const char* m_Str;
		RToken m_Token;
		RValue m_Value;
		PVOID m_VmInstance;
		PVOID m_VmDebugInfo;
		char* m_Code;
		const char* m_Name;
		int m_CodeIndex;
		YYGMLFuncs* m_Functions;
		bool m_Watch;
		int m_Offset;
		int m_LocalsCount;
		int m_ArgsCount;
		int m_Flags;
		YYObjectBase* m_Prototype;

		const char* GetName() const { return this->m_Name; }
	};

	struct CScript
	{
		int (**_vptr$CScript)(void);
		CCode* m_Code;
		YYGMLFuncs* m_Functions;
		CInstance* m_StaticObject;

		union
		{
			const char* m_Script;
			int m_CompiledIndex;
		};

		const char* m_Name;
		int m_Offset;

		const char* GetName() const { return this->m_Name; }
	};

	enum eBuffer_Format {
		eBuffer_Format_Fixed = 0,
		eBuffer_Format_Grow = 1,
		eBuffer_Format_Wrap = 2,
		eBuffer_Format_Fast = 3,
		eBuffer_Format_VBuffer = 4,
		eBuffer_Format_Network = 5,
	};

	typedef void* HYYMUTEX;
	typedef void* HSPRITEASYNC;

	struct HTTP_REQ_CONTEXT;
	typedef int (*PFUNC_async)(HTTP_REQ_CONTEXT* _pContext, void* _pPayload, int* _pMap);
	typedef void (*PFUNC_cleanup)(HTTP_REQ_CONTEXT* _pContext);
	typedef void (*PFUNC_process)(HTTP_REQ_CONTEXT* _pContext);

	//
	// Copyright (C) 2020 Opera Norway AS. All rights reserved.
	//
	// This portion of the file (struct YYRunnerInterface) is an original work developed by Opera.
	//
	struct YYRunnerInterface
	{
		// ########################################################################
		// BASIC INTERACTION WITH THE USER
		// ######################################################################## 

		/**
		 * @brief Writes a formatted string to the debug console output, analogous to printf.
		 *
		 * @param fmt A string that contains the text to be written to the output. It can include embedded format tags.
		 * @param ... (Variadic arguments) A variable number of arguments to be embedded in the format string.
		 *
		 * Usage example:
		 *
		 *		// Writes to the debug console
		 *		DebugConsoleOutput("This is a number: %d", a_number);
		 */
		void (*DebugConsoleOutput)(const char* fmt, ...);

		/**
		 * @brief Writes a formatted string to the release console output, analogous to printf.
		 *
		 * @param fmt A string that contains the text to be written to the output. It can include embedded format tags.
		 * @param ... (Variadic arguments) A variable number of arguments to be embedded in the format string.
		 *
		 * Usage example:
		 *
		 *		// Writes to the release console
		 *		ReleaseConsoleOutput("This is a number: %d", a_number);
		 */
		void (*ReleaseConsoleOutput)(const char* fmt, ...);

		/**
		 * @brief Displays a popup message on the runner side, typically to convey information or warnings to the user.
		 *
		 * @param msg The string message to be displayed in the popup.
		 *
		 * Usage example:
		 *
		 *		// Displays a message to the user
		 *		ShowMessage("Hello from the other side!");
		 */
		void (*ShowMessage)(const char* msg);

		/**
		 * @brief Writes a formatted error message to the error console and triggers a runtime error.
		 *
		 * @param _error A string that contains the text of the error message. It can include embedded format tags.
		 * @param ... (Variadic arguments) A variable number of arguments to be embedded in the error string.
		 *
		 * Usage example:
		 *
		 *		// Triggers a runtime error with a custom message
		 *		YYError("MyFunction :: incorrect number of arguments");
		 */
		void (*YYError)(const char* _error, ...);


		// ########################################################################
		// MEMORY MANAGEMENT
		// ######################################################################## 

		/**
		 * @brief Allocates a block of memory of size _size.
		 *
		 * @param _size The size of the memory block to allocate, in bytes.
		 *
		 * @return A pointer to the allocated memory block. The pointer is
		 * null if the function fails to allocate memory.
		 *
		 * Usage example:
		 *
		 *		// Allocates a block of memory of size 10 bytes.
		 *		void* pMemoryBlock = YYAlloc(10);
		 */
		void* (*YYAlloc)(int _size);

		/**
		 * @brief Reallocates a block of memory to a new size.
		 *
		 * @param pOriginal A pointer to the memory block originally allocated
		 * with YYAlloc or YYRealloc.
		 * @param _newSize The new size of the memory block, in bytes.
		 *
		 * @return A pointer to the reallocated memory block. This might be
		 * different from pOriginal if the function had to move the memory
		 * block to enlarge it. Returns null if the function fails to reallocate
		 * memory.
		 *
		 * Usage example:
		 *
		 *		// Reallocate pMemoryBlock to 20 bytes.
		 *		void* pNewMemoryBlock = YYRealloc(pMemoryBlock, 20);
		 */
		void* (*YYRealloc)(void* pOriginal, int _newSize);

		/**
		 * @brief Frees a block of memory that was previously allocated.
		 *
		 * @param p A pointer to the memory block to be freed. This should have been
		 * returned by a previous call to YYAlloc or YYRealloc.
		 *
		 * Usage example:
		 *
		 *		// Frees the allocated memory block.
		 *		YYFree(pMemoryBlock);
		 */
		void  (*YYFree)(const void* p);

		/**
		 * @brief Duplicates a string by allocating memory for a new string and copying the
		 * content.
		 *
		 * @param _pS A pointer to the null-terminated string to duplicate.
		 *
		 * @return A pointer to the newly allocated string with the same content as _pS.
		 * Returns null if the function fails to allocate memory. The returned string must be
		 * freed using YYFree when it is no longer needed.
		 *
		 * Usage example:
		 *
		 *		// Duplicates the string.
		 *		const char* duplicatedString = YYStrDup(originalString);
		 */
		const char* (*YYStrDup)(const char* _pS);


		// ########################################################################
		// ARGUMENT PARSING
		// ######################################################################## 

		/**
		 * @brief Parses and retrieves a boolean value from the argument array at the specified index.
		 *
		 * @param _pBase A pointer to the array of arguments.
		 * @param _index The index in the array from which to retrieve the boolean value.
		 *
		 * @return The boolean value present at the specified index in the argument array.
		 *
		 * Usage example:
		 *
		 *		bool value = YYGetBool(pArgs, 0);
		 */
		bool (*YYGetBool)(const RValue* _pBase, int _index);

		/**
		 * @brief Parses and retrieves a float value from the argument array at the specified index.
		 *
		 * @param _pBase A pointer to the array of arguments.
		 * @param _index The index in the array from which to retrieve the float value.
		 *
		 * @return The float value present at the specified index in the argument array.
		 *
		 * Usage example:
		 *
		 *		float value = YYGetFloat(pArgs, 1);
		 */
		float (*YYGetFloat)(const RValue* _pBase, int _index);

		/**
		 * @brief Parses and retrieves a double-precision floating-point value from the argument array at the specified index.
		 *
		 * @param _pBase A pointer to the array of arguments.
		 * @param _index The index in the array from which to retrieve the double value.
		 *
		 * @return The double-precision floating-point value present at the specified index in the argument array.
		 *
		 * Usage example:
		 *
		 *		double value = YYGetReal(pArgs, 2);
		 */
		double (*YYGetReal)(const RValue* _pBase, int _index);

		/**
		 * @brief Parses and retrieves a 32-bit integer value from the argument array at the specified index.
		 *
		 * @param _pBase A pointer to the array of arguments.
		 * @param _index The index in the array from which to retrieve the 32-bit integer value.
		 *
		 * @return The 32-bit integer value present at the specified index in the argument array.
		 *
		 * Usage example:
		 *
		 *		int32_t value = YYGetInt32(pArgs, 3);
		 */
		int32_t(*YYGetInt32)(const RValue* _pBase, int _index);

		/**
		 * @brief Parses and retrieves a 32-bit unsigned integer value from the argument array at the specified index.
		 *
		 * @param _pBase A pointer to the array of arguments.
		 * @param _index The index in the array from which to retrieve the 32-bit unsigned integer value.
		 *
		 * @return The 32-bit unsigned integer value present at the specified index in the argument array.
		 *
		 * Usage example:
		 *
		 *		uint32_t value = YYGetUint32(pArgs, 4);
		 */
		uint32_t(*YYGetUint32)(const RValue* _pBase, int _index);

		/**
		 * @brief Parses and retrieves a 64-bit integer value from the argument array at the specified index.
		 *
		 * @param _pBase A pointer to the array of arguments.
		 * @param _index The index in the array from which to retrieve the 64-bit integer value.
		 *
		 * @return The 64-bit integer value present at the specified index in the argument array.
		 *
		 * Usage example:
		 *
		 *		int64 value = YYGetInt64(pArgs, 5);
		 */
		int64_t(*YYGetInt64)(const RValue* _pBase, int _index);

		/**
		 * @brief Parses and retrieves a pointer value from the argument array at the specified index.
		 *
		 * @param _pBase A pointer to the array of arguments.
		 * @param _index The index in the array from which to retrieve the pointer value.
		 *
		 * @return The pointer value present at the specified index in the argument array.
		 *
		 * Usage example:
		 *
		 *		void* value = YYGetPtr(pArgs, 6);
		 */
		void* (*YYGetPtr)(const RValue* _pBase, int _index);

		/**
		 * @brief Parses and retrieves either a pointer value or an integer value from the argument array at the specified index.
		 *
		 * @param _pBase A pointer to the array of arguments.
		 * @param _index The index in the array from which to retrieve the pointer or integer value.
		 *
		 * @return The pointer or integer value present at the specified index in the argument array.
		 *
		 * Usage example:
		 *
		 *		intptr_t value = YYGetPtrOrInt(pArgs, 7);
		 */
		intptr_t(*YYGetPtrOrInt)(const RValue* _pBase, int _index);

		/**
		 * @brief Parses and retrieves a string value from the argument array at the specified index.
		 *
		 * @param _pBase A pointer to the array of arguments.
		 * @param _index The index in the array from which to retrieve the string value.
		 *
		 * @return The string value present at the specified index in the argument array.
		 *
		 * Usage example:
		 *
		 *		const char* value = YYGetString(pArgs, 8);
		 *
		 * @note The user should not free the returned const char*, as it is managed by the internal memory management system.
		 *       However, it is essential to duplicate it if you need to store it since the runner invalidates these strings at
		 *       the end of each step.
		 */
		const char* (*YYGetString)(const RValue* _pBase, int _index);


		// ########################################################################
		// RVALUE PARSING
		// ######################################################################## 

		/**
		 * @brief Attempts to retrieve a boolean value by parsing|casting from a single RValue.
		 *
		 * @param _pValue A pointer to the RValue.
		 *
		 * @return The boolean value obtained by casting the specified RValue.
		 *
		 * Usage example:
		 *
		 *		bool value = BOOL_RValue(pValue);
		 */
		bool (*BOOL_RValue)(const RValue* _pValue);

		/**
		 * @brief Attempts to retrieve a double-precision floating-point value by parsing|casting from a single RValue.
		 *
		 * @param _pValue A pointer to the RValue.
		 *
		 * @return The double-precision floating-point value obtained by casting the specified RValue.
		 *
		 * Usage example:
		 *
		 *		double value = REAL_RValue(pValue);
		 */
		double (*REAL_RValue)(const RValue* _pValue);

		/**
		 * @brief Attempts to retrieve a pointer value by parsing|casting from a single RValue.
		 *
		 * @param _pValue A pointer to the RValue.
		 *
		 * @return The pointer value obtained by casting the specified RValue.
		 *
		 * Usage example:
		 *
		 *		void* value = PTR_RValue(pValue);
		 */
		void* (*PTR_RValue)(const RValue* _pValue);

		/**
		 * @brief Attempts to retrieve a 64-bit integer value by parsing|casting from a single RValue.
		 *
		 * @param _pValue A pointer to the RValue.
		 *
		 * @return The 64-bit integer value obtained by casting the specified RValue.
		 *
		 * Usage example:
		 *
		 *		int64 value = INT64_RValue(pValue);
		 */
		int64_t(*INT64_RValue)(const RValue* _pValue);

		/**
		 * @brief Attempts to retrieve a 32-bit integer value by parsing|casting from a single RValue.
		 *
		 * @param _pValue A pointer to the RValue.
		 *
		 * @return The 32-bit integer value obtained by casting the specified RValue.
		 *
		 * Usage example:
		 *
		 *		int32_t value = INT32_RValue(pValue);
		 */
		int32_t(*INT32_RValue)(const RValue* _pValue);


		// ########################################################################
		// HASHING
		// ########################################################################

		/**
		 * @brief Calculates a hash value from a given RValue.
		 *
		 * @param _pValue A pointer to the RValue from which to calculate the hash value.
		 *
		 * @return The calculated hash value for the specified RValue.
		 *
		 * Usage example:
		 *
		 *		int hashValue = HASH_RValue(pValue);
		 */
		int (*HASH_RValue)(const RValue* _pValue);


		// ########################################################################
		// COPYING, GETTING, SETTING & FREEING RVALUES
		// ########################################################################

		/**
		 * @brief Assigns an RValue to another, considering a given context and index.
		 *
		 * @param _pDest Pointer to the destination RValue.
		 * @param _pV Pointer to the source RValue.
		 * @param _pPropSelf Pointer to the YYObjectBase, representing the "self" or context.
		 * @param _index Index within an array of RValues, if applicable.
		 *
		 * Usage example:
		 *
		 *		SET_RValue(&destinationRValue, &sourceRValue, pContext, arrayIndex);
		 */
		void (*SET_RValue)(RValue* _pDest, RValue* _pV, YYObjectBase* _pPropSelf, int _index);

		/**
		 * @brief Retrieves an RValue, considering a given context and index, and potentially prepares an array.
		 *
		 * @param _pRet Pointer to the RValue where the result will be stored.
		 * @param _pV Pointer to the source RValue.
		 * @param _pPropSelf Pointer to the YYObjectBase, representing the "self" or context.
		 * @param _index Index within an array of RValues, if applicable.
		 * @param fPrepareArray Boolean flag indicating whether to prepare an array.
		 * @param fPartOfSet Boolean flag indicating whether this action is part of a set operation.
		 *
		 * @return Boolean indicating the success of the get operation.
		 *
		 * @note Ensure to call FREE_RValue on the retrieved RValue once done to avoid memory leaks.
		 */
		bool (*GET_RValue)(RValue* _pRet, RValue* _pV, YYObjectBase* _pPropSelf, int _index, bool fPrepareArray, bool fPartOfSet);

		/**
		 * @brief Copies the value from one RValue to another.
		 *
		 * @param _pDest Pointer to the destination RValue.
		 * @param _pSource Pointer to the source RValue.
		 *
		 * Usage example:
		 *
		 *		COPY_RValue(&destinationRValue, &sourceRValue);
		 */
		void (*COPY_RValue)(RValue* _pDest, const RValue* _pSource);

		/**
		 * @brief Retrieves the kind/type of the given RValue.
		 *
		 * @param _pValue Pointer to the RValue to retrieve the kind from.
		 *
		 * @return Integer representing the kind/type of the RValue.
		 */
		int (*KIND_RValue)(const RValue* _pValue);

		/**
		 * @brief Frees the memory associated with a given RValue.
		 *
		 * @param _pValue Pointer to the RValue to be freed.
		 *
		 * Usage example:
		 *
		 *		FREE_RValue(&myRValue);
		 *
		 * @note Always call this function after you are done using an RValue to prevent memory leaks.
		 */
		void (*FREE_RValue)(RValue* _pValue);

		/**
		 * @brief Creates a new string RValue.
		 *
		 * @param _pVal Pointer to the RValue where the string will be stored.
		 * @param _pS Pointer to the source string.
		 *
		 * Usage example:
		 *
		 *		YYCreateString(&myRValue, "Hello, world!");
		 */
		void (*YYCreateString)(RValue* _pVal, const char* _pS);

		/**
		 * @brief Creates a new array RValue.
		 *
		 * @param pRValue Pointer to the RValue where the array will be stored.
		 * @param n_values The number of elements (double values) to store in the array.
		 * @param values Pointer to the array of double values that should be stored.
		 *
		 * Usage example:
		 *
		 *		double myValues[] = {1.0, 2.0, 3.0};
		 *		YYCreateArray(&myRValue, 3, myValues);
		 *
		 * @note This function initializes an RValue as an array and populates it with the provided double values.
		 */
		void (*YYCreateArray)(RValue* pRValue, int n_values, const double* values);

		// Finding and running user scripts from name
		int (*Script_Find_Id)(const char* name);
		bool (*Script_Perform)(int ind, CInstance* selfinst, CInstance* otherinst, int argc, RValue* res, RValue* arg);

		// Finding builtin functions
		bool  (*Code_Function_Find)(const char* name, int* ind);

		// HTTP functions
		void (*HTTP_Get)(const char* _pFilename, int _type, PFUNC_async _async, PFUNC_cleanup _cleanup, void* _pV);
		void (*HTTP_Post)(const char* _pFilename, const char* _pPost, PFUNC_async _async, PFUNC_cleanup _cleanup, void* _pV);
		void (*HTTP_Request)(const char* _url, const char* _method, const char* _headers, const char* _pBody, PFUNC_async _async, PFUNC_cleanup _cleanup, void* _pV, int _contentLength);

		// sprite loading helper functions
		int (*ASYNCFunc_SpriteAdd)(HTTP_REQ_CONTEXT* _pContext, void* _p, int* _pMap);
		void (*ASYNCFunc_SpriteCleanup)(HTTP_REQ_CONTEXT* _pContext);
		HSPRITEASYNC(*CreateSpriteAsync)(int* _pSpriteIndex, int _xOrig, int _yOrig, int _numImages, int _flags);


		// ########################################################################
		// TIMING
		// ########################################################################

		/**
		 * @brief Retrieves the current time in microseconds (since start).
		 *
		 * This function returns the current time calculated from the start of the app,
		 * measured in microseconds. It provides high-resolution time data useful for precise
		 * timing, benchmarking, and profiling.
		 *
		 * @return An int64 representing the number of microseconds elapsed since the start of the app.
		 *
		 * @note Be mindful of the data type sizes when performing calculations to avoid overflow.
		 */
		int64_t(*Timing_Time)(void);

		/**
		 * @brief Pauses the execution of the main thread for a specified time.
		 *
		 * `Timing_Sleep` halts the execution of the calling thread for at least the
		 * specified duration in microseconds, permitting the system to execute other threads.
		 * The precision of the sleep duration can be influenced by the `precise` parameter.
		 *
		 * @param slp The duration for which the thread will sleep, in microseconds.
		 * @param precise Boolean flag indicating whether to enforce precise sleeping.
		 *        - true:  Aims to ensure the sleep duration is as exact as possible.
		 *        - false: The sleep duration may be slightly shorter or longer.
		 *
		 * @note Prolonged and precise sleep durations might impact system power usage
		 *       and performance adversely. Use precise sleeps judiciously, balancing
		 *       requirement against resource utilization.
		 */
		void (*Timing_Sleep)(int64_t slp, bool precise);


		// ########################################################################
		// MUTEX HANDLING
		// ########################################################################

		/**
		 * Mutexes, or mutual exclusions, are used in concurrent programming to avoid the simultaneous
		 * execution of a piece of code that accesses shared resources, such as runtime elements like ds_maps,
		 * ds_lists, and buffers, by multiple threads.
		 *
		 * This helps in avoiding race conditions, ensuring that the shared resources are accessed in a mutually
		 * exclusive manner. When a thread acquires a lock on a mutex, any other thread attempting to acquire the
		 * same lock will block until the first thread releases the lock.
		 */

		 /**
		  * @brief Creates a new mutex with the given name.
		  *
		  * @param _name A string representing the name of the mutex to be created.
		  *
		  * @return A handle to the newly created mutex.
		  *
		  * Usage example:
		  *
		  *		HYYMUTEX myMutex = YYMutexCreate("myMutexName");
		  *
		  * @note This is the runner's own implementation of mutexes, used for ensuring controlled access
		  *       to shared resources in a concurrent environment.
		  */
		HYYMUTEX(*YYMutexCreate)(const char* _name);

		/**
		 * @brief Destroys a given mutex.
		 *
		 * @param hMutex A handle to the mutex to be destroyed.
		 *
		 * Usage example:
		 *
		 *		YYMutexDestroy(myMutex);
		 */
		void (*YYMutexDestroy)(HYYMUTEX hMutex);

		/**
		 * @brief Locks a given mutex.
		 *
		 * @param hMutex A handle to the mutex to be locked.
		 *
		 * Usage example:
		 *
		 *		YYMutexLock(myMutex);
		 *
		 * @note This function will block if the mutex is already locked by another thread,
		 *       and it will return once the mutex has been successfully locked.
		 */
		void (*YYMutexLock)(HYYMUTEX hMutex);

		/**
		 * @brief Unlocks a given mutex.
		 *
		 * @param hMutex A handle to the mutex to be unlocked.
		 *
		 * Usage example:
		 *
		 *		YYMutexUnlock(myMutex);
		 *
		 * @note After unlocking, any other threads waiting to lock the mutex will be unblocked.
		 */
		void (*YYMutexUnlock)(HYYMUTEX hMutex);


		// ########################################################################
		// ASYNC EVENTS
		// ########################################################################

		/**
		 * @brief Triggers an asynchronous event, passing a ds_map to it.
		 *
		 * @param _map The index of the ds_map to pass to the event.
		 * @param _event The event code number to trigger.
		 *
		 * Usage example:
		 *
		 *		CreateAsyncEventWithDSMap(myDsMap, myEventCode);
		 *
		 * @note The ds_map passed to the event should not be manually freed post-triggering.
		 */
		void (*CreateAsyncEventWithDSMap)(int _map, int _event);

		/**
		 * @brief Triggers an asynchronous event, passing a ds_map and a GML buffer to it.
		 *
		 * @param _map The index of the ds_map to pass to the event.
		 * @param _buffer The index of the GML buffer to pass to the event.
		 * @param _event The event code number to trigger.
		 *
		 * Usage example:
		 *
		 *		CreateAsyncEventWithDSMapAndBuffer(myDsMap, myBuffer, myEventCode);
		 *
		 * @note The ds_map and the GML buffer should not be manually freed post-triggering.
		 *       They will be automatically freed once the event is finished.
		 */
		void (*CreateAsyncEventWithDSMapAndBuffer)(int _map, int _buffer, int _event);


		// ########################################################################
		// DS_MAP MANIPULATION
		// ########################################################################

		/**
		 * @brief Creates a new ds_map with specified key-value pairs.
		 *
		 * @param _num The number of key-value pairs to be added to the ds_map.
		 * @param ... Key and value pairs to be added to the ds_map. Only double and string (char*) types are supported.
		 *
		 * @return The index referencing the created ds_map.
		 *
		 * Usage example:
		 *
		 *		int myDsMap = CreateDsMap(2, "key1", 1.23, "key2", "value2");
		 *
		 * @note Ensure that the keys and values are passed in pairs and the count (_num) matches the total number of arguments passed.
		 */
		int (*CreateDsMap)(int _num, ...);

		/**
		 * @brief Adds a key-double pair to a ds_map.
		 *
		 * @param _index The index of the ds_map to which the key-value pair will be added.
		 * @param _pKey The key to be associated with the value.
		 * @param value The double value to be added.
		 *
		 * @return True if the addition is successful, otherwise False.
		 *
		 * Usage example:
		 *
		 *		bool success = DsMapAddDouble(myDsMap, "key3", 3.45);
		 */
		bool (*DsMapAddDouble)(int _index, const char* _pKey, double value);

		/**
		 * @brief Adds a key-string pair to a ds_map.
		 *
		 * @param _index The index of the ds_map to which the key-value pair will be added.
		 * @param _pKey The key to be associated with the value.
		 * @param pVal The string value to be added.
		 *
		 * @return True if the addition is successful, otherwise False.
		 *
		 * Usage example:
		 *
		 *		bool success = DsMapAddString(myDsMap, "key4", "value4");
		 */
		bool (*DsMapAddString)(int _index, const char* _pKey, const char* pVal);

		/**
		 * @brief Adds a key-int64 pair to a ds_map.
		 *
		 * @param _index The index of the ds_map to which the key-value pair will be added.
		 * @param _pKey The key to be associated with the value.
		 * @param value The int64 value to be added.
		 *
		 * @return True if the addition is successful, otherwise False.
		 *
		 * Usage example:
		 *
		 *		bool success = DsMapAddInt64(myDsMap, "key5", 123456789012345);
		 */
		bool (*DsMapAddInt64)(int _index, const char* _pKey, int64_t value);


		// ########################################################################
		// BUFFER ACCESS
		// ########################################################################

		/**
		 * @brief Retrieves the content of a GameMaker Language (GML) buffer.
		 *
		 * @param _index The index of the GML buffer to get content from.
		 * @param _ppData A pointer to store the address of the retrieved content.
		 * @param _pDataSize A pointer to store the size of the retrieved content.
		 *
		 * @return A boolean indicating the success of the retrieval.
		 *
		 * Usage example:
		 *
		 *		void* data;
		 *		int dataSize;
		 *		bool success = BufferGetContent(bufferIndex, &data, &dataSize);
		 *
		 * @note The method creates a copy of the data from the runner, which needs to be properly freed to prevent memory leaks.
		 */
		bool (*BufferGetContent)(int _index, void** _ppData, int* _pDataSize);

		/**
		 * @brief Writes content into a GML buffer.
		 *
		 * @param _index The index of the GML buffer to write content to.
		 * @param _dest_offset The destination offset within the GML buffer to start writing data.
		 * @param _pSrcMem A pointer to the source memory to write from.
		 * @param _size The size of the data to write.
		 * @param _grow A boolean indicating whether the buffer should grow if necessary.
		 * @param _wrap A boolean indicating whether the buffer should wrap if necessary.
		 *
		 * @return The number of bytes written to the buffer.
		 *
		 * Usage example:
		 *
		 *		int bytesWritten = BufferWriteContent(bufferIndex, offset, srcData, dataSize, true, false);
		 */
		int (*BufferWriteContent)(int _index, int _dest_offset, const void* _pSrcMem, int _size, bool _grow, bool _wrap);

		/**
		 * @brief Creates a new GML buffer.
		 *
		 * @param _size The size of the new buffer.
		 * @param _bf The format of the new buffer, specified using eBuffer_Format enum.
		 * @param _alignment The memory alignment of the new buffer.
		 *
		 * @return The index of the newly created buffer.
		 *
		 * Usage example:
		 *
		 *		int newBufferIndex = CreateBuffer(1024, eBuffer_Format_Grow, 0);
		 *
		 * Enum values for eBuffer_Format:
		 * - eBuffer_Format_Fixed:    0
		 * - eBuffer_Format_Grow:     1
		 * - eBuffer_Format_Wrap:     2
		 * - eBuffer_Format_Fast:     3
		 * - eBuffer_Format_VBuffer:  4
		 * - eBuffer_Format_Network:  5
		 */
		int (*CreateBuffer)(int _size, enum eBuffer_Format _bf, int _alignment);


		// ########################################################################
		// VARIABLES
		// ########################################################################

		volatile bool* pLiveConnection;
		int* pHTTP_ID;


		// ########################################################################
		// DS_LIST AND DS_MAP MANIPULATION
		// ########################################################################

		/**
		 * @brief Creates a new ds_list.
		 *
		 * @return The index of the newly created ds_list.
		 *
		 * Usage example:
		 *
		 *		int newListIndex = DsListCreate();
		 */
		int (*DsListCreate)();

		/**
		 * @brief Adds a ds_list to a ds_map with a specified key.
		 *
		 * @param _dsMap The index of the ds_map.
		 * @param _key The key associated with the ds_list in the ds_map.
		 * @param _listIndex The index of the ds_list to add to the ds_map.
		 *
		 * @note When a ds_list added to a ds_map using DsMapAddList is removed
		 *       (e.g., via DsMapClear), it is automatically freed by the runner
		 *       and does not require manual freeing.
		 *
		 * Usage example:
		 *
		 *		DsMapAddList(mapIndex, "myListKey", listIndex);
		 */
		void (*DsMapAddList)(int _dsMap, const char* _key, int _listIndex);

		/**
		 * @brief Adds a ds_map to a ds_list.
		 *
		 * @param _dsList The index of the ds_list.
		 * @param _mapIndex The index of the ds_map to add to the ds_list.
		 *
		 * @note When a ds_map added to a ds_list using DsListAddMap is removed
		 *       (e.g., via DsListClear), it is also automatically freed by the
		 *       runner and does not require manual freeing.
		 *
		 * Usage example:
		 *
		 *		DsListAddMap(listIndex, mapIndex);
		 */
		void (*DsListAddMap)(int _dsList, int _mapIndex);

		/**
		 * @brief Clears all key-value pairs from a ds_map.
		 *
		 * @param _dsMap The index of the ds_map to clear.
		 *
		 * @note Clearing a ds_map with DsMapClear will also automatically free
		 *       any ds_lists that have been added to it with DsMapAddList,
		 *       ensuring that no memory leaks occur.
		 *
		 * Usage example:
		 *
		 *		DsMapClear(mapIndex);
		 */
		void (*DsMapClear)(int _dsMap);

		/**
		 * @brief Clears all elements from a ds_list.
		 *
		 * @param _dsList The index of the ds_list to clear.
		 *
		 * @note Clearing a ds_list with DsListClear will also automatically free
		 *       any ds_maps that have been added to it with DsListAddMap,
		 *       avoiding any memory leaks.
		 *
		 * Usage example:
		 *
		 *		DsListClear(listIndex);
		 */
		void (*DsListClear)(int _dsList);


		// ########################################################################
		// FILES
		// ########################################################################

		bool (*BundleFileExists)(const char* _pszFileName);
		bool (*BundleFileName)(char* _name, int _size, const char* _pszFileName);
		bool (*SaveFileExists)(const char* _pszFileName);
		bool (*SaveFileName)(char* _name, int _size, const char* _pszFileName);


		// ########################################################################
		// BASE64 ENCODE
		// ########################################################################

		/**
		 * @brief Encodes binary data using Base64 encoding.
		 *
		 * This function takes binary data as input and provides a Base64-encoded output.
		 * Base64 encoding is commonly used to encode binary data, especially when that data
		 * needs to be stored and transferred over media that is designed to deal with text.
		 * It can be used to encode data in HTTP post calls or to encode data read from a GML buffer, for example.
		 *
		 * @param input_buf Pointer to the binary data to be encoded.
		 * @param input_len Length (in bytes) of the data to be encoded.
		 * @param output_buf Pointer to the buffer where the encoded data will be stored.
		 * @param output_len Length of the output buffer. This should be at least 4/3 times the input length, to ensure that there is enough space to hold the encoded data.
		 *
		 * @return Returns `true` if the encoding is successful, and `false` otherwise. The function might fail if there is not enough space in the output buffer to hold the encoded data.
		 *
		 * Usage example:
		 *
		 *		const char* binaryData = "Binary data goes here";
		 *		size_t binaryDataLength = strlen(binaryData);
		 *		char encodedData[200]; // Ensure the output buffer is large enough
		 *
		 *		bool success = Base64Encode(binaryData, binaryDataLength, encodedData, sizeof(encodedData));
		 *
		 * @note Ensure the output buffer is large enough to store the encoded data.
		 */
		bool (*Base64Encode)(const void* input_buf, size_t input_len, void* output_buf, size_t output_len);

		// ########################################################################
		// DS_LIST MANIPULATION
		// ########################################################################

		/**
		 * @brief Adds a 64-bit integer value to a ds_list.
		 *
		 * Appends a 64-bit integer (int64) to the specified ds_list,
		 * ensuring that large integer values can be stored without truncation
		 * or data loss. Ds_lists are dynamic arrays that can store different
		 * types of data, in this case, a 64-bit integer.
		 *
		 * @param _dsList The index of the ds_list to which the 64-bit integer will be added.
		 * @param _value The 64-bit integer value to be added to the ds_list.
		 *
		 * @note Ensure the ds_list index provided is valid and the ds_list is properly initialized.
		 *
		 * Usage example:
		 *
		 *		int myDsList = DsListCreate(); // Assume DsListCreate is a function that creates a ds_list
		 *		int64 myValue = 1234567890123456789;
		 *		DsListAddInt64(myDsList, myValue);
		 */
		void (*DsListAddInt64)(int _dsList, int64_t _value);


		// ########################################################################
		// FILE & DIRECTORY WHITELISTING
		// ########################################################################

		void (*AddDirectoryToBundleWhitelist)(const char* _pszFilename);
		void (*AddFileToBundleWhitelist)(const char* _pszFilename);
		void (*AddDirectoryToSaveWhitelist)(const char* _pszFilename);
		void (*AddFileToSaveWhitelist)(const char* _pszFilename);

		// ########################################################################
		// UTILITIES
		// ########################################################################

		/**
		 * @brief Retrieves the string representation of the kind/type of an RValue.
		 *
		 * The function obtains the kind of an RValue, representing its type, and returns
		 * its string representation. Useful for debugging, logging, or any situation
		 * where the textual depiction of an RValue's type is necessary.
		 *
		 * @param _pV A pointer to the RValue whose kind/type name is to be retrieved.
		 *
		 * @return A string literal representing the kind/type of the given RValue.
		 *
		 * @note The returned string should not be freed or modified.
		 *
		 * Usage example:
		 *
		 *		const RValue myValue = ...; // Assume this is populated appropriately
		 *		const char* typeName = KIND_NAME_RValue(&myValue);
		 *		printf("The type of myValue is: %s\n", typeName);
		 */
		const char* (*KIND_NAME_RValue)(const RValue* _pV);

		// ########################################################################
		// DS_MAP MANIPULATION (PART 2)
		// ########################################################################

		/**
		 * @brief Adds a key-boolean pair to a ds_map.
		 *
		 * @param _index The index of the ds_map to which the key-value pair will be added.
		 * @param _pKey The key to be associated with the value.
		 * @param value The boolean value to be added.
		 *
		 * Usage example:
		 *
		 *		DsMapAddBool(myDsMap, "keyBool", true);
		 *
		 * @note Ensure the ds_map referred to by _index is valid and created before using this function.
		 */
		void (*DsMapAddBool)(int _index, const char* _pKey, bool value);

		/**
		 * @brief Adds a key-RValue pair to a ds_map.
		 *
		 * @param _index The index of the ds_map to which the key-value pair will be added.
		 * @param _pKey The key to be associated with the value.
		 * @param value A pointer to the RValue to be added.
		 *
		 * Usage example:
		 *
		 *		RValue myValue;
		 *		// ... (initialize and set myValue)
		 *		DsMapAddRValue(myDsMap, "keyRValue", &myValue);
		 *
		 * @note Ensure the ds_map referred to by _index is valid and created before using this function.
		 * @note Ensure the RValue is properly initialized and set before using it as a parameter.
		 */
		void (*DsMapAddRValue)(int _index, const char* _pKey, RValue* value);

		/**
		 * @brief Frees a ds_map from memory.
		 *
		 * @param _index The index of the ds_map to be freed.
		 *
		 * Usage example:
		 *
		 *		DestroyDsMap(myDsMap);
		 *
		 * @note Maps utilized to trigger asynchronous events do not require manual
		 *       freeing and will be automatically freed post-event triggering.
		 */
		void (*DestroyDsMap)(int _index);

		// ########################################################################
		// STRUCT MANIPULATION
		// ########################################################################

		/**
		 * @brief Initializes a new structure in the given RValue.
		 *
		 * @param _pStruct A pointer to the RValue to initialize as a structure.
		 *
		 * Usage example:
		 *
		 *		RValue pStruct = {0};
		 *		StructCreate(&pStruct);
		 */
		void (*StructCreate)(RValue* _pStruct);

		/**
		 * @brief Adds a boolean value to the specified structure with the associated key.
		 *
		 * @param _pStruct A pointer to the structure.
		 * @param _pKey The key associated with the value to be added.
		 * @param _value The boolean value to add to the structure.
		 *
		 * Usage example:
		 *
		 *		StructAddBool(pStruct, "key", true);
		 */
		void (*StructAddBool)(RValue* _pStruct, const char* _pKey, bool _value);

		/**
		 * @brief Adds a double-precision floating-point value to the specified structure with the associated key.
		 *
		 * @param _pStruct A pointer to the structure.
		 * @param _pKey The key associated with the value to be added.
		 * @param _value The double value to add to the structure.
		 *
		 * Usage example:
		 *
		 *		StructAddDouble(pStruct, "key", 3.14);
		 */
		void (*StructAddDouble)(RValue* _pStruct, const char* _pKey, double _value);

		/**
		 * @brief Adds an integer value to the specified structure with the associated key.
		 *
		 * @param _pStruct A pointer to the structure.
		 * @param _pKey The key associated with the value to be added.
		 * @param _value The integer value to add to the structure.
		 *
		 * Usage example:
		 *
		 *		StructAddInt(pStruct, "key", 42);
		 */
		void (*StructAddInt)(RValue* _pStruct, const char* _pKey, int _value);

		/**
		 * @brief Adds an RValue to the specified structure with the associated key.
		 *
		 * @param _pStruct A pointer to the structure.
		 * @param _pKey The key associated with the value to be added.
		 * @param _pValue A pointer to the RValue to add to the structure.
		 *
		 * Usage example:
		 *
		 *		StructAddRValue(pStruct, "key", pValue);
		 */
		void (*StructAddRValue)(RValue* _pStruct, const char* _pKey, RValue* _pValue);

		/**
		 * @brief Adds a string value to the specified structure with the associated key.
		 *
		 * @param _pStruct A pointer to the structure.
		 * @param _pKey The key associated with the value to be added.
		 * @param _pValue The string value to add to the structure.
		 *
		 * Usage example:
		 *
		 *		StructAddString(pStruct, "key", "value");
		 */
		void (*StructAddString)(RValue* _pStruct, const char* _pKey, const char* _pValue);

		// ########################################################################
		// DIRECTORY MANIPULATION
		// ########################################################################

		bool (*WhitelistIsDirectoryIn)(const char* _pszDirectory);
		bool (*WhiteListIsFilenameIn)(const char* _pszFilename);
		void (*WhiteListAddTo)(const char* _pszFilename, bool _bIsDir);
		bool (*DirExists)(const char* filename);

		// ########################################################################
		// BUFFER ACCESS (ADV)
		// ########################################################################

		/**
		 * @brief Retrieves an `IBuffer` interface corresponding to a GML buffer.
		 *
		 * This function retrieves an `IBuffer` interface for a given GML buffer (specified by an index).
		 * The `IBuffer` interface itself isn't directly accessible through the C++ API but can be used with
		 * `BufferGet` to access the data of a GML buffer from the runner without copying the data.
		 *
		 * @param ind The index of the GML buffer.
		 *
		 * @return A pointer to an `IBuffer` interface struct.
		 */
		IBuffer* (*BufferGetFromGML)(int ind);

		/**
		 * @brief Gets the current read position within a buffer.
		 *
		 * This function returns the current position of the read cursor within a buffer.
		 * This is important to know to ensure that data is read from the correct place in the buffer.
		 *
		 * @param buff Pointer to the `IBuffer` interface struct.
		 *
		 * @return The current read position in the buffer.
		 */
		int (*BufferTELL)(IBuffer* buff);

		/**
		 * @brief Obtains the actual memory pointer to the data of a buffer.
		 *
		 * This function retrieves a pointer to the actual data stored in a buffer,
		 * without copying the data. This is useful for efficiently accessing the data
		 * without the overhead of copying it to a new location.
		 *
		 * @param buff Pointer to the `IBuffer` interface struct.
		 *
		 * @return A pointer to the actual memory location of the buffer's data.
		 *
		 * @note Manipulating data directly through this pointer will affect the
		 *       actual data in the buffer.
		 */
		unsigned char* (*BufferGet)(IBuffer* buff);

		const char* (*FilePrePend)(void);

		// ########################################################################
		// STRUCT MANIPULATION (PART 2)
		// ########################################################################

		/**
		 * @brief Adds a 32-bit integer value to the specified structure with the associated key.
		 *
		 * @param _pStruct A pointer to the structure.
		 * @param _pKey The key associated with the value to be added.
		 * @param _value The 32-bit integer value to add to the structure.
		 *
		 * Usage example:
		 *
		 *		StructAddInt32(pStruct, "key", 42);
		 */
		void (*StructAddInt32)(RValue* _pStruct, const char* _pKey, int32_t _value);

		/**
		 * @brief Adds a 64-bit integer value to the specified structure with the associated key.
		 *
		 * @param _pStruct A pointer to the structure.
		 * @param _pKey The key associated with the value to be added.
		 * @param _value The 64-bit integer value to add to the structure.
		 *
		 * Usage example:
		 *
		 *		StructAddInt64(pStruct, "key", 42);
		 */
		void (*StructAddInt64)(RValue* _pStruct, const char* _pKey, int64_t _value);

		/**
		 * @brief Retrieves a member RValue from the specified structure using the associated key.
		 *
		 * @param _pStruct A pointer to the structure.
		 * @param _pKey The key associated with the value to be retrieved.
		 *
		 * @return A pointer to the RValue associated with the specified key in the structure.
		 *
		 * Usage example:
		 *
		 *		RValue* value = StructGetMember(pStruct, "key");
		 *
		 * @note This function returns a pointer to an RValue and should be used with care.
		 *       The user should ensure the returned RValue is correctly interpreted and handled.
		 */
		RValue* (*StructGetMember)(RValue* _pStruct, const char* _pKey);

		/**
		 * @brief Query the keys in a struct.
		 *
		 * @param _pStruct  Pointer to a VALUE_OBJECT RValue.
		 * @param _keys     Pointer to an array of const char* pointers to receive the names.
		 * @param _count    Length of _keys (in elements) on input, number filled on output.
		 *
		 * @return Total number of keys in the struct.
		 *
		 * NOTE: The strings in _keys are owned by the runner. You do not need to free them, however
		 * you should make a copy if you intend to keep them around as the runner may invalidate them
		 * in the future when performing variable modifications.
		 *
		 * Usage example:
		 *
		 *    // Get total number of keys in struct
		 *    int num_keys = YYRunnerInterface_p->StructGetKeys(struct_rvalue, NULL, NULL);
		 *
		 *    // Fetch keys from struct
		 *    std::vector<const char*> keys(num_keys);
		 *    YYRunnerInterface_p->StructGetKeys(struct_rvalue, keys.data(), &num_keys);
		 *
		 *    // Loop over struct members
		 *    for(int i = 0; i < num_keys; ++i)
		 *    {
		 *        RValue *member = YYRunnerInterface_p->StructGetMember(struct_rvalue, keys[i]);
		 *        ...
		 *    }
		 */
		int (*StructGetKeys)(RValue* _pStruct, const char** _keys, int* _count);

		/**
		 * @brief Parses and retrieves a structure as an RValue from the argument array at the specified index.
		 *
		 * @param _pBase A pointer to the array of arguments.
		 * @param _index The index in the array from which to retrieve the structure.
		 *
		 * @return A pointer to the RValue representing the structure present at the specified index in the argument array.
		 *
		 * @note Care should be taken to ensure the retrieved RValue is indeed representing a structure, and appropriate
		 *       error handling should be implemented for cases where it might not be.
		 *
		 * Usage example:
		 *
		 *		RValue* value = YYGetStruct(pArgs, 3);
		 */
		RValue* (*YYGetStruct)(RValue* _pBase, int _index);

		// ########################################################################
		// EXTENSION OPTIONS
		// ########################################################################

		/**
		 * @brief Retrieves the value of a specified extension option as an RValue.
		 *
		 * @param result An RValue reference where the result will be stored.
		 * @param _ext The asset name of the extension whose option value needs to be retrieved.
		 * @param _opt The key associated with the extension option.
		 *
		 * Usage example:
		 *
		 *		extOptGetRValue(result, "MyExtension", "OptionKey");
		 *
		 * @note This function can be used to retrieve any type of value set by the extension user,
		 *       and developers should ensure the returned RValue is interpreted and handled correctly.
		 */
		void (*extOptGetRValue)(RValue& result, const char* _ext, const char* _opt);

		/**
		 * @brief Retrieves the value of a specified extension option as a string.
		 *
		 * @param _ext The asset name of the extension whose option value needs to be retrieved.
		 * @param _opt The key associated with the extension option.
		 *
		 * @return The string value of the specified extension option.
		 *
		 * Usage example:
		 *
		 *		const char* value = extOptGetString("MyExtension", "OptionKey");
		 */
		const char* (*extOptGetString)(const char* _ext, const char* _opt);

		/**
		 * @brief Retrieves the value of a specified extension option as a double.
		 *
		 * @param _ext The asset name of the extension whose option value needs to be retrieved.
		 * @param _opt The key associated with the extension option.
		 *
		 * @return The double value of the specified extension option.
		 *
		 * Usage example:
		 *
		 *		double value = extOptGetReal("MyExtension", "OptionKey");
		 */
		double (*extOptGetReal)(const char* _ext, const char* _opt);

		// ########################################################################
		// UTILITIES (PART 2)
		// ########################################################################

		/**
		 * @brief Determines whether the current game is being run from within the IDE.
		 *
		 * @return A boolean value representing whether the game is running from within the IDE.
		 *         Returns true if it is running from within the IDE, otherwise false.
		 *
		 * Usage example:
		 *
		 *		bool runningFromIDE = isRunningFromIDE();
		 *
		 * @note This function is particularly useful for implementing security checks, allowing
		 *       developers to conditionally enable or disable features based on the running environment
		 *       of the game. Developers should use this function judiciously to ensure the security and
		 *       integrity of the game.
		 */
		bool (*isRunningFromIDE)();

		/**
		 * @brief Retrieves the length of a specified YYArray.
		 *
		 * @param pRValue A pointer to the RValue representing the YYArray.
		 *
		 * @return The length of the specified YYArray.
		 *
		 * Usage example:
		 *
		 *		int length = YYArrayGetLength(pRValue);
		 *
		 * @note Before using this function, users should ensure that the provided RValue is indeed
		 *       of type VALUE_ARRAY (pRValue->kind == VALUE_ARRAY) to avoid undefined behavior.
		 *       Failing to confirm the type of RValue can lead to runtime errors or unexpected outcomes.
		 */
		int (*YYArrayGetLength)(RValue* pRValue);

		// ########################################################################
		// EXTENSIONS
		// ########################################################################

		/**
		 * @brief Retrieves the version of a specified extension in a "X.Y.Z" format.
		 *
		 * @param _ext The asset name of the extension whose version needs to be retrieved.
		 *
		 * @return The version of the specified extension as a string in "X.Y.Z" format.
		 *
		 * Usage example:
		 *
		 *		const char* version = extGetVersion("MyExtension");
		 *
		 * @note The returned string represents the version of the extension and can be used
		 *       to perform version checks or logging. Developers should be aware of the
		 *       format in which the version is returned and handle it appropriately.
		 */
		const char* (*extGetVersion)(const char* _ext);


		YYRunnerInterface()
		{
			memset(this, 0, sizeof(*this));
		}
	};

	// Not defined in YYTK_DEFINE_INTERNAL due to YYObjectBase not being defined yet
#if not YYTK_DEFINE_INTERNAL
	struct CInstance
	{
		// Overloaded operators
		RValue& operator[](
			IN std::string_view Element
			);

		// STL-like access
		RValue& at(
			IN std::string_view Element
		);
	};
#endif

	// ExecuteIt
	using FWCodeEvent = FunctionWrapper<bool(CInstance*, CInstance*, CCode*, int, RValue*)>;
	// IDXGISwapChain::Present
	using FWFrame = FunctionWrapper<HRESULT(IDXGISwapChain*, UINT, UINT)>;
	// IDXGISwapChain::ResizeBuffers
	using FWResize = FunctionWrapper<HRESULT(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)>;
	// WndProc calls
	using FWWndProc = FunctionWrapper<LRESULT(HWND, UINT, WPARAM, LPARAM)>;

	class YYTKInterface : public Aurie::AurieInterfaceBase
	{
	public:
		// === Interface Functions ===
		virtual Aurie::AurieStatus Create() = 0;

		virtual void Destroy() = 0;

		virtual void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		) = 0;

		virtual Aurie::AurieStatus GetNamedRoutineIndex(
			IN const char* FunctionName,
			OUT int* FunctionIndex
		) = 0;

		virtual Aurie::AurieStatus GetNamedRoutinePointer(
			IN const char* FunctionName,
			OUT PVOID* FunctionPointer
		) = 0;

		virtual Aurie::AurieStatus GetGlobalInstance(
			OUT CInstance** Instance
		) = 0;

		virtual RValue CallBuiltin(
			IN const char* FunctionName,
			IN std::vector<RValue> Arguments
		) = 0;

		virtual Aurie::AurieStatus CallBuiltinEx(
			OUT RValue& Result,
			IN const char* FunctionName,
			IN CInstance* SelfInstance,
			IN CInstance* OtherInstance,
			IN std::vector<RValue> Arguments
		) = 0;

		virtual void Print(
			IN CmColor Color,
			IN std::string_view Format,
			IN ...
		) = 0;

		virtual void PrintInfo(
			IN std::string_view Format,
			IN ...
		) = 0;

		virtual void PrintWarning(
			IN std::string_view Format,
			IN ...
		) = 0;

		virtual void PrintError(
			IN std::string_view Filepath,
			IN const int Line,
			IN std::string_view Format,
			IN ...
		) = 0;

		virtual Aurie::AurieStatus CreateCallback(
			IN Aurie::AurieModule* Module,
			IN EventTriggers Trigger,
			IN PVOID Routine,
			IN int32_t Priority
		) = 0;

		virtual Aurie::AurieStatus RemoveCallback(
			IN Aurie::AurieModule* Module,
			IN PVOID Routine
		) = 0;

		virtual Aurie::AurieStatus GetInstanceMember(
			IN RValue Instance,
			IN const char* MemberName,
			OUT RValue*& Member
		) = 0;

		virtual Aurie::AurieStatus EnumInstanceMembers(
			IN RValue Instance,
			IN std::function<bool(IN const char* MemberName, RValue* Value)> EnumFunction
		) = 0;

		virtual Aurie::AurieStatus RValueToString(
			IN const RValue& Value,
			OUT std::string& String
		) = 0;

		virtual Aurie::AurieStatus StringToRValue(
			IN const std::string_view String,
			OUT RValue& Value
		) = 0;

		virtual const YYRunnerInterface& GetRunnerInterface() = 0;

		virtual void InvalidateAllCaches() = 0;

		virtual Aurie::AurieStatus GetScriptData(
			IN int Index,
			OUT CScript*& Script
		) = 0;

		virtual Aurie::AurieStatus GetBuiltinVariableIndex(
			IN std::string_view Name,
			OUT size_t& Index
		) = 0;

		virtual Aurie::AurieStatus GetBuiltinVariableInformation(
			IN size_t Index,
			OUT RVariableRoutine*& VariableInformation
		) = 0;

		virtual Aurie::AurieStatus GetBuiltin(
			IN std::string_view Name,
			IN CInstance* TargetInstance,
			OPTIONAL IN int ArrayIndex,
			OUT RValue& Value
		) = 0;

		virtual Aurie::AurieStatus SetBuiltin(
			IN std::string_view Name,
			IN CInstance* TargetInstance,
			OPTIONAL IN int ArrayIndex,
			IN RValue& Value
		) = 0;

		virtual Aurie::AurieStatus GetArrayEntry(
			IN RValue& Value,
			IN size_t ArrayIndex,
			OUT RValue*& ArrayElement
		) = 0;

		virtual Aurie::AurieStatus GetArraySize(
			IN RValue& Value,
			OUT size_t& Size
		) = 0;

		virtual Aurie::AurieStatus GetRoomData(
			IN int32_t RoomID,
			OUT CRoom*& Room
		) = 0;

		virtual Aurie::AurieStatus GetCurrentRoomData(
			OUT CRoom*& CurrentRoom
		) = 0;

		virtual Aurie::AurieStatus GetInstanceObject(
			IN int32_t InstanceID,
			OUT CInstance*& Instance
		) = 0;

		virtual Aurie::AurieStatus InvokeWithObject(
			IN const RValue& Object,
			IN std::function<void(CInstance* Self, CInstance* Other)> Method
		) = 0;

		virtual Aurie::AurieStatus GetVariableSlot(
			IN const RValue& Object,
			IN const char* VariableName,
			OUT int32_t& Hash
		) = 0;
	};

#if YYTK_DEFINE_INTERNAL
	using CHashMapHash = uint32_t;

	template <typename TKey, typename TValue>
	struct CHashMapElement
	{
		TValue m_Value;
		TKey m_Key;
		CHashMapHash m_Hash;
	};

	template <typename TKey, typename TValue, int TInitialMask>
	struct CHashMap
	{
	private:
		// Typed functions for calculating hashes
		static CHashMapHash CHashMapCalculateHash(
			IN int Key
		)
		{
			return (Key * -0x61c8864f + 1) & INT_MAX;
		}

		static CHashMapHash CHashMapCalculateHash(
			IN YYObjectBase* Key
		)
		{
			return ((static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(Key)) >> 6) * 7 + 1) & INT_MAX;
		}

		static CHashMapHash CHashMapCalculateHash(
			IN void* Key
		)
		{
			return ((static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(Key)) >> 8) + 1) & INT_MAX;
		};

		static CHashMapHash CHashMapCalculateHash(
			IN const char* Key
		)
		{
			// https://github.com/jwerle/murmurhash.c - Licensed under MIT
			size_t len = strlen(Key);
			uint32_t c1 = 0xcc9e2d51;
			uint32_t c2 = 0x1b873593;
			uint32_t r1 = 15;
			uint32_t r2 = 13;
			uint32_t m = 5;
			uint32_t n = 0xe6546b64;
			uint32_t h = 0;
			uint32_t k = 0;
			uint8_t* d = (uint8_t*)Key; // 32 bit extract from 'key'
			const uint32_t* chunks = NULL;
			const uint8_t* tail = NULL; // tail - last 8 bytes
			int i = 0;
			int l = len / 4; // chunk length

			chunks = (const uint32_t*)(d + l * 4); // body
			tail = (const uint8_t*)(d + l * 4); // last 8 byte chunk of `key'

			// for each 4 byte chunk of `key'
			for (i = -l; i != 0; ++i) {
				// next 4 byte chunk of `key'
				k = chunks[i];

				// encode next 4 byte chunk of `key'
				k *= c1;
				k = (k << r1) | (k >> (32 - r1));
				k *= c2;

				// append to hash
				h ^= k;
				h = (h << r2) | (h >> (32 - r2));
				h = h * m + n;
			}

			k = 0;

			// remainder
			switch (len & 3) { // `len % 4'
			case 3: k ^= (tail[2] << 16);
			case 2: k ^= (tail[1] << 8);

			case 1:
				k ^= tail[0];
				k *= c1;
				k = (k << r1) | (k >> (32 - r1));
				k *= c2;
				h ^= k;
			}

			h ^= len;

			h ^= (h >> 16);
			h *= 0x85ebca6b;
			h ^= (h >> 13);
			h *= 0xc2b2ae35;
			h ^= (h >> 16);

			return h;
		}

	public:
		int32_t m_CurrentSize;
		int32_t m_UsedCount;
		int32_t m_CurrentMask;
		int32_t m_GrowThreshold;
		CHashMapElement<TKey, TValue>* m_Elements;
		void(*m_DeleteValue)(TKey* Key, TValue* Value);

		bool GetContainer(
			IN TKey Key,
			OUT CHashMapElement<TKey, TValue>*& Value
		)
		{
			CHashMapHash value_hash = CHashMapCalculateHash(Key);
			int32_t ideal_position = static_cast<int>(value_hash & m_CurrentMask);

			for (
				// Start at the ideal element (the value is probably not here though)
				CHashMapElement<TKey, TValue>& current_element = this->m_Elements[ideal_position];
				// Continue looping while the hash isn't 0 (meaning we reached the end of the map)
				current_element.m_Hash != 0;
				// Go to the next position
				current_element = this->m_Elements[(++ideal_position) & this->m_CurrentMask]
				)
			{
				if (current_element.m_Key != Key)
					continue;

				Value = &current_element;
				return true;
			}

			return false;
		}

		bool GetValue(
			IN TKey Key,
			OUT TValue& Value
		)
		{
			// Try to get the container
			CHashMapElement<TKey, TValue>* object_container = nullptr;
			if (!this->GetContainer(
				Key,
				object_container
			))
			{
				return false;
			}

			Value = object_container->m_Value;
			return true;
		}
	};

	// Newer struct, later renamed to LinkedList - OLinkedList is used in older x86 games, 
	// and causes misalingment due to alignment changing from 8-bytes in x64 to 4-bytes in x86.
	template <typename T>
	struct LinkedList
	{
		T* m_First;
		T* m_Last;
		int32_t m_Count;
		int32_t m_DeleteType;
	};
#ifdef _WIN64
	static_assert(sizeof(LinkedList<CInstance>) == 0x18);
#endif // _WIN64

	template <typename T>
	struct OLinkedList
	{
		T* m_First;
		T* m_Last;
		int32_t m_Count;
	};
#ifdef _WIN64
	static_assert(sizeof(OLinkedList<CInstance>) == 0x18);
	static_assert(sizeof(OLinkedList<CInstance>) == sizeof(LinkedList<CInstance>));
#endif // _WIN64

	enum eBuffer_Type : int32_t
	{
		eBuffer_None = 0x0,
		eBuffer_U8 = 0x1,
		eBuffer_S8 = 0x2,
		eBuffer_U16 = 0x3,
		eBuffer_S16 = 0x4,
		eBuffer_U32 = 0x5,
		eBuffer_S32 = 0x6,
		eBuffer_F16 = 0x7,
		eBuffer_F32 = 0x8,
		eBuffer_D64 = 0x9,
		eBuffer_Bool = 0xA,
		eBuffer_String = 0xB,
		eBuffer_U64 = 0xC,
		eBuffer_Text = 0xD,
	};

	enum eBuffer_Seek : int32_t
	{
		eBuffer_Start = 0x0,
		eBuffer_Relative = 0x1,
		eBuffer_End = 0x2,
	};

	struct IBuffer
	{
		virtual ~IBuffer() = 0;
		virtual int Write(eBuffer_Type _type, RValue* _pIn) = 0;
		virtual int WriteArray(eBuffer_Type _type, uint8_t* _pSrc, int Size) = 0;
		virtual int Read(eBuffer_Type _type, RValue* _pOut) = 0;
		virtual int Seek(eBuffer_Seek _type, int _val) = 0;
		virtual void Peek(int _offset, eBuffer_Type _type, RValue* _pOut) = 0;
		virtual void Poke(int _offset, eBuffer_Type _type, RValue* _pIn) = 0;
		virtual int Save(const char* _pURI, int _offset, int _size) = 0;
		virtual int Load(const char* _pURI, int _src_offset, int _src_size, int _dest_offset) = 0;
		virtual void Base64Encode(RValue* _pOut, int _offset, int _size) = 0;
		virtual void Base64Decode(const char* _pBASE64, int _offset, int _size) = 0;
		virtual void MD5(RValue* _pOut, int _offset, int _size) = 0;
		virtual void SHA1(RValue* _pOut, int _offset, int _size) = 0;
		virtual void Resize(int _newsize) = 0;
		virtual void Copy(int _src_offset, int _src_size, IBuffer* _pDest, int _dest_off) = 0;
		virtual void Fill(int _offset, int _size, eBuffer_Type _type, RValue* _pIn, int _stride, bool fill_gaps) = 0;
		virtual void GetSurface(int _surface) = 0;
		virtual void SetSurface(int _surface, int _offset) = 0;
		virtual uint8_t* Compress(int _offset, int _size, uint32_t& resultSize) = 0;
		virtual uint8_t* Decompress(uint32_t& resultSize) = 0;
	};
#ifdef _WIN64
	static_assert(sizeof(IBuffer) == 0x8);
#endif // _WIN64

	struct CLayerElementBase
	{
		int32_t m_Type;
		int32_t m_ID;
		bool m_RuntimeDataInitialized;
		const char* m_Name;
		CLayer* m_Layer;
		union
		{
			CLayerInstanceElement* m_InstanceFlink;
			CLayerSpriteElement* m_SpriteFlink;
			CLayerElementBase* m_Flink;
		};
		union
		{
			CLayerInstanceElement* m_InstanceBlink;
			CLayerSpriteElement* m_SpriteBlink;
			CLayerElementBase* m_Blink;
		};
	};
#ifdef _WIN64
	static_assert(sizeof(CLayerElementBase) == 0x30);
#endif // _WIN64

	struct CLayerInstanceElement : CLayerElementBase
	{
		int32_t m_InstanceID;
		CInstance* m_Instance;
	};
#ifdef _WIN64
	static_assert(sizeof(CLayerInstanceElement) == 0x40);
#endif // _WIN64

	struct CLayerSpriteElement : CLayerElementBase
	{
		int32_t m_SpriteIndex;
		float m_SequencePosition;
		float m_SequenceDirection;
		float m_ImageIndex;
		float m_ImageSpeed;
		int32_t m_SpeedType;
		float m_ImageScaleX;
		float m_ImageScaleY;
		float m_ImageAngle;
		uint32_t m_ImageBlend;
		float m_ImageAlpha;
		float m_X;
		float m_Y;
	};
#ifdef _WIN64
	static_assert(sizeof(CLayerSpriteElement) == 0x68);
#endif // _WIN64

	struct CLayer
	{
		int32_t m_Id;
		int32_t m_Depth;
		float m_XOffset;
		float m_YOffset;
		float m_HorizontalSpeed;
		float m_VerticalSpeed;
		bool m_Visible;
		bool m_Deleting;
		bool m_Dynamic;
		const char* m_Name;
		RValue m_BeginScript;
		RValue m_EndScript;
		bool m_EffectEnabled;
		bool m_EffectPendingEnabled;
		RValue m_Effect;
		CLayerEffectInfo* m_InitialEffectInfo;
		int32_t m_ShaderID;
		LinkedList<CLayerElementBase> m_Elements;
		CLayer* m_Flink;
		CLayer* m_Blink;
		PVOID m_GCProxy;
	};
#ifdef _WIN64
	static_assert(sizeof(CLayer) == 0xA0);
#endif // _WIN64

	// A representation of a room, as from the data.win file
	struct YYRoom
	{
		// The name of the room
		uint32_t m_NameOffset;
		// The caption of the room, legacy variable, used pre-GMS
		uint32_t m_Caption;
		// The width of the room
		int32_t m_Width;
		// The height of the room
		int32_t m_Height;
		// Speed of the room
		int32_t m_Speed;
		// Whether the room is persistent (UMT marks it as a bool, but it seems to be int32_t)
		int32_t m_Persistent;
		// The background color
		int32_t m_Color;
		// Whether to show the background color
		int32_t m_ShowColor;
		// Creation code of the room
		uint32_t m_CreationCode;
		int32_t m_EnableViews;
		uint32_t pBackgrounds;
		uint32_t pViews;
		uint32_t pInstances;
		uint32_t pTiles;
		int32_t m_PhysicsWorld;
		int32_t m_PhysicsWorldTop;
		int32_t m_PhysicsWorldLeft;
		int32_t m_PhysicsWorldRight;
		int32_t m_PhysicsWorldBottom;
		float m_PhysicsGravityX;
		float m_PhysicsGravityY;
		float m_PhysicsPixelToMeters;
	};
#ifdef _WIN64
	static_assert(sizeof(YYRoom) == 0x58);
#endif // _WIN64

	// Note: this is not how RValues store arrays
	template <typename T>
	struct CArrayStructure
	{
		int32_t Length;
		T* Array;
	};
#ifdef _WIN64
	static_assert(sizeof(CArrayStructure<int>) == 0x10);
#endif // _WIN64

	struct CRoomInternal
	{
		// CBackGM* m_Backgrounds[8];
		bool m_EnableViews;
		bool m_ClearScreen;
		bool m_ClearDisplayBuffer;
		CViewGM* m_Views[8];
		const char* m_LegacyCode;
		CCode* m_CodeObject;
		bool m_HasPhysicsWorld;
		int32_t m_PhysicsGravityX;
		int32_t m_PhysicsGravityY;
		float m_PhysicsPixelToMeters;
		OLinkedList<CInstance> m_ActiveInstances;
		LinkedList<CInstance> m_InactiveInstances;
		CInstance* m_MarkedFirst;
		CInstance* m_MarkedLast;
		int32_t* m_CreationOrderList;
		int32_t m_CreationOrderListSize;
		YYRoom* m_WadRoom;
		PVOID m_WadBaseAddress;
		CPhysicsWorld* m_PhysicsWorld;
		int32_t m_TileCount;
		CArrayStructure<RTile> m_Tiles;
		YYRoomTiles* m_WadTiles;
		YYRoomInstances* m_WadInstances;
		const char* m_Name;
		bool m_IsDuplicate;
		LinkedList<CLayer> m_Layers;
		CHashMap<int32_t, CLayer*, 7> m_LayerLookup;
		CHashMap<int32_t, CLayerElementBase*, 7> m_LayerElementLookup;
		CLayerElementBase* m_LastElementLookedUp;
		CHashMap<int32_t, CLayerInstanceElement*, 7> m_InstanceElementLookup;
		int32_t* m_SequenceInstanceIDs;
		int32_t m_SequenceInstanceIdCount;
		int32_t m_SequenceInstanceIdMax;
		int32_t* m_EffectLayerIDs;
		int32_t m_EffectLayerIdCount;
		int32_t m_EffectLayerIdMax;
	};

	// Seems to be mostly stable, some elements at the end are however omitted
	struct CRoom
	{
		int32_t m_LastTile;
		CRoom* m_InstanceHandle;
		const char* m_Caption;
		int32_t m_Speed;
		int32_t m_Width;
		int32_t m_Height;
		bool m_Persistent;
		uint32_t m_Color;
		bool m_ShowColor;
	private:

		// Last confirmed use in 2023.8, might be later even
		struct
		{
			CBackGM* Backgrounds[8];
			CRoomInternal Internals;
		} WithBackgrounds;

		// 2024.6 (first confirmed use) has Backgrounds removed.
		// CRoomInternal cannot be properly aligned (due to bool having 1-byte alignment),
		// so GetMembers() crafts the pointer manually instead of having a defined struct here.

	public:
		CRoomInternal& GetMembers();
	};
#ifdef _WIN64
	static_assert(sizeof(CRoom) == 0x218);
#endif // _WIN64

	struct CInstanceBase
	{
		virtual ~CInstanceBase() = 0;

		RValue* m_YYVars;
	};
#ifdef _WIN64
	static_assert(sizeof(CInstanceBase) == 0x10);
#endif // _WIN64

	enum EJSRetValBool : int32_t
	{
		EJSRVB_FALSE,
		EJSRVB_TRUE,
		EJSRVB_TYPE_ERROR
	};

	using FNGetOwnProperty = void(*)(
		IN YYObjectBase* Object,
		OUT RValue& Result,
		IN const char* Name
		);

	using FNDeleteProperty = void(*)(
		IN YYObjectBase* Object,
		OUT RValue& Result,
		IN const char* Name,
		IN bool ThrowOnError
		);

	using FNDefineOwnProperty = EJSRetValBool(*)(
		IN YYObjectBase* Object,
		IN const char* Name,
		OUT RValue& Result,
		IN bool ThrowOnError
		);

	enum YYObjectKind : int32_t
	{
		OBJECT_KIND_YYOBJECTBASE = 0,
		OBJECT_KIND_CINSTANCE,
		OBJECT_KIND_ACCESSOR,
		OBJECT_KIND_SCRIPTREF,
		OBJECT_KIND_PROPERTY,
		OBJECT_KIND_ARRAY,
		OBJECT_KIND_WEAKREF,
		OBJECT_KIND_CONTAINER,
		OBJECT_KIND_SEQUENCE,
		OBJECT_KIND_SEQUENCEINSTANCE,
		OBJECT_KIND_SEQUENCETRACK,
		OBJECT_KIND_SEQUENCECURVE,
		OBJECT_KIND_SEQUENCECURVECHANNEL,
		OBJECT_KIND_SEQUENCECURVEPOINT,
		OBJECT_KIND_SEQUENCEKEYFRAMESTORE,
		OBJECT_KIND_SEQUENCEKEYFRAME,
		OBJECT_KIND_SEQUENCEKEYFRAMEDATA,
		OBJECT_KIND_SEQUENCEEVALTREE,
		OBJECT_KIND_SEQUENCEEVALNODE,
		OBJECT_KIND_SEQUENCEEVENT,
		OBJECT_KIND_NINESLICE,
		OBJECT_KIND_FILTERHOST,
		OBJECT_KIND_EFFECTINSTANCE,
		OBJECT_KIND_SKELETON_SKIN,
		OBJECT_KIND_AUDIOBUS,
		OBJECT_KIND_AUDIOEFFECT,
		OBJECT_KIND_MAX
	};

	struct YYObjectBase : CInstanceBase
	{
		virtual RValue& InternalGetYYVarRef(
			IN int Index
		) = 0;

		virtual RValue& InternalGetYYVarRefL(
			IN int Index
		) = 0;

		virtual bool Mark4GC(
			uint32_t*,
			int
		) = 0;

		virtual bool MarkThisOnly4GC(
			uint32_t*,
			int
		) = 0;

		virtual bool MarkOnlyChildren4GC(
			uint32_t*,
			int
		) = 0;

		virtual void Free(
			bool preserve_map
		) = 0;

		virtual void ThreadFree(
			bool preserve_map,
			PVOID GCContext
		) = 0;

		virtual void PreFree() = 0;

		virtual RValue* GetDispose() = 0;

		bool Add(
			IN const char* Name,
			IN const RValue& Value,
			IN int Flags
		);

		bool IsExtensible();

		RValue* FindOrAllocValue(
			IN const char* Name
		);

		YYObjectBase* m_Flink;
		YYObjectBase* m_Blink;
		YYObjectBase* m_Prototype;
		const char* m_Class;
		FNGetOwnProperty m_GetOwnProperty;
		FNDeleteProperty m_DeleteProperty;
		FNDefineOwnProperty m_DefineOwnProperty;
		// Use GetInstanceMember instead
		CHashMap<int32_t, RValue*, 3>* m_YYVarsMap;
		CWeakRef** m_WeakRef;
		uint32_t m_WeakRefCount;
		uint32_t m_VariableCount;
		uint32_t m_Flags;
		uint32_t m_Capacity;
		uint32_t m_Visited;
		uint32_t m_VisitedGC;
		int32_t m_GCGeneration;
		int32_t m_GCCreationFrame;
		int32_t m_Slot;
		YYObjectKind m_ObjectKind;
		int32_t m_RValueInitType;
		int32_t m_CurrentSlot;
	};
#ifdef _WIN64
	static_assert(sizeof(YYObjectBase) == 0x88);
#endif // _WIN64

	struct CScriptRef : YYObjectBase
	{
		CScript* m_CallScript;
		TRoutine m_CppCall;
		PFUNC_YYGMLScript m_CallYYC;
		RValue m_Scope;
		RValue m_BoundThis;
		YYObjectBase* m_Static;
		PVOID m_HasInstance;
		PVOID m_Construct;
		const char* m_Tag;
	};
#ifdef _WIN64
	static_assert(sizeof(CScriptRef) == 0xE0);
#endif // _WIN64

	struct CPhysicsDataGM
	{
		float* m_PhysicsVertices;
		bool m_IsPhysicsObject;
		bool m_IsPhysicsSensor;
		bool m_IsPhysicsAwake;
		bool m_IsPhysicsKinematic;
		int m_PhysicsShape;
		int m_PhysicsGroup;
		float m_PhysicsDensity;
		float m_PhysicsRestitution;
		float m_PhysicsLinearDamping;
		float m_PhysicsAngularDamping;
		float m_PhysicsFriction;
		int m_PhysicsVertexCount;
	};
#ifdef _WIN64
	static_assert(sizeof(CPhysicsDataGM) == 0x30);
#endif // _WIN64

	struct CEvent
	{
		CCode* m_Code;
		int32_t m_OwnerObjectID;
	};
#ifdef _WIN64
	static_assert(sizeof(CEvent) == 0x10);
#endif // _WIN64

	struct CObjectGM
	{
		const char* m_Name;
		CObjectGM* m_ParentObject;
		CHashMap<int, CObjectGM*, 2>* m_ChildrenMap;
		CHashMap<int, CEvent*, 3>* m_EventsMap;
		CPhysicsDataGM m_PhysicsData;
		LinkedList<CInstance> m_Instances;
		LinkedList<CInstance> m_InstancesRecursive;
		uint32_t m_Flags;
		int32_t m_SpriteIndex;
		int32_t m_Depth;
		int32_t m_Parent;
		int32_t m_Mask;
		int32_t m_ID;
	};
#ifdef _WIN64
	static_assert(sizeof(CObjectGM) == 0x98);
#endif // _WIN64

	struct GCObjectContainer : YYObjectBase
	{
		CHashMap<YYObjectBase*, YYObjectBase*, 3>* m_YYObjectMap;
	};
#ifdef _WIN64
	static_assert(sizeof(GCObjectContainer) == 0x90);
#endif // _WIN64

	struct YYRECT
	{
		float m_Left;
		float m_Top;
		float m_Right;
		float m_Bottom;
	};

	// Not a runner type per-se, used to prevent code duplication in CInstance unions
	struct CInstanceInternal
	{
		uint32_t m_InstanceFlags;
		int32_t m_ID;
		int32_t m_ObjectIndex;
		int32_t m_SpriteIndex;
		float m_SequencePosition;
		float m_LastSequencePosition;
		float m_SequenceDirection;
		float m_ImageIndex;
		float m_ImageSpeed;
		float m_ImageScaleX;
		float m_ImageScaleY;
		float m_ImageAngle;
		float m_ImageAlpha;
		uint32_t m_ImageBlend;
		float m_X;
		float m_Y;
		float m_XStart;
		float m_YStart;
		float m_XPrevious;
		float m_YPrevious;
		float m_Direction;
		float m_Speed;
		float m_Friction;
		float m_GravityDirection;
		float m_Gravity;
		float m_HorizontalSpeed;
		float m_VerticalSpeed;
		YYRECT m_BoundingBox;
		int m_Timers[12];
		int64_t m_RollbackFrameKilled;
		PVOID m_TimelinePath;
		CCode* m_InitCode;
		CCode* m_PrecreateCode;
		CObjectGM* m_OldObject;
		int32_t m_LayerID;
		int32_t m_MaskIndex;
		int16_t m_MouseOverCount;
		CInstance* m_Flink;
		CInstance* m_Blink;
	};
#ifdef _WIN64
	static_assert(sizeof(CInstanceInternal) == 0xF8);
#endif // _WIN64

	struct CInstance : YYObjectBase
	{
		int64_t m_CreateCounter;
		CObjectGM* m_Object;
		CPhysicsObject* m_PhysicsObject;
		CSkeletonInstance* m_SkeletonAnimation;

	private:
		// Structs misalign between 2022.1 and 2023.8
		// Easy way to check which to use is to check m_ID and compare
		// it to the result of GetBuiltin("id") on the same instance.
		// Use GetMembers() to get a CInstanceVariables reference.
		union
		{
			// Islets 1.0.0.3 Steam (x86), GM 2022.6
			struct
			{
			public:
				CInstanceInternal Members;
			} MembersOnly;
#ifdef _WIN64
			static_assert(sizeof(MembersOnly) == 0xF8);
#endif // _WIN64

			// 2023.x => 2023.8 (and presumably 2023.11)
			struct
			{
			private:
				PVOID m_SequenceInstance;
			public:
				CInstanceInternal Members;
			} SequenceInstanceOnly;
#ifdef _WIN64
			static_assert(sizeof(SequenceInstanceOnly) == 0x100);
#endif // _WIN64

			// 2022.1 => 2023.1 (may be used later, haven't checked)
			struct
			{
			private:
				PVOID m_SkeletonMask;
				PVOID m_SequenceInstance;
			public:
				CInstanceInternal Members;
			} WithSkeletonMask;
#ifdef _WIN64
			static_assert(sizeof(WithSkeletonMask) == 0x108);
#endif // _WIN64
		};
	public:

		CInstanceInternal& GetMembers();

		// Overloaded operators
		RValue& operator[](
			IN std::string_view Element
			);

		// STL-like access
		RValue& at(
			IN std::string_view Element
		);
	};
	// sizeof(0x1A8) is for PreMasked instances
	// sizeof(0x1B0) is for Masked instances
#ifdef _WIN64
	static_assert(sizeof(CInstance) == 0x1A8 || sizeof(CInstance) == 0x1B0);
#endif // _WIN64

#endif // YYTK_DEFINE_INTERNAL
}

#endif // YYTK_SHARED_H_