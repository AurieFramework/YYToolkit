#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include "../Enums/Enums.hpp"
#include "../Structures/Documented/CDynamicArray/CDynamicArray.hpp"
#include <Windows.h>
#include <dxgiformat.h>
#include <vector>
#include <string>
struct CInstance;
struct YYRValue;
struct CCode;
struct YYTKPlugin;
struct YYTKEvent;
struct YYTKEventState;
struct YYTKObject;

using FNEventHandler = YYTKStatus(*)(YYTKPlugin* pPlugin, YYTKEvent* pEvent);
using FNPluginEntry = YYTKStatus(*)(YYTKPlugin* pPlugin);
using FNPluginUnload = YYTKStatus(*)(YYTKPlugin* pPlugin);

struct YYTKPlugin
{
	FNPluginEntry PluginEntry;		// Pointer to the entry function - set by the core.
	FNPluginUnload PluginUnload;	// Pointer to the unload function - optional, set by the plugin.
	FNEventHandler PluginHandler;	// Pointer to an event handler function - optional, set by the plugin.

	void* PluginStart;				// The base address of the plugin (can be casted to a HMODULE).
	void* CoreStart;				// The base address of the core (can be casted to a HMODULE).

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

struct YYTKObject
{
private:
	void* pObj;
public:
	YYTKObject(void* object)
	{
		this->pObj = object;
	}

	template <typename T>
	T& Get()
	{
		return *reinterpret_cast<T*>(pObj);
	}
};

// User events can extend on this! Just inherit from this class, create your own type, and on a merry way you go!
struct YYTKEvent
{
protected:
	struct YYTKEventState
	{
		bool s_CalledOriginal;
		std::string s_Name;
		bool s_Reserved;
		YYTKObject s_ReturnValue = YYTKObject(nullptr);
	} State;

	std::vector<YYTKObject> Objects;
	void* pfnOriginal;

public:
	template <typename T>
	T GetOriginal()
	{
		return reinterpret_cast<T>(pfnOriginal);
	}

	bool& CalledOriginal()
	{
		return this->State.s_CalledOriginal;
	}

	void SetReturnValue()

	std::vector<YYTKObject>* GetObjects()
	{
		return &Objects;
	}

	const std::string& GetEventName()
	{
		return State.s_Name;
	}

	YYTKEvent(const char* Name, void* pfnCall, std::vector<YYTKObject> Arguments)
	{
		this->State.s_Name = Name;
		this->State.s_CalledOriginal = false;
		this->State.s_Reserved = false;

		this->Objects = Arguments;
		this->pfnOriginal = pfnCall;
	}
};