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
	// Call events scope
	{
		YYTKResizeBuffersEvent Event = YYTKResizeBuffersEvent(pfnOriginal, _this, BufferCount, Width, Height, NewFormat, SwapChainFlags);
		Plugins::RunHooks(&Event);

		if (Event.CalledOriginal())
			return Event.GetReturn();
	}

	// Release all needed resources
	if (gAPIVars.RenderView) 
	{
		ID3D11RenderTargetView* pRenderTarget = static_cast<decltype(pRenderTarget)>(gAPIVars.RenderView);
		ID3D11DeviceContext* pContext = static_cast<decltype(pContext)>(gAPIVars.DeviceContext);

		pContext->OMSetRenderTargets(0, 0, 0);
		pRenderTarget->Release();

		gAPIVars.RenderView = nullptr;
	}


	// Resize the game buffer
	HRESULT hr = pfnOriginal(_this, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	// Recreate the buffer
	{
		ID3D11Device* pDevice = static_cast<decltype(pDevice)>(gAPIVars.Window_Device);
		ID3D11RenderTargetView** ppRenderTarget = reinterpret_cast<decltype(ppRenderTarget)>(&gAPIVars.RenderView);
		ID3D11DeviceContext* pContext = static_cast<decltype(pContext)>(gAPIVars.DeviceContext);

		ID3D11Texture2D* pBuffer;
		_this->GetBuffer(0, IID_PPV_ARGS(&pBuffer));
		// TODO: Perform error handling here!

		pDevice->CreateRenderTargetView(pBuffer, NULL, ppRenderTarget);
		// TODO: Perform error handling here!
		pBuffer->Release();

		pContext->OMSetRenderTargets(1, ppRenderTarget, NULL);
	}

	return hr;
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
