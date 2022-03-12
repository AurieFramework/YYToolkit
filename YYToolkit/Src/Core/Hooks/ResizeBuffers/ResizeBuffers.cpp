#include "ResizeBuffers.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Error/Error.hpp"
#include "../../Utils/D3D11 Hooker/D3D11Hooker.hpp"

HRESULT __stdcall Hooks::ResizeBuffers::Function(IDXGISwapChain* _this, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
	// Call events scope
	{
		YYTKResizeBuffersEvent Event = YYTKResizeBuffersEvent(pfnOriginal, _this, BufferCount, Width, Height, NewFormat, SwapChainFlags);
		//Plugins::RunHooks(&Event);

		if (Event.CalledOriginal())
			return Event.GetReturn();
	}

	// Release all needed resources
	if (API::gAPIVars.Globals.g_pRenderView) 
	{
		API::gAPIVars.Globals.g_pDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
		API::gAPIVars.Globals.g_pRenderView->Release();

		API::gAPIVars.Globals.g_pRenderView = nullptr;
	}


	// Resize the game buffer
	HRESULT hr = pfnOriginal(_this, BufferCount, Width, Height, NewFormat, SwapChainFlags);

	// Recreate the buffer
	{
		ID3D11Device* pDevice = reinterpret_cast<ID3D11Device*>(API::gAPIVars.Globals.g_pWindowDevice);

		ID3D11Texture2D* pBuffer;
		_this->GetBuffer(0, IID_PPV_ARGS(&pBuffer));
		// TODO: Perform error handling here!

		pDevice->CreateRenderTargetView(pBuffer, NULL, &API::gAPIVars.Globals.g_pRenderView);
		// TODO: Perform error handling here!
		pBuffer->Release();

		API::gAPIVars.Globals.g_pDeviceContext->OMSetRenderTargets(1, &API::gAPIVars.Globals.g_pRenderView, NULL);
	}

	return hr;
}

void* Hooks::ResizeBuffers::GetTargetAddress()
{
	return Utils::D3D11::GetVMTEntry(13);
}
