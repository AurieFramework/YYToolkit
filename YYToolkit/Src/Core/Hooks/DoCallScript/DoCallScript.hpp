#pragma once
#include "../../SDK/FwdDecls/FwdDecls.hpp"

namespace Hooks
{
	namespace DoCallScript
	{
		char* Function(CScript* pScript, int argc, char* pStackPointer, VMExec* pVM, YYObjectBase* pLocals, YYObjectBase* pArguments);
		void* GetTargetAddress();

		inline decltype(&Function) pfnOriginal;
	}
}