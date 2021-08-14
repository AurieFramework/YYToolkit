#pragma once
#include <Windows.h>

namespace Hooks
{
	namespace MessageBoxW
	{
		int __stdcall Function(HWND Hwnd, LPCWSTR lpwText, LPCWSTR lpwCaption, UINT Type);
		void* GetTargetAddress();

		inline decltype(&Function) pfnOriginal;
	}
}