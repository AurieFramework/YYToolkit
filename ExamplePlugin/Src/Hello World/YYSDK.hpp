#pragma once

#ifdef __cplusplus
#define DllExport extern "C" __declspec(dllexport)
#else //!__cplusplus
#define DllExport __declspec(dllexport)
#endif //__cplusplus

#ifdef _MSC_VER
#pragma warning(disable : 26812)
#define _CRT_SECURE_NO_WARNINGS 1
#define alignedTo(x) __declspec(align(x))
#else //!MSC_VER
#define alignedTo(x) __attribute__((aligned (x)))
//#define strcpy_s(x,y,z) strncpy(x,z,y)
#include <inttypes.h>
#endif //MSC_VER

#include <cstring>

#define YYTK_MAGIC 'TFSI'

// Enums
enum YYTKStatus : int
{
	YYTK_OK = 0,				// The operation completed successfully.
	YYTK_FAIL = 1,				// Unspecified error occured, see source code.
	YYTK_UNAVAILABLE = 2,		// The called function is not available in the current context.
	YYTK_NO_MEMORY = 3,			// No more memory is available to the process.
	YYTK_NOT_FOUND = 4,			// The specified value could not be found.
	YYTK_NOT_IMPLEMENTED = 5,	// The specified function doesn't exist. (IPC error)
	YYTK_INVALID = 6			// One or more arguments were invalid.
};

enum b2BodyType : int
{
	b2_staticBody = 0x0,
	b2_kinematicBody = 0x1,
	b2_dynamicBody = 0x2,
};

enum EJSRetValBool : int
{
	EJSRVB_FALSE = 0x0,
	EJSRVB_TRUE = 0x1,
	EJSRVB_TYPE_ERROR = 0x2,
};

enum eGML_TYPE : unsigned int
{
	eGMLT_NONE = 0x0,
	eGMLT_ERROR = 0xFFFF0000,
	eGMLT_DOUBLE = 0x1,
	eGMLT_STRING = 0x2,
	eGMLT_INT32 = 0x4,
};

// Forward Declarations
struct YYRValue;
struct CInstanceBase;
struct YYObjectBase;
template <typename, typename>
struct CHashMap;
struct yyMatrix;
struct YYRect;
struct SLink;
struct CInstance;
struct CWeakRef;
template <typename T>
struct CArray;
template <typename T>
struct CDynamicArrayRef;
struct RToken;
struct YYVAR;
struct SYYStackTrace;
struct SLLVMVars;
struct YYGMLFuncs;
struct VMBuffer;
struct YYTKPlugin;
struct CCode;

typedef void (*TRoutine)(YYRValue* Result, YYObjectBase* Self, YYObjectBase* Other, int argc, YYRValue* Args);
typedef void (*TGMLRoutine)(YYObjectBase* Self, YYObjectBase* Other);
typedef bool (*TCodeExecuteRoutine)(YYObjectBase* Self, YYObjectBase* Other, CCode* code, YYRValue* res, int flags);
typedef void (*TGetTheFunctionRoutine)(int id, char** bufName, void** bufRoutine, int* bufArgs, void* unused);

typedef void (*TPluginCodeExecuteRoutine)(CInstance*& pSelf, CInstance*& pOther, CCode*& Code, YYRValue*& res, int& flags);
typedef void (*TPluginPresentRoutine)(void*& IDXGISwapChain, unsigned int& Sync, unsigned int& Flags);
typedef void (*TPluginEndSceneRoutine)(void*& DIRECT3DDEVICE9);
typedef void (*TPluginDrawTextRoutine)(float& x, float& y, const char*& str, int& linesep, int& linewidth);


typedef void GetOwnPropertyFunc(YYObjectBase*, YYRValue*, const char*);
typedef void DeletePropertyFunc(YYObjectBase*, YYRValue*, const char*, bool);
typedef EJSRetValBool DefineOwnPropertyFunc(YYObjectBase*, const char*, YYRValue*, bool);

typedef unsigned int uint32;
typedef int int32;
typedef float float32;
typedef __int64 int64;
typedef unsigned __int16 uint16;
typedef const char* String;

enum RVKind
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

#pragma pack(push, 4)
struct YYRValue
{
private:
	union
	{
		union
		{
			YYObjectBase* pObject;
			CDynamicArrayRef<YYRValue>* pArray;
		};

		int I32;
		long long I64;
		double Val;
	};
public:
	int Flags;
	int Kind;

	template <typename T>
	T& As()
	{
		return (T&)(*(&I64));
	}
	YYRValue() {}

	YYRValue(double d)
	{
		this->Kind = VALUE_REAL;
		this->Val = d;
	}
};
#pragma pack(pop)
using RValue = YYRValue;

#ifndef YYSDK_NODEFS

#pragma pack(push, 4)
struct CInstanceBase
{
	int (**_vptr$CInstanceBase)(void);
	RValue* yyvars;
};
#pragma pack(pop)
struct YYObjectBase : CInstanceBase
{
	YYObjectBase* m_pNextObject;
	YYObjectBase* m_pPrevObject;
	YYObjectBase* m_prototype;
	void* m_pcre; // Linux Runners only
	void* m_pcreExtra; // Linux Runners only
	const char* m_class;
	GetOwnPropertyFunc* m_getOwnProperty;
	DeletePropertyFunc* m_deleteProperty;
	DefineOwnPropertyFunc* m_defineOwnProperty;
	CHashMap<int, RValue*>* m_yyvarsMap;
	CWeakRef** m_pWeakRefs;
	uint32 m_numWeakRefs;
	uint32 m_nvars;
	uint32 m_flags;
	uint32 m_capacity;
	uint32 m_visited;
	uint32 m_visitedGC;
	int32 m_GCgen;
	int32 m_GCcreationframe;
	int m_slot;
	int m_kind;
	int m_rvalueInitType;
	int m_curSlot;
};

template <typename Key, typename Value>
struct CHashMap
{
	int m_curSize;
	int m_numUsed;
	int m_curMask;
	int m_growThreshold;
	struct CElement
	{
		Value v;
		Key k;
		unsigned int Hash;
	} *m_pBuckets;

	bool CompareKeys(Key k1, Key k2)
	{
		return k1 == k2;
	}

	unsigned int CalculatePtrHash(unsigned char* k)
	{
		return k + 1;
	}

	unsigned int CalculateIntHash(int k)
	{
		return 0x9E3779B1 * k + 1;
	}
};

struct yyMatrix
{
	union
	{
		float f[4][4];
		int n[4][4];
	};
};

struct YYRect
{
	int32 left;
	int32 top;
	int32 right;
	int32 bottom;
};

struct SLink
{
	SLink* next;
	SLink* prev;
	void* list;
};

struct CInstance : YYObjectBase
{
	__int64 m_CreateCounter;
	void* m_pObject;
	void* m_pPhysicsObject;
	void* m_pSkeletonAnimation;
	void* m_pControllingSeqInst;
	unsigned int m_Instflags;
	int i_id;
	int i_objectindex;
	int i_spriteindex;
	float i_sequencePos;
	float i_lastSequencePos;
	float i_sequenceDir;
	float i_imageindex;
	float i_imagespeed;
	float i_imagescalex;
	float i_imagescaley;
	float i_imageangle;
	float i_imagealpha;
	unsigned int i_imageblend;
	float i_x;
	float i_y;
	float i_xstart;
	float i_ystart;
	float i_xprevious;
	float i_yprevious;
	float i_direction;
	float i_speed;
	float i_friction;
	float i_gravitydir;
	float i_gravity;
	float i_hspeed;
	float i_vspeed;
	YYRect i_bbox;
	int i_timer[12];
	void* m_pPathAndTimeline;
	CCode* i_initcode;
	CCode* i_precreatecode;
	void* m_pOldObject;
	int m_nLayerID;
	int i_maskindex;
	__int16 m_nMouseOver;
	CInstance* m_pNext;
	CInstance* m_pPrev;
	SLink m_collisionLink;
	SLink m_dirtyLink;
	SLink m_withLink;
	float i_depth;
	float i_currentdepth;
	float i_lastImageNumber;
	unsigned int m_collisionTestNumber;
};

struct CWeakRef : YYObjectBase
{
	YYObjectBase* pWeakRef;
};

template <typename T>
struct CArray
{
	int Length;
	T* pArray;
};

template <typename T>
struct CDynamicArrayRef
{
	int Length;
	CArray<T>* Array;
	T* pOwner;
};

struct RToken
{
	int kind;
	eGML_TYPE type;
	int ind;
	int ind2;
	RValue value;
	int itemnumb;
	RToken* items;
	int position;
};

struct alignedTo(8) YYVAR
{
	const char* pName;
	int val;
};

struct alignedTo(8) SYYStackTrace
{
	SYYStackTrace* pNext;
	const char* pName;
	int line;
};

struct SLLVMVars {
	char* pWad;				// pointer to the Wad
	int	nWadFileLength;		// the length of the wad
	int	nGlobalVariables;	// global variables
	int	nInstanceVariables;	// instance variables
	int	nYYCode;
	YYVAR** ppVars;
	YYVAR** ppFuncs;
	YYGMLFuncs* pGMLFuncs;
	void* pYYStackTrace;		// pointer to the stack trace
};

struct YYGMLFuncs
{
	const char* pName;
	TGMLRoutine pFunc;
	YYVAR* pFuncVar;
};

struct VMBuffer
{
	void** vTable;
	int m_size;
	int m_numLocalVarsUsed;
	int m_numArguments;
	char* m_pBuffer;
	void** m_pConvertedBuffer;
	char* m_pJumpBuffer;
};

struct CCode
{
	int (**_vptr$CCode)(void);
	CCode* m_pNext;
	int i_kind;
	int i_compiled;
	String i_str;
	RToken i_token;
	RValue i_value;
	VMBuffer* i_pVM;
	VMBuffer* i_pVMDebugInfo;
	char* i_pCode;
	const char* i_pName;
	int i_CodeIndex;
	YYGMLFuncs* i_pFunc;
	bool i_watch;
	int i_offset;
	int i_locals;
	int i_args;
	int i_flags;
	YYObjectBase* i_pPrototype;
};

struct RefString
{
	const char* m_Thing;
	int m_refCount;
	int m_Size;
};

#endif // YYSDK_NODEFS

struct FunctionInfo_t
{
	int Index;
	char* Name;
	TRoutine Function;
	int Arguments;
};

constexpr int CALLBACK_TABLE_MAX_ENTRIES = 4;
constexpr int FUNCTION_TABLE_MAX_ENTRIES = 12;

// Indices into the CALLBACK_TABLE array
constexpr int CTIDX_CodeExecute = 0;
constexpr int CTIDX_EndScene = 1;
constexpr int CTIDX_Present = 2;
constexpr int CTIDX_Drawing = 3;

struct FUNCTION_ENTRY
{
	void* Function;
	const char* Name;
};

using PLUGIN_UNLOAD = YYTKStatus(*)(YYTKPlugin* pPlugin);
using PLUGIN_ENTRY = YYTKStatus(*)(YYTKPlugin* pPlugin);
using CALLBACK_TABLE = void* [CALLBACK_TABLE_MAX_ENTRIES];
using FUNCTION_TABLE = FUNCTION_ENTRY[FUNCTION_TABLE_MAX_ENTRIES];

struct YYTKPlugin
{
	const char* Path;
	PLUGIN_ENTRY Entry;
	PLUGIN_UNLOAD Unload;
	CALLBACK_TABLE Callbacks;
	FUNCTION_TABLE* Functions;
	void* PluginModule;

	YYTKPlugin() : Entry(nullptr), Unload(nullptr), Path(nullptr), Functions(nullptr), PluginModule(nullptr)
	{
		// Duplicate code my beloved
		for (int i = 0; i < CALLBACK_TABLE_MAX_ENTRIES; i++)
		{
			Callbacks[i] = nullptr;
		}
	}

	YYTKPlugin(const char* Path) : Path(Path), Entry(nullptr), Unload(nullptr), Functions(nullptr), PluginModule(nullptr)
	{
		// Duplicate comments my beloved
		for (int i = 0; i < CALLBACK_TABLE_MAX_ENTRIES; i++)
		{
			Callbacks[i] = nullptr;
		}
	}

	template <typename Fn>
	inline Fn LookupFunction(const char* Name)
	{
		for (int n = 0; n < FUNCTION_TABLE_MAX_ENTRIES; n++)
		{
			if (_stricmp(Name, (*Functions)[n].Name) == 0)
				return (Fn)(*Functions)[n].Function;
		}

		return nullptr;
	}
};