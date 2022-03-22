#include "Plugins.hpp"
#include "../FwdDecls/FwdDecls.hpp"

DllExport inline const char* __PluginGetSDKVersion()
{
	return YYSDK_VERSION;
}