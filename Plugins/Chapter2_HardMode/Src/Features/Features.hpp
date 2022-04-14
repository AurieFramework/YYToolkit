#pragma once
#define YYSDK_PLUGIN
#include "../SDK/SDK.hpp"	// Include the SDK.
#include <vector>			// Include the STL vector class.

namespace Features
{
	YYRValue CallBuiltinWrapper(CInstance* Instance, const char* Name, const std::vector<YYRValue>& rvArgs);

	void RemoveSavePoints(CInstance* Self);

	void ChangeEnemyStats(CInstance* Self, double KromerMul, double HPMul, double ATKMul);

	int GetSnowGraveProgression();
}