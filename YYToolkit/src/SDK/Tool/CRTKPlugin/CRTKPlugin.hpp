#ifndef RTK_SDK_TOOL_CRTKPLUGIN_H_
#define RTK_SDK_TOOL_CRTKPLUGIN_H_

// Includes <cstdint>
#include "../CRTKObject/CRTKObject.hpp"

struct CRTKPlugin;
struct CRTKString;

using FNPluginInitEntry = void(*)(const CRTKString& PluginName, CRTKPlugin& PluginObject);
using FNPluginGetSDK = int32_t(*)();
using FNPluginUnload = void(*)(CRTKPlugin& PluginObject);

struct CRTKPlugin : CRTKObject
{
	virtual EObjectType GetObjectType() const override
	{
		return EObjectType::kPlugin;
	}

	CRTKPlugin()
	{
		this->m_BaseAddress = nullptr;
		this->m_PluginEntry = nullptr;
		this->m_PluginUnload = nullptr;
		this->m_PluginInitialize = nullptr;
	}

	void* m_BaseAddress;
	FNPluginInitEntry m_PluginEntry;
	FNPluginInitEntry m_PluginInitialize;
	FNPluginUnload m_PluginUnload;
};

#endif // RTK_SDK_TOOL_CRTKPLUGIN_H_