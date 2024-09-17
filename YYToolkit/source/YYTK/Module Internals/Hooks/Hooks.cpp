#include "../Module Internals.hpp"
using namespace Aurie;

namespace YYTK
{
	namespace Hooks
	{
		inline WNDPROC g_OriginalWindowProc = nullptr;

		template <typename T>
		T GetHookTrampoline(const char* Name)
		{
			return reinterpret_cast<T>(MmGetHookTrampoline(g_ArSelfModule, Name));
		}

		BOOL WINAPI HkHeapValidate(
			IN HANDLE Heap,
			IN DWORD Flags,
			IN LPCVOID Memory
		)
		{
			CmWriteInfo("Invalidating heap at 0x%p", Memory);
			return false;
		}

		LRESULT WINAPI HkWndProc(
			IN HWND WindowHandle,
			IN UINT Message,
			IN WPARAM WP,
			IN LPARAM LP
		)
		{
			auto original_function = g_OriginalWindowProc;

			// decltype apparently doesn't work in x86 bruh
			FunctionWrapper<LRESULT(HWND, UINT, WPARAM, LPARAM)> func_wrapper(
				original_function,
				WindowHandle,
				Message,
				WP,
				LP
			);

			g_ModuleInterface.YkDispatchCallbacks(
				EVENT_WNDPROC,
				func_wrapper
			);

			if (func_wrapper.CalledOriginal())
				return func_wrapper.Result();

			return CallWindowProcW(
				original_function,
				WindowHandle, 
				Message, 
				WP, 
				LP
			);
		}

		HRESULT WINAPI HkPresent(
			IN IDXGISwapChain* _this, 
			IN unsigned int Sync, 
			IN unsigned int Flags
		)
		{
			// decltype apparently doesn't work in x86 bruh
			auto original_function = GetHookTrampoline<decltype(&HkPresent)>("Present");

			FunctionWrapper<HRESULT(IDXGISwapChain*, UINT, UINT)> func_wrapper(
				original_function,
				_this,
				Sync,
				Flags
			);

			g_ModuleInterface.YkDispatchCallbacks(
				EVENT_FRAME,
				func_wrapper
			);

			if (func_wrapper.CalledOriginal())
				return func_wrapper.Result();

			return original_function(
				_this,
				Sync,
				Flags
			);
		}

		HRESULT WINAPI HkResizeBuffers(
			IN IDXGISwapChain* _this,
			IN UINT BufferCount,
			IN UINT Width,
			IN UINT Height,
			IN DXGI_FORMAT NewFormat,
			IN UINT SwapChainFlags
		)
		{
			auto original_function = GetHookTrampoline<decltype(&HkResizeBuffers)>("ResizeBuffers");

			FunctionWrapper<HRESULT(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)> func_wrapper(
				original_function, 
				_this, 
				BufferCount, 
				Width, 
				Height, 
				NewFormat, 
				SwapChainFlags
			);

			g_ModuleInterface.YkDispatchCallbacks(
				EVENT_RESIZE,
				func_wrapper
			);

			if (func_wrapper.CalledOriginal())
				return func_wrapper.Result();

			return original_function(
				_this,
				BufferCount,
				Width,
				Height,
				NewFormat,
				SwapChainFlags
			);
		}

		bool HkExecuteIt(
			IN CInstance* SelfInstance,
			IN CInstance* OtherInstance,
			IN CCode* CodeObject,
			IN RValue* Arguments,
			IN INT Flags
		)
		{
			auto original_function = GetHookTrampoline<decltype(&HkExecuteIt)>("ExecuteIt");

			FunctionWrapper<decltype(HkExecuteIt)> func_wrapper(
				original_function,
				SelfInstance,
				OtherInstance,
				CodeObject,
				Arguments,
				Flags
			);

			g_ModuleInterface.YkDispatchCallbacks(
				EVENT_OBJECT_CALL,
				func_wrapper
			);

			if (func_wrapper.CalledOriginal())
				return func_wrapper.Result();

			return original_function(
				SelfInstance,
				OtherInstance,
				CodeObject,
				Arguments,
				Flags
			);
		}

		AurieStatus HkPreinitialize()
		{
			/*
				How hooks are done:

				how 2 CodeExecute:
					*reusing old YYTK v2 code*
					scan for AOB:
						E8 ?? ?? ?? ??		call <ExecuteIt>
						0F B6 D8			movzx ebx, al
						3C 01				cmp al, 01
				how 2 IDXGISwapChain::whatever (for dummies):
					https://manual.yoyogames.com/GameMaker_Language/GML_Reference/OS_And_Compiler/os_get_info.htm
					os_get_info returns pointers to swapchain and device
					hook swapchain Present + ResizeBuffers
				how 2 windowproc
					SetWindowLongW
			*/
			AurieStatus last_status = AURIE_SUCCESS;

			PVOID code_execute = nullptr;
			last_status = GmpFindCodeExecute(
				&code_execute
			);

			if (!AurieSuccess(last_status))
				return last_status;

			// Hook ExecuteIt
			last_status = MmCreateHook(
				g_ArSelfModule,
				"ExecuteIt",
				code_execute,
				HkExecuteIt,
				nullptr
			);

			if (!AurieSuccess(last_status))
				return last_status;

			// We know this to be either true or false, since HkPreinitialize runs after
			// the runner interface lookup code.
			if (g_ModuleInterface.m_IsUsingVeh)
			{
				last_status = MmCreateHook(
					g_ArSelfModule,
					"HeapValidate",
					HeapValidate,
					HkHeapValidate,
					nullptr
				);
			}
			
			return last_status;
		}

		Aurie::AurieStatus HkInitialize(
			IN HWND WindowHandle,
			IN IDXGISwapChain* EngineSwapChain
		)
		{

			/*
			* This function hooks methods based off information gathered in stage 2 of YYTK's init.
			* 
				- D3D11 hooks
					Simple manual VMT hooks. Nothing special.
					IDXGISwapChain::Present is VMT entry 8
					IDXGISwapChain::ResizeBuffers is VMT entry 13
				- WindowProc hooks
					Just use SetWindowLongW
			*/

			AurieStatus last_status = AURIE_SUCCESS;

			if (!EngineSwapChain)
				return AURIE_MODULE_INITIALIZATION_FAILED;

			PVOID* swapchain_vtable = *reinterpret_cast<PVOID**>(EngineSwapChain);
			
			if (!swapchain_vtable)
				return AURIE_MODULE_INITIALIZATION_FAILED;

			last_status = MmCreateHook(
				g_ArSelfModule,
				"Present",
				swapchain_vtable[8],
				HkPresent,
				nullptr
			);

			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INITIALIZATION_FAILED;

			last_status = MmCreateHook(
				g_ArSelfModule,
				"ResizeBuffers",
				swapchain_vtable[13],
				HkResizeBuffers,
				nullptr
			);

			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INITIALIZATION_FAILED;

			g_OriginalWindowProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(
				WindowHandle,
				GWLP_WNDPROC,
				reinterpret_cast<LONG_PTR>(HkWndProc)
			));

			assert(g_OriginalWindowProc != nullptr);

			return AURIE_SUCCESS;
		}

		Aurie::AurieStatus HkUninitialize(
			IN HWND WindowHandle
		)
		{
			SetWindowLongPtr(
				WindowHandle,
				GWLP_WNDPROC,
				reinterpret_cast<LONG_PTR>(g_OriginalWindowProc)
			);

			return AURIE_SUCCESS;
		}
	}
}