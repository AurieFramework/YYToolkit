#include "../Utils/Error.hpp"
#include "../Utils/MH/MinHook.h"
#include "Code_Execute/Code_Execute.hpp"
#include "DoCallScript/DoCallScript.hpp"
#include "EndScene/EndScene.hpp"
#include "MessageBoxW/MessageBoxW.hpp"
#include "Present/Present.hpp"
#include "ResizeBuffers/ResizeBuffers.hpp"
#include "WindowProc/WindowProc.hpp"
#include "YYError/YYError.hpp"
#include "Drawing/GR_Draw_Text/GR_Draw_Text.hpp"
#include "Drawing/GR_Draw_Text_Color/GR_Draw_Text_Color.hpp"
#include "Drawing/GR_Draw_Text_Transformed/GR_Draw_Text_Transformed.hpp"
#include "Drawing/GR_Draw_Text_TC/GR_Draw_Text_TC_header.hpp"
#include "../Features/API/API.hpp"
#include "Hooks.hpp"
#include <chrono>
#include <array>

#define ReCa reinterpret_cast

namespace Hooks
{
	void Initialize()
	{
		MH_Initialize();
		{
			Utils::Error::Message(CLR_LIGHTBLUE, "\nStarting hooks...");

			auto Hook = [](void* NewFunc, void* GetTargetFunc, void** pfnOriginal, const char* Name)
			{
				if (void* lpFunc = reinterpret_cast<void*(*)()>(GetTargetFunc)()) // "C isn't that hard" moment
				{
					auto Status = MH_CreateHook(lpFunc, NewFunc, pfnOriginal);
					if (Status != MH_OK)
						Utils::Error::Error(false, "Cannot create a hook on %s!\nError Code: %s", Name, MH_StatusToString(Status));
					else
						MH_EnableHook(lpFunc);

					Utils::Error::Message(CLR_GRAY, "- &%s = 0x%p", Name, lpFunc);
				}
			};

			auto TimeStart = std::chrono::high_resolution_clock::now();

			Hook
			(
				ReCa<void*>(&Hooks::Code_Execute::Function), 
				ReCa<void*>(&Hooks::Code_Execute::GetTargetAddress), 
				ReCa<void**>(&Hooks::Code_Execute::pfnOriginal),
				"Code_Execute"
			);

			Hook
			(
				ReCa<void*>(&Hooks::DoCallScript::Function),
				ReCa<void*>(&Hooks::DoCallScript::GetTargetAddress),
				ReCa<void**>(&Hooks::DoCallScript::pfnOriginal),
				"DoCallScript"
			);

			Hook
			(
				ReCa<void*>(&Hooks::YYError::Function), 
				ReCa<void*>(&Hooks::YYError::GetTargetAddress),	
				ReCa<void**>(&Hooks::YYError::pfnOriginal),
				"YYError"
			);

			Hook
			(
				ReCa<void*>(&Hooks::GR_Draw_Text::Function), 
				ReCa<void*>(&Hooks::GR_Draw_Text::GetTargetAddress), 
				ReCa<void**>(&Hooks::GR_Draw_Text::pfnOriginal),
				"GR_Draw_Text"
			);

			Hook
			(
				ReCa<void*>(&Hooks::GR_Draw_Text_Color::Function),	
				ReCa<void*>(&Hooks::GR_Draw_Text_Color::GetTargetAddress),	
				ReCa<void**>(&Hooks::GR_Draw_Text_Color::pfnOriginal),
				"GR_Draw_Text_Color"
			);

			Hook
			(
				ReCa<void*>(&Hooks::GR_Draw_Text_Transformed::Function),	
				ReCa<void*>(&Hooks::GR_Draw_Text_Transformed::GetTargetAddress),	
				ReCa<void**>(&Hooks::GR_Draw_Text_Transformed::pfnOriginal),
				"GR_Draw_Text_Transformed"
			);

			Hook
			(
				ReCa<void*>(&Hooks::GR_Draw_Text_TC::Function), 
				ReCa<void*>(&Hooks::GR_Draw_Text_TC::GetTargetAddress),	
				ReCa<void**>(&Hooks::GR_Draw_Text_TC::pfnOriginal), 
				"GR_Draw_Text_TC"
			);

			Hook
			(
				ReCa<void*>(&Hooks::MessageBoxW::Function),
				ReCa<void*>(&Hooks::MessageBoxW::GetTargetAddress),
				ReCa<void**>(&Hooks::MessageBoxW::pfnOriginal),
				"MessageBoxW"
			);

			if (GetModuleHandleA("d3d11.dll"))
			{
				Hook
				(
					ReCa<void*>(Hooks::Present::Function),
					ReCa<void*>(&Hooks::Present::GetTargetAddress),
					ReCa<void**>(&Hooks::Present::pfnOriginal),
					"Present"
				);

				Hook
				(
					ReCa<void*>(Hooks::ResizeBuffers::Function),
					ReCa<void*>(&Hooks::ResizeBuffers::GetTargetAddress),
					ReCa<void**>(&Hooks::ResizeBuffers::pfnOriginal),
					"ResizeBuffers"
				);
			}

			else if (GetModuleHandleA("d3d9.dll"))
			{
				Hook
				(
					ReCa<void*>(Hooks::EndScene::Function),
					ReCa<void*>(&Hooks::EndScene::GetTargetAddress),
					ReCa<void**>(&Hooks::EndScene::pfnOriginal),
					"EndScene"
				);
			}

			WindowProc::_SetWindowsHook();

			auto TimeEnd = std::chrono::high_resolution_clock::now();

			ShowWindow(reinterpret_cast<HWND>(gAPIVars.Window_Device), SW_SHOW);
			SetForegroundWindow(reinterpret_cast<HWND>(gAPIVars.Window_Device));
			Utils::Error::Message(CLR_LIGHTBLUE, "All hooks enabled - took %.3f seconds!", static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(TimeEnd - TimeStart).count()) / 1000.0f);
			Utils::Error::Message(CLR_BLUE, "Hint: Press F10 to run GML functions!");
		}
	}

	void Uninitialize()
	{
		MH_DisableHook(MH_ALL_HOOKS);
		Sleep(100);
		MH_Uninitialize();

		SetWindowLong((HWND)(gAPIVars.Window_Handle), GWL_WNDPROC, (LONG)Hooks::WindowProc::pfnOriginal);

		if (gAPIVars.RenderView)
			reinterpret_cast<ID3D11RenderTargetView*>(gAPIVars.RenderView)->Release();
	}
}