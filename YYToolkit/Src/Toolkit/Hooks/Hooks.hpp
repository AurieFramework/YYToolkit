#pragma once
#include "../../Shared.hpp"
#include <d3d9.h>
#include <d3d11.h>
namespace Tool::Hooks
{
	void* oPresent = nullptr;
	void* oEndScene = nullptr;
	void* oDoCallScript = nullptr;
	void* oCode_Execute = nullptr;
	void* oYYError = nullptr;

	extern char* DoCallScript(void* pScript, int argc, char* Esp, void* VM, YYObjectBase* pLocals, YYObjectBase* pArgs);
	extern bool Code_Execute(YYObjectBase* pSelf, YYObjectBase* pOther, CCode* Code, RValue* Res, int Flags);
	extern void YYError(const char* pFormat, ...);
}