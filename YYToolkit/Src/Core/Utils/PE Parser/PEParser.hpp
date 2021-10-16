#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <Windows.h>

namespace Utils
{
	DWORD RVA_To_Offset(PIMAGE_NT_HEADERS pNTHeader, DWORD dwRVA);

	bool DoesPEExportRoutine(const char* FilePath, const char* RoutineName);
}