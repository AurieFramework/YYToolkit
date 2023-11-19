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
#include <functional>

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
		EVENT_RESIZE = 3,		// The event represents an IDXGISwapChain::ResizeBuffers() call.
		EVENT_SCRIPT_CALL = 4,	// The event represents a DoCallScript() call.
		EVENT_WNDPROC = 5		// The event represents a WndProc() call.
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
	class YYTKInterface;
	struct RValue;

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

			CInstance* m_Object;
			PVOID m_Pointer;
		};

		unsigned int m_Flags;
		RValueType m_Kind;

		// Constructors
		RValue();

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
			IN std::string_view Value,
			IN YYTKInterface* Interface
		);

		bool AsBool() const;

		double AsReal() const;

		std::string_view AsString(
			IN YYTKInterface* Interface
		);
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

	// https://github.com/YoYoGames/GMEXT-Steamworks/blob/main/source/Steamworks_vs/Steamworks/Extension_Interface.h#L106
	struct YYRunnerInterface
	{
		// basic interaction with the user
		void (*DebugConsoleOutput)(const char* fmt, ...); // hook to YYprintf
		void (*ReleaseConsoleOutput)(const char* fmt, ...);
		void (*ShowMessage)(const char* msg);

		// for printing error messages
		void (*YYError)(const char* _error, ...);

		// alloc, realloc and free
		void* (*YYAlloc)(int _size);
		void* (*YYRealloc)(void* pOriginal, int _newSize);
		void  (*YYFree)(const void* p);
		const char* (*YYStrDup)(const char* _pS);

		// yyget* functions for parsing arguments out of the arg index
		bool (*YYGetBool)(const RValue* _pBase, int _index);
		float (*YYGetFloat)(const RValue* _pBase, int _index);
		double (*YYGetReal)(const RValue* _pBase, int _index);
		int32_t(*YYGetInt32)(const RValue* _pBase, int _index);
		uint32_t(*YYGetUint32)(const RValue* _pBase, int _index);
		int64_t(*YYGetInt64)(const RValue* _pBase, int _index);
		void* (*YYGetPtr)(const RValue* _pBase, int _index);
		intptr_t(*YYGetPtrOrInt)(const RValue* _pBase, int _index);
		const char* (*YYGetString)(const RValue* _pBase, int _index);

		// typed get functions from a single rvalue
		bool (*BOOL_RValue)(const RValue* _pValue);
		double (*REAL_RValue)(const RValue* _pValue);
		void* (*PTR_RValue)(const RValue* _pValue);
		int64_t(*INT64_RValue)(const RValue* _pValue);
		int32_t(*INT32_RValue)(const RValue* _pValue);

		// calculate hash values from an RValue
		int (*HASH_RValue)(const RValue* _pValue);

		// copying, setting and getting RValue
		void (*SET_RValue)(RValue* _pDest, RValue* _pV, YYObjectBase* _pPropSelf, int _index);
		bool (*GET_RValue)(RValue* _pRet, RValue* _pV, YYObjectBase* _pPropSelf, int _index, bool fPrepareArray, bool fPartOfSet);
		void (*COPY_RValue)(RValue* _pDest, const RValue* _pSource);
		int (*KIND_RValue)(const RValue* _pValue);
		void (*FREE_RValue)(RValue* _pValue);
		void (*YYCreateString)(RValue* _pVal, const char* _pS);

		void (*YYCreateArray)(RValue* pRValue, int n_values, const double* values);

		// finding and running user scripts from name
		int (*Script_Find_Id)(const char* name);
		bool (*Script_Perform)(int ind, CInstance* selfinst, CInstance* otherinst, int argc, RValue* res, RValue* arg);

		// finding builtin functions
		bool  (*Code_Function_Find)(const char* name, int* ind);

		// http functions
		void (*HTTP_Get)(const char* _pFilename, int _type, PFUNC_async _async, PFUNC_cleanup _cleanup, void* _pV);
		void (*HTTP_Post)(const char* _pFilename, const char* _pPost, PFUNC_async _async, PFUNC_cleanup _cleanup, void* _pV);
		void (*HTTP_Request)(const char* _url, const char* _method, const char* _headers, const char* _pBody, PFUNC_async _async, PFUNC_cleanup _cleanup, void* _pV, int _contentLength);

		// sprite loading helper functions
		int (*ASYNCFunc_SpriteAdd)(HTTP_REQ_CONTEXT* _pContext, void* _p, int* _pMap);
		void (*ASYNCFunc_SpriteCleanup)(HTTP_REQ_CONTEXT* _pContext);
		HSPRITEASYNC(*CreateSpriteAsync)(int* _pSpriteIndex, int _xOrig, int _yOrig, int _numImages, int _flags);

		// timing
		int64_t(*Timing_Time)(void);
		void (*Timing_Sleep)(int64_t slp, bool precise);

		// mutex handling
		HYYMUTEX(*YYMutexCreate)(const char* _name);
		void (*YYMutexDestroy)(HYYMUTEX hMutex);
		void (*YYMutexLock)(HYYMUTEX hMutex);
		void (*YYMutexUnlock)(HYYMUTEX hMutex);

		// ds map manipulation for 
		void (*CreateAsyncEventWithDSMap)(int _map, int _event);
		void (*CreateAsyncEventWithDSMapAndBuffer)(int _map, int _buffer, int _event);
		int (*CreateDsMap)(int _num, ...);

		bool (*DsMapAddDouble)(int _index, const char* _pKey, double value);
		bool (*DsMapAddString)(int _index, const char* _pKey, const char* pVal);
		bool (*DsMapAddInt64)(int _index, const char* _pKey, int64_t value);

		// buffer access
		bool (*BufferGetContent)(int _index, void** _ppData, int* _pDataSize);
		int (*BufferWriteContent)(int _index, int _dest_offset, const void* _pSrcMem, int _size, bool _grow, bool _wrap);
		int (*CreateBuffer)(int _size, eBuffer_Format _bf, int _alignment);

		// variables
		volatile bool* pLiveConnection;
		int* pHTTP_ID;

		int (*DsListCreate)();
		void (*DsMapAddList)(int _dsMap, const char* _key, int _listIndex);
		void (*DsListAddMap)(int _dsList, int _mapIndex);
		void (*DsMapClear)(int _dsMap);
		void (*DsListClear)(int _dsList);

		bool (*BundleFileExists)(const char* _pszFileName);
		bool (*BundleFileName)(char* _name, int _size, const char* _pszFileName);
		bool (*SaveFileExists)(const char* _pszFileName);
		bool (*SaveFileName)(char* _name, int _size, const char* _pszFileName);

		bool (*Base64Encode)(const void* input_buf, size_t input_len, void* output_buf, size_t output_len);

		void (*DsListAddInt64)(int _dsList, int64_t _value);

		void (*AddDirectoryToBundleWhitelist)(const char* _pszFilename);
		void (*AddFileToBundleWhitelist)(const char* _pszFilename);
		void (*AddDirectoryToSaveWhitelist)(const char* _pszFilename);
		void (*AddFileToSaveWhitelist)(const char* _pszFilename);

		const char* (*KIND_NAME_RValue)(const RValue* _pV);

		void (*DsMapAddBool)(int _index, const char* _pKey, bool value);
		void (*DsMapAddRValue)(int _index, const char* _pKey, RValue* value);
		void (*DestroyDsMap)(int _index);

		void (*StructCreate)(RValue* _pStruct);
		void (*StructAddBool)(RValue* _pStruct, const char* _pKey, bool _value);
		void (*StructAddDouble)(RValue* _pStruct, const char* _pKey, double _value);
		void (*StructAddInt)(RValue* _pStruct, const char* _pKey, int _value);
		void (*StructAddRValue)(RValue* _pStruct, const char* _pKey, RValue* _pValue);
		void (*StructAddString)(RValue* _pStruct, const char* _pKey, const char* _pValue);

		bool (*WhitelistIsDirectoryIn)(const char* _pszDirectory);
		bool (*WhiteListIsFilenameIn)(const char* _pszFilename);
		void (*WhiteListAddTo)(const char* _pszFilename, bool _bIsDir);
		bool (*DirExists)(const char* filename);
		IBuffer* (*BufferGetFromGML)(int ind);
		int (*BufferTELL)(IBuffer* buff);
		unsigned char* (*BufferGet)(IBuffer* buff);
		const char* (*FilePrePend)(void);

		void (*StructAddInt32)(RValue* _pStruct, const char* _pKey, int32_t _value);
		void (*StructAddInt64)(RValue* _pStruct, const char* _pKey, int64_t _value);
		RValue* (*StructGetMember)(RValue* _pStruct, const char* _pKey);

		int (*StructGetKeys)(RValue* _pStruct, const char** _keys, int* _count);

		RValue* (*YYGetStruct)(RValue* _pBase, int _index);

		void (*extOptGetRValue)(RValue& result, const char* _ext, const  char* _opt);
		const char* (*extOptGetString)(const char* _ext, const  char* _opt);
		double (*extOptGetReal)(const char* _ext, const char* _opt);

		bool (*isRunningFromIDE)();
	};

	// ExecuteIt
	using FWCodeEvent = FunctionWrapper<bool(CInstance*, CInstance*, CCode*, int, RValue*)>;
	// IDXGISwapChain::Present
	using FWFrame = FunctionWrapper<HRESULT(IDXGISwapChain*, UINT, UINT)>;
	// IDXGISwapChain::ResizeBuffers
	using FWResize = FunctionWrapper<HRESULT(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)>;
	// DoCallScript (only in VM)
	using FWScriptEvent = FunctionWrapper<PVOID(CScript*, int, char*, PVOID, CInstance*, CInstance*)>;
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
	};
}

#endif // YYTK_SHARED_H_