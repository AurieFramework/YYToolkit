#include "GR_Draw_Text_Color.hpp"
#include "../../../Features/API/API.hpp"
#include "../../../Utils/Error/Error.hpp"
#include "../../../Features/PluginManager/PluginManager.hpp"

namespace Hooks
{
	namespace GR_Draw_Text_Color
	{
		void Function(float x, float y, const char* str, int linesep, int linewidth, unsigned int c1, unsigned int c2, unsigned int c3, float alpha, unsigned int c4)
		{
			API::PluginManager::CallTextCallbacks(x, y, str, linesep, linewidth);

			return pfnOriginal(x, y, str, linesep, linewidth, c1, c2, c3, alpha, c4);
		}

		void* GetTargetAddress()
		{
			TRoutine Routine = nullptr;

			if (!API::GetFunctionByName("draw_text_color", Routine))
			{
				Utils::Error::Error(
					false,
					__FILE__,
					__LINE__,
					"The draw_text_color function couldn't be found"
				);

				return nullptr;
			}

			unsigned long Pattern = 0;

			Pattern = API::FindPattern("\xE8\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x2C", "x?????x????xxx", reinterpret_cast<unsigned long>(Routine), 0xFFu);

			// Fix for DR 1.10 apery
			if (Pattern == 0)
			{
				Pattern = API::FindPattern("\xE8\x00\x00\x00\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x2C", "x????????x????xxx", reinterpret_cast<unsigned long>(Routine), 0xFFu);
			}

			if (!Pattern)
				return nullptr;

			unsigned long Relative = *reinterpret_cast<unsigned long*>(Pattern + 1);
			Relative = (Pattern + 5) + Relative; // eip = instruction base + 5 + relative offset

			return reinterpret_cast<void*>(Relative);
		}

	}
}

