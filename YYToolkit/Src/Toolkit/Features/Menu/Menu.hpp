#pragma once
#include "../../Utils/ImGui/imgui_impl_dx11.h"
#include "../../Utils/ImGui/imgui_impl_dx9.h"
#include "../../Utils/ImGui/imgui_impl_win32.h"

#include <d3d9.h>
#include <d3d11.h>

// A new namespace? :o
namespace Tool::Menu
{
	inline ImFont* pBaseFont = nullptr;

	void Initialize(IDXGISwapChain* pSwap, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11RenderTargetView** pView);
	void Initialize(LPDIRECT3DDEVICE9 pDevice);


	void Run();
}