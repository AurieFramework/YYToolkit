#include "../Utils/Error.hpp"
#include "../Utils/MH/MinHook.h"
#include "Code_Execute/Code_Execute.hpp"
#include "EndScene/EndScene.hpp"
#include "Present/Present.hpp"
#include "WindowProc/WindowProc.hpp"
#include "YYError/YYError.hpp"

namespace Hooks
{
	void Initialize()
	{
		YYTKTrace(__FUNCTION__ "()", __LINE__);

		MH_Initialize();
		{
			// YYError Hook
			if (void* lpFunc = YYError::GetTargetAddress())
			{
				auto Status = MH_CreateHook(lpFunc, YYError::Function, (PVOID*)&YYError::pfnOriginal);
				if (Status != MH_OK)
					Utils::Error::Error(1, "Unable to create a hook on YYError.\nError Code: %s", MH_StatusToString(Status));

				MH_EnableHook(lpFunc);
			}

			if (void* lpFunc = Code_Execute::GetTargetAddress())
			{
				auto Status = MH_CreateHook(lpFunc, Code_Execute::Function, (PVOID*)&Code_Execute::pfnOriginal);
				if (Status != MH_OK)
					Utils::Error::Error(1, "Unable to create a hook on Code_Execute.\nError Code: %s", MH_StatusToString(Status));

				MH_EnableHook(lpFunc);
			}

			// Present Hook (D3D11)
			if (GetModuleHandleA("d3d11.dll"))
			{
				if (void* lpFunc = Present::GetTargetAddress())
				{
					auto Status = MH_CreateHook(lpFunc, Present::Function, (PVOID*)&Present::pfnOriginal);
					if (Status != MH_OK)
						Utils::Error::Error(1, "Unable to create a hook on Present.\nError Code: %s", MH_StatusToString(Status));

					MH_EnableHook(lpFunc);
				}
			}

			if (GetModuleHandleA("d3d9.dll"))
			{
				if (void* lpFunc = EndScene::GetTargetAddress())
				{
					auto Status = MH_CreateHook(lpFunc, EndScene::Function, (PVOID*)&EndScene::pfnOriginal);
					if (Status != MH_OK)
						Utils::Error::Error(1, "Unable to create a hook on EndScene.\nError Code: %s", MH_StatusToString(Status));

					MH_EnableHook(lpFunc);
				}
			}

			WindowProc::_SetWindowsHook();
		}
	}
}