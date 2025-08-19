// File: YYTK_Shared_Base.hpp
// 
// Defines macros and stuff not dependant on any engine structs.

#ifndef YYTK_SHARED_BASE_H_
#include <Aurie/shared.hpp>

#define YYTK_SHARED_BASE_H_

#ifndef YYTK_STRINGIFY
#define STRINGIFY(x) #x
#endif // YYTK_STRINGIFY

#ifndef YYTK_TO_STRING
#define YYTK_TO_STRING(x) STRINGIFY(x)
#endif // YYTK_TO_STRING

#ifndef YYTK_MAJOR
#define YYTK_MAJOR 5
#endif // YYTK_MAJOR

#ifndef YYTK_MINOR
#define YYTK_MINOR 0
#endif // YYTK_MINOR

#ifndef YYTK_PATCH
#define YYTK_PATCH 0
#endif

#ifndef YYTK_VERSION_STRING
#if YYTK_EXPERIMENTAL
#define YYTK_VERSION_STRING (YYTK_TO_STRING(YYTK_MAJOR) "." YYTK_TO_STRING(YYTK_MINOR) "." YYTK_TO_STRING(YYTK_PATCH) "-EXPERIMENTAL")
#else
#define YYTK_VERSION_STRING (YYTK_TO_STRING(YYTK_MAJOR) "." YYTK_TO_STRING(YYTK_MINOR) "." YYTK_TO_STRING(YYTK_PATCH))
#endif // YYTK_EXPERIMENTAL
#endif // YYTK_VERSION_STRING

#ifndef YYTK_CPP_VERSION
#ifndef _MSVC_LANG
#define YYTK_CPP_VERSION __cplusplus
#else
#define YYTK_CPP_VERSION _MSVC_LANG
#endif // _MSVC_LANG
#endif // YYTK_CPP_VERSION

#if YYTK_CPP_VERSION < 202002L
#error "YYToolkit v5 Shared Headers require at least C++20."
#endif // YYTK_CPP_VERSION

#ifndef UTEXT
#define UTEXT(x) ((const unsigned char*)(x))
#endif // UTEXT

template <typename T>
concept CIntegerCompatible = requires(T Param)
{
	requires std::is_convertible_v<T, int64_t>;
	requires std::is_integral_v<T>;
	requires !std::is_same_v<bool, T>;
	requires !std::is_pointer_v<T>;
};

#ifndef NULL_INDEX
#define NULL_INDEX INT_MIN
#endif // NULL_INDEX
#endif // YYTK_SHARED_BASE_H_