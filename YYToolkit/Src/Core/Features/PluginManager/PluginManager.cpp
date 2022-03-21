#include "PluginManager.hpp"
#include "../API/API.hpp"
#include "../../Utils/PE Parser/PEParser.hpp"
#include "../../Utils/Logging/Logging.hpp"
#include <filesystem>

void* API::PluginManager::GetPluginRoutine(const char* Name)
{
	for (auto& Element : gAPIVars.Globals.g_PluginStorage)
	{
		if (void* Result = Element.second.GetExport<void*>(Name))
			return Result;
	}

	return nullptr;
}

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

	// Check version scope
	{
		using FNGetSDKVersion = const char* (*)();
		auto __PluginGetSDKVersion = reinterpret_cast<FNGetSDKVersion>(GetProcAddress(PluginModule, "__PluginGetSDKVersion"));

		if (__PluginGetSDKVersion)
		{
			std::string PluginSDKVersion(__PluginGetSDKVersion());
			std::string CoreSDKVersion(YYSDK_VERSION);

			std::string PluginMajorVersion = std::string(PluginSDKVersion).substr(0, PluginSDKVersion.find_first_of('.'));
			std::string CoreMajorVersion = std::string(CoreSDKVersion).substr(0, CoreSDKVersion.find_first_of('.'));

			if (PluginMajorVersion != CoreMajorVersion)
			{
				std::string FileName(Buffer);

				std::string AlertMessage(
					"The plugin \"" + FileName.substr(FileName.find_last_of("/\\") + 1) + "\" was made for\n"
					"SDK version " + std::string(PluginSDKVersion) + ", but the expected one is " + std::string(YYSDK_VERSION) + ".\n"
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
		}
	}

	// Emplace in map scope
	{
		auto PluginObject = YYTKPlugin();
		memset(&PluginObject, 0, sizeof(YYTKPlugin));

		PluginObject.PluginPreload = lpPluginPreloadEntry;
		PluginObject.PluginEntry = lpPluginEntry;
		PluginObject.PluginStart = PluginModule;

		PluginObject.CoreStart = gAPIVars.Globals.g_hMainModule;

		gAPIVars.Globals.g_PluginStorage.emplace(std::make_pair(reinterpret_cast<unsigned long>(PluginObject.PluginStart), PluginObject));
	}

	YYTKPlugin* refVariableInMap = &gAPIVars.Globals.g_PluginStorage.at(reinterpret_cast<unsigned long>(PluginModule));

	return refVariableInMap;
}

bool API::PluginManager::UnloadPlugin(YYTKPlugin* pPlugin, bool Notify)
{
	if (!pPlugin)
		return false;

	if (pPlugin->PluginUnload && Notify)
		pPlugin->PluginUnload(pPlugin);

	FreeLibrary(reinterpret_cast<HMODULE>(pPlugin->PluginStart));

	return true;
}

void API::PluginManager::RunHooks(YYTKEventBase* pEvent)
{
	for (auto& Pair : gAPIVars.Globals.g_PluginStorage)
	{
		if (Pair.second.PluginHandler)
			Pair.second.PluginHandler(&Pair.second, pEvent);
	}
}

void API::PluginManager::RunPluginMains()
{
	for (auto& [__base, Plugin] : gAPIVars.Globals.g_PluginStorage)
	{
		UNREFERENCED_PARAMETER(__base);

		if (Plugin.PluginEntry)
			Plugin.PluginEntry(&Plugin);
	}
}

DllExport void API::PluginManager::RunPluginPreloads()
{
	for (auto& [__base, Plugin] : gAPIVars.Globals.g_PluginStorage)
	{
		UNREFERENCED_PARAMETER(__base);

		if (Plugin.PluginPreload)
			Plugin.PluginPreload(&Plugin);
	}
}

void API::PluginManager::CallTextCallbacks(float& x, float& y, const char*& str, int& linesep, int& linewidth)
{
	for (auto& Pair : gAPIVars.Globals.g_PluginStorage)
	{
		if (Pair.second.OnTextRender)
			Pair.second.OnTextRender(x, y, str, linesep, linewidth);
	}
}

void API::PluginManager::Initialize()
{
	namespace fs = std::filesystem;
	std::wstring Path = fs::current_path().wstring().append(L"\\autoexec");

	if (fs::is_directory(Path) && !fs::is_empty(Path))
	{
		for (auto& entry : fs::directory_iterator(Path))
		{
			if (entry.path().extension().string().find(".dll") != std::string::npos)
			{
				// We have a DLL, try loading it
				if (YYTKPlugin* p = LoadPlugin(entry.path().string().c_str()))
					Utils::Logging::Message(CLR_GREEN, "[+] Loaded '%s' - mapped to 0x%p.", entry.path().filename().string().c_str(), p->PluginStart);
				else
					Utils::Logging::Message(CLR_RED, "[-] Failed to load '%s' - the file may not be a plugin.", entry.path().filename().string().c_str());
			}
		}
	}
}

void API::PluginManager::Uninitialize()
{
	for (auto it = gAPIVars.Globals.g_PluginStorage.begin(); it != gAPIVars.Globals.g_PluginStorage.end();)
	{
		UnloadPlugin(&it->second, true);
		it = gAPIVars.Globals.g_PluginStorage.erase(it);
	}

	gAPIVars.Globals.g_PluginStorage.clear(); // Just in case.
}
