#pragma once
#include <d3d11.h>

namespace Hooks::Present
{
	inline ID3D11RenderTargetView* pView = nullptr;
	inline ID3D11Device* pDevice = nullptr;
	inline ID3D11DeviceContext* pContext = nullptr;

	HRESULT __stdcall Function(IDXGISwapChain* _this, unsigned int Sync, unsigned int Flags);
	void* GetTargetAddress();

	inline decltype(&Function) pfnOriginal;
}