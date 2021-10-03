#include "../Hooks.hpp"
#include "../../ImGui/imgui_impl_dx11.h"
#include "../../ImGui/imgui_impl_win32.h"
#include "../../Menu/Framework/Framework.hpp"
#include "../../Menu/Menu.hpp"
#include <mutex> //call_once

static ID3D11Device*			g_pd3dDevice		= nullptr;
static ID3D11DeviceContext*		g_pd3dDeviceContext = nullptr;
static ID3D11RenderTargetView** g_ppRenderView		= nullptr;
static std::once_flag			g_flPrepared;

// Drawing hook - responsible for drawing the menu in newer versions.
void Hooks::Present(YYTKPresentEvent* _evt, IDXGISwapChain* _this, UINT _Sync, UINT _Flags)
{
	// Initialize
	std::call_once(g_flPrepared, [&]()
		{
			if (!g_pCurrentPlugin)
				return;

			APIVars_t* pAPIVars = nullptr;

			// Get the API Vars
			g_pCurrentPlugin->GetCoreExport<YYTKStatus(*)(APIVars_t**)>("GetAPIVars")(&pAPIVars);

			if (!pAPIVars)
				return;

			g_pd3dDevice = reinterpret_cast<ID3D11Device*>(pAPIVars->Window_Device);
			g_pd3dDeviceContext = reinterpret_cast<ID3D11DeviceContext*>(pAPIVars->DeviceContext);
			g_ppRenderView = reinterpret_cast<ID3D11RenderTargetView**>(&pAPIVars->RenderView);

			ImGui::CreateContext();

			Framework::GUI::StyleFonts(ImGui::GetIO());
			Framework::GUI::ApplyStylingDark();

			ImGui_ImplWin32_Init(pAPIVars->Window_Handle);
			ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
		}
	);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	Menu::Draw();
	ImGui::ShowDemoWindow();

	ImGui::Render();
	g_pd3dDeviceContext->OMSetRenderTargets(1, g_ppRenderView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}