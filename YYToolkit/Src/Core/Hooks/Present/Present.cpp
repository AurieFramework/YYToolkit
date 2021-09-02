#include "Present.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Error.hpp"

static void SetupDescriptor(DXGI_SWAP_CHAIN_DESC* pDesc)
{
	YYRValue Result;

	pDesc->BufferCount = 1;
	pDesc->BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	pDesc->BufferDesc = { 0, 0, { 60, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM };
	pDesc->OutputWindow = (HWND)gAPIVars.Window_Handle;

	if (auto Status = API::CallBuiltinFunction(0, 0, Result, 0, "window_get_fullscreen", 0))
		Utils::Error::Error(1, "Unspecified error while calling window_get_fullscreen.\nError Code: %i", Status);

	pDesc->Windowed = static_cast<bool>(Result);

	pDesc->Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	pDesc->SampleDesc = { 1, 0 };
	pDesc->SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
}

namespace Hooks::Present
{
	HRESULT __stdcall Function(IDXGISwapChain* _this, unsigned int Sync, unsigned int Flags)
	{
		return pfnOriginal(_this, Sync, Flags);
	}

	void* GetTargetAddress()
	{
		using Fn = PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN;

		HMODULE Module = GetModuleHandleA("d3d11.dll");

		if (!Module)
			Utils::Error::Error(1, "Cannot obtain the D3D11.dll module.");

		Fn pFn = (Fn)(GetProcAddress(Module, "D3D11CreateDeviceAndSwapChain"));

		if (!pFn)
			Utils::Error::Error(1, "Cannot obtain the CreateDevice function pointer.");

		DXGI_SWAP_CHAIN_DESC Descriptor = { 0 };
		IDXGISwapChain* pSwap = nullptr;

		SetupDescriptor(&Descriptor);

		auto Result = pFn(0, D3D_DRIVER_TYPE_NULL, 0, 0, 0, 0, D3D11_SDK_VERSION, &Descriptor, &pSwap, 0, 0, 0);

		if (FAILED(Result))
			Utils::Error::Error(1, "Failed to create a dummy swapchain!");

		void** ppVMT = *(void***)(pSwap);
		void* pPresent = ppVMT[8];

		//Throw these away, they're useless now.
		pSwap->Release();

		return pPresent;
	}
}