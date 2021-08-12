#pragma once
#include "../../SDK/SDK.hpp"

namespace Hooks::Code_Execute
{
	bool Function(CInstance* pSelf, CInstance* pOther, CCode* Code, RValue* Res, int Flags);
	void* GetTargetAddress();

	inline decltype(&Function) pfnOriginal;
}