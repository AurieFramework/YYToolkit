#include "../../Module Internals.hpp"

using namespace Aurie;

namespace YYTK
{
	namespace VM
	{
		static CRoom*** g_RoomArray = nullptr;
		static CRoom* GmpGetRoomEntry(
			IN int Index
		)
		{
			return (*g_RoomArray)[Index];
		}

		AurieStatus GmpFindRoomData(
			IN TRoutine RoomInstanceClear,
			OUT FNRoomData* RoomData
		)
		{
			// In F_RoomInstanceClear (VM), the Room_Data function is inlined.
			// At the beginning of the function, the Room pointer is computed:
			// mov rax, cs:g_RoomArray.Rooms ; move a pointer to the first element in the room array into rax
			// mov rbx, [rax+rbx*8]		     ; compute the offset of the room at index rbx (sizeof(CRoom*) == 8)
			// test rbx, rbx				 ; check the room is not null
			// jz <??>						 ; jump if null

			// Disassemble the function
			std::vector<TargettedInstruction> instructions = GmpDisassemble(
				RoomInstanceClear,
				0x100,
				0xFF
			);

			// The first match should be it
			size_t pattern_index = 0;
			AurieStatus last_status = GmpFindMnemonicPattern(
				instructions,
				{
					ZYDIS_MNEMONIC_MOV,
					ZYDIS_MNEMONIC_MOV,
					ZYDIS_MNEMONIC_TEST,
					ZYDIS_MNEMONIC_JZ
				},
				pattern_index
			);

			// If we didn't get a match, something is wrong
			if (!AurieSuccess(last_status))
				return last_status;

			assert(instructions[pattern_index].RawForm.info.mnemonic == ZYDIS_MNEMONIC_MOV);

			ZydisDisassembledInstruction& mov_instruction = instructions[pattern_index].RawForm;

			// Make sure the mov has two operands
			if (mov_instruction.info.operand_count != 2)
				return AURIE_INVALID_SIGNATURE;

			// We're supposed to be moving to a register
			if (mov_instruction.operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
				return AURIE_INVALID_SIGNATURE;

			// We're supposed to be moving from memory
			if (mov_instruction.operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
				return AURIE_INVALID_SIGNATURE;

			// Calculate the address of the room array
			ZyanU64 array_address = 0;
			if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
				&mov_instruction.info,
				&mov_instruction.operands[1],
				mov_instruction.runtime_address,
				&array_address
			)))
			{
				return AURIE_EXTERNAL_ERROR;
			}

			g_RoomArray = reinterpret_cast<CRoom***>(array_address);
			*RoomData = GmpGetRoomEntry;
			return AURIE_SUCCESS;
		}
	}
}