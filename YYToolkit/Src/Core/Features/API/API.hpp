#pragma once
#include "../../SDK/Structures/Documented/YYRValue/YYRValue.hpp"
#include "../../SDK/Structures/Documented/APIVars/APIVars.hpp"
#include "../../SDK/FwdDecls/FwdDecls.hpp"
#include <string>
#include <vector>

namespace API
{
	inline CAPIVars gAPIVars;

	DllExport bool GetFunctionByName(
		const std::string& Name,
		TRoutine& outRoutine
	);

	DllExport const char* GetSDKVersion();

	DllExport bool GetGlobalInstance(
		CInstance*& outInstance
	);

	DllExport bool IsYYC();

	DllExport bool IsGameYYC();

	DllExport bool CallBuiltin(
		YYRValue& Result,
		const std::string& Name,
		CInstance* Self,
		CInstance* Other,
		const std::vector<YYRValue>& Args
	);

	DllExport uintptr_t FindPattern(
		const char* Pattern,
		const char* Mask,
		uintptr_t Base,
		uintptr_t Size
	);

	DllExport void PopToastNotification(
		const std::string& Text,
		const std::string& Caption,
		int IconType
	);

	DllExport void PopFileOpenDialog(
		const std::string& WindowTitle,
		const std::string& InitialPath,
		const std::vector<std::string>& Filters,
		bool AllowMultiselect,
		std::vector<std::string>& outSelected
	);

	DllExport void PrintMessage(
		Color color,
		const char* fmt,
		...
	);

	DllExport void PrintMessageNoNewline(
		Color color,
		const char* fmt,
		...
	);

	DllExport void PrintError(
		const char* File,
		const int& Line,
		const char* fmt,
		...
	);
}