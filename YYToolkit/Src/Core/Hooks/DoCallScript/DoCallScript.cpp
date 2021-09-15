#include "DoCallScript.hpp"
#include "../../Features/API/API.hpp"

char* Hooks::DoCallScript::Function(CScript* pScript, int argc, char* pStackPointer, VMExec* pVM, YYObjectBase* pLocals, YYObjectBase* pArguments)
{
	YYTKScriptEvent Event = YYTKScriptEvent(pfnOriginal, pScript, argc, pStackPointer, pVM, pLocals, pArguments);
	Plugins::RunCallback(&Event);

	if (Event.CalledOriginal())
		return Event.GetReturn();

	return pfnOriginal(pScript, argc, pStackPointer, pVM, pLocals, pArguments);
}

void* Hooks::DoCallScript::GetTargetAddress()
{
	auto p = API::FindPattern("\x8B\x00\x24\x00\x00\x8B\x00\x24\x00\x85\x00\x75\x0E\x68", "x?x??x?x?x?xxx", 0, 0); // FIXME

	return reinterpret_cast<void*>(p);
}
