#include "../Hooks.hpp"
#include "../../Features/AUMI_API/Exports.hpp"

namespace Hooks
{
	void YYError(const char* pFormat, ...)
	{
		// Code has executed unsuccessfully!
		va_list vaArgs;
		va_start(vaArgs, pFormat);

		char Message[2048] = { 0 };
		strncpy(Message, pFormat, 2048);

		vsprintf_s(Message, 2048, pFormat, vaArgs);

		MessageBoxA(0, Message, "Code Error!", MB_OK | MB_ICONERROR | MB_TOPMOST);

		return oYYError(pFormat, vaArgs);
	}

	void* YYError_Address()
	{
		YYTKTrace(__FUNCTION__ "()", __LINE__);

		AUMIFunctionInfo mInfo;

		if (AUMI_GetFunctionByName("camera_create", &mInfo))
			return NULL;

		unsigned long Pattern = 0;

		if (AUMI_FindPatternEx("\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4", "x????x????xx", (ULONG)mInfo.Function, 64, &Pattern))
			return nullptr;

		unsigned long Relative = *(unsigned long*)(Pattern + 6);
		Relative = (Pattern + 10) + Relative; // eip = instruction base + 5 + relative offset

		return (void*)Relative;
	}
}