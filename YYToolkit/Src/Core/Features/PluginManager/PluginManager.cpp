#include "../../SDK/Plugins/YYTKEvent/YYTKEvent.hpp"
#include "../../Utils/PE Parser/PEParser.hpp"
#include "../../Utils/Logging/Logging.hpp"
#include "PluginManager.hpp"
#include "../API/API.hpp"
#include <filesystem>

YYTKPlugin* API::PluginManager::LoadPlugin(const char* Path)
{
	char Buffer[MAX_PATH] = { 0 };
	FNPluginEntry lpPluginEntry = nullptr;
	FNPluginPreloadEntry lpPluginPreloadEntry = nullptr;

	GetFullPathNameA(Path, MAX_PATH, Buffer, 0);

	if (!Utils::DoesPEExportRoutine(Buffer, "PluginEntry"))
		return nullptr;

	if (!Utils::DoesPEExportRoutine(Buffer, "__PluginGetSDKVersion"))
	{
		std::string FileName(Buffer);

		std::string AlertMessage(
			"The version of plugin \"" + FileName.substr(FileName.find_last_of("/\\") + 1) + "\" couldn't be fetched.\n"
			"This usually means it was made for an old version of YYToolkit.\n"
			"Loading this plugin may introduce instability / outright crash the game.\n"
			"Try updating the plugin if a newer version is available.\n\n"
			"Load anyway?");

		int Result = MessageBoxA(0, AlertMessage.c_str(), "Warning", MB_ICONWARNING | MB_YESNO | MB_TOPMOST | MB_SETFOREGROUND);

		if (Result == IDNO)
			return nullptr;
	}

	HMODULE PluginModule = LoadLibraryA(Buffer);

	if (!PluginModule)
		return nullptr;

	lpPluginEntry = reinterpret_cast<FNPluginEntry>(GetProcAddress(PluginModule, "PluginEntry"));
	lpPluginPreloadEntry = reinterpret_cast<FNPluginPreloadEntry>(GetProcAddress(PluginModule, "PluginPreload"));

	if (!IsPluginCompatible(PluginModule))
	{
		std::string FileName(Buffer);
		std::string AlertMessage(
			"The plugin \"" + FileName.substr(FileName.find_last_of("/\\") + 1) + "\" was made for\n"
			"an older YYToolkit version than is currently loaded.\n"
			"Loading this plugin may introduce instability / outright crash the game.\n"
			"Try updating the plugin if a newer version is available.\n\n"
			"Load anyway?");

		int Result = MessageBoxA(0, AlertMessage.c_str(), "Warning", MB_ICONWARNING | MB_YESNO | MB_TOPMOST | MB_SETFOREGROUND);

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

bool API::PluginManager::IsPluginCompatible(HMODULE Plugin)
{
	using FNGetSDKVersion = const char* (*)();
	auto __PluginGetSDKVersion = reinterpret_cast<FNGetSDKVersion>(GetProcAddress(Plugin, "__PluginGetSDKVersion"));

	if (!__PluginGetSDKVersion)
		return false;

	std::string PluginSDKVersion(__PluginGetSDKVersion());
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

	if (fs::is_empty(Path))
		return;

	for (auto& entry : fs::directory_iterator(Path))
	{
		if (entry.path().extension().string().find(".dll") == std::string::npos)
			continue;

		// We have a DLL, try loading it
		if (YYTKPlugin* p = LoadPlugin(entry.path().string().c_str()))
			Utils::Logging::Message(CLR_GREEN, "[+] Loaded '%s' - mapped to 0x%p.", entry.path().filename().string().c_str(), p->PluginStart);
		else
			Utils::Logging::Message(CLR_RED, "[-] Failed to load '%s' - the file may not be a plugin.", entry.path().filename().string().c_str());
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

PluginAttributes_t* API::PluginManager::PmGetPluginAttributes(YYTKPlugin* pObject)
{
	if (!pObject)
		return nullptr;

	for (auto& Attributes : g_PluginStorage)
	{
		if (Attributes == *pObject)
			return &Attributes;
	}

	return nullptr;
}

CallbackAttributes_t* API::PluginManager::PmCreateCallback(PluginAttributes_t* pObjectAttributes, FNEventHandler pfnCallback, EventType Flags, void* OptionalArgument)
{
	if (!pObjectAttributes)
		return nullptr;

	if (!pfnCallback)
		return nullptr;

	CallbackAttributes_t Attributes;
	Attributes.Callback = pfnCallback;
	Attributes.CallbackType = Flags;
	Attributes.Argument = OptionalArgument;
	pObjectAttributes->RegisteredCallbacks.push_back(Attributes);

	return &pObjectAttributes->RegisteredCallbacks.back();
}

DllExport YYTKStatus API::PluginManager::PmRemoveCallback(CallbackAttributes_t* CallbackAttributes)
{
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
