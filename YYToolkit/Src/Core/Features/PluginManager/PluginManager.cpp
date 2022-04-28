#include "../../SDK/Plugins/YYTKEvent/YYTKEvent.hpp"
#include "../../Utils/PE Parser/PEParser.hpp"
#include "../../Utils/Logging/Logging.hpp"
#include "PluginManager.hpp"
#include "../API/API.hpp"
#include <filesystem>

YYTKPlugin* API::PluginManager::LoadPlugin(const wchar_t* Path)
{
	wchar_t Buffer[MAX_PATH] = { 0 };
	FNPluginEntry lpPluginEntry = nullptr;
	FNPluginPreloadEntry lpPluginPreloadEntry = nullptr;

	GetFullPathNameW(Path, MAX_PATH, Buffer, 0);

	if (!Utils::DoesPEExportRoutine(Buffer, "PluginEntry"))
		return nullptr;

	if (!Utils::DoesPEExportRoutine(Buffer, "__PluginGetSDKVersion"))
	{
		std::wstring FileName(Buffer);

		std::wstring AlertMessage(
			L"The version of plugin \"" + FileName.substr(FileName.find_last_of(L"/\\") + 1) + L"\" couldn't be fetched.\n"
			L"This usually means it was made for an old version of YYToolkit.\n"
			L"Loading this plugin may introduce instability / outright crash the game.\n"
			L"Try updating the plugin if a newer version is available.\n\n"
			L"Load anyway?");

		int Result = MessageBoxW(0, AlertMessage.c_str(), L"Warning", MB_ICONWARNING | MB_YESNO | MB_TOPMOST | MB_SETFOREGROUND);

		if (Result == IDNO)
			return nullptr;
	}

	HMODULE PluginModule = LoadLibraryW(Buffer);

	if (!PluginModule)
		return nullptr;

	lpPluginEntry = reinterpret_cast<FNPluginEntry>(GetProcAddress(PluginModule, "PluginEntry"));
	lpPluginPreloadEntry = reinterpret_cast<FNPluginPreloadEntry>(GetProcAddress(PluginModule, "PluginPreload"));

	if (!IsPluginCompatible(PluginModule))
	{
		std::wstring FileName(Buffer);
		std::string PluginVersionString(GetPluginVersionString(PluginModule));
		std::wstring wPluginVersionString(PluginVersionString.begin(), PluginVersionString.end());
		std::string CoreVersionString(YYSDK_VERSION);
		std::wstring wCoreVersionString(CoreVersionString.begin(), CoreVersionString.end());

		std::wstring AlertMessage(
			L"The plugin \"" + FileName.substr(FileName.find_last_of(L"/\\") + 1) + L"\" was made for\n"
			L"YYTK version " + wPluginVersionString + L", but you are running " + wCoreVersionString + ".\n"
			L"Loading this plugin may crash the game.\n"
			L"Try updating the plugin if a newer version is available.\n\n"
			L"Load anyway?");

		int Result = MessageBoxW(0, AlertMessage.c_str(), L"Warning", MB_ICONWARNING | MB_YESNO | MB_TOPMOST | MB_SETFOREGROUND);

		if (Result == IDNO)
		{
			FreeLibrary(PluginModule);
			return nullptr;
		}
	}

	// Emplace in map scope
	auto PluginObject = YYTKPlugin();
	memset(&PluginObject, 0, sizeof(YYTKPlugin));

	PluginObject.PluginPreload = lpPluginPreloadEntry;
	PluginObject.PluginEntry = lpPluginEntry;
	PluginObject.PluginStart = PluginModule;

	PluginObject.CoreStart = gAPIVars.Globals.g_hMainModule;

	g_PluginStorage.push_back(PluginAttributes_t(PluginObject));

	YYTKPlugin* refVariable = &g_PluginStorage.back().GetPluginObject();

	return refVariable;
}

bool API::PluginManager::UnloadPlugin(YYTKPlugin& pPlugin, bool Notify)
{
	if (pPlugin.PluginUnload && Notify)
		pPlugin.PluginUnload();

	FreeLibrary(reinterpret_cast<HMODULE>(pPlugin.PluginStart));

	return true;
}

void API::PluginManager::RunHooks(YYTKEventBase* pEvent)
{
	for (auto& PluginAttributes : g_PluginStorage)
	{
		for (auto& CallbackAttributes : PluginAttributes.RegisteredCallbacks)
		{
			if (CallbackAttributes.CallbackType & pEvent->GetEventType())
			{
				CallbackAttributes.Callback(pEvent, CallbackAttributes.Argument);
			}
		}
	}
}

std::string API::PluginManager::GetPluginVersionString(HMODULE Plugin)
{
	using FNGetSDKVersion = const char* (*)();
	auto __PluginGetSDKVersion = reinterpret_cast<FNGetSDKVersion>(GetProcAddress(Plugin, "__PluginGetSDKVersion"));

	if (!__PluginGetSDKVersion)
		return "";

	return __PluginGetSDKVersion();
}

bool API::PluginManager::IsPluginCompatible(HMODULE Plugin)
{
	std::string PluginSDKVersion(GetPluginVersionString(Plugin));
	std::string CoreSDKVersion(YYSDK_VERSION);

	std::string PluginMajorVersion = PluginSDKVersion.substr(0, PluginSDKVersion.find_first_of('.'));
	std::string CoreMajorVersion = CoreSDKVersion.substr(0, CoreSDKVersion.find_first_of('.'));

	return PluginMajorVersion == CoreMajorVersion;
}

void API::PluginManager::RunPluginMains()
{
	for (auto& Plugin : g_PluginStorage)
	{
		if (!Plugin.GetPluginObject().PluginEntry)
			continue;

		Plugin.GetPluginObject().PluginEntry(&Plugin.GetPluginObject());
	}
}

void API::PluginManager::RunPluginPreloads()
{
	for (auto& Plugin: g_PluginStorage)
	{
		if (!Plugin.GetPluginObject().PluginPreload)
			continue;

		Plugin.GetPluginObject().PluginPreload(&Plugin.GetPluginObject());
	}
}

void API::PluginManager::Initialize()
{
	namespace fs = std::filesystem;
	std::wstring Path = fs::current_path().wstring().append(L"\\autoexec");

	if (!fs::is_directory(Path))
		return;

	for (auto& entry : fs::directory_iterator(Path))
	{
		if (entry.path().extension().wstring().find(L".dll") == std::wstring::npos)
			continue;

		// We have a DLL, try loading it
		if (YYTKPlugin* p = LoadPlugin(entry.path().wstring().c_str()))
			Utils::Logging::Message(CLR_GREEN, "[+] Loaded '%S' - mapped to 0x%p.", entry.path().filename().wstring().c_str(), p->PluginStart);
		else
			Utils::Logging::Message(CLR_RED, "[-] Failed to load '%S' - the file may not be a plugin.", entry.path().filename().wstring().c_str());
	}
}

void API::PluginManager::Uninitialize()
{
	for (auto it = g_PluginStorage.begin(); it != g_PluginStorage.end();)
	{
		UnloadPlugin(it->GetPluginObject(), true);
		it = g_PluginStorage.erase(it);
	}

	g_PluginStorage.clear(); // Just in case.
}

YYTKStatus API::PluginManager::PmGetPluginAttributes(YYTKPlugin* pObject, PluginAttributes_t*& outAttributes)
{
	if (!pObject)
		return YYTK_INVALIDARG;

	for (auto& Attributes : g_PluginStorage)
	{
		if (Attributes == *pObject)
		{
			outAttributes = &Attributes;
			return YYTK_OK;
		}
	}

	return YYTK_NOT_FOUND;
}

YYTKStatus API::PluginManager::PmCreateCallback(PluginAttributes_t* pObjectAttributes, CallbackAttributes_t*& outAttributes, FNEventHandler pfnCallback, EventType Flags, void* OptionalArgument)
{
	if (!pObjectAttributes)
		return YYTK_INVALIDARG;

	if (!pfnCallback)
		return YYTK_INVALIDARG;

	CallbackAttributes_t Attributes;
	Attributes.Callback = pfnCallback;
	Attributes.CallbackType = Flags;
	Attributes.Argument = OptionalArgument;
	pObjectAttributes->RegisteredCallbacks.push_back(Attributes);

	outAttributes = &pObjectAttributes->RegisteredCallbacks.back();
	return YYTK_OK;
}

YYTKStatus API::PluginManager::PmRemoveCallback(CallbackAttributes_t* CallbackAttributes)
{
	if (!CallbackAttributes)
		return YYTK_INVALIDARG;

	for (auto& PluginAttributes : g_PluginStorage)
	{
		for (auto& RegisteredCallback : PluginAttributes.RegisteredCallbacks)
		{
			if ((RegisteredCallback.Callback == CallbackAttributes->Callback) &&
				(RegisteredCallback.CallbackType == CallbackAttributes->CallbackType))
			{
				PluginAttributes.RegisteredCallbacks.remove(RegisteredCallback);
				return YYTK_OK;
			}
		}
	}

	return YYTK_NOT_FOUND;
}

YYTKStatus API::PluginManager::PmSetExported(PluginAttributes_t* pObjectAttributes, const char* szRoutineName, void* pfnRoutine)
{
	if (!pObjectAttributes)
		return YYTK_INVALIDARG;

	if (!pfnRoutine)
		return YYTK_INVALIDARG;

	if (!szRoutineName || *szRoutineName == '\0')
		return YYTK_INVALIDARG;

	ExportedRoutine_t newExport;
	newExport.pfnRoutine = pfnRoutine;
	newExport.sRoutineName = szRoutineName;

	pObjectAttributes->Exports.insert(newExport);

	return YYTK_OK;
}

YYTKStatus API::PluginManager::PmGetExported(const char* szRoutineName, void*& pfnOutRoutine)
{
	if (!szRoutineName || *szRoutineName == '\0')
		return YYTK_INVALIDARG;

	for (auto& PluginAttributes : g_PluginStorage)
	{
		for (auto& Export : PluginAttributes.Exports)
		{
			if (Export.sRoutineName == szRoutineName)
			{
				pfnOutRoutine = Export.pfnRoutine;
				return YYTK_OK;
			}
		}
	}

	return YYTK_NOT_FOUND;
}

YYTKStatus API::PluginManager::PmLoadPlugin(const char* szPath, void*& pOutBaseAddress)
{
	if (!szPath || !*szPath)
		return YYTK_INVALIDARG;

	std::string sPath(szPath);

	YYTKPlugin* pPlugin = LoadPlugin(std::wstring(sPath.begin(), sPath.end()).c_str());

	if (!pPlugin)
		return YYTK_FAIL;

	pOutBaseAddress = pPlugin->PluginStart;
	return YYTK_OK;
}

DllExport YYTKStatus API::PluginManager::PmUnloadPlugin(void* pBaseAddress)
{
	if (!pBaseAddress)
		return YYTK_INVALIDARG;

	for (auto& PluginAttributes : g_PluginStorage)
	{
		if (PluginAttributes.GetPluginObject().PluginStart == pBaseAddress)
		{
			UnloadPlugin(PluginAttributes.GetPluginObject(), true);
			return YYTK_OK;
		}
	}

	return YYTK_NOT_FOUND;
}
