#include "../../Module Internals.hpp"

using namespace Aurie;

namespace YYTK
{
	namespace VM
	{
		AurieStatus GmpGetBuiltinInformation(
			OUT int32_t*& BuiltinCount,
			OUT RVariableRoutine*& BuiltinArray
		)
		{
			AurieStatus last_status = AURIE_SUCCESS;
			std::wstring game_name;

			last_status = MdGetImageFilename(
				g_ArInitialImage,
				game_name
			);

			if (!AurieSuccess(last_status))
				return last_status;

			// We're looking for a pattern in Variable_BuiltIn_Add.

			// The rough decompilation of this function is as follows:
			/*
				void Variable_BuiltIn_Add(
					IN const char* Name,
					IN FNGetVariable GetVariable
					IN FNSetVariable SetVariable
				)
				{
					if (g_BuiltinVariableCount == 500)
					{
						ShowMessage("INTERNAL ERROR: Adding too many variables"); // <=== Good string xref!
						return;
					}

					const char* builtin_name = YYStrDup(Name);
					g_BuiltinVariables[g_BuiltinVariableCount].m_GetVariable = GetVariable;
					g_BuiltinVariables[g_BuiltinVariableCount].m_SetVariable = SetVariable;
					g_BuiltinVariables[g_BuiltinVariableCount].m_CanBeSet = SetVariable != nullptr;

					g_BuiltinVarLookup->Insert(Name);
					++g_BuiltinVariableCount;
				}
			*/

			// We scan for the "if (g_BuiltinVariableCount == 500)" check
			// In YYC, there's only one match.
			// In VM, there may be multiple that match (inlined code), but only one is correct (the true function)

			uint64_t text_section_base = 0;
			size_t text_section_size = 0;

			// Get the .text section address for the game executable
			last_status = Internal::PpiGetModuleSectionBounds(
				GetModuleHandleW(nullptr),
				".text",
				text_section_base,
				text_section_size
			);

			if (!AurieSuccess(last_status))
				return last_status;

			// Since PpiGetModuleSectionBounds returns the offset to the .text section
			// we need the base address of the game to add to the offset
			char* game_base = reinterpret_cast<char*>(GetModuleHandleW(nullptr));

			// Scan for all occurences of this pattern in memory
			// Note that in the below pattern, not having a register in the first opcode
			// is crucial! Some games, it's eax, others it's r8d or some other register!
			std::vector<size_t> pattern_matches = {};
			GmpSigscanRegionEx(
				reinterpret_cast<const unsigned char*>((game_base + text_section_base)),
				text_section_size,
				UTEXT(
					"\xF4\x01\x00\x00"  // 500 in hex, no opcode
					"\x75\x00"			// jnz <??>
				),
				"xxxxx?",
				pattern_matches
			);

			if (pattern_matches.empty())
				return AURIE_OBJECT_NOT_FOUND;

			for (const auto& match : pattern_matches)
			{
				// We disassemble the function starting at -20h offset from the pattern
				// This is because we're "in the middle" of some instruction with the pattern,
				// so we can't just start disassembling there, as we'd get garbage instructions
				std::vector<TargettedInstruction> instructions = GmpDisassemble(
					reinterpret_cast<PVOID>(match - 0x20),
					0x40,
					0xFF
				);

				// Now, scan for the first jnz instruction
				// This should be the second one (instructions[1])
				size_t jnz_instruction_index = 0;
				last_status = GmpFindMnemonicPattern(
					instructions,
					{
						ZYDIS_MNEMONIC_JNZ
					},
					jnz_instruction_index
				);

				// If no jnz instruction exists, skip to the next entry
				if (!AurieSuccess(last_status))
					continue;

				ZyanU64 jnz_target = 0;

				// Follow the jnz instruction (ie. we "pass" the check for eax < 500)
				ZyanStatus zyan_status = ZydisCalcAbsoluteAddress(
					&instructions[jnz_instruction_index].RawForm.info,
					&instructions[jnz_instruction_index].RawForm.operands[0],
					instructions[jnz_instruction_index].RawForm.runtime_address,
					&jnz_target
				);

				// Translation failed? This shouldn't happen.
				if (!ZYAN_SUCCESS(zyan_status) || !jnz_target)
					continue;

				// Now we disassemble again, but this time at the target of the jnz
				// ie. where the CPU jumps to if we pass the bounds check
				instructions = GmpDisassemble(
					reinterpret_cast<PVOID>(jnz_target),
					0x40,
					0xFF
				);

				ZyanU64 array_base_address = 0;
				ZyanU64 array_numb_address = 0;

				for (const auto& instruction : instructions)
				{
					const auto& raw_instruction = instruction.RawForm;

					// Until we have the base address of the array, we have to check for LEA instructions
					// TODO: 2023-newer-IDA.i64 seems to use different format?
					// 48 89 83 00 FC 06 01		mov qword ptr ds:builtin_variables.f_name[rbx], rax

					// We're searching for two instructions that have the same format:
					// Instruction 1: lea register, memory
					// Instruction 2: movsxd register, memory
					// 
					// We can therefore check for this format up front, reducing code duplication
					if (raw_instruction.info.operand_count != 2)
					{
						continue;
					}

					// Check that the operand types match
					if (raw_instruction.operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
					{
						continue;
					}

					// Check that the operand types match (part 2)
					if (raw_instruction.operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
					{
						continue;
					}

					// Until we find the base address of the builtin variable array,
					// we check any LEA instruction we encounter.
					if ((raw_instruction.info.mnemonic == ZYDIS_MNEMONIC_LEA) && (array_base_address == 0))
					{
						// Try to calculate the absolute address of the target
						// It doesn't matter if we fail here - if we do, we try the next LEA.
						ZydisCalcAbsoluteAddress(
							&raw_instruction.info,
							&raw_instruction.operands[1],
							raw_instruction.runtime_address,
							&array_base_address
						);
					}

					// Until we have the address of the array "numb" (ie. the amount of elements used up)
					// we have to check for MOVSXD instructions. It's the first one we encounter after the initial jmp
					if ((raw_instruction.info.mnemonic == ZYDIS_MNEMONIC_MOVSXD) && (array_numb_address == 0))
					{
						ZydisCalcAbsoluteAddress(
							&raw_instruction.info,
							&raw_instruction.operands[1],
							raw_instruction.runtime_address,
							&array_numb_address
						);
					}
				}

				if (!array_base_address || !array_numb_address)
					continue;

				BuiltinCount = reinterpret_cast<int32_t*>(array_numb_address);
				BuiltinArray = reinterpret_cast<RVariableRoutine*>(array_base_address);

				return AURIE_SUCCESS;
			}

			return AURIE_OBJECT_NOT_FOUND;
		}
	}
}