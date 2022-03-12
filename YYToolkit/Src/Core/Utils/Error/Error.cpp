#include <Windows.h>
#include <string>
#include "Error.hpp"
#include "../../Features/API/API.hpp"

static std::string ParseVA(const char* fmt, va_list Args)
{
	const static size_t MaxStringLength = 1024;

	char Buf[MaxStringLength];
	memset(Buf, 0, MaxStringLength);

	strncpy(Buf, fmt, MaxStringLength);

	vsprintf_s(Buf, fmt, Args);

	return std::string(Buf);
}

namespace Utils::Error
{
	void SetPrintColor(Color color)
	{
		static HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, static_cast<WORD>(color));
	}

	void Error(bool critical, const char* fmt, ...)
	{
		va_list vaArgs;
		va_start(vaArgs, fmt);
		auto String = ParseVA(fmt, vaArgs);
		va_end(vaArgs);

		if (critical)
		{
			MessageBoxA(0, String.c_str(), "Sorry!", MB_TOPMOST | (critical ? MB_ICONERROR : MB_ICONWARNING) | MB_OK);
			exit(0);
		}
		else
		{
			SetPrintColor(CLR_RED);
			printf("[Error] %s\n", String.c_str());
			SetPrintColor(CLR_DEFAULT);
		}
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