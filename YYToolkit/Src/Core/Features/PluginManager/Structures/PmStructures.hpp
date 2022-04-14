#pragma once
#include "../../../SDK/Plugins/Plugins.hpp"
#include <list>
#include <set>
#include <string>

struct CallbackAttributes_t
{
	EventType CallbackType;
	FNEventHandler Callback;
	void* Argument;

	bool operator==(const CallbackAttributes_t& other) const
	{
		return (this->Callback == other.Callback && this->CallbackType == other.CallbackType);
	}
};

struct ExportedRoutine_t
{
	void* pfnRoutine;
	std::string sRoutineName;

	// needed for std::set
	bool operator==(const ExportedRoutine_t& other) const
	{
		return other.pfnRoutine == this->pfnRoutine;
	}

	bool operator<(const ExportedRoutine_t& other) const
	{
		return (uintptr_t)(other.pfnRoutine) < (uintptr_t)(this->pfnRoutine);
	}
};

struct PluginAttributes_t
{
private:
	YYTKPlugin PluginObject;
public:
	std::list<CallbackAttributes_t> RegisteredCallbacks;
	std::set<ExportedRoutine_t> Exports;

	YYTKPlugin& GetPluginObject()
	{
		return PluginObject;
	}

	const YYTKPlugin& GetPluginObject() const
	{
		return PluginObject;
	}

	PluginAttributes_t(YYTKPlugin& pPlugin)
	{
		this->PluginObject = pPlugin;
	}

	bool operator==(const YYTKPlugin& other) const 
	{
		return this->GetPluginObject().PluginStart == other.PluginStart;
	}

	bool operator==(const PluginAttributes_t& other) const
	{
		return this->GetPluginObject().PluginStart == this->GetPluginObject().CoreStart;
	}
};