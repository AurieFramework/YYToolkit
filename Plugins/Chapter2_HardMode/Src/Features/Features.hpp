#pragma once
#ifndef YYSDK_PLUGIN
#define YYSDK_PLUGIN
#endif
#include "../SDK/SDK.hpp"	// Include the SDK.
#include <vector>			// Include the STL vector class.

namespace Features
{
	YYRValue CallBuiltinWrapper(YYTKPlugin* pPlugin, CInstance* Instance, const char* Name, const std::vector<YYRValue>& rvArgs);

	void RemoveSavePoints(YYTKPlugin* Plugin, CInstance* Self);

	void ChangeEnemyStats(YYTKPlugin* Plugin, CInstance* Self, double KromerMul, double HPMul, double ATKMul);

	bool IsSnowGraveRoute(YYTKPlugin* Plugin);
}