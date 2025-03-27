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

			// Message header
			std::string yytk_crash_message =
				"\n"
				"A fatal game error has occured.\n"
				"Runner caught the exception; the exception_unhandled_handler callback has not been invoked.\n"
				"\n"
				"Runner-given error information:\n";

			// Append the runner-provided information
			yytk_crash_message.append(runner_provided_info_formatted);

			// Stacktrace header
			yytk_crash_message.append(
				"\n\n"
				"Stacktrace:\n"
				" #\tRetAddr         \tCall Site\n"
			);

			// Capture stacktrace
			const auto stacktrace = std::stacktrace::current();

			// Log all functions in the trace
			size_t function_index = 0;
			for (auto& entry : stacktrace)
			{
				// description will be empty if resolve_game_symbol fails.
				std::string description = GmResolveGameSymbolFromAddress(stacktrace._To_voidptr_array()[function_index]);
				if (description.empty())
					description = entry.description();

				yytk_crash_message.append(
					std::format("{:02}\t{:016X}\t{}\n", 
						function_index, 
						reinterpret_cast<uint64_t>(stacktrace._To_voidptr_array()[function_index]),
						description
					)
				);
				function_index++;
			}
			
			// More information
			yytk_crash_message.append("\n");
			yytk_crash_message.append("YYTK Version: ");
			yytk_crash_message.append(YYTK_VERSION_STRING);
			yytk_crash_message.append("\n");
			yytk_crash_message.append("Aurie Module List:\n");

			// Loop all modules - this is undocumented and plugins should not use this, but we do.
			AurieModule* current_module = g_ArSelfModule;
			do
			{
				// The method doesn't modify module_name unless it succeeds.
				std::wstring module_name = L"<unknown>";
				MdGetImageFilename(current_module, module_name);

				// Convert to UTF-8
				const std::string module_name_utf8(module_name.begin(), module_name.end());

				const uint64_t module_address = reinterpret_cast<uint64_t>(Internal::MdpGetModuleBaseAddress(current_module));
				yytk_crash_message.append(std::format("- {:016X} {}\n", module_address, module_name_utf8));

				// Go to the next module
				Internal::MdpGetNextModule(current_module, current_module);
			} while (current_module != g_ArSelfModule);

			CmWriteLogOutput(yytk_crash_message);
			
			std::string yytk_info = "\r\n\r\n********************************************\r\n";
			yytk_info.append("YYToolkit is loaded. Relevant information has been logged to YYToolkit.log in the game directory.\r\n");
			yytk_info.append("Please provide the entire log file to aid in debugging.\r\n");
			yytk_info.append("********************************************\r\n");

			return GetHookTrampoline<decltype(&HkYYError)>("YYError")(
				(runner_provided_info_formatted + yytk_info).c_str()
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

			g_ModuleInterface.m_CodeExecute = code_execute;
			
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