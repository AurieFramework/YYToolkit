#include "Present.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Error/Error.hpp"
#include "../../Utils/D3D11 Hooker/D3D11Hooker.hpp"
#include <mutex> // std::call_once
static std::once_flag g_CreatedRenderView;

namespace Hooks::Present
{
	HRESULT __stdcall Function(IDXGISwapChain* _this, unsigned int Sync, unsigned int Flags)
	{
		YYTKPresentEvent Event = YYTKPresentEvent(pfnOriginal, _this, Sync, Flags);

		//Plugins::RunHooks(&Event);

		std::call_once(g_CreatedRenderView, [&]()
			{
				ID3D11Device* pDevice = reinterpret_cast<ID3D11Device*>(API::gAPIVars.Globals.g_pWindowDevice);
				ID3D11RenderTargetView** ppRenderView = &API::gAPIVars.Globals.g_pRenderView;

				ID3D11Texture2D* pBackBuffer;
				_this->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
				pDevice->CreateRenderTargetView(pBackBuffer, NULL, ppRenderView);
				pBackBuffer->Release();
			}
		);

		if (Event.CalledOriginal())
			return Event.GetReturn();

		return pfnOriginal(_this, Sync, Flags);
	}

	void* GetTargetAddress()
	{
		ID3D11Device* pDevice = reinterpret_cast<ID3D11Device*>(API::gAPIVars.Globals.g_pWindowDevice);
		
		if (pDevice)
			pDevice->GetImmediateContext(&API::gAPIVars.Globals.g_pDeviceContext);

		return Utils::D3D11::GetVMTEntry(8);
	}
}