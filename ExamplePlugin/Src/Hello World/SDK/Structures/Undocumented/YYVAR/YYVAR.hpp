#pragma once
#ifdef _MSC_VER
#pragma warning(disable : 26812)
#define _CRT_SECURE_NO_WARNINGS 1
#define alignedTo(x) __declspec(align(x))
#else //!MSC_VER
#define alignedTo(x) __attribute__((aligned (x)))
//#define strcpy_s(x,y,z) strncpy(x,z,y)
#include <inttypes.h>
#endif //MSC_VER

struct alignedTo(8) YYVAR
{
	const char* pName;
	int val;
};