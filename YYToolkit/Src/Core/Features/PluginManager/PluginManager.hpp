#pragma once
#include "../../SDK/FwdDecls/FwdDecls.hpp"
#include "Structures/PmStructures.hpp"

namespace API 
{
	namespace PluginManager 
	{
		inline std::list<PluginAttributes_t> g_PluginStorage;

		YYTKPlugin* LoadPlugin(const wchar_t* Path);
		bool UnloadPlugin(YYTKPlugin& pPlugin, bool Notify);

		std::string GetPluginVersionString(HMODULE Plugin);
		bool IsPluginCompatible(HMODULE Plugin);

		void RunHooks(YYTKEventBase* pEvent);
		void RunPluginMains();
		void RunPluginPreloads();

		void Initialize();
		void Uninitialize();

		DllExport YYTKStatus PmGetPluginAttributes(YYTKPlugin* pObject, PluginAttributes_t*& outAttributes);
		DllExport YYTKStatus PmCreateCallback(PluginAttributes_t* pObjectAttributes, CallbackAttributes_t*& outAttributes, FNEventHandler pfnCallback, EventType Flags, void* OptionalArgument);
		DllExport YYTKStatus PmRemoveCallback(CallbackAttributes_t* CallbackAttributes);
		DllExport YYTKStatus PmSetExported(PluginAttributes_t* pObjectAttributes, const char* szRoutineName, void* pfnRoutine);
		DllExport YYTKStatus PmGetExported(const char* szRoutineName, void*& pfnOutRoutine);
		DllExport YYTKStatus PmLoadPlugin(const char* szPath, void*& pOutBaseAddress);
		DllExport YYTKStatus PmUnloadPlugin(void* pBaseAddress);
	}
}