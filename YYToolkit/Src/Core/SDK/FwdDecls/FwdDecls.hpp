#pragma once
#pragma region Documented
// APIVars.hpp
struct CGlobals;
struct CFunctions;
struct CAPIVars;

// CCode.hpp
struct CCode;

// CDynamicArray.hpp
template <typename T>
struct CDynamicArray;
template <typename T>
struct CDynamicArrayRef;

// FunctionInfo.hpp
struct FunctionInfo_t;

// CHashMap.hpp
template <typename Key, typename Value>
struct CHashMap;

// Math.hpp
struct Vector3D;

// RefThing.hpp
struct RefString;

// RToken.hpp
struct RToken;

// VMBuffer.hpp
struct VMBuffer;

// YYRValue.hpp
struct RValue;
struct YYRValue;

#pragma endregion 

#pragma region Undocumented

// YYGMLFuncs.hpp
struct YYGMLFuncs;

// YYVAR.hpp
struct YYVAR;

// YYObjectBase.hpp
struct CInstanceBase;
struct YYObjectBase;
struct CInstance;
struct CWeakRef;

// CScript.hpp
struct CScript;
struct CStream;

// VMExec.hpp
struct VMExec;

#pragma endregion

#pragma region Opaque
struct PluginAttributes_t;
struct CallbackAttributes_t;
#pragma endregion

// Typedefs
using TRoutine = void(__cdecl*)(RValue* _result, CInstance* _self, CInstance* _other, int _argc, RValue* _args);
using PFUNC_YYGML = void(__cdecl*)(CInstance* _self, CInstance* _other);
typedef void (*FNCodeFunctionGetTheFunction)(int id, char** bufName, void** bufRoutine, int* bufArgs, void* unused);
typedef bool (*FNCodeExecute)(YYObjectBase* Self, YYObjectBase* Other, CCode* code, YYRValue* res, int flags);

// Macros

#define WIN32_LEAN_AND_MEAN 1
#define YYTK_MAGIC 'TFSI'

static const char* YYSDK_VERSION = "2.1.2";

// Macros, but complicated
#ifdef _MSC_VER
#pragma warning(disable : 26812)
#define _CRT_SECURE_NO_WARNINGS 1
#define alignedTo(x) __declspec(align(x))
#else //!MSC_VER
#define alignedTo(x) __attribute__((aligned (x)))
//#define strcpy_s(x,y,z) strncpy(x,z,y)
#include <inttypes.h>
#endif //MSC_VER

#ifdef __cplusplus
#define DllExport extern "C" __declspec(dllexport)
#else //!__cplusplus
#define DllExport __declspec(dllexport)
#endif //__cplusplus