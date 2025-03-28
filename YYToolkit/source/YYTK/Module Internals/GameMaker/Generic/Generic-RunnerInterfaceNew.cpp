#include "../../Module Internals.hpp"

#define PAGE_SIZE 0x1000
#define PAGE_SHIFT 12L
#define KERNEL_STACK_SIZE (4 * PAGE_SIZE)
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

	void GmpRunnerInterfaceHook(
		IN ProcessorContext& ProcessorContext
	)
	{
#if _WIN64

		/*
		* WinDbg output before jmp to hook
			0:000> r
			rax=0000000000000000 rbx=000001ca00009070 rcx=60e617d3ec4e0000
			rdx=00007ff7574991a0 rsi=0000000000000228 rdi=0000000000001100
			rip=00007ff756f0b381 rsp=000000296073e150 rbp=000000296073e250
			 r8=0000000000000312  r9=0000000000000301 r10=000001ca6dea0000
			r11=000000296073e750 r12=0000000000000001 r13=0000000000000003
			r14=0000000000000008 r15=000000001e66bb28
			iopl=0         nv up ei pl nz na po nc
			cs=0033  ss=002b  ds=002b  es=002b  fs=0053  gs=002b             efl=00000206
			Coffee_Story_Extra+0x19eb381:
			00007ff7`56f0b381 e9864c60fe      jmp     00007ff7`5551000c

		* Hook output
			rax=0000000000000000 rbx=000001ca00009070 rcx=60e617d3ec4e0000
			rdx=00007ff7574991a0 rsi=0000000000000228 rdi=0000000000001100
			rip=00007ff755510000 rsp=000000296073e150 rbp=000000296073e250
			r8=0000000000000312 r9=0000000000000301 r10=000001ca6dea0000
			r11=000000296073e750 r12=0000000000000001 r13=0000000000000003
			r14=0000000000000008 r15=000000001e66bb28 tsp=000000296073e148
		
		*/
		DbgPrintEx(
			LOG_SEVERITY_TRACE,
			"Dumping register state from thread ID %x",
			GetCurrentThreadId()
		);

		DbgPrintEx(
			LOG_SEVERITY_TRACE,
			"    rax=%016llx rbx=%016llx rcx=%016llx",
			ProcessorContext.RAX, ProcessorContext.RBX, ProcessorContext.RCX
		);

		DbgPrintEx(
			LOG_SEVERITY_TRACE,
			"    rdx=%016llx rsi=%016llx rdi=%016llx",
			ProcessorContext.RDX, ProcessorContext.RSI, ProcessorContext.RDI
		);

		DbgPrintEx(
			LOG_SEVERITY_TRACE,
			"    rip=%016llx rsp=%016llx rbp=%016llx",
			ProcessorContext.RIP, ProcessorContext.RSP, ProcessorContext.RBP
		);

		DbgPrintEx(
			LOG_SEVERITY_TRACE,
			"    r8=%016llx r9=%016llx r10=%016llx",
			ProcessorContext.R8, ProcessorContext.R9, ProcessorContext.R10
		);

		DbgPrintEx(
			LOG_SEVERITY_TRACE,
			"    r11=%016llx r12=%016llx r13=%016llx",
			ProcessorContext.R11, ProcessorContext.R12, ProcessorContext.R13
		);

		DbgPrintEx(
			LOG_SEVERITY_TRACE,
			"    r14=%016llx r15=%016llx tsp=%016llx",
			ProcessorContext.R14, ProcessorContext.R15, ProcessorContext.TrampolineRSP
		);

		uint64_t current_rip = g_ModuleInterface.m_RunnerInterfaceBase;
		ZydisDisassembledInstruction current_instruction;

		// Get a bit of memory - let's pray the stack isn't this big in the function so we don't go OOB
		BYTE* new_stack = new BYTE[KERNEL_STACK_SIZE];
		memset(new_stack, 0, KERNEL_STACK_SIZE);

		ZydisRegister last_saved_register = ZYDIS_REGISTER_NONE;
		uint64_t last_saved_value = 0;

		// RBP + delta_rsp_rbp = RSP
		const int64_t delta_rsp_rbp = ProcessorContext.RSP - ProcessorContext.RBP;

		// Shift 2 pages into new_stack, such that negative offsets from either don't cause OOB writes.
		BYTE* new_rbp = reinterpret_cast<BYTE*>(new_stack + (2 * PAGE_SIZE));
		BYTE* new_rsp = reinterpret_cast<BYTE*>(new_stack + (2 * PAGE_SIZE) + delta_rsp_rbp);

		int64_t rsp_offset_to_runner_interface = 0;

		// Loop disassemble
		while (ZYAN_SUCCESS(ZydisDisassembleIntel(
			ZYDIS_MACHINE_MODE_LONG_64,
			current_rip,
			reinterpret_cast<PVOID>(current_rip),
			128,
			&current_instruction
		)))
		{
			DbgPrintEx(
				LOG_SEVERITY_TRACE, 
				"[%s:%d] %016llx | %s", 
				__FILE__,
				__LINE__, 
				current_instruction.runtime_address, 
				current_instruction.text
			);

			// The chain unconditionally ends on a call instruction - we shouldn't get here though, as 
			if (current_instruction.info.mnemonic == ZYDIS_MNEMONIC_CALL)
				break;

			// LEA always comes before a MOV
			if (current_instruction.info.mnemonic == ZYDIS_MNEMONIC_LEA)
			{
				if (current_instruction.operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
				{
					current_rip += current_instruction.info.length;
					continue;
				}

				if (current_instruction.operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
				{
					current_rip += current_instruction.info.length;
					continue;
				}

				last_saved_register = current_instruction.operands[0].reg.value;
				if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
					&current_instruction.info,
					&current_instruction.operands[1],
					current_instruction.runtime_address,
					&last_saved_value
				)))
				{
					// If RSP-relative, it's the instruction before the call, moving RI address to RCX
					if (current_instruction.operands[1].mem.base == ZYDIS_REGISTER_RSP)
					{
						DbgPrintEx(
							LOG_SEVERITY_TRACE,
							"[%s:%d] Found RSP-relative LEA: %016llx | %s",
							__FILE__, __LINE__,
							current_instruction.runtime_address,
							current_instruction.text
						);

						rsp_offset_to_runner_interface = current_instruction.operands[1].mem.disp.value;
						break;
					}

					DbgPrintEx(
						LOG_SEVERITY_WARNING,
						"- Failed to calculate RIP-relative address: %016llx | %s", 
						current_instruction.runtime_address,
						current_instruction.text
					);

					current_rip += current_instruction.info.length;
					continue;
				}
			}

			// MOV [rsp|rbp+disp], last_saved_register
			if (current_instruction.info.mnemonic == ZYDIS_MNEMONIC_MOV)
			{
				if (current_instruction.operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY)
				{
					current_rip += current_instruction.info.length;
					continue;
				}

				if (current_instruction.operands[1].type != ZYDIS_OPERAND_TYPE_REGISTER || 
					current_instruction.operands[1].reg.value != last_saved_register
				)
				{
					current_rip += current_instruction.info.length;
					continue;
				}

				switch (current_instruction.operands[0].mem.base)
				{
				case ZYDIS_REGISTER_RSP:
					*reinterpret_cast<uint64_t*>(new_rsp + current_instruction.operands[0].mem.disp.value) = last_saved_value;
					break;
				case ZYDIS_REGISTER_RBP:
					*reinterpret_cast<uint64_t*>(new_rbp + current_instruction.operands[0].mem.disp.value) = last_saved_value;
					break;
				}
			}

			current_rip += current_instruction.info.length;
		}

		// Copy out everything
		memcpy(&g_ModuleInterface.m_RunnerInterface, new_rsp + rsp_offset_to_runner_interface, sizeof(YYRunnerInterface));

		DbgPrintEx(
			LOG_SEVERITY_TRACE, 
			"[%s:%d] New RSP: %llx",
			__FILE__, 
			__LINE__, 
			new_rsp
		);

		DbgPrintEx(
			LOG_SEVERITY_TRACE, 
			"[%s:%d] New RBP: %llx",
			__FILE__, 
			__LINE__,
			new_rbp
		);

		for (int offset_to_buffer = 0; offset_to_buffer < (KERNEL_STACK_SIZE / sizeof(uint64_t)); offset_to_buffer += 4)
		{
			// Print the address
			DbgPrintEx(
				LOG_SEVERITY_TRACE,
				"[%s:%d] 0x%llx | %016llx %016llx %016llx %016llx", 
				__FILE__, __LINE__,
				new_stack + (offset_to_buffer * sizeof(uint64_t)),
				((ULONG64*)(new_stack))[offset_to_buffer],
				((ULONG64*)(new_stack))[offset_to_buffer + 1],
				((ULONG64*)(new_stack))[offset_to_buffer + 2],
				((ULONG64*)(new_stack))[offset_to_buffer + 3]
			);
		}

		delete[] new_stack;
		SetEvent(g_ModuleInterface.m_RunnerInterfacePopulatedEvent);
#endif
	}

	// Used if GmpGetRunnerInterface fails in x64.
	AurieStatus GmpCreateHookOnInterfaceCreation(
		OPTIONAL OUT PVOID* Rip,
		IN AurieMidHookFunction Handler
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

		DbgPrint(
			"Please wait while the game is being disassembled. This can take up to a minute on slower hardware."
		);

		size_t code_pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(
			game_base + text_section_offset,
			text_section_size
		);

		DbgPrintEx(
			LOG_SEVERITY_TRACE,
			"[%s:%d] GmpCreateHookOnInterfaceCreation() => .text section spans %lld pages",
			__FILE__,
			__LINE__,
			code_pages
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

			DbgPrintEx(
				LOG_SEVERITY_TRACE,
				"[%s:%d] GmpCreateHookOnInterfaceCreation() => Found RI instructions at 0x%llX",
				__FILE__,
				__LINE__,
				runner_interface_instructions_base
			);

			// Disassemble some instructions before the runner interface init code begins
			auto pre_ri_instructions = GmpDisassemble(
				reinterpret_cast<PVOID>(runner_interface_instructions_base - 0xFFF),
				0xFFF,
				SIZE_MAX
			);

			// Some games (Coffee Story, NSFW) have Extension_Main_Number == 0, which means no runner interface
			// is ever actually present on the stack, and our breakpointed instructions never run.
			// 
			// To counter this, we breakpoint in the function prologue of Extension_Preprepare, get RSP and RBP,
			// and create our own stack with the runner interface.
			auto last_rsp_sub_iterator = std::find_if(
				pre_ri_instructions.rbegin(),
				pre_ri_instructions.rend(),
				[](const TargettedInstruction& instr)
				{
					// Looking for sub rsp, constant
					if (instr.RawForm.info.mnemonic != ZYDIS_MNEMONIC_SUB)
						return false;

					if (instr.RawForm.operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
						return false;

					if (instr.RawForm.operands[0].reg.value != ZYDIS_REGISTER_RSP)
						return false;

					return true;
				}
			);

			// If we failed to find a stack subtraction instruction?
			if (last_rsp_sub_iterator == pre_ri_instructions.rend())
			{
				DbgPrintEx(
					LOG_SEVERITY_TRACE,
					"[%s:%d] GmpCreateHookOnInterfaceCreation() => last_rsp_sub_iterator == pre_ri_instructions.rend()",
					__FILE__,
					__LINE__,
					runner_interface_instructions_base
				);

				continue;
			}

			// Get the actual instruction
			const auto& last_rsp_sub = *last_rsp_sub_iterator;
			// Get the instruction just after the sub rsp (iterator is reversed, so -1 instead of +1)
			const auto& instruction_just_after = *(last_rsp_sub_iterator - 1);

			DbgPrintEx(
				LOG_SEVERITY_TRACE,
				"[%s:%d] GmpCreateHookOnInterfaceCreation() => 0x%llX | %s",
				__FILE__,
				__LINE__,
				last_rsp_sub.RawForm.runtime_address,
				last_rsp_sub.RawForm.text
			);

			DbgPrintEx(
				LOG_SEVERITY_TRACE,
				"[%s:%d] GmpCreateHookOnInterfaceCreation() => 0x%llX | %s",
				__FILE__,
				__LINE__,
				instruction_just_after.RawForm.runtime_address,
				instruction_just_after.RawForm.text
			);

			// By now we know we have the correct address

			// We will hook the instruction right before the call instruction.
			// Hooking the call instruction will cause the game to hang, idfk why.
			const PVOID bp_address = reinterpret_cast<PVOID>(instruction_just_after.RawForm.runtime_address);

			last_status = MmCreateMidfunctionHook(
				g_ArSelfModule,
				"RunnerInterface",
				bp_address,
				Handler
			);

			DbgPrintEx(
				LOG_SEVERITY_TRACE,
				"[%s:%d] GmpCreateHookOnInterfaceCreation() => breakpoint at %p => %s",
				__FILE__,
				__LINE__,
				bp_address,
				AurieStatusToString(last_status)
			);

			// If we failed, return the error code.
			if (!AurieSuccess(last_status))
				return last_status;

			// Else return the RIP if needed
			if (Rip)
				*Rip = bp_address;

			g_ModuleInterface.m_RunnerInterfaceBase = runner_interface_instructions_base;
			return AURIE_SUCCESS;
		}

		return AURIE_OBJECT_NOT_FOUND;
	}
}