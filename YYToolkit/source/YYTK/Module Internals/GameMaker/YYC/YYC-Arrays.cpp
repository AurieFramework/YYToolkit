#include "../../Module Internals.hpp"

using namespace Aurie;

namespace YYTK
{
	namespace YYC
	{
		AurieStatus GmpFindRVArrayOffsetX64(
			IN TRoutine F_ArrayEquals,
			OUT int64_t* ArrayOffset
		)
		{
			// Okay... this one is a doozy
			// A TLDR is probably this: We scan for a call to ArrayEquals inside F_ArrayEquals.
			// Inside ArrayEquals, we look for the access of pArray1->pArray and pArray2->pArray.

			// Decompilation of F_ArrayEquals is this:
			/*
				void F_ArrayEquals(
					OUT RValue& Result,
					IN CInstance* Self,
					IN CInstance* Other,
					IN int ArgumentCount,
					IN RValue* Arguments
				)
				{
					Result.m_Kind = VALUE_BOOL;
					Result.m_i64 = 0;

					if (ArgumentCount != 2)
						YYError("array_equals :: takes 2 arguments");

					RefDynamicArrayOfRValue* first_array = YYGetArray(Arguments, 0, false);
					RefDynamicArrayOfRValue* second_array = YYGetArray(Arguments, 0, false);

					if (first_array && second_array)
					{
						// We find the call to ArrayEquals
						Result.m_Real = static_cast<double>(ArrayEquals(first_array, second_array) == 0);
					}
				}
			*/

			// So let's disassemble F_ArrayEquals
			std::vector<TargettedInstruction> instructions = GmpDisassemble(
				F_ArrayEquals,
				0xFF,
				0xFF
			);

			AurieStatus last_status = AURIE_SUCCESS;
			size_t start_index = 0;

			// Now look for this pattern
			// The call instruction target is the ArrayEquals function
			last_status = GmpFindMnemonicPattern(
				instructions,
				{
					ZYDIS_MNEMONIC_MOV,
					ZYDIS_MNEMONIC_MOV,
					ZYDIS_MNEMONIC_CALL
				},
				start_index
			);

			// Make sure we found that
			if (!AurieSuccess(last_status))
				return last_status;

			// We know the index of the call instruction is two away from the first mov
			size_t call_index = start_index + 2;

			const ZydisDisassembledInstruction& call_instruction = instructions.at(call_index).RawForm;

			// This should always be the case.
			// But if it's not, it might cause unforeseen bugs, so we assert that in debug builds
			assert(call_instruction.info.mnemonic == ZYDIS_MNEMONIC_CALL);

			ZyanU64 array_equals_internal_address = 0;
			ZydisCalcAbsoluteAddress(
				&call_instruction.info,
				&call_instruction.operands[0],
				call_instruction.runtime_address,
				&array_equals_internal_address
			);

			if (!array_equals_internal_address)
				return AURIE_INVALID_PARAMETER;

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

		AurieStatus GmpFindRVArrayOffsetX86(
			IN TRoutine F_ArrayEquals,
			OUT int64_t* ArrayOffset
		)
		{
			// Disassemble F_ArrayEquals
			std::vector<TargettedInstruction> instructions = GmpDisassemble(
				F_ArrayEquals,
				0xFF,
				0xFF
			);

			AurieStatus last_status = AURIE_SUCCESS;

			// x86 runners have the two movs inside the F_ArrayEquals function.
			// This cuts out one point of failure, which we take advantage of here.
			// We look for two movs that move into different registers each, but move
			// the same offset.
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

		AurieStatus GmpFindRVArrayOffset(
			IN TRoutine F_ArrayEquals,
			OUT int64_t* ArrayOffset
		)
		{
			USHORT architecture = 0;
			AurieStatus last_status = PpGetCurrentArchitecture(architecture);

			if (!AurieSuccess(last_status))
				return last_status;

			if (architecture == IMAGE_FILE_MACHINE_AMD64)
				return GmpFindRVArrayOffsetX64(F_ArrayEquals, ArrayOffset);

			return GmpFindRVArrayOffsetX86(F_ArrayEquals, ArrayOffset);
		}
	}
}