#pragma once
#include <d3d11.h>

namespace Hooks
{
	namespace Present
	{
		inline ID3D11RenderTargetView* pView = nullptr;

		HRESULT __stdcall Function(IDXGISwapChain* _this, unsigned int Sync, unsigned int Flags);
		void* GetTargetAddress();

		inline decltype(&Function) pfnOriginal;
	}
}