#include "../../Module Internals.hpp"

using namespace Aurie;

namespace YYTK
{

	AurieStatus GmpFindCurrentRoomDataX64(
		IN FNSetVariable SV_BackgroundColor,
		OUT CRoom*** Run_Room
	)
	{
		// Disassemble 80 bytes at the function
		std::vector<TargettedInstruction> instructions = GmpDisassemble(
			SV_BackgroundColor,
			0x50,
			0xFF
		);

		// The first mov in this pattern is a mov reg, [memory]
		size_t target_mov_index = 0;
		AurieStatus last_status = GmpFindMnemonicPattern(
			instructions,
			{
				ZYDIS_MNEMONIC_MOV,
				ZYDIS_MNEMONIC_TEST,
				ZYDIS_MNEMONIC_JZ
			},
			target_mov_index
		);

		if (!AurieSuccess(last_status))
			return last_status;

		const ZydisDisassembledInstruction& move_instruction = instructions.at(target_mov_index).RawForm;

		// This should always be the case.
		// But if it's not, it might cause unforeseen bugs, so we assert that in debug builds
		assert(move_instruction.info.mnemonic == ZYDIS_MNEMONIC_MOV);
		assert(move_instruction.info.operand_count == 2);
		assert(move_instruction.operands[1].type == ZYDIS_OPERAND_TYPE_MEMORY);

		ZyanU64 run_room_address = 0;
		ZydisCalcAbsoluteAddress(
			&move_instruction.info,
			&move_instruction.operands[1],
			move_instruction.runtime_address,
			&run_room_address
		);

		// Make sure we have a valid address
		if (!run_room_address)
			return AURIE_MODULE_INITIALIZATION_FAILED;

		*Run_Room = reinterpret_cast<CRoom**>(run_room_address);
		return AURIE_SUCCESS;
	}

	AurieStatus GmpFindCurrentRoomDataX86(
		IN FNSetVariable SV_BackgroundColor,
		OUT CRoom*** Run_Room
	)
	{
		// Disassemble 32 bytes at the function
		std::vector<TargettedInstruction> instructions = GmpDisassemble(
			SV_BackgroundColor,
			0x20,
			0xFF
		);

		// Find the first cmp instruction
		size_t target_cmp_index = 0;
		AurieStatus last_status = GmpFindMnemonicPattern(
			instructions,
			{
				ZYDIS_MNEMONIC_CMP // cmp Run_Room, 0
			},
			target_cmp_index
		);

		if (!AurieSuccess(last_status))
			return last_status;

		const ZydisDisassembledInstruction& compare_instruction = instructions.at(target_cmp_index).RawForm;

		// This should always be the case.
		// But if it's not, it might cause unforeseen bugs, so we assert that in debug builds
		assert(compare_instruction.info.mnemonic == ZYDIS_MNEMONIC_CMP);
		assert(compare_instruction.info.operand_count_visible == 2);
		assert(compare_instruction.operands[0].type == ZYDIS_OPERAND_TYPE_MEMORY);
		assert(compare_instruction.operands[1].type == ZYDIS_OPERAND_TYPE_IMMEDIATE);

		ZyanU64 run_room_address = 0;
		ZydisCalcAbsoluteAddress(
			&compare_instruction.info,
			&compare_instruction.operands[0],
			compare_instruction.runtime_address,
			&run_room_address
		);

		// Make sure we have a valid address
		if (!run_room_address)
			return AURIE_MODULE_INITIALIZATION_FAILED;

		*Run_Room = reinterpret_cast<CRoom**>(run_room_address);
		return AURIE_SUCCESS;
	}

	AurieStatus GmpFindCurrentRoomData(
		IN FNSetVariable SV_BackgroundColor,
		OUT CRoom*** Run_Room
	)
	{
		USHORT architecture = 0;
		AurieStatus last_status = PpGetCurrentArchitecture(architecture);

		if (!AurieSuccess(last_status))
			return last_status;

		if (architecture == IMAGE_FILE_MACHINE_AMD64)
			return GmpFindCurrentRoomDataX64(SV_BackgroundColor, Run_Room);

		return GmpFindCurrentRoomDataX86(SV_BackgroundColor, Run_Room);
	}
}