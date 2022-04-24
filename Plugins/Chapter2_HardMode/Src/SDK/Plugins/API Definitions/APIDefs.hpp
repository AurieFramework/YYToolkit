#pragma once
#include "../../FwdDecls/FwdDecls.hpp"
#include "../Plugins.hpp"
#include <string>
#include <vector>

#ifdef YYSDK_PLUGIN

HMODULE GetYYTKModule();

bool GetFunctionByName(
	const std::string& Name,
	TRoutine& outRoutine
);

const char* GetSDKVersion();

bool GetGlobalInstance(
	CInstance*& outInstance
);

bool IsGameYYC();

bool CallBuiltin(
	YYRValue& Result,
	const std::string& Name,
	CInstance* Self,
	CInstance* Other,
	const std::vector<YYRValue>& Args
);

uintptr_t FindPattern(
	const char* Pattern,
	const char* Mask,
	uintptr_t Base,
	uintptr_t Size
);

void PopToastNotification(
	const std::string& Text,
	const std::string& Caption,
	int IconType
);

void PopFileOpenDialog(
	const std::string& WindowTitle,
	const std::string& InitialPath,
	const std::vector<std::string>& Filters,
	bool AllowMultiselect,
	std::vector<std::string>& outSelected
);

void PrintMessage(
	Color color,
	const char* fmt,
	...
);

void PrintError(
	const char* File,
	const int& Line,
	const char* fmt,
	...
);

void PrintMessageNoNewline(
	Color color,
	const char* fmt,
	...
);

YYTKStatus PmGetPluginAttributes(
	YYTKPlugin* pObject,
	PluginAttributes_t*& outAttributes
);

YYTKStatus PmCreateCallback(
	PluginAttributes_t* pObjectAttributes, 
	CallbackAttributes_t*& outAttributes, 
	FNEventHandler pfnCallback,
	EventType Flags, 
	void* OptionalArgument
);

YYTKStatus PmRemoveCallback(
	CallbackAttributes_t* CallbackAttributes
);

YYTKStatus PmSetExported(
	PluginAttributes_t* pObjectAttributes, 
	const char* szRoutineName, 
	void* pfnRoutine
);

YYTKStatus PmGetExported(
	const char* szRoutineName, 
	void*& pfnOutRoutine
);

YYTKStatus PmLoadPlugin(
	const char* szPath,
	void*& pOutBaseAddress
);

YYTKStatus PmUnloadPlugin(
	void* pBaseAddress
);
#endif