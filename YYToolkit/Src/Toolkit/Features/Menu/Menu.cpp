#include "Menu.hpp"
#include <Windows.h>
#include "../../Utils/Error.hpp"
#include "../../Utils/ImGui/imgui_internal.h"
#include "../../Features/AUMI_API/Exports.hpp"

void Tool::Menu::InitializeFont()
{
	auto& IO = ImGui::GetIO();

	char FontPath[MAX_PATH] = { 0 };
	GetEnvironmentVariableA("SystemRoot", FontPath, MAX_PATH);

	strcat(FontPath, "\\Fonts\\verdana.ttf"); // set to 'verdanab.ttf' for bold

	Tool::Menu::pBaseFont = IO.Fonts->AddFontFromFileTTF(FontPath, 18.0f, 0, 0);
	Tool::Menu::pSmallerFont = IO.Fonts->AddFontFromFileTTF(FontPath, 15.0f, 0, 0);
	Tool::Menu::pBiggerFont = IO.Fonts->AddFontFromFileTTF(FontPath, 24.0f, 0, 0);
	IO.Fonts->Build();
}

void Tool::Menu::Initialize(IDXGISwapChain* pSwap, ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ID3D11RenderTargetView** pView)
{
	static bool sbInitialized = false;

	if (sbInitialized)
		return;

	if (!pDevice || !pContext)
		return;

	RValue Result;
	if (AUMI_CallBuiltinFunction("window_handle", &Result, 0, 0, 0, 0))
		return Utils::Error::Error(0, "Failed to get the window handle!");

	auto InitializeRenderView = [pSwap, pDevice](ID3D11RenderTargetView** pView)
	{
		ID3D11Texture2D* pBackBuffer;

		HRESULT ret = pSwap->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

		if (FAILED(ret))
			Utils::Error::Error(1, "Getting the back buffer failed.");

		ret = pDevice->CreateRenderTargetView(pBackBuffer, NULL, pView);

		if (FAILED(ret))
			Utils::Error::Error(1, "Creating the target view failed.");

		pBackBuffer->Release();
	};

	ImGui::CreateContext();
	
	ImGui_ImplWin32_Init(Result.Pointer);
	ImGui_ImplDX11_Init(pDevice, pContext);

	InitializeFont();
	InitializeRenderView(pView);

	sbInitialized = true;
}

void Tool::Menu::Initialize(LPDIRECT3DDEVICE9 pDevice)
{
	static bool sbInitialized = false;

	if (sbInitialized)
		return;

	if (!pDevice)
		return;

	RValue Result;
	if (AUMI_CallBuiltinFunction("window_handle", &Result, 0, 0, 0, 0))
		return Utils::Error::Error(0, "Failed to get the window handle!");

	ImGui::CreateContext();

	ImGui_ImplWin32_Init(Result.Pointer);
	ImGui_ImplDX9_Init(pDevice);
	InitializeFont();

	sbInitialized = true;
}

bool ImGui::ButtonColored(const ImVec4& Color, const char* Label, const ImVec2& Size)
{
	PushStyleColor(ImGuiCol_Text, Color);
	bool Pressed = ImGui::Button(Label, Size);
	ImGui::PopStyleColor();
	return Pressed;
}