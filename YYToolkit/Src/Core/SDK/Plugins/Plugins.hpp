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

using FNEventHandler = YYTKStatus(*)(YYTKEventBase* pEvent, void* Argument);
using FNPluginEntry = YYTKStatus(*)(YYTKPlugin* pPluginObject);
using FNPluginUnload = YYTKStatus(*)();
using FNPluginPreloadEntry = YYTKStatus(*)(YYTKPlugin* pPluginObject);

#pragma pack(push, 1)

struct YYTKPlugin
{
	FNPluginEntry PluginEntry;			// Pointer to the entry function - set by the core.
	FNPluginUnload PluginUnload;		// Pointer to the unload function - optional, set by the plugin.
	FNPluginPreloadEntry PluginPreload; // Pointer to the plugin preload handler - set by the core if the plugin has one defined.

	void* PluginStart;					// The base address of the plugin (can be casted to a HMODULE).
	void* CoreStart;					// The base address of the core (can be casted to a HMODULE).
};

#pragma pack(pop)

#ifdef YYSDK_PLUGIN

DllExport inline const char* __PluginGetSDKVersion();

#endif