#include "GR_Draw_Text_Transformed.hpp"
#include "../../../Features/API/API.hpp"
#include "../../../Utils/Error/Error.hpp"
#include "../../../Features/PluginManager/PluginManager.hpp"

namespace Hooks
{
	namespace GR_Draw_Text_Transformed
	{
		void Function(float x, float y, const char* str, int linesep, int linewidth, float xsc, float ysc, float angle)
		{
			API::PluginManager::CallTextCallbacks(x, y, str, linesep, linewidth);

			return pfnOriginal(x, y, str, linesep, linewidth, xsc, ysc, angle);
		}

		void* GetTargetAddress()
		{
			TRoutine Routine = nullptr;

			if (!API::GetFunctionByName("draw_text_transformed", Routine))
			{
				Utils::Error::Error(false, "Failed to find the draw_text_transformed_color function.");
				return nullptr;
			}

			unsigned long Pattern = API::FindPattern("\xE8\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x24", "x?????x????xxx", reinterpret_cast<unsigned long>(Routine), 0xFFu);

			if (!Pattern)
				return nullptr;

			unsigned long Relative = *reinterpret_cast<unsigned long*>(Pattern + 1);
			Relative = (Pattern + 5) + Relative; // eip = instruction base + 5 + relative offset

			return reinterpret_cast<void*>(Relative);
		}
	}
}

