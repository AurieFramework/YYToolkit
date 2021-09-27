#include "Present.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Error.hpp"

static void SetupDescriptor(DXGI_SWAP_CHAIN_DESC* pDesc)
{
	pDesc->BufferCount = 2;
	pDesc->BufferDesc.Width = 0;
	pDesc->BufferDesc.Height = 0;
	pDesc->BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	pDesc->BufferDesc.RefreshRate.Numerator = 60;
	pDesc->BufferDesc.RefreshRate.Denominator = 1;
	pDesc->Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	pDesc->BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	pDesc->OutputWindow = (HWND)gAPIVars.Window_Handle;
	pDesc->SampleDesc.Count = 1;
	pDesc->SampleDesc.Quality = 0;
	pDesc->SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	YYRValue Result = YYRValue();
	if (auto Status = API::CallBuiltinFunction(0, 0, Result, 0, "window_get_fullscreen", 0))
		Utils::Error::Error(1, "Unspecified error while calling window_get_fullscreen.\nError Code: %i", Status);

	pDesc->Windowed = static_cast<bool>(Result);
}

static HRESULT GetDummySwapChain_NullMethod(PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN pFn, D3D_FEATURE_LEVEL& outLevel, IDXGISwapChain*& outSwapChain)
{
	DXGI_SWAP_CHAIN_DESC Descriptor;

	SetupDescriptor(&Descriptor);
	return pFn(0, D3D_DRIVER_TYPE_NULL, 0, 0, 0, 0, D3D11_SDK_VERSION, &Descriptor, &outSwapChain, 0, &outLevel, 0);
}

static HRESULT GetDummySwapChain_HardwareMethod(PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN pFn, D3D_FEATURE_LEVEL& outLevel, IDXGISwapChain*& outSwapChain)
{
	DXGI_SWAP_CHAIN_DESC Descriptor;

	SetupDescriptor(&Descriptor);
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	return pFn(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevelArray, 2, D3D11_SDK_VERSION, &Descriptor, &outSwapChain, nullptr, &outLevel, nullptr);
}

static HRESULT GetDummySwapChain_ReferenceMethod(PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN pFn, D3D_FEATURE_LEVEL& outLevel, IDXGISwapChain*& outSwapChain)
{
	DXGI_SWAP_CHAIN_DESC Descriptor;
	
	SetupDescriptor(&Descriptor);
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	return pFn(nullptr, D3D_DRIVER_TYPE_REFERENCE, NULL, 0, featureLevelArray, 2, D3D11_SDK_VERSION, &Descriptor, &outSwapChain, nullptr, &outLevel, nullptr);
}

namespace Hooks::Present
{
	HRESULT __stdcall Function(IDXGISwapChain* _this, unsigned int Sync, unsigned int Flags)
	{
		YYTKPresentEvent Event = YYTKPresentEvent(pfnOriginal, _this, Sync, Flags);
		Plugins::RunCallback(&Event);

		if (Event.CalledOriginal())
			return Event.GetReturn();

		return pfnOriginal(_this, Sync, Flags);
	}

	void* GetTargetAddress()
	{
		using Fn = PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN;

		HMODULE Module = GetModuleHandleA("d3d11.dll");
		D3D_FEATURE_LEVEL SupportedLevel;
		IDXGISwapChain* pSwapChain;
		HRESULT ResultBuffer = E_FAIL;

		if (!Module)
			Utils::Error::Error(1, "Cannot obtain the D3D11.dll module.");

		Fn pFn = (Fn)(GetProcAddress(Module, "D3D11CreateDeviceAndSwapChain"));

		if (!pFn)
		{
			Utils::Error::Error(false, "Cannot obtain the CreateDevice function pointer.");
			return nullptr;
		}

		ResultBuffer = GetDummySwapChain_NullMethod(pFn, SupportedLevel, pSwapChain);

		if (FAILED(ResultBuffer))
		{
			Utils::Error::Error(false, "Dummy swapchain (NULL Method) failed with error 0x%X. Obtained level 0x%X.", ResultBuffer, SupportedLevel);

			ResultBuffer = GetDummySwapChain_HardwareMethod(pFn, SupportedLevel, pSwapChain);

			if (FAILED(ResultBuffer))
			{
				Utils::Error::Error(false, "Dummy swapchain (HW Method) failed with error 0x%X. Obtained level 0x%X.", ResultBuffer, SupportedLevel);

				ResultBuffer = GetDummySwapChain_ReferenceMethod(pFn, SupportedLevel, pSwapChain);

				if (FAILED(ResultBuffer))
				{
					Utils::Error::Error(false, "Dummy swapchain (Ref Method) failed with error 0x%X. Obtained level 0x%X. Giving up.", ResultBuffer, SupportedLevel);
					return nullptr;
				}
			}
		}
		
		void** ppVMT = *(void***)(pSwapChain);
		void* pPresent = ppVMT[8];

		//Throw these away, they're useless now.
		pSwapChain->Release();

		// Setup gAPIVars members

		ID3D11Device* pDevice = static_cast<ID3D11Device*>(gAPIVars.Window_Device);
		
		if (pDevice)
			pDevice->GetImmediateContext(reinterpret_cast<ID3D11DeviceContext**>(&gAPIVars.DeviceContext));

		return pPresent;
	}
}