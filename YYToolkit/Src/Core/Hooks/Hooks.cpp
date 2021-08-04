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
#include "Drawing/GR_Draw_Text/GR_Draw_Text.hpp"
#include "Drawing/GR_Draw_Text_Color/GR_Draw_Text_Color.hpp"
#include "../Features/API/API.hpp"


namespace Hooks
{
	void Initialize()
	{
		MH_Initialize();
		{
			// MessageBoxW Hook - intercepts generic errors
			if (void* lpFunc = MessageBoxW::GetTargetAddress())
			{
				auto Status = MH_CreateHook(lpFunc, MessageBoxW::Function, reinterpret_cast<void**>(&MessageBoxW::pfnOriginal));
				if (Status != MH_OK)
					Utils::Error::Error(false, "Unable to create a hook on MessageBoxW.\nThis means YYToolkit won't block game message boxes.\nError Code: %s", MH_StatusToString(Status));
				else
					MH_EnableHook(lpFunc);
			}

			// YYError Hook - intercepts code errors
			if (void* lpFunc = YYError::GetTargetAddress())
			{
				auto Status = MH_CreateHook(lpFunc, YYError::Function, reinterpret_cast<void**>(&YYError::pfnOriginal));
				if (Status != MH_OK)
					Utils::Error::Error(false, "Unable to create a hook on YYError.\nThis means YYToolkit won't block GML errors.\nError Code: %s", MH_StatusToString(Status));
				else
					MH_EnableHook(lpFunc);
			}

			// Code_Execute Hook - intercepts code execution
			if (void* lpFunc = Code_Execute::GetTargetAddress())
			{
				auto Status = MH_CreateHook(lpFunc, Code_Execute::Function, reinterpret_cast<void**>(&Code_Execute::pfnOriginal));
				if (Status != MH_OK)
					Utils::Error::Error(false, "Unable to create a hook on Code_Execute.\nThis means YYToolkit won't intercept code entries.\nError Code: %s", MH_StatusToString(Status));
				else
					MH_EnableHook(lpFunc);
			}

			// GR_Draw_Text Hook - intercepts draw_text()
			if (void* lpFunc = GR_Draw_Text::GetTargetAddress())
			{
				auto Status = MH_CreateHook(lpFunc, GR_Draw_Text::Function, reinterpret_cast<void**>(&GR_Draw_Text::pfnOriginal));
				if (Status != MH_OK)
					Utils::Error::Error(false, "Unable to create a hook on GR_Draw_Text.\nThis means YYToolkit won't intercept certain drawing calls.\nError Code: %s", MH_StatusToString(Status));
				else
					MH_EnableHook(lpFunc);
			}

			// GR_Draw_Text_Color Hook - intercepts draw_text_color()
			if (void* lpFunc = GR_Draw_Text_Color::GetTargetAddress())
			{
				auto Status = MH_CreateHook(lpFunc, GR_Draw_Text_Color::Function, reinterpret_cast<void**>(&GR_Draw_Text_Color::pfnOriginal));
				if (Status != MH_OK)
					Utils::Error::Error(false, "Unable to create a hook on GR_Draw_Text_Color.\nThis means YYToolkit won't intercept certain drawing calls.\nError Code: %s", MH_StatusToString(Status));
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
						Utils::Error::Error(false, "Unable to create a hook on Present.\nError Code: %s", MH_StatusToString(Status));
					else
						MH_EnableHook(lpFunc);
				}
				
				// ResizeBuffers Hook (D3D11)
				if (void* lpFunc = ResizeBuffers::GetTargetAddress())
				{
					auto Status = MH_CreateHook(lpFunc, ResizeBuffers::Function, reinterpret_cast<void**>(&ResizeBuffers::pfnOriginal));
					if (Status != MH_OK)
						Utils::Error::Error(false, "Unable to create a hook on ResizeBuffers.\nError Code: %s", MH_StatusToString(Status));
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
						Utils::Error::Error(false, "Unable to create a hook on EndScene.\nError Code: %s", MH_StatusToString(Status));
					else
						MH_EnableHook(lpFunc);
				}
			}

			WindowProc::_SetWindowsHook();
		}

		Utils::Error::Message("YYToolkit", "Loaded!");
	}

	void Uninitialize()
	{
		MH_DisableHook(MH_ALL_HOOKS);
		Sleep(100);
		MH_Uninitialize();

		SetWindowLong((HWND)(gAPIVars.Window_Handle), GWL_WNDPROC, (LONG)Hooks::WindowProc::pfnOriginal);

		if (Hooks::Present::pView)
			Hooks::Present::pView->Release();


		NOTIFYICONDATAA IconNotifyData = { sizeof(NOTIFYICONDATAA) };
		IconNotifyData.hWnd = (HWND)gAPIVars.Window_Handle;
		IconNotifyData.uFlags = NIF_INFO;
		Shell_NotifyIconA(NIM_DELETE, &IconNotifyData);

		MessageBoxA((HWND)gAPIVars.Window_Handle, "Unloaded successfully.", "YYToolkit", MB_OK | MB_ICONINFORMATION | MB_TOPMOST);
	}
}