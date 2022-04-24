#include "APIDefs.hpp"
#include <TlHelp32.h>

HMODULE GetYYTKModule()
{
	HANDLE ModuleSnapshot = INVALID_HANDLE_VALUE;

	MODULEENTRY32 ModuleEntry;
	ModuleEntry.dwSize = sizeof(MODULEENTRY32);

	ModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if (ModuleSnapshot == INVALID_HANDLE_VALUE)
		return nullptr;

	Module32First(ModuleSnapshot, &ModuleEntry);

	do
	{
		if (GetProcAddress(ModuleEntry.hModule, "GetSDKVersion"))
		{
			CloseHandle(ModuleSnapshot);
			return ModuleEntry.hModule;
		}

	} while (Module32Next(ModuleSnapshot, &ModuleEntry));

	CloseHandle(ModuleSnapshot);
	return nullptr;
}

bool GetFunctionByName(const std::string& Name, TRoutine& outRoutine)
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&GetFunctionByName) Func = reinterpret_cast<decltype(&GetFunctionByName)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(Name, outRoutine);
}

const char* GetSDKVersion()
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&GetSDKVersion) Func = reinterpret_cast<decltype(&GetSDKVersion)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func();
}

bool GetGlobalInstance(CInstance*& outInstance)
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&GetGlobalInstance) Func = reinterpret_cast<decltype(&GetGlobalInstance)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(outInstance);
}

bool IsGameYYC()
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&IsGameYYC) Func = reinterpret_cast<decltype(&IsGameYYC)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func();
}

bool CallBuiltin(YYRValue& Result, const std::string& Name, CInstance* Self, CInstance* Other, const std::vector<YYRValue>& Args)
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&CallBuiltin) Func = reinterpret_cast<decltype(&CallBuiltin)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(Result, Name, Self, Other, Args);
}

uintptr_t FindPattern(const char* Pattern, const char* Mask, uintptr_t Base, uintptr_t Size)
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&FindPattern) Func = reinterpret_cast<decltype(&FindPattern)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(Pattern, Mask, Base, Size);
}

void PopToastNotification(const std::string& Text, const std::string& Caption, int IconType)
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&PopToastNotification) Func = reinterpret_cast<decltype(&PopToastNotification)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(Text, Caption, IconType);
}

void PopFileOpenDialog(const std::string& WindowTitle, const std::string& InitialPath, const std::vector<std::string>& Filters, bool AllowMultiselect, std::vector<std::string>& outSelected)
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&PopFileOpenDialog) Func = reinterpret_cast<decltype(&PopFileOpenDialog)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(WindowTitle, InitialPath, Filters, AllowMultiselect, outSelected);
}

void PrintMessage(Color color, const char* fmt, ...)
{
	constexpr size_t MaxStringLength = 1024;

	if (strlen(fmt) >= MaxStringLength)
		return;

	va_list vaArgs;
	va_start(vaArgs, fmt);

	char Buf[MaxStringLength];
	memset(Buf, 0, MaxStringLength);
	strncpy(Buf, fmt, MaxStringLength);
	vsprintf_s(Buf, fmt, vaArgs);
	va_end(vaArgs);

	HMODULE YYTKModule = GetYYTKModule();

	decltype(&PrintMessage) Func = reinterpret_cast<decltype(&PrintMessage)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(color, Buf);
}

void PrintError(const char* File, const int& Line, const char* fmt, ...)
{
	constexpr size_t MaxStringLength = 1024;

	if (strlen(fmt) >= MaxStringLength)
		return;

	va_list vaArgs;
	va_start(vaArgs, fmt);

	char Buf[MaxStringLength];
	memset(Buf, 0, MaxStringLength);
	strncpy(Buf, fmt, MaxStringLength);
	vsprintf_s(Buf, fmt, vaArgs);
	va_end(vaArgs);

	HMODULE YYTKModule = GetYYTKModule();

	decltype(&PrintError) Func = reinterpret_cast<decltype(&PrintError)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(File, Line, Buf);
}

void PrintMessageNoNewline(Color color, const char* fmt, ...)
{
	constexpr size_t MaxStringLength = 1024;

	if (strlen(fmt) >= MaxStringLength)
		return;

	va_list vaArgs;
	va_start(vaArgs, fmt);

	char Buf[MaxStringLength];
	memset(Buf, 0, MaxStringLength);
	strncpy(Buf, fmt, MaxStringLength);
	vsprintf_s(Buf, fmt, vaArgs);
	va_end(vaArgs);

	HMODULE YYTKModule = GetYYTKModule();

	decltype(&PrintMessageNoNewline) Func = reinterpret_cast<decltype(&PrintMessageNoNewline)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(color, Buf);
}

YYTKStatus PmGetPluginAttributes(YYTKPlugin* pObject, PluginAttributes_t*& outAttributes)
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&PmGetPluginAttributes) Func = reinterpret_cast<decltype(&PmGetPluginAttributes)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(pObject, outAttributes);
}

YYTKStatus PmCreateCallback(PluginAttributes_t* pObjectAttributes, CallbackAttributes_t*& outAttributes, FNEventHandler pfnCallback, EventType Flags, void* OptionalArgument)
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&PmCreateCallback) Func = reinterpret_cast<decltype(&PmCreateCallback)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(pObjectAttributes, outAttributes, pfnCallback, Flags, OptionalArgument);
}

YYTKStatus PmRemoveCallback(CallbackAttributes_t* CallbackAttributes)
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&PmRemoveCallback) Func = reinterpret_cast<decltype(&PmRemoveCallback)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(CallbackAttributes);
}

YYTKStatus PmSetExported(PluginAttributes_t* pObjectAttributes, const char* szRoutineName, void* pfnRoutine)
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&PmSetExported) Func = reinterpret_cast<decltype(&PmSetExported)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(pObjectAttributes, szRoutineName, pfnRoutine);
}

YYTKStatus PmGetExported(const char* szRoutineName, void*& pfnOutRoutine)
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&PmGetExported) Func = reinterpret_cast<decltype(&PmGetExported)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(szRoutineName, pfnOutRoutine);
}

YYTKStatus PmLoadPlugin(const char* szPath, void*& pOutBaseAddress)
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&PmLoadPlugin) Func = reinterpret_cast<decltype(&PmLoadPlugin)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(szPath, pOutBaseAddress);
}

YYTKStatus PmUnloadPlugin(void* pBaseAddress)
{
	HMODULE YYTKModule = GetYYTKModule();

	decltype(&PmUnloadPlugin) Func = reinterpret_cast<decltype(&PmUnloadPlugin)>(GetProcAddress(YYTKModule, __FUNCTION__));

	return Func(pBaseAddress);
}
