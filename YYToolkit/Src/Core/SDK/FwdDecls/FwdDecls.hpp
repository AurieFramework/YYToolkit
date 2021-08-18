#pragma once
#pragma region Documented
// APIVars.hpp
struct APIVars_t;

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

// VMExec.hpp
struct VMExec;

#pragma endregion

// Typedefs
using TRoutine = void(__cdecl*)(RValue* _result, CInstance* _self, CInstance* _other, int _argc, RValue* _args);
using PFUNC_YYGML = void(__cdecl*)(CInstance* _self, CInstance* _other);
typedef bool (*FNCodeExecute)(YYObjectBase* Self, YYObjectBase* Other, CCode* code, YYRValue* res, int flags);
typedef void (*FNCodeFunctionGetTheFunction)(int id, char** bufName, void** bufRoutine, int* bufArgs, void* unused);

// Macros

#define WIN32_LEAN_AND_MEAN 1
#define YYSDK_VERSION "0.0.2" // YYToolkit version - don't change this!
#define YYTK_MAGIC 'TFSI'