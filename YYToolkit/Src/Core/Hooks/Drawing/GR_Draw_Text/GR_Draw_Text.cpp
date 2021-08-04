#include "GR_Draw_Text.hpp"
#include "../../../Features/API/API.hpp"
#include "../../../Utils/Error.hpp"

void Hooks::GR_Draw_Text::Function(float x, float y, const char* str, int linesep, int linewidth)
{
	Plugins::RunDrawingCallbacks(x, y, str, linesep, linewidth);
	return pfnOriginal(x, y, str, linesep, linewidth);
}

void* Hooks::GR_Draw_Text::GetTargetAddress()
{
	FunctionInfo_t mInfo;

	if (auto Status = API::GetFunctionByName("draw_text", mInfo))
	{
		Utils::Error::Error(1, "Failed to find the draw_text function.\nError Code: %s", Utils::Error::YYTKStatus_ToString(Status).data());
		return nullptr;
	}

	unsigned long Pattern = API::FindPattern("\xE8\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x18", "x?????x????xxx", reinterpret_cast<long>(mInfo.Function), 0xFF);

	if (!Pattern)
		return nullptr;

	unsigned long Relative = *reinterpret_cast<unsigned long*>(Pattern + 1);
	Relative = (Pattern + 5) + Relative; // eip = instruction base + 5 + relative offset

	return reinterpret_cast<void*>(Relative);
}
