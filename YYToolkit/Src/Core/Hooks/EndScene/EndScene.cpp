#include "EndScene.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Error.hpp"

namespace Hooks::EndScene
{
	HRESULT __stdcall Function(LPDIRECT3DDEVICE9 _this)
	{
		using YYTKEndSceneEvent = YYTKEvent<HRESULT, HRESULT(__stdcall*)(LPDIRECT3DDEVICE9), EventType::EVT_ENDSCENE, LPDIRECT3DDEVICE9>;

		YYTKEndSceneEvent Event = YYTKEndSceneEvent(&Function, _this);
		Plugins::RunCallback(&Event);

		if (Event.CalledOriginal())
			return Event.GetReturn();

		return pfnOriginal(_this);
	}

	void* GetTargetAddress()
	{
		void* ppTable[119];

		memcpy(ppTable, *(void***)(gAPIVars.Window_Device), sizeof(ppTable));

		if (!ppTable[42])
			Utils::Error::Error(1, "Failed to get the EndScene function pointer.");

		return ppTable[42];
	}
}	