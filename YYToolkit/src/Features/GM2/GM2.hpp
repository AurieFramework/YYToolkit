#pragma once
#include <cstdint>
#define DllExport __declspec(dllexport)
namespace API
{
	DllExport uintptr_t FindPattern(
		const char* Pattern,
		const char* Mask,
		uintptr_t Base,
		uintptr_t Size
	);
}