#include "../Utils/Error.hpp"
#include "../Utils/MH/MinHook.h"
#include "Code_Execute/Code_Execute.hpp"
#include "EndScene/EndScene.hpp"
#include "Hooks.hpp"
#include "MessageBoxW/MessageBoxW.hpp"
#include "Present/Present.hpp"
#include "ResizeBuffers/ResizeBuffers.hpp"
#include "WindowProc/WindowProc.hpp"
#include "YYError/YYError.hpp"
#include "../Features/API/API.hpp"

namespace Hooks
{
	void Initialize()
	{
		MH_Initialize();
		{
			// MessageBoxW hook
			if (void* lpFunc = MessageBoxW::GetTargetAddress())
			{
				auto Status = MH_CreateHook(lpFunc, MessageBoxW::Function, reinterpret_cast<void**>(&MessageBoxW::pfnOriginal));
				if (Status != MH_OK)
					Utils::Error::Error(false, "Unable to create a hook on MessageBoxW.\nThis means YYToolkit won't block game message boxes.\nError Code: %s", MH_StatusToString(Status));
				else
					MH_EnableHook(lpFunc);
			}

			// YYError Hook
			if (void* lpFunc = YYError::GetTargetAddress())
			{
				auto Status = MH_CreateHook(lpFunc, YYError::Function, reinterpret_cast<void**>(&YYError::pfnOriginal));
				if (Status != MH_OK)
					Utils::Error::Error(false, "Unable to create a hook on YYError.\nThis means YYToolkit won't block GML errors.\nError Code: %s", MH_StatusToString(Status));
				else
					MH_EnableHook(lpFunc);
			}

			if (void* lpFunc = Code_Execute::GetTargetAddress())
			{
				auto Status = MH_CreateHook(lpFunc, Code_Execute::Function, reinterpret_cast<void**>(&Code_Execute::pfnOriginal));
				if (Status != MH_OK)
					Utils::Error::Error(false, "Unable to create a hook on Code_Execute.\nThis means YYToolkit won't intercept code entries.\nError Code: %s", MH_StatusToString(Status));
				else
					MH_EnableHook(lpFunc);
			}

			// Present Hook (D3D11)
			if (GetModuleHandleA("d3d11.dll"))
			{
				if (void* lpFunc = Present::GetTargetAddress())
				{
					auto Status = MH_CreateHook(lpFunc, Present::Function, reinterpret_cast<void**>(&Present::pfnOriginal));
					if (Status != MH_OK)
						Utils::Error::Error(true, "Unable to create a hook on Present.\nError Code: %s", MH_StatusToString(Status));
					else
						MH_EnableHook(lpFunc);
				}

				if (void* lpFunc = ResizeBuffers::GetTargetAddress())
				{
					auto Status = MH_CreateHook(lpFunc, ResizeBuffers::Function, reinterpret_cast<void**>(&ResizeBuffers::pfnOriginal));
					if (Status != MH_OK)
						Utils::Error::Error(true, "Unable to create a hook on ResizeBuffers.\nError Code: %s", MH_StatusToString(Status));
					else
						MH_EnableHook(lpFunc);
				}
			}

			// EndScene Hook (D3D9)
			if (GetModuleHandleA("d3d9.dll"))
			{
				if (void* lpFunc = EndScene::GetTargetAddress())
				{
					auto Status = MH_CreateHook(lpFunc, EndScene::Function, reinterpret_cast<void**>(&EndScene::pfnOriginal));
					if (Status != MH_OK)
						Utils::Error::Error(true, "Unable to create a hook on EndScene.\nError Code: %s", MH_StatusToString(Status));
					else
						MH_EnableHook(lpFunc);
				}
			}

			WindowProc::_SetWindowsHook();
		}
	}

	void Uninitialize()
	{
		MH_DisableHook(MH_ALL_HOOKS);
		Sleep(100);
		MH_Uninitialize();

		SetWindowLong((HWND)(gAPIVars.Window_Handle), GWL_WNDPROC, (LONG)Hooks::WindowProc::pfnOriginal);

		Hooks::Present::pView->Release();
	}
}