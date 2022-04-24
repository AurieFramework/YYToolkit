#include "../../Features/API/API.hpp"
#include "../../SDK/Structures/Documented/YYRValue/YYRValue.hpp"
#include "../../SDK/FwdDecls/FwdDecls.hpp"
#include "Logging.hpp"
#include <Windows.h>

namespace Utils::Logging
{
	std::string ParseVA(const char* fmt, va_list Args)
	{
		const static size_t MaxStringLength = 1024;

		if (strlen(fmt) >= 1024)
			Utils::Logging::Critical(__FILE__, __LINE__, "Buffer overflow attempted. Too long of a string sent to ParseVA.");

		char Buf[MaxStringLength];
		memset(Buf, 0, MaxStringLength);

		strncpy_s(Buf, fmt, MaxStringLength);

		vsprintf_s(Buf, fmt, Args);

		return std::string(Buf);
	}

	void SetPrintColor(Color color)
	{
		static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, static_cast<WORD>(color));
	}

	void Error(const char* File, const int& Line, const char* fmt, ...)
	{
		va_list vaArgs;
		va_start(vaArgs, fmt);
		auto String = ParseVA(fmt, vaArgs);
		va_end(vaArgs);

		std::string sFileName(File);
		size_t LastSlashPos = sFileName.find_last_of('\\');

		// We don't wanna do anything if the slash isn't in the string
		// Nor do we wanna cut it if it's at the end, since that +1 is gonna crash!
		if (LastSlashPos != std::string::npos && LastSlashPos != sFileName.length())
		{
			sFileName = sFileName.substr(LastSlashPos + 1);
		}
		
		SetPrintColor(CLR_DEFAULT);
		printf("[");
		SetPrintColor(CLR_BLUE);
		printf("%s(%d)", sFileName.c_str(), Line);
		SetPrintColor(CLR_DEFAULT);
		printf("]");
		SetPrintColor(CLR_WHITE);
		printf(" => ");
		SetPrintColor(CLR_TANGERINE);
		printf("%s\n", String.c_str());
		SetPrintColor(CLR_DEFAULT);
	}

	void Critical(const char* File, const int& Line, const char* fmt, ...)
	{
		// Get VA sorted first
		va_list vaArgs;
		va_start(vaArgs, fmt);
		auto sMessage = ParseVA(fmt, vaArgs);
		va_end(vaArgs);

		std::string sFinalText =
			"A fatal error has occured inside YYToolkit.\n"
			"Please report this occurence to the GitHub issue tracker.\n\n";

		sFinalText += "Message: " + sMessage + "\n\n";

		sFinalText += "Version: YYToolkit " + std::string(YYSDK_VERSION) + "\n";

		if (API::gAPIVars.Globals.g_bWasPreloaded)
			sFinalText += "Early Injection: Enabled\n";
		else
			sFinalText += "Early Injection: Disabled\n";
		
		sFinalText += "File: " + std::string(File) + "\n";
		sFinalText += "Line: " + std::to_string(Line) + "\n";
		sFinalText += "Debugger Present: " + std::string(IsDebuggerPresent() ? "Yes" : "No");

		MessageBoxA(0, sFinalText.c_str(), "YYToolkit - Fatal Error", MB_OK | MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);

		exit(0);
	}

	void Message(Color C, const char* fmt, ...)
	{
		va_list vaArgs;
		va_start(vaArgs, fmt);
		auto String = ParseVA(fmt, vaArgs);
		va_end(vaArgs);

		SetPrintColor(C);
		printf("%s\n", String.c_str());
		SetPrintColor(CLR_DEFAULT);
	}

	void NoNewlineMessage(Color C, const char* fmt, ...)
	{
		va_list vaArgs;
		va_start(vaArgs, fmt);
		auto String = ParseVA(fmt, vaArgs);
		va_end(vaArgs);

		SetPrintColor(C);
		printf("%s", String.c_str());
		SetPrintColor(CLR_DEFAULT);
	}

	std::string YYTKStatus_ToString(YYTKStatus Status)
	{
		switch (Status)
		{
		case YYTK_OK:
			return "YYTK_OK";
		case YYTK_FAIL:
			return "YYTK_FAIL";
		case YYTK_INVALIDARG:
			return "YYTK_INVALIDARG";
		case YYTK_INVALIDRESULT:
			return "YYTK_INVALIDRESULT";
		case YYTK_NOMATCH:
			return "YYTK_NOMATCH";
		case YYTK_UNAVAILABLE:
			return "YYTK_UNAVAILABLE";
		case YYTK_NOT_FOUND:
			return "YYTK_NOT_FOUND";
		default:
			return std::to_string(Status);
		}
	}
}