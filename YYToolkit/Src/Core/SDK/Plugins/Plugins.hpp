#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include "../Enums/Enums.hpp"
#include <Windows.h>
struct CInstance;
struct YYRValue;
struct CCode;
struct YYTKPlugin;

using FNPresentCallback = void(*)(void*& IDXGISwapChain, unsigned int& Sync, unsigned int& Flags);
using FNEndSceneCallback = void(*)(void*& LPDIRECT3DDEVICE);
using FNDrawCallback = void(*)(float& x, float& y, const char*& str, int& linesep, int& linewidth);
using FNCodeCallback = void(*)(CInstance*& pSelf, CInstance*& pOther, CCode*& Code, YYRValue*& Res, int& Flags);
using FNPluginEntry = YYTKStatus(*)(YYTKPlugin* pPlugin);
using FNPluginUnload = YYTKStatus(*)(YYTKPlugin* pPlugin);

struct YYTKPlugin
{
	FNPluginEntry PluginEntry;		// Pointer to the entry function - set by the core.
	FNPluginUnload PluginUnload;	// Pointer to the unload function - optional, set by the plugin.

	FNEndSceneCallback EndSceneCallback;	// Pointer to the EndScene() callback, called in EndScene if set.
	FNPresentCallback PresentCallback;		// Pointer to the Present() callback, called in Present if set.
	FNDrawCallback DrawCallback;			// Pointer to the GR_Draw_*_Text() callback, called by the game.
	FNCodeCallback CodeCallback;			// Pointer to the Code_Execute() callback, called by the game.

	void* PluginStart;				// The base address of the plugin (can be casted to a HMODULE)

	template <typename T>
	T GetExport(const char* Name)
	{
		if (PluginStart) return reinterpret_cast<T>(GetProcAddress(reinterpret_cast<HMODULE>(PluginStart), Name));
		return nullptr;
	}
};