#include "GR_Draw_Text_Transformed.hpp"
#include "../../../Features/API/API.hpp"
#include "../../../Utils/Error.hpp"

void Hooks::GR_Draw_Text_Transformed::Function(float x, float y, const char* str, int linesep, int linewidth, float xsc, float ysc, float angle)
{
	Plugins::RunDrawingCallbacks(x, y, str, linesep, linewidth);
	return pfnOriginal(x, y, str, linesep, linewidth, xsc, ysc, angle);
}

void* Hooks::GR_Draw_Text_Transformed::GetTargetAddress()
{
	FunctionInfo_t mInfo;

	if (auto Status = API::GetFunctionByName("draw_text_transformed", mInfo))
	{
		Utils::Error::Error(1, "Failed to find the draw_text_transformed function.\nError Code: %s", Utils::Error::YYTKStatus_ToString(Status).data());
		return nullptr;
	}

	unsigned long Pattern = API::FindPattern("\xE8\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x24", "x?????x????xxx", reinterpret_cast<long>(mInfo.Function), 0xFF);

	if (!Pattern)
		return nullptr;

	unsigned long Relative = *reinterpret_cast<unsigned long*>(Pattern + 1);
	Relative = (Pattern + 5) + Relative; // eip = instruction base + 5 + relative offset

	return reinterpret_cast<void*>(Relative);
}
