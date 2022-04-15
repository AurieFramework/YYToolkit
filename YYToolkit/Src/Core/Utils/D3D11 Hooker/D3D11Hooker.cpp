#include "D3D11Hooker.hpp"
#include "../../Features/API/API.hpp"
#include "../Logging/Logging.hpp"

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
	pDesc->OutputWindow = API::gAPIVars.Globals.g_hwWindowHandle;
	pDesc->SampleDesc.Count = 1;
	pDesc->SampleDesc.Quality = 0;
	pDesc->SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	pDesc->Windowed = TRUE;
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
	const D3D_FEATURE_LEVEL featureLevelArray[3] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_10_1 };

	return pFn(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, featureLevelArray, 3, D3D11_SDK_VERSION, &Descriptor, &outSwapChain, nullptr, &outLevel, nullptr);
}

static HRESULT GetDummySwapChain_ReferenceMethod(PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN pFn, D3D_FEATURE_LEVEL& outLevel, IDXGISwapChain*& outSwapChain)
{
	DXGI_SWAP_CHAIN_DESC Descriptor;

	SetupDescriptor(&Descriptor);
	const D3D_FEATURE_LEVEL featureLevelArray[3] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_10_1 };

	return pFn(nullptr, D3D_DRIVER_TYPE_REFERENCE, NULL, 0, featureLevelArray, 3, D3D11_SDK_VERSION, &Descriptor, &outSwapChain, nullptr, &outLevel, nullptr);
}

IDXGISwapChain* Utils::D3D11::GetSwapChain()
{
	using Fn = PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN;

	HMODULE Module = GetModuleHandleA("d3d11.dll");
	D3D_FEATURE_LEVEL SupportedLevel;
	IDXGISwapChain* pSwapChain;
	HRESULT ResultBuffer = E_FAIL;

	Fn pFn = (Fn)(GetProcAddress(Module, "D3D11CreateDeviceAndSwapChain"));

	if (!pFn)
	{
		Utils::Logging::Error(
			__FILE__,
			__LINE__,
			"The D3D11CreateDeviceAndSwapChain function couldn't be found"
		);

		return nullptr;
	}

	ResultBuffer = GetDummySwapChain_NullMethod(pFn, SupportedLevel, pSwapChain);

	if (FAILED(ResultBuffer))
	{
		Utils::Logging::Message(CLR_RED, "Dummy swapchain (NULL Method) failed.");
		Utils::Logging::Message(CLR_RED, "  - Returned: 0x%X", ResultBuffer);
		Utils::Logging::Message(CLR_RED, "  - Supported level: 0x%X", SupportedLevel);

		ResultBuffer = GetDummySwapChain_HardwareMethod(pFn, SupportedLevel, pSwapChain);

		if (FAILED(ResultBuffer))
		{
			Utils::Logging::Message(CLR_RED, "Dummy swapchain (HW Method) failed.");
			Utils::Logging::Message(CLR_RED, "  - Returned: 0x%X", ResultBuffer);
			Utils::Logging::Message(CLR_RED, "  - Supported level: 0x%X", SupportedLevel);

			ResultBuffer = GetDummySwapChain_ReferenceMethod(pFn, SupportedLevel, pSwapChain);

			if (FAILED(ResultBuffer))
			{
				Utils::Logging::Message(CLR_RED, "Dummy swapchain (Reference Method) failed.");
				Utils::Logging::Message(CLR_RED, "  - Returned: 0x%X", ResultBuffer);
				Utils::Logging::Message(CLR_RED, "  - Supported level: 0x%X", SupportedLevel);

				Utils::Logging::Critical(__FILE__, __LINE__, "Failed to acquire a dummy swapchain");

				return nullptr;
			}
		}
	}
	return pSwapChain;
}

void* Utils::D3D11::GetVMTEntry(int index)
{
	IDXGISwapChain* pSwapChain = GetSwapChain();

	if (!pSwapChain)
	{
		Utils::Logging::Error(__FILE__, __LINE__, "GetSwapChain() returned nullptr");

		return nullptr;
	}

	void** ppVMT = *(void***)(pSwapChain);
	
	if (!ppVMT)
	{
		Utils::Logging::Error(__FILE__, __LINE__, "ppVMT was nullptr");

		return nullptr;
	}

	void* Result = ppVMT[index];

	pSwapChain->Release();
	pSwapChain = nullptr;

	return Result;
}
