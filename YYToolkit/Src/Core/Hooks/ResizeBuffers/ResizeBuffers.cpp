#include "ResizeBuffers.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Error.hpp"
#include "../Present/Present.hpp"

static void SetupDescriptor(DXGI_SWAP_CHAIN_DESC* pDesc)
{
	pDesc->BufferCount = 1;
	pDesc->BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	pDesc->BufferDesc = { 0, 0, { 60, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM };
	pDesc->OutputWindow = (HWND)gAPIVars.Window_Handle;

	YYRValue Result;
	if (auto Status = API::CallBuiltinFunction(nullptr, nullptr, Result, 0, "window_get_fullscreen", nullptr))
		Utils::Error::Error(1, "Unspecified error while calling window_get_fullscreen.\nError Code: %i", Status);

	pDesc->Windowed = static_cast<bool>(Result);

	pDesc->Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	pDesc->SampleDesc = { 1, 0 };
	pDesc->SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
}

HRESULT __stdcall Hooks::ResizeBuffers::Function(IDXGISwapChain* _this, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	using namespace Hooks::Present;

	if (gAPIVars.RenderView)
		reinterpret_cast<ID3D11RenderTargetView*>(gAPIVars.RenderView)->Release();

	Plugins::RunResizeCallbacks(reinterpret_cast<void*&>(_this), BufferCount, Width, Height, NewFormat, SwapChainFlags);

	HRESULT _Result = pfnOriginal(_this, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	{
		ID3D11Texture2D* pBackBuffer;

		HRESULT ret = _this->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

		if (FAILED(ret))
			Utils::Error::Error(1, "Getting the back buffer failed.");

		ret = static_cast<ID3D11Device*>(gAPIVars.Window_Device)->CreateRenderTargetView(pBackBuffer, NULL, reinterpret_cast<ID3D11RenderTargetView**>(&gAPIVars.RenderView));

		if (FAILED(ret))
			Utils::Error::Error(1, "Creating the target view failed.");

		pBackBuffer->Release();
	}

	return _Result;
}

void* Hooks::ResizeBuffers::GetTargetAddress()
{
	using Fn = PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN;

	HMODULE Module = GetModuleHandleA("d3d11.dll");

	if (!Module)
		Utils::Error::Error(1, "Cannot obtain the D3D11.dll module.");

	Fn pFn = reinterpret_cast<Fn>(GetProcAddress(Module, "D3D11CreateDeviceAndSwapChain"));

	if (!pFn)
		Utils::Error::Error(1, "Cannot obtain the CreateDevice function pointer.");

	DXGI_SWAP_CHAIN_DESC Descriptor = { 0 };
	IDXGISwapChain* pSwap = nullptr;

	SetupDescriptor(&Descriptor);

	auto Result = pFn(0, D3D_DRIVER_TYPE_NULL, 0, 0, 0, 0, D3D11_SDK_VERSION, &Descriptor, &pSwap, 0, 0, 0);

	if (FAILED(Result))
		Utils::Error::Error(1, "Failed to create a dummy swapchain!");

	void** ppVMT = *reinterpret_cast<void***>(pSwap);
	void* pPresent = ppVMT[13]; // Literally the only thing changed.

	//Throw this away, it's useless now.
	pSwap->Release();

	return pPresent;
}
