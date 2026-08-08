#include "MI.hpp"
using namespace Aurie;

namespace YYTK
{
	AurieStatus YYTKInterfaceImpl::Create()
	{
		return YkSetupEarlyInitialization();
	}

	void YYTKInterfaceImpl::Destroy()
	{
		if (m_RunnerInterfacePopulatedEvent)
			CloseHandle(m_RunnerInterfacePopulatedEvent);

		Hooks::HkUninitialize(
			m_WindowHandle
		);
	}

	void YYTKInterfaceImpl::QueryVersion(
		OUT short& Major,
		OUT short& Minor,
		OUT short& Patch
	)
	{
		Major = YYTK_MAJOR;
		Minor = YYTK_MINOR;
		Patch = YYTK_PATCH;
	}

	AurieStatus YYTKInterfaceImpl::YkSetupEarlyInitialization()
	{
		// If the initialization is already complete, we noop.
		if (m_FirstInitComplete)
			return AURIE_SUCCESS;

		AurieStatus last_status = AURIE_SUCCESS;
		Generic::MiPrintLoadInfo();

		// Try to look for the runner interface.
		last_status = Zeus::FindRunnerInterfaceHookpoint(
			&this->m_RunnerInterfaceHookIP
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::FindRunnerInterfaceHookpoint => %s", AurieStatusToString(last_status));

		// If not found, unload YYTK.
		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to create a runner interface hook!", AurieStatusToString(last_status));
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		// Initialize an event object.
		// This event object will become signaled when the runner interface is created.
		this->m_RunnerInterfacePopulatedEvent = CreateEventA(
			nullptr,
			true,
			false,
			nullptr
		);

		// Register the runner interface hook on the target IP.
		// Direct it to the HandleRunnerInterfaceCreation function.
		last_status = Zeus::RegisterRunnerInterfaceHook(
			this->m_RunnerInterfaceHookIP,
			Zeus::HandleRunnerInterfaceCreation
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::RegisterRunnerInterfaceHook => %s", AurieStatusToString(last_status));

		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to create a runner interface hook!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		// Inject stage 1 hooks. These are mainly for code execution and secret features.
		last_status = Hooks::InitializeStage1Hooks();

		DbgPrintEx(LOG_SEVERITY_TRACE, "Hooks::InitializeStage1Hooks => %s", AurieStatusToString(last_status));

		// Make sure hooks got injected properly.
		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to create stage 1 hooks!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		m_FirstInitComplete = true;
		return AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::YkSetupLateInitialization()
	{
		if (m_SecondInitComplete)
			return AURIE_SUCCESS;

		DbgPrintEx(LOG_SEVERITY_TRACE, "YkSetupLateInitialization() starts waiting...");

		AurieStatus last_status = AURIE_SUCCESS;
		DWORD object_status = WaitForSingleObject(m_RunnerInterfacePopulatedEvent, 10000);

		// If we waited 10 seconds, but no runner interface initialization took place,
		// we can safely conclude that something is broken.
		if (object_status != WAIT_OBJECT_0)
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to await runner interface creation!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		m_IsRunnerInterfaceReady = true;
		DbgPrintEx(LOG_SEVERITY_DEBUG, "Runner interface created, continuing...");

		// Now we have to figure out if the runner is YYC or VM
		// We can do that by calling "code_is_compiled".

		// First we assume the runner is YYC, and look for the functions array.
		last_status = Zeus::YYC::FindFunctionsArray(
			m_RunnerInterface,
			&m_FunctionsArray
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::YYC::FindFunctionsArray => %s", AurieStatusToString(last_status));

		// We use another status, because old YYTK just discarded it, and having only one status variable causes logic issues for my tired brain.
		auto size_status = Zeus::DetermineFunctionEntrySize(
			m_FunctionsArray,
			&m_FunctionEntrySize
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::DetermineFunctionEntrySize => %s", AurieStatusToString(size_status));

		// If the YYC version did not succeed, we might get more luck with the VM version.
		// Note that some VM games also succeed on the "YYC" variant, which is why code_is_compiled is the better way of judging the runner type.
		if (!AurieSuccess(last_status) || !AurieSuccess(size_status))
		{
			// Try the VM variant
			last_status = Zeus::VM::FindFunctionsArray(
				m_RunnerInterface,
				&m_FunctionsArray
			);

			DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::VM::FindFunctionsArray => %s", AurieStatusToString(last_status));

			// We need to re-determine the function entry size.
			size_status = Zeus::DetermineFunctionEntrySize(
				m_FunctionsArray,
				&m_FunctionEntrySize
			);

			DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::DetermineFunctionEntrySize => %s", AurieStatusToString(size_status));

			// If neither YYC nor VM is successful, there's nothing we can do.
			if (!AurieSuccess(last_status) || !AurieSuccess(size_status))
			{
				DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to find functions array!");
				return AURIE_MODULE_INTERNAL_ERROR;
			}
		}

		// Query the runner type - YYC and VM runners have seemingly different optimization settings, which have to be accounted for.
		RValue result_rvalue;
		last_status = CallBuiltinEx(
			result_rvalue,
			"code_is_compiled",
			nullptr,
			nullptr,
			{}
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "code_is_compiled => %s", AurieStatusToString(last_status));

		// If we failed to call the function, we're done for.
		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to query runner type!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		// Look at the result to get our answer as to what runner we're in.
		m_IsYYCRunner = result_rvalue.ToBoolean();
		DbgPrintEx(LOG_SEVERITY_TRACE, "m_IsYYCRunner => %s", m_IsYYCRunner ? "true" : "false");

		// Now we get the built-ins.
		if (m_IsYYCRunner)
		{
			last_status = Zeus::YYC::GetBuiltinInformation(
				&m_BuiltinCount,
				&m_BuiltinArray
			);

			DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::YYC::GetBuiltinInformation => %s", AurieStatusToString(last_status));
		}
		else
		{
			last_status = Zeus::VM::GetBuiltinInformation(
				&m_BuiltinCount,
				&m_BuiltinArray
			);

			DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::VM::GetBuiltinInformation => %s", AurieStatusToString(last_status));
		}

		// If we fail getting the built-ins:
		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to locate built-in variables!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		// Now find @@CopyStatic@@, which is used to look up the ScriptData() function.
		PVOID copy_static = nullptr;
		last_status = GetNamedRoutinePointer(
			"@@CopyStatic@@",
			&copy_static
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "@@CopyStatic@@ => %s", AurieStatusToString(last_status));

		// Make sure we got that.
		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to locate game scripts!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		// And now find the actual function.
		last_status = Zeus::FindScriptData(
			m_RunnerInterface,
			copy_static,
			&m_GetScriptData
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::FindScriptData => %s", AurieStatusToString(last_status));

		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to locate game scripts!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		// Now find array_equals, which is used to look up the offset from RValue base to the array length.
		PVOID array_equals = nullptr;
		last_status = GetNamedRoutinePointer(
			"array_equals",
			&array_equals
		);

		// Try to find the offset required to go from the address pointed-to by an RValue to the member storing the array length.
		if (m_IsYYCRunner)
		{
			last_status = Zeus::YYC::FindArrayOffsetFromRValue(
				array_equals,
				&m_RValueArrayOffset
			);

			DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::YYC::FindArrayOffsetFromRValue => %s", AurieStatusToString(last_status));
		}
		else
		{
			last_status = Zeus::VM::FindArrayOffsetFromRValue(
				array_equals,
				&m_RValueArrayOffset
			);

			DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::VM::FindArrayOffsetFromRValue => %s", AurieStatusToString(last_status));
		}

		// If we failed doing that, we quit.
		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_WARNING, "Failed to locate RValue array offset!");
		}

		// Trace printing ftw!
		DbgPrintEx(LOG_SEVERITY_TRACE, "Array offset is %llx", m_RValueArrayOffset);

		// Get the room_instance_clear function. We use it to find room data.
		PVOID room_instance_clear = nullptr;
		last_status = this->GetNamedRoutinePointer(
			"room_instance_clear",
			&room_instance_clear
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "room_instance_clear => %s", AurieStatusToString(last_status));

		// If we can't get the function, exit.
		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to locate room data!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		// Try to find the actual room data using the function.
		if (m_IsYYCRunner)
		{
			last_status = Zeus::YYC::FindRoomData(
				room_instance_clear,
				&m_GetRoomData
			);

			DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::YYC::FindRoomData => %s", AurieStatusToString(last_status));
		}
		else
		{
			last_status = Zeus::VM::FindRoomData(
				room_instance_clear,
				&m_GetRoomData
			);

			DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::VM::FindRoomData => %s", AurieStatusToString(last_status));
		}

		// Make sure we got it.
		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to locate room data!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		size_t room_width_index = 0;
		last_status = GetBuiltinVariableIndex(
			"room_width",
			room_width_index
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "room_width => %s", AurieStatusToString(last_status));

		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to locate current room data!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		RVariableRoutine* room_width_handlers = nullptr;
		last_status = GetBuiltinVariableInformation(
			room_width_index,
			room_width_handlers
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "room_width handlers => %s", AurieStatusToString(last_status));

		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to locate current room data!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		last_status = Zeus::FindCurrentRoomData(
			room_width_handlers->m_SetVariable,
			&m_RunRoom
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::FindCurrentRoomData => %s", AurieStatusToString(last_status));

		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to locate current room data!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		last_status = Zeus::FindSlotAdditionFunction(
			m_RunnerInterface,
			&m_AddToYYObjectBase
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::FindSlotAdditionFunction => %s", AurieStatusToString(last_status));

		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to find the slot addition function!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		last_status = Zeus::FindSlotAllocationFunction(
			m_AddToYYObjectBase,
			&m_FindAllocSlot
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::FindSlotAllocationFunction => %s", AurieStatusToString(last_status));

		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to find the slot allocation function!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		TRoutine is_nan = nullptr;
		last_status = GetNamedRoutinePointer(
			"is_nan",
			reinterpret_cast<PVOID*>(&is_nan)
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "is_nan => %s", AurieStatusToString(last_status));

		last_status = Zeus::FindErrorSuppressionVariable(
			is_nan,
			&m_RunnerErrorsDisabled
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::FindErrorSuppressionVariable => %s", AurieStatusToString(last_status));

		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_WARNING, "Failed to find flag-bit 0 dependant variable.");
			// no return, ignore.
		}

		RValue window_handle;
		last_status = CallBuiltinEx(
			window_handle,
			"window_handle",
			nullptr,
			nullptr,
			{}
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "window_handle => %s", AurieStatusToString(last_status));

		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to get the window handle!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		m_WindowHandle = window_handle.ToPointer<HWND>();

		// Build the symbol table.
		// This can take a while (~100ms), so we do it once instead of on every exception.
		Zeus::BuildApproximateSymbolTable(m_KnownGameSymbols);

		last_status = Hooks::InitializeStage2Hooks(
			m_WindowHandle
		);

		DbgPrintEx(LOG_SEVERITY_TRACE, "Hooks::InitializeStage2Hooks => %s", AurieStatusToString(last_status));

		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to create hooks!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		m_SecondInitComplete = true;

		// Run the RUNNER_INIT callback. We're done, and the runner is set up.
		FunctionWrapper dummy_wrapper = FunctionWrapper<void(int)>([](int) {}, 0);
		YkDispatchCallbacks(EVENT_RUNNER_INIT, dummy_wrapper);

		return Aurie::AURIE_SUCCESS;
	}

	AurieStatus YYTKInterfaceImpl::YkSetupFinalInitialization()
	{
		if (m_ThirdInitComplete)
			return AURIE_SUCCESS;
		
		AurieStatus last_status = AURIE_SUCCESS;
		last_status = YkSetupLateInitialization();

		if (!AurieSuccess(last_status))
			return last_status;

		DbgPrintEx(LOG_SEVERITY_TRACE, "YkSetupFinalInitialization() starts waiting...");

		for (int d3d11_try_limit = 300; d3d11_try_limit > 0; d3d11_try_limit--)
		{
			last_status = YkFetchD3D11Info(
				nullptr, // unused
				&m_EngineSwapchain
			);

			// If we succeeded, **and got the swapchain** we can simply break out without sleeping
			// Otherwise, wait 10ms and try again.
			if (AurieSuccess(last_status) && m_EngineSwapchain)
				break;

			Sleep(10);
		}

		// If we didn't succeed, or the swapchain is nullptr, we failed.
		if (!AurieSuccess(last_status) || !m_EngineSwapchain)
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to get D3D11 information!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		// Install the D3D11 Present/ResizeBuffers hooks backing EVENT_FRAME
		// and EVENT_RESIZE now that we have a live swapchain. Without this
		// call, CreateCallback(..., EVENT_FRAME/EVENT_RESIZE, ...) silently
		// registers a callback that can never actually fire, since nothing
		// ever hooks Present()/ResizeBuffers() in the first place.
		last_status = Hooks::InitializeStage3Hooks(m_WindowHandle, m_EngineSwapchain);

		DbgPrintEx(LOG_SEVERITY_TRACE, "Hooks::InitializeStage3Hooks => %s", AurieStatusToString(last_status));

		if (!AurieSuccess(last_status))
		{
			DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to create D3D11 hooks!");
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		m_ThirdInitComplete = true;
		return AURIE_SUCCESS;
	}

}