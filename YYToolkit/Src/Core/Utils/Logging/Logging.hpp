#include "../../SDK/Enums/Enums.hpp"
#include <string>

namespace Utils
{
	namespace Logging
	{
		std::string ParseVA(const char* fmt, va_list Args);

		void SetPrintColor(Color color);

		void Message(Color C, const char* fmt, ...);

		void Error(const char* File, const int& Line, const char* fmt, ...);

		void Critical(const char* File, const int& Line, const char* fmt, ...);

		void NoNewlineMessage(Color C, const char* fmt, ...);

		// Apparently this has to be inline and here, or else you get link errors
		// I don't understand the linker
		std::string YYTKStatus_ToString(YYTKStatus Status);
	}
}