#pragma once
#include <d3d11.h>

namespace Hooks::ResizeBuffers
{
	HRESULT __stdcall Function(IDXGISwapChain* _this, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
	void* GetTargetAddress();

	inline decltype(&Function) pfnOriginal;
}