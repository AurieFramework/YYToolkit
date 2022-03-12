#pragma once
#include "../../SDK/FwdDecls/FwdDecls.hpp"
#include "../../SDK/Plugins/Plugins.hpp"

namespace API 
{
	namespace PluginManager 
	{
		DllExport void* GetPluginRoutine(const char* Name);

		DllExport YYTKPlugin* LoadPlugin(const char* Path);

		DllExport bool UnloadPlugin(YYTKPlugin* pPlugin, bool Notify);

		DllExport void RunHooks(YYTKEventBase* pEvent);

		// This doesn't run hooks, this actually runs a callback
		DllExport void CallTextCallbacks(float& x, float& y, const char*& str, int& linesep, int& linewidth);

		void Initialize();

		void Uninitialize();
	}
}