#include "EndScene.hpp"
#include "../../Features/AUMI_API/Exports.hpp"
#include "../../Features/Menu/Menu.hpp"
#include "../../Utils/Error.hpp"

namespace Hooks::EndScene
{
	HRESULT __stdcall Function(LPDIRECT3DDEVICE9 _this)
	{
		auto Result = pfnOriginal(_this);

		Tool::Menu::Initialize(_this);

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();

		Tool::Menu::Run();

		ImGui::Render();

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		ImGui::EndFrame();

		return Result;
	}

	void* GetTargetAddress()
	{
		YYTKTrace(__FUNCTION__ "()", __LINE__);

		void* ppTable[119];
		RValue Result;

		if (auto Status = AUMI_CallBuiltinFunction("window_device", &Result, 0, 0, 0, 0))
			Utils::Error::Error(1, "Failed to get the window device.\nError Code: %s", Utils::Error::YYTKStatus_ToString(Status).data());

		if (!Result.Pointer)
			Utils::Error::Error(1, "Failed to get the graphics device pointer.");

		memcpy(ppTable, *(void***)(Result.Pointer), sizeof(ppTable));

		if (!ppTable[42])
			Utils::Error::Error(1, "Failed to get the EndScene function pointer.");

		return ppTable[42];
	}
}	