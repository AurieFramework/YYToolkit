#include "YYError.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Error/Error.hpp"
#include "../../Features/PluginManager/PluginManager.hpp"

namespace Hooks
{
	namespace YYError
	{
		void Function(const char* pFormat, ...)
		{
			// Call events scope
			{
				YYTKErrorEvent Event = YYTKErrorEvent(pfnOriginal, pFormat);
				API::PluginManager::RunHooks(&Event);

				if (Event.CalledOriginal())
					return;
			}

			// Override scope
			{

				va_list vaArgs;
				va_start(vaArgs, pFormat);

				char Message[2048] = { 0 };
				strncpy(Message, pFormat, 2048);

				vsprintf(Message, pFormat, vaArgs);

				va_end(vaArgs);

				MessageBoxA(0, Message, "Code Error!", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
			}
		}

		void* GetTargetAddress()
		{
			TRoutine Routine;

			if (!API::GetFunctionByName("camera_create", Routine))
			{
				Utils::Error::Error(
					false,
					__FILE__,
					__LINE__,
					"Failed to find the camera_create function"
				);

				return nullptr;
			}

			unsigned long Pattern = API::FindPattern("\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4", "x????x????xx", reinterpret_cast<unsigned long>(Routine), 64u);

			if (!Pattern)
				return nullptr;

			unsigned long Relative = *reinterpret_cast<unsigned long*>(Pattern + 6);
			Relative = (Pattern + 10) + Relative; // eip = instruction base + 5 + relative offset

			return reinterpret_cast<void*>(Relative);
		}
	}
}
