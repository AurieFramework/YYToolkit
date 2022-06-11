#ifndef RTK_DEPENDENCIES_PEPARSER_H_
#define RTK_DEPENDENCIES_PEPARSER_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif // WIN32_LEAN_AND_MEAN

#include <Windows.h>

namespace PE
{
	DWORD RVA_To_Offset(PIMAGE_NT_HEADERS pNTHeader, DWORD dwRVA);

	bool DoesPEExportRoutine(const wchar_t* FilePath, const char* RoutineName);
}

#endif // RTK_DEPENDENCIES_PEPARSER_H_