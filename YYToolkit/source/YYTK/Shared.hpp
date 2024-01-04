// File: Shared.hpp
// 
// Defines stuff shared between the tool and its plugins.
// Structs are meant to be opaque here, unless YYTK_INCLUDE_PRIVATE is defined.
// YYTK_INCLUDE_PRIVATE is set to 1 in the main YYTK project through Visual Studio's Project Properties.

#ifndef YYTK_SHARED_H_
#define YYTK_SHARED_H_

#define YYTK_MAJOR 3
#define YYTK_MINOR 1
#define YYTK_PATCH 3

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

		RValue(
			IN const char8_t* Value
		);

		RValue(
			IN std::string_view Value
		);

		RValue(
			IN std::u8string_view Value
		);

		RValue(
			IN const std::string& Value
		);

		RValue(
			IN const std::u8string& Value
		);

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

		int (*YYArrayGetLength)(RValue* _pArray);

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
			IN RValue InstanceID,
			OUT CInstance*& Instance
		) = 0;
	};

#if YYTK_DEFINE_INTERNAL
	template <typename T>
	struct LinkedList
	{
		T* m_First;
		T* m_Last;
		int32_t m_Count;
	};
	static_assert(sizeof(LinkedList<CInstance>) == 0x18);

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
	static_assert(sizeof(CLayerElementBase) == 0x30);

	struct CLayerInstanceElement : CLayerElementBase
	{
		int32_t m_InstanceID;
		CInstance* m_Instance;
	};
	static_assert(sizeof(CLayerInstanceElement) == 0x40);

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
	static_assert(sizeof(CLayerSpriteElement) == 0x68);

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
	static_assert(sizeof(CLayer) == 0xA0);

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
	static_assert(sizeof(YYRoom) == 0x58);

	// Note: this is not how RValues store arrays
	template <typename T>
	struct CArrayStructure
	{
		int32_t Length;
		T* Array;
	};
	static_assert(sizeof(CArrayStructure<int>) == 0x10);

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
		CBackGM* m_Backgrounds[8];
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
		LinkedList<CInstance> m_ActiveInstances;
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
	};

	struct CInstanceBase
	{
		virtual ~CInstanceBase() = 0;

		virtual RValue& InternalGetYYVarRef(
			IN int Index
		) = 0;

		virtual RValue& InternalGetYYVarRefL(
			IN int Index
		) = 0;

		RValue* m_YYVars;
	};
	static_assert(sizeof(CInstanceBase) == 0x10);

	enum EJSRetValBool : int
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
		YYObjectBase* m_Flink;
		YYObjectBase* m_Blink;
		YYObjectBase* m_Prototype;
		const char* m_Class;
		FNGetOwnProperty m_GetOwnProperty;
		FNDeleteProperty m_DeleteProperty;
		FNDefineOwnProperty m_DefineOwnProperty;
		PVOID m_YYVarsMap; // Use GetInstanceMember
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
	static_assert(sizeof(YYObjectBase) == 0x88);

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
	static_assert(sizeof(CScriptRef) == 0xE0);

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
	static_assert(sizeof(CPhysicsDataGM) == 0x30);

	struct CObjectGM
	{
		const char* m_Name;
		CObjectGM* m_ParentObject;
		PVOID m_ChildrenMap; // CHashMap<int, CObjectGM*, 2>
		PVOID m_EventsMap; // CHashMap<ULONGLONG, CEvent*, 3>
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
	static_assert(sizeof(CObjectGM) == 0x98);

	struct GCObjectContainer : YYObjectBase
	{
		PVOID m_yyObjMap; // CHashMap<YYObjectBase*, YYObjectBase*, 3>
	};
	static_assert(sizeof(GCObjectContainer) == 0x90);

	struct CInstance : YYObjectBase
	{
		int64_t m_CreateCounter;
		CObjectGM* m_Object;
		CPhysicsObject* m_PhysicsObject;
		CSkeletonInstance* m_SkeletonAnimation;
		// Structs misalign between 2022.1 and 2023.8 - omitting members

		// Overloaded operators
		RValue& operator[](
			IN std::string_view Element
			);

		// STL-like access
		RValue& at(
			IN std::string_view Element
		);
	};
	static_assert(sizeof(CInstance) == 0xA8);

#endif // YYTK_DEFINE_INTERNAL
}

#endif // YYTK_SHARED_H_