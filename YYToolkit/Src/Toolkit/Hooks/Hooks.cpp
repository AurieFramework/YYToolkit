#include "Hooks.hpp"
#include "../Utils/Error.hpp"
#include "../Utils/MH/MinHook.h"
#include "../Features/AUMI_API/Exports.hpp"

namespace Hooks
{
	void Initialize()
	{
		YYTKTrace(__FUNCTION__ "()", __LINE__);

		MH_Initialize();
		{
			// YYError Hook
			if (void* lpFunc = YYError_Address())
			{
				auto Status = MH_CreateHook(lpFunc, YYError, (PVOID*)&oYYError);
				if (Status != MH_OK)
					Utils::Error::Error(1, "Unable to create a hook on YYError.\nError Code: %i", MH_StatusToString(Status));

				MH_EnableHook(lpFunc);
			}

			// Code_Execute hook
			{
				void* lpFunc;

				{
					auto Status = AUMI_GetCodeExecuteAddress(&lpFunc);
					if (Status != YYTK_OK)
						Utils::Error::Error(1, "Unable to get a pointer to Code_Execute.\nError Code: %i", Status);
				}

				{
					auto Status = MH_CreateHook(lpFunc, Code_Execute, (PVOID*)&oCode_Execute);
					if (Status != MH_OK)
						Utils::Error::Error(1, "Unable to create a hook on Code_Execute.\nError Code: %s", MH_StatusToString(Status));
				}

				MH_EnableHook(lpFunc);
			}

			// Present Hook (D3D11)
			if (GetModuleHandleA("d3d11.dll"))
			{
				if (void* lpFunc = Present_Address())
				{
					auto Status = MH_CreateHook(lpFunc, Present, (PVOID*)&oPresent);
					if (Status != MH_OK)
						Utils::Error::Error(1, "Unable to create a hook on Present.\nError Code: %i", MH_StatusToString(Status));

					MH_EnableHook(lpFunc);
				}
			}

			if (GetModuleHandleA("d3d9.dll"))
			{
				if (void* lpFunc = EndScene_Address())
				{
					auto Status = MH_CreateHook(lpFunc, EndScene, (PVOID*)&oEndScene);
					if (Status != MH_OK)
						Utils::Error::Error(1, "Unable to create a hook on EndScene.\nError Code: %i", MH_StatusToString(Status));

					MH_EnableHook(lpFunc);
				}
			}

			WindowProc_Init();
		}
	}
}