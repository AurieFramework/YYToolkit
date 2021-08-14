#pragma once
#include <Windows.h>

namespace Hooks
{
	namespace WindowProc
	{
		LRESULT __stdcall Function(HWND hwnd, unsigned int Msg, WPARAM w, LPARAM l);
		void _SetWindowsHook();

		inline decltype(&Function) pfnOriginal;
	}
}