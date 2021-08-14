#include "YYError.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Error.hpp"

namespace Hooks::YYError
{
	void Function(const char* pFormat, ...)
	{
		// Code has executed unsuccessfully!
		va_list vaArgs;
		va_start(vaArgs, pFormat);

		char Message[2048] = { 0 };
		strncpy(Message, pFormat, 2048);

		vsprintf(Message, pFormat, vaArgs);

		MessageBoxA(0, Message, "Code Error!", MB_OK | MB_ICONERROR | MB_TOPMOST);
	}

	void* GetTargetAddress()
	{
		FunctionInfo_t mInfo;

		if (auto Status = API::GetFunctionByName("camera_create", mInfo))
		{
			Utils::Error::Error(1, "Failed to find the camera_create function.\nError Code: %s", Utils::Error::YYTKStatus_ToString(Status));
			return nullptr;
		}

		unsigned long Pattern = API::FindPattern("\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4", "x????x????xx", reinterpret_cast<long>(mInfo.Function), 64);

		if (!Pattern)
			return nullptr;

		unsigned long Relative = *reinterpret_cast<unsigned long*>(Pattern + 6);
		Relative = (Pattern + 10) + Relative; // eip = instruction base + 5 + relative offset

		return reinterpret_cast<void*>(Relative);
	}
}
