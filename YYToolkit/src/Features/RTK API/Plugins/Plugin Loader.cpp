#include "../../../Dependencies/PE Parser/PEParser.hpp"
#include "../../../SDK/Tool/CRTKPlugin/CRTKPlugin.hpp"
#include "../../../SDK/Tool/CRTKString/CRTKString.hpp"
#include "../API.hpp"
#include <filesystem>
#include <set>
#include <map>

static std::map<void*, CRTKPlugin> s_PluginObjects;
static std::map<void*, CRTKString> s_PluginNames;

namespace rtk
{
	EStatus PmInitialize()
	{
		// Evil :P
		using namespace std;

		// Check if the autoexec directory exists
		auto Path = filesystem::current_path().append("\\autoexec");
		if (!filesystem::is_directory(Path))
			return EStatus::kFileNotFound;

		// Get all entries and sort them by name
		std::set<filesystem::path> setEntries;
		for (const auto& Entry : filesystem::directory_iterator(Path))
			setEntries.insert(Entry.path());

		// Load plugins
		for (auto& Entry : setEntries)
		{
			CRTKString PluginName;
			OmInitString(&PluginName, Entry.wstring().c_str());

			EStatus Status = PmLoadPlugin(&PluginName, 3);

			// TODO: Check status, show MessageBox errors accordingly
		}

		// TODO: Maybe return kFail if one or more plugins failed to load?
		return EStatus::kSuccess;
	}

	EStatus PmUninitialize()
	{
		for (auto& [BaseAddress, Plugin] : s_PluginObjects)
			PmUnloadPlugin(BaseAddress);

		return EStatus::kSuccess;
	}

	EStatus PmLoadPlugin(CRTKString* pPath, int MajorVersionToCheck)
	{
		if (!OmIsObjectType(pPath, EObjectType::kString))
			return EStatus::kInvalidObject;

		if (!pPath->m_pBuffer)
			return EStatus::kFileNotFound;

		// Check if the PluginEntry is exported - if it's not, the file's not a valid plugin
		if (!PE::DoesPEExportRoutine(pPath->m_pBuffer, "PluginEntry"))
			return EStatus::kExportNotFound;

		// This routine should always be exported - it's done by the API headers themselves
		if (!PE::DoesPEExportRoutine(pPath->m_pBuffer, "__builtin_GetAPIMajor"))
			return EStatus::kExportNotFound;

		CRTKPlugin NewPlugin;
		HMODULE hPluginHandle = LoadLibraryW(pPath->m_pBuffer);

		if (!hPluginHandle)
			return EStatus::kFail;

		// Look for the exported functions
		FARPROC lpPluginPreload = GetProcAddress(hPluginHandle, "PluginPreload");
		FARPROC lpPluginEntry = GetProcAddress(hPluginHandle, "PluginEntry");
		FARPROC lpGetAPIMajor = GetProcAddress(hPluginHandle, "__builtin_GetAPIMajor");

		// If we for some reason fail to find them
		if (!lpPluginEntry || !lpGetAPIMajor)
		{
			// Unload the plugin, PluginEntry or PluginPreload is never called
			FreeLibrary(hPluginHandle);
			return EStatus::kExportNotFound;
		}

		// Check the major version of the plugin we're loading
		FNPluginGetSDK lpfnGetAPIMajor = reinterpret_cast<FNPluginGetSDK>(lpGetAPIMajor);

		// If the version doesn't match
		int nPluginExpectedAPIMajor = lpfnGetAPIMajor();
		if (nPluginExpectedAPIMajor != MajorVersionToCheck)
		{
			wchar_t Message[512] = { 0 };

			swprintf_s(
				Message,
				L"Plugin \"%S\" expects API version v%d, but you are running version v%d.\n\n"
				L"If you choose to load this plugin, you may experience instabilities or crashes.\n"
				L"Load anyway?",
				std::filesystem::path(pPath->m_pBuffer).filename().wstring().c_str(),
				nPluginExpectedAPIMajor,
				MajorVersionToCheck
			);

			int nResult = MessageBoxW(
				0,
				Message, 
				L"RTK Plugin Loader",
				MB_ICONWARNING | MB_TOPMOST | MB_SETFOREGROUND | MB_YESNO
			);

			// If the user selected "No, don't load it", we unload the plugin
			if (nResult != IDYES)
			{
				// PluginEntry or PluginPreload is never called
				FreeLibrary(hPluginHandle);
				return EStatus::kExportNotFound;
			}
		}
		
		// m_PluginUnload is already initialized to nullptr
		NewPlugin.m_BaseAddress = reinterpret_cast<void*>(hPluginHandle);
		NewPlugin.m_PluginEntry = reinterpret_cast<FNPluginInitEntry>(lpPluginEntry);

		// This one might be nullptr
		NewPlugin.m_PluginInitialize = reinterpret_cast<FNPluginInitEntry>(lpPluginPreload);

		// Copy the string and store it in PluginNames
		// The passed in Path string is expected to be freed after this function returns
		CRTKString PluginName;
		if (OmInitString(&PluginName, pPath->m_pBuffer) == EStatus::kSuccess)
			s_PluginNames[NewPlugin.m_BaseAddress] = PluginName;

		// Add the plugin object to the map
		s_PluginObjects[NewPlugin.m_BaseAddress] = NewPlugin;
		
		return EStatus::kSuccess;
	}

	EStatus PmUnloadPlugin(void* pBaseAddress)
	{
		// If it's not a plugin
		if (!s_PluginObjects.count(pBaseAddress))
			return EStatus::kFail;

		if (!s_PluginNames.count(pBaseAddress))
			return EStatus::kFail;

		CRTKPlugin& TargetPlugin = s_PluginObjects[pBaseAddress];

		if (TargetPlugin.m_PluginUnload)
			TargetPlugin.m_PluginUnload(TargetPlugin);

		s_PluginObjects.erase(pBaseAddress);
		s_PluginNames.erase(pBaseAddress);

		// Might fail, but I don't care
		FreeLibrary(reinterpret_cast<HMODULE>(pBaseAddress));

		return EStatus::kSuccess;
	}

	void PmRunInitializeRoutines()
	{
		for (auto& [BaseAddress, Plugin] : s_PluginObjects)
		{
			if (!Plugin.m_PluginInitialize)
				continue;

			if (!s_PluginNames.count(BaseAddress))
				continue;

			Plugin.m_PluginInitialize(s_PluginNames[BaseAddress], Plugin);
		}
	}

	void PmRunEntryRoutines()
	{
		for (auto& [BaseAddress, Plugin] : s_PluginObjects)
		{
			// PluginEntry should always be initialized (non-null)
			if (!s_PluginNames.count(BaseAddress))
				continue;

			Plugin.m_PluginEntry(s_PluginNames[BaseAddress], Plugin);
		}
	}
}