#include "../Module Internals.hpp"
#include <iostream>

namespace YYTK
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
		SetConsoleMode(input_stream, ENABLE_EXTENDED_FLAGS | (console_mode & ~ENABLE_QUICK_EDIT_MODE));

		// Write credits
		SetConsoleTitleA("YYToolkit Log");
		CmWriteOutput(CM_LIGHTBLUE, "YYToolkit by @archie_uwu");
	}

	void CmWriteInfo(
		IN std::string_view Format,
		IN ...
	)
	{
		// Parse the VA arguments
		va_list va_args;
		va_start(va_args, Format);
		std::string formatted_output = CmpParseVa(Format.data(), va_args);
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
		std::string formatted_output = CmpParseVa(Format.data(), va_args);
		va_end(va_args);

		// Print the text
		CmpSetTextColor(Color);
		printf("%s\n", formatted_output.c_str());
		CmpSetTextColor(CM_WHITE);
	}

	void CmWriteWarning(
		IN std::string_view Format,
		IN ...
	)
	{
		// Parse the VA arguments
		va_list va_args;
		va_start(va_args, Format);
		std::string formatted_output = CmpParseVa(Format.data(), va_args);
		va_end(va_args);

		// Print the text
		CmpSetTextColor(CM_LIGHTYELLOW);
		printf("[!] %s\n", formatted_output.c_str());
		CmpSetTextColor(CM_WHITE);
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
		std::string formatted_output = CmpParseVa(Format.data(), va_args);
		va_end(va_args);

		// Get the filename instead of the full path
		std::filesystem::path filepath = Filepath;
		if (filepath.has_filename())
			filepath = filepath.filename();

		std::string filename = filepath.string();

		// Print the output together with our fancy formatting
		CmpSetTextColor(CM_WHITE);
		printf("[");
		CmpSetTextColor(CM_LIGHTBLUE);
		printf("%s(%d)", filename.data(), Line);
		CmpSetTextColor(CM_WHITE);
		printf("]");
		CmpSetTextColor(CM_BRIGHTWHITE);
		printf(" => ");
		CmpSetTextColor(CM_LIGHTRED);
		printf("%s\n", formatted_output.c_str());
		CmpSetTextColor(CM_WHITE);
	}
}