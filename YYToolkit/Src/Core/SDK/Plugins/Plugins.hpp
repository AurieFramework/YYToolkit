#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include "../Enums/Enums.hpp"
#include <Windows.h>
#include <dxgiformat.h>

struct CInstance;
struct YYRValue;
struct CCode;
struct YYTKPlugin;
class YYTKEventBase;

using FNEventHandler = YYTKStatus(*)(YYTKPlugin* pPlugin, YYTKEventBase* pEvent);
using FNPluginEntry = YYTKStatus(*)(YYTKPlugin* pPlugin);
using FNPluginUnload = YYTKStatus(*)(YYTKPlugin* pPlugin);
using FNPluginPreloadEntry = YYTKStatus(*)(YYTKPlugin* pPlugin);
using FNTextRenderCallback = void(*)(float& x, float& y, const char*& str, int& linesep, int& linewidth);

#pragma pack(push, 1)

struct YYTKPlugin
{
	FNPluginEntry PluginEntry;			// Pointer to the entry function - set by the core.
	FNPluginUnload PluginUnload;		// Pointer to the unload function - optional, set by the plugin.
	FNEventHandler PluginHandler;		// Pointer to an event handler function - optional, set by the plugin.
	FNTextRenderCallback OnTextRender;	// Pointer to a text render callback - optional, set by the plugin.

	void* PluginStart;					// The base address of the plugin (can be casted to a HMODULE).
	void* CoreStart;					// The base address of the core (can be casted to a HMODULE).

	FNPluginPreloadEntry PluginPreload; // Pointer to the plugin preload handler - set by the core if the plugin has one defined.

	template <typename T>
	T GetCoreExport(const char* Name)
	{
		if (CoreStart) return reinterpret_cast<T>(GetProcAddress(reinterpret_cast<HMODULE>(CoreStart), Name));
		return nullptr;
	}

	// Left here only for the GetPluginRoutine() core API
	template <typename T>
	T GetExport(const char* Name)
	{
		if (PluginStart) return reinterpret_cast<T>(GetProcAddress(reinterpret_cast<HMODULE>(PluginStart), Name));
		return nullptr;
	}
};

#pragma pack(pop)

#ifdef YYSDK_PLUGIN

DllExport inline const char* __PluginGetSDKVersion();

#endif