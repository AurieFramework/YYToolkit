#include "EndScene.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Error/Error.hpp"
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

			memcpy(ppTable, *(void***)(pDevice), sizeof(ppTable));

			return ppTable[42];
		}
	}
}	