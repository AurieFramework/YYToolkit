#include "DoCallScript.hpp"
#include "../../Features/API/API.hpp"

// FIX THIS AWFUL MESS CARRIED FROM PROJECT DELTA PLEASE!!!
static uint8_t FixMyBrokenShit(uint32_t Base)
{
	constexpr char sub_esp_12[] = { "\x83\xEC\x0C" };

	if (memcmp(reinterpret_cast<void*>(Base - 4), sub_esp_12, 3) == 0)
		return 4;

	uint8_t Instruction;
	memcpy(&Instruction, reinterpret_cast<const void*>(Base - 1), 1);

	if (Instruction > 0x4F && Instruction < 0x58) /* push eax <-> push edi */
	{
		/* TS!US version? */
		uint8_t SecondInstruction;
		memcpy(&SecondInstruction, reinterpret_cast<const void*>(Base - 2), 1);

		if (SecondInstruction > 0x4F && SecondInstruction < 0x58) /* push eax <-> push edi */
			return 2;

		return 1;
	}

	return 0;
}

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
	auto ModuleInfo = API::GetModuleInfo();

	unsigned long Pattern = API::FindPattern("\x8B\x00\x24\x00\x00\x8B\x00\x24\x00\x85\x00\x75\x0E\x68", "x?x??x?x?x?xxx", ModuleInfo.Base, ModuleInfo.Size);

	if (!Pattern)
		return nullptr;

	// :clown:
	Pattern -= FixMyBrokenShit(Pattern);

	return reinterpret_cast<void*>(Pattern);
}
