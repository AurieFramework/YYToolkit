#include "../Hooks.hpp"
#include "../../ImGui/imgui_impl_dx9.h"
#include "../../ImGui/imgui_impl_win32.h"
#include "../../Menu/Framework/Framework.hpp"
#include "../../Menu/Menu.hpp"
#include <mutex> //call_once

static std::once_flag g_flPrepared;

void Hooks::EndScene(YYTKEndSceneEvent* _evt, LPDIRECT3DDEVICE9 _this)
{
	std::call_once(g_flPrepared, [&]()
		{
			if (!g_pCurrentPlugin)
				return;

			APIVars_t* pAPIVars = nullptr;

			// Get the API Vars
			g_pCurrentPlugin->GetCoreExport<YYTKStatus(*)(APIVars_t**)>("GetAPIVars")(&pAPIVars);

			if (!pAPIVars)
				return;

			ImGui::CreateContext();

			Framework::GUI::StyleFonts(ImGui::GetIO());
			Framework::GUI::ApplyStylingDark();

			ImGui_ImplWin32_Init(pAPIVars->Window_Handle);
			ImGui_ImplDX9_Init(reinterpret_cast<IDirect3DDevice9*>(pAPIVars->Window_Device));
		}
	);

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	Menu::Draw();
	ImGui::ShowDemoWindow();

	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}