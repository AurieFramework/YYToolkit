#pragma once
#include "../../SDK/FwdDecls/FwdDecls.hpp"
#include "Structures/PmStructures.hpp"

namespace API 
{
	namespace PluginManager 
	{
		inline std::list<PluginAttributes_t> g_PluginStorage;

		YYTKPlugin* LoadPlugin(const char* Path);
		bool UnloadPlugin(YYTKPlugin& pPlugin, bool Notify);

		bool IsPluginCompatible(HMODULE Plugin);

		void RunHooks(YYTKEventBase* pEvent);
		void RunPluginMains();
		void RunPluginPreloads();

		void Initialize();
		void Uninitialize();

		DllExport PluginAttributes_t* PmGetObjectAttributes(YYTKPlugin* pObject);
		DllExport CallbackAttributes_t* PmCreateCallback(PluginAttributes_t* pObjectAttributes, FNEventHandler pfnCallback, EventType Flags, void* Argument1);
		DllExport YYTKStatus PmRemoveCallback(CallbackAttributes_t* CallbackAttributes);
	}
}