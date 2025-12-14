#include "PI.hpp"
using namespace Aurie;

namespace YYTK
{
	bool YYTKPrivateInterfaceImpl::YkIsLowerLevelInterfaceReady()
	{
		return g_ModuleInterface.m_FirstInitComplete && g_ModuleInterface.m_IsRunnerInterfaceReady;
	}

	void YYTKPrivateInterfaceImpl::YkWaitForLowerLevelInit(
		IN const char* Function
	)
	{
		if (!YkIsLowerLevelInterfaceReady())
			DbgPrintEx(LOG_SEVERITY_ERROR, "Low-level runner function '%s' called before runner interface is initialized!", Function);

		while (!YkIsLowerLevelInterfaceReady())
			std::this_thread::yield();
	}
}


