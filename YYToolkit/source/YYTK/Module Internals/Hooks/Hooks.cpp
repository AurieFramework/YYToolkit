#include "../Module Internals.hpp"
using namespace Aurie;

namespace YYTK
{
	namespace Hooks
	{
		template <typename T>
		T GetHookTrampoline(const char* Name)
		{
			return reinterpret_cast<T>(MmGetHookTrampoline(g_ArSelfModule, Name));
		}

		HRESULT WINAPI HkPresent(
			IN IDXGISwapChain* _this, 
			IN unsigned int Sync, 
			IN unsigned int Flags
		)
		{
			return E_NOTIMPL;
		}

		bool HkExecuteIt(
			IN CInstance* SelfInstance,
			IN CInstance* OtherInstance,
			IN CCode* CodeObject,
			IN RValue* Arguments,
			IN INT Flags
		)
		{
			g_ModuleInterface.GetRunnerInterface().YYError(
				"Archie was here. Welcome YYToolkit Next.\n"
				"Huge shoutout to the HoloCure and Risk of Rain Returns modding community\n\n"
				"and everyone else that's ever used YYTK.\n\nYou guys (and gals) are awesome.\n\n\n\nReleasing next week.\n\n"
			);

			return GetHookTrampoline<decltype(&HkExecuteIt)>("ExecuteIt")(
				SelfInstance,
				OtherInstance,
				CodeObject,
				Arguments,
				Flags
			);
		}

		PVOID HkDoCallScript(
			IN CScript* Script,
			IN int ArgumentCount,
			IN char* VmStackPointer,
			IN PVOID VmInstance,
			IN CInstance* Locals,
			IN CInstance* Arguments
		)
		{
			return GetHookTrampoline<decltype(&HkDoCallScript)>("DoCallScript")(
				Script,
				ArgumentCount,
				VmStackPointer,
				VmInstance,
				Locals,
				Arguments
			);
		}

		AurieStatus HkPreinitialize()
		{
			/*
				how 2 DoCallScript:
					Find DoCallGML, 2nd call
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
			
			// Get the name of the game executable
			std::wstring game_name;
			last_status = MdGetImageFilename(
				g_ArInitialImage,
				game_name
			);

			if (!AurieSuccess(last_status))
				return last_status;

			// We're looking for a pattern in Code_Execute
			size_t pattern_match = MmSigscanModule(
				game_name.c_str(),
				UTEXT(
					"\xE8\x00\x00\x00\x00"	// call <ExecuteIt>
					"\x0F\xB6\xD8"			// mozvx ebx, al
					"\x3C\x01"				// cmp al, 1
				),
				"x????xxxxx"
			);

			if (!pattern_match)
				return AURIE_MODULE_INITIALIZATION_FAILED;

			std::vector<TargettedInstruction> instructions = GmpDisassemble(
				reinterpret_cast<PVOID>(pattern_match),
				0x10,
				0xFF
			);

			// Get the first instruction at that address (the call instruction), and make sure it has the
			// parameters we expect it to have (ie. is a call, and has 1 visible operand - the address.)
			ZydisDisassembledInstruction& call_instruction = instructions.front().RawForm;
			if (call_instruction.info.mnemonic != ZYDIS_MNEMONIC_CALL)
				return AURIE_MODULE_INITIALIZATION_FAILED;

			if (call_instruction.info.operand_count_visible < 1)
				return AURIE_MODULE_INITIALIZATION_FAILED;

			// Calculate the address of the function which we're calling (ExecuteIt)
			ZyanU64 execute_it_address = 0;
			if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
				&call_instruction.info,
				&call_instruction.operands[0],
				call_instruction.runtime_address,
				&execute_it_address
			)))
			{
				return AURIE_MODULE_INITIALIZATION_FAILED;
			}

			// We should've never gotten here if the pattern or translation fails.
			assert(execute_it_address != 0);

			// Hook ExecuteIt
			last_status = MmCreateHook(
				g_ArSelfModule,
				"ExecuteIt",
				reinterpret_cast<PVOID>(execute_it_address),
				reinterpret_cast<PVOID>(HkExecuteIt),
				nullptr
			);

			if (!AurieSuccess(last_status))
				return last_status;

			return AURIE_SUCCESS;
		}

		Aurie::AurieStatus HkInitialize()
		{
			return AURIE_NOT_IMPLEMENTED;
		}
	}
}