#include "YYError.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Logging/Logging.hpp"
#include "../../Features/PluginManager/PluginManager.hpp"
#include "../../SDK/Plugins/YYTKEvent/YYTKEvent.hpp"

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

				va_list vaArgs{};
				va_start(vaArgs, pFormat);

				char Message[2048] = { 0 };
				strncpy_s(Message, pFormat, 2048);

				vsprintf_s(Message, pFormat, vaArgs);

				va_end(vaArgs);

				MessageBoxA(0, Message, "Code Error!", MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
			}
		}

		void* GetTargetAddress()
		{
			TRoutine Routine;

			if (!API::GetFunctionByName("camera_create", Routine))
			{
				Utils::Logging::Error(
					__FILE__,
					__LINE__,
					"Failed to find the camera_create function"
				);

				return nullptr;
			}
#ifdef _WIN64
			uintptr_t Pattern = API::FindPattern("\x48\x83\xC4\x20\x5B\xE9\x00\x00\x00\x00", "xxxxxx????", reinterpret_cast<uintptr_t>(Routine), 64u);
#else
			uintptr_t Pattern = API::FindPattern("\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4", "x????x????xx", reinterpret_cast<uintptr_t>(Routine), 64u);
#endif
			if (!Pattern)
				return nullptr;

			uintptr_t Relative = *reinterpret_cast<unsigned long*>(Pattern + 6);
			Relative = (Pattern + 10) + Relative; // eip = instruction base + 5 + relative offset

			return reinterpret_cast<void*>(Relative);

		}
	}
}
