#include <Windows.h>
#include <string>
#include "Error.hpp"
#include "../Features/API/API.hpp"

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
	void Error(bool critical, const char* fmt, ...)
	{
		va_list vaArgs;
		va_start(vaArgs, fmt);
		auto String = ParseVA(fmt, vaArgs);
		va_end(vaArgs);

		MessageBoxA(0, String.c_str(), "Sorry!", MB_TOPMOST | (critical ? MB_ICONERROR : MB_ICONWARNING) | MB_OK);

		if (critical)
			exit(0);
	}

	void Message(const char* Header, const char* Message)
	{
		NOTIFYICONDATAA IconNotifyData = { sizeof(NOTIFYICONDATAA) };
		IconNotifyData.hWnd = (HWND)gAPIVars.Window_Handle;
		IconNotifyData.uFlags = NIF_INFO;
		IconNotifyData.dwInfoFlags = NIIF_INFO;
		strncpy(IconNotifyData.szInfoTitle, Header, 64);
		strncpy(IconNotifyData.szInfo, Message, 256);
		Shell_NotifyIconA(NIM_DELETE, &IconNotifyData);
		Shell_NotifyIconA(NIM_ADD, &IconNotifyData);
	}
}