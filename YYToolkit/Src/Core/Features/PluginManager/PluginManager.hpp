#pragma once
#include "../../SDK/FwdDecls/FwdDecls.hpp"
#include "../../SDK/Plugins/Plugins.hpp"

namespace API 
{
	namespace PluginManager 
	{
		inline bool __bInitialized = false;

		DllExport YYTKPlugin* LoadPlugin(const char* Path);

		DllExport bool UnloadPlugin(YYTKPlugin* pPlugin, bool Notify);

		DllExport void* GetPluginRoutine(const char* Name);

		DllExport void RunHooks(YYTKEventBase* pEvent);

		DllExport void RunPluginMains();

		DllExport void RunPluginPreloads();

		// This doesn't run hooks, this actually runs a callback
		DllExport void CallTextCallbacks(float& x, float& y, const char*& str, int& linesep, int& linewidth);

		void Initialize();

		void Uninitialize();
	}
}