#pragma once
#include <Windows.h>

namespace Hooks::YYError
{
	void Function(const char* pFormat, ...);
	void* GetTargetAddress();

	inline decltype(&Function) pfnOriginal;
}