#include "../SDK/SDK.hpp"

namespace Utils
{
	namespace Error
	{
		void Error(bool critical, const char* fmt, ...);

		void Message(const char* fmt, ...);

		// Apparently this has to be inline and here, or else you get link errors
		// I don't understand the linker
		const char* YYTKStatus_ToString(YYTKStatus Status);
	}
}