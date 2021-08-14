#pragma once
#include <d3d9.h>

namespace Hooks
{
	namespace EndScene
	{
		HRESULT __stdcall Function(LPDIRECT3DDEVICE9 _this);
		void* GetTargetAddress();

		inline decltype(&Function) pfnOriginal;
	}
}