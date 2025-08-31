#include "../Module Internals.hpp"
#include <stacktrace>
using namespace Aurie;

namespace YYTK
{
	namespace Hooks
	{
		static WNDPROC g_OriginalWindowProc = nullptr;

		template <typename T>
		T GetHookTrampoline(const char* Name)
		{
			return reinterpret_cast<T>(MmGetHookTrampoline(g_ArSelfModule, Name));
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
			static auto original_function = GetHookTrampoline<decltype(&HkPresent)>("Present");

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
			static auto original_function = GetHookTrampoline<decltype(&HkResizeBuffers)>("ResizeBuffers");

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
			static auto original_function = GetHookTrampoline<decltype(&HkExecuteIt)>("ExecuteIt");

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

		void HkYYError(
			IN const char* Format,
			IN ...
		)
		{	
			// Parse Format and va_args to get the runner-provided information.
			std::string runner_provided_info_formatted;
			{
				va_list list, copy_of_list;
				va_start(list, Format);
				va_copy(copy_of_list, list);

				// Compute the required size. We use a copy of "list" to prevent the internal stack pointer from being moved.
				const int size_required = vsnprintf(nullptr, 0, Format, copy_of_list) + 1;

				va_end(copy_of_list);

				// Allocate memory for the formatted string
				char* formatted_cstring = static_cast<char*>(MmAllocateMemory(g_ArSelfModule, size_required));
				vsnprintf(formatted_cstring, size_required, Format, list);
				va_end(list);

				runner_provided_info_formatted = formatted_cstring;

				MmFreeMemory(g_ArSelfModule, formatted_cstring);
				formatted_cstring = nullptr;
			}

			std::string new_info = runner_provided_info_formatted;
			new_info += "\r\n";
			new_info += "Current engine stacktrace:\r\n";
			// Works for both VM games by unwinding the VM stack, and for YYC ones by unwinding the g_YYStackTrace.
			RValue gm_callstack = g_ModuleInterface.CallBuiltin("debug_get_callstack", {});
			if (gm_callstack.IsArray())
			{
				// Convert the array to a vector, as that's easier to work with for us.
				auto callstack_vector = gm_callstack.ToVector();
				for (const auto& frame : callstack_vector)
				{
					new_info += "- ";
					new_info += frame.ToString();
					new_info += "\r\n";
				}
			}

			new_info += "\r\n";
			new_info += "Current native stacktrace:\r\n";

			// Capture the native stacktrace
			const auto native_stacktrace = std::stacktrace::current();

			// Log all functions in the trace
			for (size_t function_index = 0; function_index < native_stacktrace.size(); function_index++)
			{
				const void* function = native_stacktrace._To_voidptr_array()[function_index];

				// description will be empty if GuessSymbolFromGameInstructionAddress fails.
				std::string description = Zeus::GuessSymbolFromGameInstructionAddress(function);

				// If we failed, we need to craft a WinDbg-esque symbol print ourselves,
				// since entry.description() is terribly slow.
				if (description.empty())
				{
					// If a module owns this
					char filename[256] = {};
					HMODULE ip_module = nullptr;
					if (GetModuleHandleExA(
						GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
						static_cast<LPCSTR>(function),
						&ip_module
					))
					{
						GetModuleFileNameA(ip_module, filename, 256);
					}

					description = std::format(
						"{}+0x{:06X}",
						fs::path(filename).filename().string().c_str(),
						reinterpret_cast<uintptr_t>(function) - reinterpret_cast<uintptr_t>(ip_module)
					);
				}

				new_info += std::format("- {:016X} => {}\n",
					reinterpret_cast<uint64_t>(function),
					description
				);
			}

			// Loop all modules - this is undocumented and plugins should not use this, but we do.
			new_info += "Aurie Module List:\n";
			AurieModule* current_module = g_ArSelfModule;
			do
			{
				// The method doesn't modify module_name unless it succeeds.
				std::wstring module_name = L"<unknown>";
				MdGetImageFilename(current_module, module_name);

				// Convert to UTF-8
				const std::string module_name_utf8(module_name.begin(), module_name.end());

				const uint64_t module_address = reinterpret_cast<uint64_t>(Internal::MdpGetModuleBaseAddress(current_module));
				new_info += std::format("- {:016X} {}\n", module_address, module_name_utf8);

				// Go to the next module
				Internal::MdpGetNextModule(current_module, current_module);
			} while (current_module != g_ArSelfModule);

			DbgPrintEx(Aurie::LOG_SEVERITY_CRITICAL, "The GameMaker runtime has encountered an error!");
			DbgPrintEx(Aurie::LOG_SEVERITY_TRACE, new_info.c_str());

			std::string yytk_info = "\r\n\r\n********************************************\r\n";
			yytk_info.append("YYToolkit is loaded. Relevant information has been logged to Aurie.log in the game directory.\r\n");
			yytk_info.append("Please provide the entire log file to aid in debugging.\r\n");
			yytk_info.append("********************************************\r\n");

			return GetHookTrampoline<decltype(&HkYYError)>("YYError")(
				(runner_provided_info_formatted + yytk_info).c_str()
			);
		}

		AurieStatus InitializeStage1Hooks()
		{
			AurieStatus last_status = AURIE_SUCCESS;
			PVOID code_execute = nullptr;

			// Try to hook Code_Execute
			last_status = Zeus::FindCodeExecutionHookpoint(&code_execute);

			DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::FindCodeExecutionHookpoint => %s", AurieStatusToString(last_status));

			if (!AurieSuccess(last_status))
				return AURIE_MODULE_INTERNAL_ERROR;

			last_status = MmCreateHook(
				g_ArSelfModule,
				"ExecuteIt",
				code_execute,
				HkExecuteIt,
				nullptr
			);

			return AURIE_SUCCESS;
		}

		Aurie::AurieStatus InitializeStage2Hooks(
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

			last_status = MmCreateHook(
				g_ArSelfModule,
				"YYError",
				g_ModuleInterface.GetRunnerInterface().YYError,
				HkYYError,
				nullptr
			);

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