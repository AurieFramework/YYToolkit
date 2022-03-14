#include "EndScene.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Logging/Logging.hpp"
#include "../../Features/PluginManager/PluginManager.hpp"

namespace Hooks
{
	namespace EndScene
	{
		HRESULT __stdcall Function(LPDIRECT3DDEVICE9 _this)
		{
			YYTKEndSceneEvent Event = YYTKEndSceneEvent(pfnOriginal, _this);
			API::PluginManager::RunHooks(&Event);

			if (Event.CalledOriginal())
				return Event.GetReturn();

			return pfnOriginal(_this);
		}

		void* GetTargetAddress()
		{
			void* ppTable[119];
			IDirect3DDevice9* pDevice = nullptr;

			pDevice = reinterpret_cast<IDirect3DDevice9*>(API::gAPIVars.Globals.g_pWindowDevice);

			if (pDevice == nullptr)
			{
				Utils::Logging::Error(
					__FILE__,
					__LINE__,
					"The D3D9 device was nullptr"
				);

				return nullptr;
			}
			
			memcpy(ppTable, *(void***)(pDevice), sizeof(ppTable));

			return ppTable[42];
		}
	}
}	