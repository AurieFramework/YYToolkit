#include "../../Module Internals.hpp"

using namespace Aurie;

namespace YYTK
{
	namespace VM
	{
		AurieStatus GmpFindRVArrayOffset(
			IN TRoutine F_ArrayEquals,
			IN YYRunnerInterface& Interface,
			OUT int64_t* ArrayOffset
		)
		{
			UNREFERENCED_PARAMETER(Interface);
			// So there's one of two ways this is implemented:
			// Either it's a mov-call-test mnemonic pattern (and the function is called like normal)
			// In the other case, it's inlined into F_ArrayEquals, in which case we don't care >:(

			std::vector<TargettedInstruction> instructions = GmpDisassemble(
				F_ArrayEquals,
				0x100,
				0xFF
			);

			// The first match should be it
			size_t pattern_index = 0;
			AurieStatus last_status = GmpFindMnemonicPattern(
				instructions,
				{
					ZYDIS_MNEMONIC_MOV,
					ZYDIS_MNEMONIC_CALL,
					ZYDIS_MNEMONIC_TEST
				},
				pattern_index
			);

			if (!AurieSuccess(last_status))
				return AURIE_OBJECT_NOT_FOUND;

			ZydisDisassembledInstruction& call_instruction = instructions.at(pattern_index + 1).RawForm;

			assert(call_instruction.info.mnemonic == ZYDIS_MNEMONIC_CALL);

			// Get the ArrayEquals internal handler
			ZyanU64 array_equals_internal_address = 0;
			ZydisCalcAbsoluteAddress(
				&call_instruction.info,
				&call_instruction.operands[0],
				call_instruction.runtime_address,
				&array_equals_internal_address
			);

			if (!array_equals_internal_address)
				return AURIE_INVALID_PARAMETER;

			// The rest is just from the YYC handler
			instructions = GmpDisassemble(
				reinterpret_cast<PVOID>(array_equals_internal_address),
				0xFF,
				0xFF
			);

			size_t two_movs_index = SIZE_MAX;
			while (instructions.size())
			{
				// Find a potential match
				last_status = GmpFindMnemonicPattern(
					instructions,
					{
						ZYDIS_MNEMONIC_MOV,
						ZYDIS_MNEMONIC_MOV
					},
					two_movs_index
				);

				// If no matches exist, end the loop
				if (!AurieSuccess(last_status))
				{
					// Reset the counter
					two_movs_index = SIZE_MAX;
					break;
				}

				ZydisDisassembledInstruction& first_mov = instructions.at(two_movs_index).RawForm;
				ZydisDisassembledInstruction& second_mov = instructions.at(two_movs_index + 1).RawForm;

				// TODO: I don't know how to invert this properly
				// To explain this whole thing, we're searching for two consecutive movs that fulfill:
				// - Moving from some memory addresses (offset by a common value) to (any) registers
				// - That's about it?
				if ((first_mov.info.operand_count == 2 && second_mov.info.operand_count == 2) &&
					(first_mov.operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER && second_mov.operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER) &&
					(first_mov.operands[1].type == ZYDIS_OPERAND_TYPE_MEMORY && second_mov.operands[1].type == ZYDIS_OPERAND_TYPE_MEMORY) &&
					(first_mov.operands[1].mem.disp.has_displacement && second_mov.operands[1].mem.disp.has_displacement) &&
					(first_mov.operands[1].mem.disp.value == second_mov.operands[1].mem.disp.value)
					)
				{
					break;
				}

				// Create a new vector, starting at where the two movs ended, up until the end of the current vector
				std::vector<TargettedInstruction> new_instructions(
					instructions.cbegin() + two_movs_index + 1, instructions.cend()
				);

				// Move from new_instructions to instructions, effectively replacing them
				instructions = std::move(new_instructions);

				// Reset the index
				two_movs_index = SIZE_MAX;
			}

			// If we couldn't find two movs that match, return an error
			if (two_movs_index == SIZE_MAX)
				return AURIE_OBJECT_NOT_FOUND;

			*ArrayOffset = instructions.at(two_movs_index).RawForm.operands[1].mem.disp.value;

			return AURIE_SUCCESS;
		}
	}
}