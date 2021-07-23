#pragma once
#include "../../Utils/ImGui/imgui_impl_dx11.h"
#include "../../Utils/ImGui/imgui_impl_dx9.h"
#include "../../Utils/ImGui/imgui_impl_win32.h"

#include <d3d9.h>
#include <d3d11.h>

// A new namespace? :o
namespace Tool::Menu
{
	inline ImFont* pSmallerFont = nullptr; // Same as pBaseFont, but it's smaller :P
	inline ImFont* pBaseFont = nullptr;
	inline ImFont* pBiggerFont = nullptr; // Same as pBaseFont, but it's bigger :P
	
	inline float flGuiScale = 1.0f; // 1.0f == 100%
	inline int nTabPicked = 0;
	inline int nSubtabPicked = 0;

	void InitializeFont();
	void Initialize(IDXGISwapChain* pSwap, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11RenderTargetView** pView, bool force = false);
	void Initialize(LPDIRECT3DDEVICE9 pDevice);

	void Run();
}

// Variables
namespace Vars
{
	inline bool bEnableWatermark = true;
}

// ImGui extensions
namespace ImGui
{
	bool ButtonColored(const ImVec4& Color, const char* Label, const ImVec2& Size);
	
}