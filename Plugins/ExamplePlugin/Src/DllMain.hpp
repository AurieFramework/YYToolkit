#pragma once
#define YYSDK_PLUGIN	// Make the API work with plugins
#include "SDK/SDK.hpp"	// Include the SDK headers

DllExport YYTKStatus PluginEntry(YYTKPlugin* PluginObject);
YYTKStatus PluginUnload();
YYTKStatus FrameCallback(YYTKEventBase* pEvent, void* OptionalArgument);