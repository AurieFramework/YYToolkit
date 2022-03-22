#include "GR_Draw_Text_TC_header.hpp"
#include "../../../Features/API/API.hpp"
#include "../../../Utils/Logging/Logging.hpp"
#include "../../../Features/PluginManager/PluginManager.hpp"
#include "../../../SDK/Plugins/YYTKEvent/YYTKEvent.hpp"

namespace Hooks
{
	namespace GR_Draw_Text_TC
	{
		void Function(float x, float y, const char* str, int linesep, int linewidth, float xsc, float ysc, float angle, unsigned int c1, unsigned int c2, unsigned int c3, float alpha, unsigned int c4)
		{
			API::PluginManager::CallTextCallbacks(x, y, str, linesep, linewidth);

			return pfnOriginal(x, y, str, linesep, linewidth, xsc, ysc, angle, c1, c2, c3, alpha, c4);
		}

		void* GetTargetAddress()
		{
			TRoutine Routine = nullptr;

			if (!API::GetFunctionByName("draw_text_transformed_color", Routine))
			{
				Utils::Logging::Error(
					__FILE__,
					__LINE__,
					"The draw_text_transformed_color couldn't be found"
				);

				return nullptr;
			}

			unsigned long Pattern = 0;

			Pattern = API::FindPattern("\xE8\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x38", "x?????x????xxx", reinterpret_cast<unsigned long>(Routine), 0xFFu);

			// Fix for DR 1.10 apery
			if (Pattern == 0)
			{
				Pattern = API::FindPattern("\xE8\x00\x00\x00\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x38", "x????????x????xxx", reinterpret_cast<unsigned long>(Routine), 0xFFu);
			}

			if (!Pattern)
				return nullptr;

			unsigned long Relative = *reinterpret_cast<unsigned long*>(Pattern + 1);
			Relative = (Pattern + 5) + Relative; // eip = instruction base + 5 + relative offset

			return reinterpret_cast<void*>(Relative);
		}
	}
}
