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

		// Disassemble at RIP - 0xFFF
		const auto instructions_prior_to_rip = GmpDisassemble(
			// - 0xFFF is to offset by the size we want to disassemble, and +5 is the size of the call instruction.
			// The call instruction is at cpu_context->Rip, and will be the last instruction in instructions_prior_to_rip.
			reinterpret_cast<PVOID>(cpu_context->Rip - 0xFFF + 5),
			0xFFF,
			UINT64_MAX
		);

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
		{
			Internal::MmpUnsetBreakpoint(
				reinterpret_cast<PVOID>(cpu_context->Rip)
			);

			return true;
		}

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

		Internal::MmpUnsetBreakpoint(
			reinterpret_cast<PVOID>(cpu_context->Rip)
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