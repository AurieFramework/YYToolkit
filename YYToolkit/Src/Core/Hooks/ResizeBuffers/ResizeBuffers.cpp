#include "ResizeBuffers.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Error/Error.hpp"
#include "../../Utils/D3D11 Hooker/D3D11Hooker.hpp"

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
	return Utils::D3D11::GetVMTEntry(13);
}
