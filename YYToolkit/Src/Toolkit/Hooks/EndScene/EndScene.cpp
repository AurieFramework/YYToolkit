#include "../Hooks.hpp"
#include "../../Features/AUMI_API/Exports.hpp"
#include "../../Utils/Error.hpp"

namespace Hooks
{
	HRESULT __stdcall EndScene(LPDIRECT3DDEVICE9 _this)
	{
		using Fn = decltype(&EndScene);

		return ((Fn)oEndScene)(_this);
	}

	void* EndScene_Address()
	{
		void* ppTable[119];
		RValue Result;

		if (auto Status = AUMI_CallBuiltinFunction("window_device", &Result, 0, 0, 0, 0))
			Utils::Error::Error(1, "Unspecified error while calling window_device.\nError Code: %i", Status);

		if (!Result.Pointer)
			Utils::Error::Error(1, "Failed to get the graphics device pointer.");

		memcpy(ppTable, *(void***)(Result.Pointer), sizeof(ppTable));

		if (!ppTable[42])
			Utils::Error::Error(1, "Failed to get the EndScene function pointer.");

		return ppTable[42];
	}
}