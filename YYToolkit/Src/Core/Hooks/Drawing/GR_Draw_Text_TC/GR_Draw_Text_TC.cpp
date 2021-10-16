#include "GR_Draw_Text_TC_header.hpp"
#include "../../../Features/API/API.hpp"
#include "../../../Utils/Error/Error.hpp"

namespace Hooks
{
	namespace GR_Draw_Text_TC
	{
		void Function(float x, float y, const char* str, int linesep, int linewidth, float xsc, float ysc, float angle, unsigned int c1, unsigned int c2, unsigned int c3, float alpha, unsigned int c4)
		{
			Plugins::CallTextCallbacks(x, y, str, linesep, linewidth);

			return pfnOriginal(x, y, str, linesep, linewidth, xsc, ysc, angle, c1, c2, c3, alpha, c4);
		}

		void* GetTargetAddress()
		{
			FunctionInfo_t mInfo;

			if (auto Status = API::GetFunctionByName("draw_text_transformed_color", mInfo))
			{
				Utils::Error::Error(1, "Failed to find the draw_text_transformed_color function.\nError Code: %s", Utils::Error::YYTKStatus_ToString(Status));
				return nullptr;
			}

			unsigned long Pattern = API::FindPattern("\xE8\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x38", "x?????x????xxx", reinterpret_cast<long>(mInfo.Function), 0xFF);

			if (!Pattern)
				return nullptr;

			unsigned long Relative = *reinterpret_cast<unsigned long*>(Pattern + 1);
			Relative = (Pattern + 5) + Relative; // eip = instruction base + 5 + relative offset

			return reinterpret_cast<void*>(Relative);
		}
	}
}
