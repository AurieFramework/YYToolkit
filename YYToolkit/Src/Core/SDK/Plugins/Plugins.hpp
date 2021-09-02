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
struct YYTKEvent;

using FNEventHandler = YYTKStatus(*)(YYTKPlugin* pPlugin, YYTKEvent* pEvent);
using FNPluginEntry = YYTKStatus(*)(YYTKPlugin* pPlugin);
using FNPluginUnload = YYTKStatus(*)(YYTKPlugin* pPlugin);

struct YYTKPlugin
{
	FNPluginEntry PluginEntry;		// Pointer to the entry function - set by the core.
	FNPluginUnload PluginUnload;	// Pointer to the unload function - optional, set by the plugin.

	void* PluginStart;				// The base address of the plugin (can be casted to a HMODULE).
	void* CoreStart;				// The base address of the core (can be casted to a HMODULE).

	template <typename T>
	T GetCoreExport(const char* Name)
	{
		if (CoreStart) return reinterpret_cast<T>(GetProcAddress(reinterpret_cast<HMODULE>(CoreStart), Name));
		return nullptr;
	}

	template <typename T>
	T GetExport(const char* Name)
	{
		if (PluginStart) return reinterpret_cast<T>(GetProcAddress(reinterpret_cast<HMODULE>(PluginStart), Name));
		return nullptr;
	}
};