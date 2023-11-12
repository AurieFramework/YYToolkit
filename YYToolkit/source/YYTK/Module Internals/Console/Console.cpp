#include "../Module Internals.hpp"
#include <iostream>

namespace YYTK
{
	namespace Internal
	{
		std::string CmpParseVa(
			IN const char* Format,
			IN va_list Arguments
		)
		{
			constexpr size_t max_length = 1024;
			size_t length = strlen(Format);

			if (length >= max_length)
				return "<string too long to print>";

			char buffer[max_length] = { 0 };

			strncpy_s(buffer, Format, max_length);
			vsprintf_s(buffer, Format, Arguments);

			return buffer;
		}

		void CmpSetTextColor(
			IN CmColor color
		)
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, color);
		}

		void CmpCreateConsole()
		{
			// Open and allocate streams needed for the console to work
			AllocConsole();

			FILE* dummy_file;
			freopen_s(&dummy_file, "CONIN$", "r", stdin);
			freopen_s(&dummy_file, "CONOUT$", "w", stderr);
			freopen_s(&dummy_file, "CONOUT$", "w", stdout);

			// Disable the console quick edit mode - this prevents copypasting, but also 
			// prevents accidental clicks into the console from suspending the entire tool.
			HANDLE input_stream = GetStdHandle(STD_INPUT_HANDLE);
			DWORD console_mode;
			GetConsoleMode(input_stream, &console_mode);
			//SetConsoleMode(input_stream, ENABLE_EXTENDED_FLAGS | (console_mode & ~ENABLE_QUICK_EDIT_MODE));

			// Write credits
			CmWriteOutput(CM_LIGHTBLUE, "YYToolkit by @archie_uwu");
		}
	}

	void CmWriteInfo(
		IN std::string_view Format,
		IN ...
	)
	{
		// Parse the VA arguments
		va_list va_args;
		va_start(va_args, Format);
		std::string formatted_output = Internal::CmpParseVa(Format.data(), va_args);
		va_end(va_args);

		printf("[I] %s\n", formatted_output.c_str());
	}

	void CmWriteOutput(
		IN CmColor Color, 
		IN std::string_view Format,
		IN ...
	)
	{
		// Parse the VA arguments
		va_list va_args;
		va_start(va_args, Format);
		std::string formatted_output = Internal::CmpParseVa(Format.data(), va_args);
		va_end(va_args);

		// Print the text
		Internal::CmpSetTextColor(Color);
		printf("%s\n", formatted_output.c_str());
		Internal::CmpSetTextColor(CM_WHITE);
	}

	void CmWriteWarning(
		IN std::string_view Format,
		IN ...
	)
	{
		// Parse the VA arguments
		va_list va_args;
		va_start(va_args, Format);
		std::string formatted_output = Internal::CmpParseVa(Format.data(), va_args);
		va_end(va_args);

		// Print the text
		Internal::CmpSetTextColor(CM_LIGHTYELLOW);
		printf("[!] %s\n", formatted_output.c_str());
		Internal::CmpSetTextColor(CM_WHITE);
	}

	void CmWriteError(
		IN std::string_view Filepath,
		IN const int& Line, 
		IN std::string_view Format,
		IN ...
	)
	{
		// Parse the VA arguments
		va_list va_args;
		va_start(va_args, Format);
		std::string formatted_output = Internal::CmpParseVa(Format.data(), va_args);
		va_end(va_args);
		
		// Get the filename instead of the full path
		size_t last_path_delimiter = Filepath.find_last_of("/\\");

		// Make sure a valid path with a filename was passed in
		if (last_path_delimiter == std::string::npos || last_path_delimiter == Filepath.length())
			return;

		std::string_view filename = Filepath.substr(last_path_delimiter + 1);

		// Print the output together with our fancy formatting
		Internal::CmpSetTextColor(CM_WHITE);
		printf("[");
		Internal::CmpSetTextColor(CM_LIGHTBLUE);
		printf("%s(%d)", filename.data(), Line);
		Internal::CmpSetTextColor(CM_WHITE);
		printf("]");
		Internal::CmpSetTextColor(CM_BRIGHTWHITE);
		printf(" => ");
		Internal::CmpSetTextColor(CM_LIGHTRED);
		printf("%s\n", formatted_output.c_str());
		Internal::CmpSetTextColor(CM_WHITE);
	}
}