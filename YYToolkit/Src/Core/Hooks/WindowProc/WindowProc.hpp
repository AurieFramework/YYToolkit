#pragma once
#include <Windows.h>

namespace Hooks::WindowProc
{
	LRESULT __stdcall Function(HWND hwnd, unsigned int Msg, WPARAM w, LPARAM l);
	void _SetWindowsHook();

	inline decltype(&Function) pfnOriginal;
}