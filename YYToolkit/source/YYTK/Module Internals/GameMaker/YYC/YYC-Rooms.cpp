#include "../../Module Internals.hpp"

using namespace Aurie;

namespace YYTK
{
	namespace YYC
	{
		AurieStatus GmpFindRoomDataX86(
			IN TRoutine RoomInstanceClear,
			OUT FNRoomData* RoomData
		)
		{
			/*
				We're disassembling F_RoomInstanceClear (different in x86)

				void F_RoomInstanceClear(
					OUT RValue& Result,
					IN CInstance* Self,
					IN CInstance* Other,
					IN int ArgumentCount,
					IN RValue* Arguments
				)
				{
					int room_id = YYGetReal(Arguments, 0);
					CRoom* room_data = Room_Data(room_id); // <=== looking for this

					if (room_data)
						room_data->ClearStorageInstances();
				}

				It's the second call instruction.
			*/

			// Disassemble 80 bytes at the function
			std::vector<TargettedInstruction> instructions = GmpDisassemble(
				RoomInstanceClear,
				0x50,
				0xFF
			);

			size_t target_call_index = 0;
			AurieStatus last_status = GmpFindMnemonicPattern(
				instructions,
				{
					ZYDIS_MNEMONIC_CALL,	// call <Room_Data>
					ZYDIS_MNEMONIC_ADD,		// add esp, 0xC
					ZYDIS_MNEMONIC_TEST		// test eax, eax
				},
				target_call_index
			);

			if (!AurieSuccess(last_status))
				return last_status;

			const ZydisDisassembledInstruction& call_instruction = instructions.at(target_call_index).RawForm;

			// This should always be the case.
			// But if it's not, it might cause unforeseen bugs, so we assert that in debug builds
			assert(call_instruction.info.mnemonic == ZYDIS_MNEMONIC_CALL);

			ZyanU64 room_data_address = 0;
			ZydisCalcAbsoluteAddress(
				&call_instruction.info,
				&call_instruction.operands[0],
				call_instruction.runtime_address,
				&room_data_address
			);

			// Make sure we have a valid address
			if (!room_data_address)
				return AURIE_MODULE_INITIALIZATION_FAILED;

			*RoomData = reinterpret_cast<FNRoomData>(room_data_address);
			return AURIE_SUCCESS;
		}

		AurieStatus GmpFindRoomDataX64(
			IN TRoutine RoomInstanceClear,
			OUT FNRoomData* RoomData
		)
		{
			/*
				We're disassembling F_RoomInstanceClear

				void F_RoomInstanceClear(
					OUT RValue& Result,
					IN CInstance* Self,
					IN CInstance* Other,
					IN int ArgumentCount,
					IN RValue* Arguments
				)
				{
					int room_count = Room_Number();
					int room_id = YYGetRef(arg, 0, 0x1000003, 0, 0);
					CRoom* room_data = Room_Data(room_id); // <=== looking for this

					if (room_data)
						room_data->ClearStorageInstances();
				}

				It's the third call instruction.
			*/

			// Disassemble 80 bytes at the function
			std::vector<TargettedInstruction> instructions = GmpDisassemble(
				RoomInstanceClear,
				0x50,
				0xFF
			);

			size_t target_call_index = 0;
			AurieStatus last_status = GmpFindMnemonicPattern(
				instructions,
				{
					ZYDIS_MNEMONIC_CALL,
					ZYDIS_MNEMONIC_TEST
				},
				target_call_index
			);

			if (!AurieSuccess(last_status))
				return last_status;

			const ZydisDisassembledInstruction& call_instruction = instructions.at(target_call_index).RawForm;

			// This should always be the case.
			// But if it's not, it might cause unforeseen bugs, so we assert that in debug builds
			assert(call_instruction.info.mnemonic == ZYDIS_MNEMONIC_CALL);

			ZyanU64 room_data_address = 0;
			ZydisCalcAbsoluteAddress(
				&call_instruction.info,
				&call_instruction.operands[0],
				call_instruction.runtime_address,
				&room_data_address
			);

			// Make sure we have a valid address
			if (!room_data_address)
				return AURIE_MODULE_INITIALIZATION_FAILED;

			*RoomData = reinterpret_cast<FNRoomData>(room_data_address);
			return AURIE_SUCCESS;
		}

		AurieStatus GmpFindRoomData(
			IN TRoutine RoomInstanceClear,
			OUT FNRoomData* RoomData
		)
		{
			USHORT architecture = 0;
			AurieStatus last_status = PpGetCurrentArchitecture(architecture);

			if (!AurieSuccess(last_status))
				return last_status;

			if (architecture == IMAGE_FILE_MACHINE_AMD64)
				return GmpFindRoomDataX64(RoomInstanceClear, RoomData);

			return GmpFindRoomDataX86(RoomInstanceClear, RoomData);
		}
	}
}