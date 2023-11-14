// File: Shared.hpp
// 
// Defines stuff shared between the tool and its plugins.
// Structs are meant to be opaque here, unless YYTK_INCLUDE_PRIVATE is defined.
// YYTK_INCLUDE_PRIVATE is set to 1 in the main YYTK project through Visual Studio's Project Properties.

#ifndef YYTK_SHARED_H_
#define YYTK_SHARED_H_

#include <Aurie/shared.hpp>
#include <FunctionWrapper/FunctionWrapper.hpp>
#include <d3d11.h>

#include <cstdint>
#include <string>

#ifndef UTEXT
#define UTEXT(x) ((const unsigned char*)(x))
#endif // UTEXT

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

	// These cannot be bitwise-operated on anymore
	enum EventTriggers : uint32_t
	{
		EVENT_OBJECT_CALL = 1,	// The event represents a Code_Execute() call.
		EVENT_FRAME = 2,		// The event represents an IDXGISwapChain::Present() call.
		EVENT_SCRIPT_CALL = 3,	// The event represents a DoCallScript() call.
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

	struct IBuffer;
	struct CInstance;
	struct YYObjectBase;
	struct YYRunnerInterface;
	struct RValue;

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

	using PFUNC_YYGMLScript = RValue * (*)(
		IN CInstance* Self,
		IN CInstance* Other,
		OUT RValue* ReturnValue,
		IN int ArgumentCount,
		IN RValue** Arguments
		);

#pragma pack(push, 4)
	struct RValue
	{
		union
		{
			int32_t m_i32;
			int64_t m_i64;
			double m_Real;

			PVOID m_Pointer;
		};

		unsigned int m_Flags;
		RValueType m_Kind;

		RValue()
		{
			this->m_Real = 0;
			this->m_Flags = 0;
			this->m_Kind = VALUE_UNSET;
		}
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
			PFUNC_YYGML m_Function;
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

	template <typename TFunction>
	using FNCallbackRoutine = void(*)(
		IN FunctionWrapper<TFunction>& Context
	);

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

	// ExecuteIt
	using FWCodeEvent = FNCallbackRoutine<bool(CInstance*, CInstance*, CCode*, int, RValue*)>;
	// IDXGISwapChain::Present
	using FWFrame = FNCallbackRoutine<HRESULT(IDXGISwapChain*, UINT, UINT)>;
	// DoCallScript (only in VM)
	using FWScriptEvent = FNCallbackRoutine<PVOID(CScript*)>;

	class YYTKInterface : public Aurie::AurieInterfaceBase
	{
	public:
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
			IN const char* Format,
			IN ...
		) = 0;

		virtual void PrintInfo(
			IN const char* Format,
			IN ...
		) = 0;

		virtual void PrintWarning(
			IN const char* Format,
			IN ...
		) = 0;

		virtual void PrintError(
			IN const char* Function,
			IN const int Line,
			IN const char* Format,
			IN ...
		) = 0;

		virtual Aurie::AurieStatus CreateCallback(
			IN const Aurie::AurieModule* Module,
			IN EventTriggers Trigger,
			IN PVOID Routine
		) = 0;

		virtual Aurie::AurieStatus RemoveCallback(
			IN const Aurie::AurieModule* Module,
			IN PVOID Routine
		) = 0;

		virtual Aurie::AurieStatus InvalidateAllCaches() = 0;
	};
}

#endif // YYTK_SHARED_H_