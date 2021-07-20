#include "../Hooks.hpp"
#include "../../Utils/Error.hpp"
#include "../../Features/AUMI_API/Exports.hpp"

static void SetupDescriptor(DXGI_SWAP_CHAIN_DESC* pDesc)
{
	YYTKTrace(__FUNCTION__ "()", __LINE__);

	HWND hwWindow; RValue Result;

	if (auto Status = AUMI_CallBuiltinFunction("window_handle", &Result, 0, 0, 0, 0))
		Utils::Error::Error(1, "Unspecified error while calling window_handle.\nError Code: %i", Status);

	if (!Result.Pointer)
		Utils::Error::Error(1, "Cannot obtain the window handle.");

	pDesc->BufferCount = 1;
	pDesc->BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	pDesc->BufferDesc = { 0, 0, { 60, 1 }, DXGI_FORMAT_R8G8B8A8_UNORM };
	pDesc->OutputWindow = (HWND)Result.Pointer;

	if (auto Status = AUMI_CallBuiltinFunction("window_get_fullscreen", &Result, 0, 0, 0, 0))
		Utils::Error::Error(1, "Unspecified error while calling window_get_fullscreen.\nError Code: %i", Status);

	pDesc->Windowed = !(bool)Result.Value;

	pDesc->Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	pDesc->SampleDesc = { 1, 0 };
	pDesc->SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
}

namespace Hooks
{
	HRESULT __stdcall Present(IDXGISwapChain* _this, unsigned int Sync, unsigned int Flags)
	{
		using Fn = decltype(&Present);

		return ((Fn)oPresent)(_this, Sync, Flags);
	}

	void* Hooks::Present_Address()
	{
		YYTKTrace(__FUNCTION__ "()", __LINE__);

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