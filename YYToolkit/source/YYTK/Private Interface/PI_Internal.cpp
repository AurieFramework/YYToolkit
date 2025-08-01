#include "PI.hpp"
using namespace Aurie;

namespace YYTK
{
	bool YYTKPrivateInterfaceImpl::YkIsLowerLevelInterfaceReady()
	{
		return g_ModuleInterface.m_FirstInitComplete && g_ModuleInterface.m_IsRunnerInterfaceReady;
	}

	void YYTKPrivateInterfaceImpl::YkWaitForLowerLevelInit()
	{
		while (!YkIsLowerLevelInterfaceReady())
			std::this_thread::yield();
	}
}


