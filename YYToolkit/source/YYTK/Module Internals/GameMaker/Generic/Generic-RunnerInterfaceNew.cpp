#include "../../Module Internals.hpp"

#define PAGE_SIZE 0x1000
#define PAGE_SHIFT 12L
#define BYTE_OFFSET(Va) ((ULONG)((LONG_PTR)(Va) & (PAGE_SIZE - 1)))
#define ADDRESS_AND_SIZE_TO_SPAN_PAGES(Va,Size) \
    ((BYTE_OFFSET (Va) + ((SIZE_T) (Size)) + (PAGE_SIZE - 1)) >> PAGE_SHIFT)

using namespace Aurie;

namespace YYTK
{
	static std::vector<TargettedInstruction> GmpFindRunnerInterfaceInstructionsX64(
		IN std::vector<size_t> PatternMatches
	)
	{
		// Loop through all the matches, and check if they have a long function chain
		std::vector<TargettedInstruction> instructions = {};
		for (const size_t& match : PatternMatches)
		{
			// Magic numbers, look at the disassembly in that huge comment above
			instructions = GmpDisassemble(
				reinterpret_cast<PVOID>(match),
				0x1000,
				4
			);

			size_t function_count = GmpCountInstructionReferences(instructions);

			// If there's less than 80 functions, it's probably not the interface
			// YYRunnerInterface has 96 functions in LTS + 2 variables = 98 lea matches
			// 2022.3 YYC has 88 functions
			if (function_count > 87 && function_count < 104)
				break;

			instructions.clear();
		}

		return instructions;
	}

	// Handles #GP(0) raised on interface creation.
	bool GmpHandleInterfaceCreationBP(
		IN PVOID ProcessorContext,
		IN uint32_t ExceptionCode
	)
	{
#if _WIN64
		PCONTEXT cpu_context = static_cast<PCONTEXT>(ProcessorContext);
		CmWriteOutput(
			CM_LIGHTPURPLE,
			"#GP(0) raised - RIP: 0x%llX, RSP: 0x%llX, RBP: 0x%llX", 
			cpu_context->Rip, 
			cpu_context->Rsp, 
			cpu_context->Rbp
		);

		// Restore original bytes of the breakpointed instruction.
		Internal::MmpUnsetBreakpoint(
			reinterpret_cast<PVOID>(cpu_context->Rip)
		);

		// Restore original bytes of the JS instruction, which we temporarily patched 
		// such that we hit the breakpoint unconditionally.
		WriteProcessMemory(
			GetCurrentProcess(),
			g_ModuleInterface.m_ExtensionPatchBase,
			g_ModuleInterface.m_ExtensionPatchBytes.data(),
			g_ModuleInterface.m_ExtensionPatchBytes.size(),
			nullptr
		);

		// Disassemble at RIP - 0xFFF
		const auto instructions_prior_to_rip = GmpDisassemble(
			// - 0xFFF is to offset by the size we want to disassemble, and +5 is the size of the call instruction.
			// The call instruction is at cpu_context->Rip, and will be the last instruction in instructions_prior_to_rip.
			reinterpret_cast<PVOID>(cpu_context->Rip - 0xFFF + 5),
			0xFFF,
			UINT64_MAX
		);

		// We have modified the runner to forcefully hit this call instruction, even if it shouldn't.
		// 
		// If RIP shouldn't be here, the call instruction will be the one causing a fault, trying
		// to call into a non-canonical address.
		//
		// We check the register for nullptr.

		// Get the last call prior to RIP
		auto last_call_instruction_iterator = std::find_if(
			instructions_prior_to_rip.rbegin(),
			instructions_prior_to_rip.rend(),
			[](const TargettedInstruction& Instruction)
			{
				return Instruction.RawForm.info.mnemonic == ZYDIS_MNEMONIC_CALL;
			}
		);

		// No call instruction in the disassembly???
		if (last_call_instruction_iterator == instructions_prior_to_rip.rend())
			return true;

		const auto& last_call_instruction = last_call_instruction_iterator->RawForm;

		// Instruction should be call qword ptr [reg+offset]
		assert(last_call_instruction.info.mnemonic == ZYDIS_MNEMONIC_CALL);

		// Get the register of the call instruction
		const ZydisRegister call_register = last_call_instruction.operands[0].mem.base;
		
		// Determine if the call instruction would cause an access fault
		bool should_advance_rip = false;
		switch (call_register)
		{
		case ZYDIS_REGISTER_RAX:
			should_advance_rip = (cpu_context->Rax == 0);
			break;
		case ZYDIS_REGISTER_RBX:
			should_advance_rip = (cpu_context->Rbx == 0);
			break;
		case ZYDIS_REGISTER_RCX:
			should_advance_rip = (cpu_context->Rcx == 0);
			break;
		case ZYDIS_REGISTER_RDX:
			should_advance_rip = (cpu_context->Rdx == 0);
			break;
		case ZYDIS_REGISTER_RSI:
			should_advance_rip = (cpu_context->Rsi == 0);
			break;
		case ZYDIS_REGISTER_RDI:
			should_advance_rip = (cpu_context->Rdi == 0);
			break;
		case ZYDIS_REGISTER_R9:
			should_advance_rip = (cpu_context->R9 == 0);
			break;
		case ZYDIS_REGISTER_R10:
			should_advance_rip = (cpu_context->R10 == 0);
			break;
		case ZYDIS_REGISTER_R11:
			should_advance_rip = (cpu_context->R11 == 0);
			break;
		case ZYDIS_REGISTER_R12:
			should_advance_rip = (cpu_context->R12 == 0);
			break;
		case ZYDIS_REGISTER_R13:
			should_advance_rip = (cpu_context->R13 == 0);
			break;
		case ZYDIS_REGISTER_R14:
			should_advance_rip = (cpu_context->R14 == 0);
			break;
		case ZYDIS_REGISTER_R15:
			should_advance_rip = (cpu_context->R15 == 0);
			break;
		default:
			CmWriteError(__FILE__, __LINE__, "Unsupported VEH register %lld!", call_register);
			break;
		}

		// Skip the call instruction
		if (should_advance_rip)
			cpu_context->Rip += last_call_instruction.info.length;

		size_t lea_mov_chain_start = 0;
		AurieStatus last_status = AURIE_SUCCESS;

		// Look up the lea-mov chain
		// We look for at least 3 consecutive ones, since 
		last_status = GmpFindMnemonicPattern(
			instructions_prior_to_rip,
			{
				ZYDIS_MNEMONIC_LEA,
				ZYDIS_MNEMONIC_MOV,
				ZYDIS_MNEMONIC_LEA,
				ZYDIS_MNEMONIC_MOV,
				ZYDIS_MNEMONIC_LEA,
				ZYDIS_MNEMONIC_MOV
			},
			lea_mov_chain_start
		);

		if (!AurieSuccess(last_status))
			return true;

		// Extract the runner interface instructions starting at the lea-mov chain.
		auto runner_interface_instructions = GmpFindRunnerInterfaceInstructionsX64(
			{ instructions_prior_to_rip[lea_mov_chain_start].RawForm.runtime_address }
		);

		// Determine the interface start on the stack
		uint64_t lowest_stack_point = UINT64_MAX;
		for (auto& instruction : runner_interface_instructions)
		{
			// Skip non-mov instructions
			if (instruction.RawForm.info.mnemonic != ZYDIS_MNEMONIC_MOV)
				continue;

			// Skip mov instructions that don't move to memory
			// We're looking for mov [mem+something], reg
			if (instruction.RawForm.operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY)
				continue;

			if (!instruction.RawForm.operands[0].mem.disp.has_displacement)
				continue;

			if (instruction.RawForm.operands[1].type != ZYDIS_OPERAND_TYPE_REGISTER)
				continue;

			const ZydisRegister operand_base = instruction.RawForm.operands[0].mem.base;
			const int64_t operand_offset = instruction.RawForm.operands[0].mem.disp.value;

			switch (operand_base)
			{
			case ZYDIS_REGISTER_RBP: /* fallthrough */
				if (lowest_stack_point > (cpu_context->Rbp + operand_offset))
					lowest_stack_point = cpu_context->Rbp + operand_offset;
				break;
			case ZYDIS_REGISTER_RSP:
				if (lowest_stack_point > (cpu_context->Rsp + operand_offset))
					lowest_stack_point = cpu_context->Rsp + operand_offset;
				break;
			}
		}

		g_ModuleInterface.m_RunnerInterface = *reinterpret_cast<YYRunnerInterface*>(lowest_stack_point);
		
		CmWriteOutput(
			CM_LIGHTPURPLE,
			"Captured runner interface at stack address 0x%llX",
			lowest_stack_point
		);

		SetEvent(g_ModuleInterface.m_RunnerInterfacePopulatedEvent);

#endif
		return true;
	}

	// Used if GmpGetRunnerInterface fails in x64.
	AurieStatus GmpBreakpointInterfaceCreation(
		OPTIONAL OUT PVOID* Rip,
		IN AurieBreakpointCallback Callback
	)
	{
		// Scan the memory for this pattern:
		/*

			In Runner 2023.8 symbols (standard)
				E8 CF 30 18 00        call    class DLL_RFunction * __ptr64 __cdecl DLL_GetFunc(int)
				33 C9                 xor     ecx, ecx
				48 89 8D 88 00 00 00  mov     [rbp+7C0h+Interface.Script_Perform], rcx
				48 8D 0D CF 9F 1D 00  lea     rcx, void __cdecl YYprintf(char const * __ptr64,...)
				48 89 4D 90           mov     [rbp+7C0h+Interface.DebugConsoleOutput], rcx
				48 8D 0D B4 05 00 00  lea     rcx, void __cdecl ReleaseConsoleOutput(char const * __ptr64,...)

			In Risk of Rain Returns (standard)
				E8 9F 2D 00 00        call    DLL_GetFunc
				33 C9                 xor     ecx, ecx
				48 89 8D 88 00 00 00  mov     [rbp+136], rcx
				48 8D 0D 4F 30 F1 FF  lea     rcx, YYprintf
				48 89 4D 90           mov     [rbp-112], rcx
				48 8D 0D B4 05 00 00  lea     rcx, ReleaseConsoleInput
				48 89 4D 98           mov     [rbp-104], rcx

			In Will You Snail (standard)
				E8 0F E2 03 00        call    sub_140A1D220
				33 C9                 xor     ecx, ecx
				48 89 8D 88 00 00 00  mov     [rbp+7C0h+var_830.Script_Perform], rcx
				48 8D 0D 9F CB F4 FF  lea     rcx, sub_14092BBC0
				48 89 4D 90           mov     [rbp+7C0h+var_830.DebugConsoleOutput], rcx
				48 8D 0D 34 05 00 00  lea     rcx, sub_1409DF560
				48 89 4D 98           mov     [rbp+7C0h+var_830.ReleaseConsoleOutput], rcx

			In old Will You Snail (missing the call instruction)
				33 C0                 xor     eax, eax
				48 89 85 88 00 00 00  mov     [rbp+7D0h+var_840.Script_Perform], rax
				48 8D 05 9B 97 0F 00  lea     rax, sub_14027C600
				48 89 45 90           mov     [rbp+7D0h+var_840.DebugConsoleOutput], rax
				48 8D 05 20 E9 FF FF  lea     rax, sub_140181790
				48 89 45 98           mov     [rbp+7D0h+var_840.ReleaseConsoleOutput], rax

			In Fields of Mistria (GM 2024.6, missing xor instruction since Script_Perform is implemented)
				E8 8E 3F 00 00        call    Dll_GetFunc
				48 8D 0D E7 32 F5 FF  lea     rcx, sub_141241B60
				48 89 4C 24 60        mov     [rsp+800h+var_7A0.DebugConsoleOutput], rcx
				48 8D 0D BB 05 00 00  lea     rcx, sub_1412EEE40
				48 89 4C 24 68        mov     [rsp+800h+var_7A0.ReleaseConsoleOutput], rcx
				48 8D 0D 8F 78 08 00  lea     rcx, sub_141376120

			Due to instruction length mismatches, I think scanning for raw bytes is out of the question.
			An alternative approach is to scan the whole of the game's text section for large mov-lea chains.

			YYRunnerInterface initialization code can be, however, broken up by the occasional instruction:
				48 89 8D D0 01 00 00  mov     [rbp+700h+var_7A0.StructAddInt], rcx
				48 8D 0D EF 02 00 00  lea     rcx, sub_1412EEF60
				BA 20 03 00 00        mov     edx, 320h
				48 89 8D D8 01 00 00  mov     [rbp+700h+var_7A0.StructAddRValue], rcx
				48 8D 0D EC 02 00 00  lea     rcx, sub_1412EEF70
				48 89 8D E0 01 00 00  mov     [rbp+700h+var_7A0.StructAddString], rcx

			The mov-lea chain is terminated by a call [reg] instruction:
				48 89 8D 58 02 00 00  mov     [rbp+700h+var_7A0.extOptGetString], rcx
				48 8D 0D CC 00 00 00  lea     rcx, sub_1412EEE30
				48 89 8D 68 02 00 00  mov     [rbp+700h+var_7A0.isRunningFromIDE], rcx
				48 8D 0D AE 03 00 00  lea     rcx, sub_1412EF120
				48 89 8D 70 02 00 00  mov     [rbp+700h+var_490], rcx
				48 8D 0D B0 E3 FF FF  lea     rcx, sub_1412ED130
				48 89 8D 78 02 00 00  mov     [rbp+700h+var_488], rcx
				48 8D 4C 24 60        lea     rcx, [rsp+800h+var_7A0]
				FF 50 18              call    qword ptr [rax+18h]
		*/


		AurieStatus last_status = AURIE_SUCCESS;
		uint64_t text_section_offset = 0;
		size_t text_section_size = 0;

		// Query the game's base address
		const uint64_t game_base = reinterpret_cast<uint64_t>(GetModuleHandleW(nullptr));

		// Get the .text section address for the game executable
		last_status = Internal::PpiGetModuleSectionBounds(
			GetModuleHandleW(nullptr),
			".text",
			text_section_offset,
			text_section_size
		);

		if (!AurieSuccess(last_status))
			return last_status;

		CmWriteOutput(
			CM_LIGHTAQUA,
			"Please wait while the game is being disassembled. This can take up to a minute on slower hardware."
		);

		size_t code_pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(
			game_base + text_section_offset,
			text_section_size
		);

		for (size_t page_num = 0; page_num < code_pages; page_num++)
		{
			PVOID page_base = reinterpret_cast<PVOID>(game_base + text_section_offset + (page_num * PAGE_SIZE));

			// Disassemble the ENTIRE game
			auto instructions_in_the_current_page = GmpDisassemble(
				page_base,
				PAGE_SIZE,
				SIZE_MAX
			);

			std::vector<std::pair<size_t, ZydisDisassembledInstruction>> lea_mov_pairs = {};

			do
			{
				const std::vector<ZydisMnemonic> lea_mov_pattern
					= {
					ZYDIS_MNEMONIC_LEA,
					ZYDIS_MNEMONIC_MOV,
					ZYDIS_MNEMONIC_LEA,
					ZYDIS_MNEMONIC_MOV,
					ZYDIS_MNEMONIC_LEA,
					ZYDIS_MNEMONIC_MOV,
					ZYDIS_MNEMONIC_LEA,
					ZYDIS_MNEMONIC_MOV,
				};

				// The index of last match that was found in the previous iteration of this loop.
				size_t last_match_start = 0;

				// Set last_match_start to the last found match index. 
				if (!lea_mov_pairs.empty())
					last_match_start = lea_mov_pairs.back().first;

				// Scan all_game_instructions for a mov-lea pattern from indices:
				// [last_match_start + lea_mov_pattern.size()] => [all_game_instructions.size()].
				size_t current_match_start = 0;
				last_status = GmpFindMnemonicPattern(
					instructions_in_the_current_page,
					lea_mov_pattern,
					current_match_start,
					last_match_start + lea_mov_pattern.size()
				);

				// If we don't find a match, the loop is done, and we have all indices already.
				if (!AurieSuccess(last_status))
					break;

				// Push back the current_match_start to the vector.
				lea_mov_pairs.push_back({ current_match_start, instructions_in_the_current_page[current_match_start].RawForm });

			} while (true);

			// Contains only valid lea-mov pairs. Valid = have the correct operand types
			std::vector<size_t> valid_lea_mov_pairs_addresses = {};

			// Clean up lea_mov_pairs from invalid instructions, 
			// store results in valid_lea_mov_pairs_addresses.
			for (auto& pair : lea_mov_pairs)
			{
				const size_t& index = pair.first;
				const ZydisDisassembledInstruction& lea_instruction = instructions_in_the_current_page[index].RawForm;
				const ZydisDisassembledInstruction& mov_instruction = instructions_in_the_current_page[index + 1].RawForm;

				// lea has to move into a register
				if (lea_instruction.operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
					continue;

				// lea has to be from a memory address
				if (lea_instruction.operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
					continue;

				// TODO: Use PpGetCurrentArchitecture instead of comparing sizeof()
				ZydisRegister lea_target_register = ZydisRegisterGetLargestEnclosing(
					sizeof(void*) == sizeof(uint64_t) ? ZYDIS_MACHINE_MODE_LONG_64 : ZYDIS_MACHINE_MODE_LEGACY_32,
					lea_instruction.operands[0].reg.value
				);

				// We move to a memory address
				if (mov_instruction.operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY)
					continue;

				// We move from a register
				if (mov_instruction.operands[1].type != ZYDIS_OPERAND_TYPE_REGISTER)
					continue;

				ZydisRegister mov_source_register = ZydisRegisterGetLargestEnclosing(
					sizeof(void*) == sizeof(uint64_t) ? ZYDIS_MACHINE_MODE_LONG_64 : ZYDIS_MACHINE_MODE_LEGACY_32,
					mov_instruction.operands[1].reg.value
				);

				if (mov_source_register != lea_target_register)
					continue;

				// We're moving to [rbp+something] or [rsp+something], never [other_gpr+something]
				if (mov_instruction.operands[0].mem.base != ZYDIS_REGISTER_RBP && mov_instruction.operands[0].mem.base != ZYDIS_REGISTER_RSP)
					continue;

				valid_lea_mov_pairs_addresses.push_back(static_cast<size_t>(pair.second.runtime_address));
			}

			// Filter valid_lea_mov_pairs_addresses by checking the length of function chains.
			// TODO: Check if this has false positives? Doesn't seem to have them tho...
			auto runner_interface_instructions = GmpFindRunnerInterfaceInstructionsX64(
				valid_lea_mov_pairs_addresses
			);

			// Try again if no runner interface instructions exist
			if (runner_interface_instructions.empty())
				continue;
			
			// Capture the base address of the runner interface instructions
			const uintptr_t runner_interface_instructions_base = 
				runner_interface_instructions.front().RawForm.runtime_address;

			// Disassemble some instructions before the runner interface init code begins
			auto pre_ri_instructions = GmpDisassemble(
				reinterpret_cast<PVOID>(runner_interface_instructions_base - 0x20),
				0x20,
				SIZE_MAX
			);

			// Find the last js instruction before the runner interface init code.
			// 
			// If no extension has the runner interface init function, the runner interface init code will
			// never be hit in the runner, therefore placing a breakpoint on the call instruction
			// after will never work.
			//
			// This js instruction will be the last instruction hit. 
			// We modify the instruction to never jump (nopping it), 
			auto last_js_iterator = std::find_if(
				pre_ri_instructions.rbegin(),
				pre_ri_instructions.rend(),
				[](const TargettedInstruction& instr)
				{
					return instr.RawForm.info.mnemonic == ZYDIS_MNEMONIC_JS;
				}
			);

			// If we failed to find a JS instruction prior to the interface init code, continue
			if (last_js_iterator == pre_ri_instructions.rend())
				continue;

			// Get the last js instruction from the iterator
			const auto last_js_instruction = *last_js_iterator;

			// Save the base address of the JS instruction for restoration purposes.
			g_ModuleInterface.m_ExtensionPatchBase = 
				reinterpret_cast<PVOID>(last_js_instruction.RawForm.runtime_address);

			// Nop the instruction
			for (size_t i = 0; i < last_js_instruction.RawForm.info.length; i++)
			{
				constexpr unsigned char nop = 0x90;

				// Save the original bytes for restoration in the VEH handler
				g_ModuleInterface.m_ExtensionPatchBytes.push_back(
					*reinterpret_cast<uint8_t*>(last_js_instruction.RawForm.runtime_address + i)
				);

				// Overwrite with a NOP
				WriteProcessMemory(
					GetCurrentProcess(),
					reinterpret_cast<PVOID>(last_js_instruction.RawForm.runtime_address + i),
					&nop,
					sizeof(nop),
					nullptr
				);
			}

			// Nopping the above instruction will cause several issues if no extension that 
			// has the method exists.
			// Namely, the call instruction after will fault, due to trying to call into a nullptr address.
			// We will have to correct that (advance RIP) in the VEH handler.

			// The runner interface init code is terminated with a call instruction.
			// This instruction is executed unconditionally.
			size_t call_index = 0;

			last_status = GmpFindMnemonicPattern(
				runner_interface_instructions,
				{
					ZYDIS_MNEMONIC_CALL
				},
				call_index
			);

			// If we failed to look up the call instruction, we had the wrong address anyway.
			// Tough luck.
			if (!AurieSuccess(last_status))
				continue;

			// By now we know we have the correct address

			// We will breakpoint the call instruction.
			const PVOID bp_address = reinterpret_cast<PVOID>(
				runner_interface_instructions[call_index].RawForm.runtime_address
				);

			// Set the breakpoint.
			last_status = Internal::MmpSetBreakpoint(
				bp_address,
				Callback
			);

			// If we failed, return the error code.
			if (!AurieSuccess(last_status))
				return last_status;

			// Else return the RIP if needed
			if (Rip)
				*Rip = bp_address;

			return AURIE_SUCCESS;
		}

		return AURIE_OBJECT_NOT_FOUND;
	}
}