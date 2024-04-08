#include "../Module Internals.hpp"
#include <cinttypes>

using namespace Aurie;

namespace YYTK
{
	namespace YYC
	{
		AurieStatus GmpGetBuiltinInformationX86(
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
			size_t pattern_match = MmSigscanModule(
				game_name.c_str(),
				UTEXT(
					"\x3D\xF4\x01\x00\x00"	// cmp eax, 0x1F4
					"\x75\x00"				// jnz short ??
				),
				"xxxxxx?"
			);

			if (!pattern_match)
				return AURIE_OBJECT_NOT_FOUND;

			// We disassemble the function starting at the pattern
			std::vector<TargettedInstruction> instructions = GmpDisassemble(
				reinterpret_cast<PVOID>(pattern_match),
				0x20,
				0xFF
			);

			// Now, scan for the first jnz instruction
			// This should be the second one (instructions[1]),
			// but I don't want to hardcode it...
			intptr_t jnz_instruction_index = -1;
			for (size_t i = 0; i < instructions.size(); i++)
			{
				const auto& instruction = instructions.at(i).RawForm;

				if (instruction.info.mnemonic != ZYDIS_MNEMONIC_JNZ)
					continue;

				jnz_instruction_index = i;
			}

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
				return AURIE_EXTERNAL_ERROR;

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
					array_base_address = raw_instruction.operands[1].mem.disp.value;
				}

				// Until we have the address of the array "numb" (ie. the amount of elements used up)
				// we have to check for MOVSXD instructions. It's the first one we encounter after the initial jmp
				if ((raw_instruction.info.mnemonic == ZYDIS_MNEMONIC_MOV) && (array_numb_address == 0))
				{
					array_numb_address = raw_instruction.operands[1].mem.disp.value;
				}
			}

			if (!array_base_address || !array_numb_address)
				return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

			BuiltinCount = reinterpret_cast<int32_t*>(array_numb_address);
			BuiltinArray = reinterpret_cast<RVariableRoutine*>(array_base_address);

			return AURIE_SUCCESS;
		}

		AurieStatus GmpGetBuiltinInformationX64(
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
			size_t pattern_match = MmSigscanModule(
				game_name.c_str(),
				UTEXT(
					"\x3D\xF4\x01\x00\x00"	// cmp eax, 0x1F4
					"\x75\x00"				// jnz short ??
				),
				"xxxxxx?"
			);

			if (!pattern_match)
				return AURIE_OBJECT_NOT_FOUND;

			// We disassemble the function starting at the pattern
			std::vector<TargettedInstruction> instructions = GmpDisassemble(
				reinterpret_cast<PVOID>(pattern_match),
				0x20,
				0xFF
			);

			// Now, scan for the first jnz instruction
			// This should be the second one (instructions[1]),
			// but I don't want to hardcode it...
			intptr_t jnz_instruction_index = -1;
			for (size_t i = 0; i < instructions.size(); i++)
			{
				const auto& instruction = instructions.at(i).RawForm;

				if (instruction.info.mnemonic != ZYDIS_MNEMONIC_JNZ)
					continue;

				jnz_instruction_index = i;
			}

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
				return AURIE_EXTERNAL_ERROR;

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
				return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

			BuiltinCount = reinterpret_cast<int32_t*>(array_numb_address);
			BuiltinArray = reinterpret_cast<RVariableRoutine*>(array_base_address);

			return AURIE_SUCCESS;
		}
		

		AurieStatus GmpGetBuiltinInformation(
			OUT int32_t*& BuiltinCount,
			OUT RVariableRoutine*& BuiltinArray
		)
		{
			USHORT architecture = 0;
			AurieStatus last_status = PpGetCurrentArchitecture(architecture);

			if (!AurieSuccess(last_status))
				return last_status;

			if (architecture == IMAGE_FILE_MACHINE_AMD64)
				return GmpGetBuiltinInformationX64(BuiltinCount, BuiltinArray);

			return GmpGetBuiltinInformationX86(BuiltinCount, BuiltinArray);
		}

		AurieStatus GmpFindFunctionsArrayX86(
			IN const YYRunnerInterface& Interface,
			OUT RFunction*** FunctionsArray
		)
		{
			if (!Interface.Code_Function_Find)
				return AURIE_MODULE_INTERNAL_ERROR;

			// Disassemble this function
			std::vector<TargettedInstruction> instructions = GmpDisassemble(
				Interface.Code_Function_Find,
				0x7F,
				0xFF
			);

			// It just so happens the first 6-byte long MOV (1 byte difference from x64) 
			// instruction references the the_functions array.
			// It usually looks like mov <32bit register>, [the_functions]
			for (auto& instruction : instructions)
			{
				// The instruction has to be a mov
				if (instruction.RawForm.info.mnemonic != ZYDIS_MNEMONIC_MOV)
					continue;

				// The instruction has to be 6 bytes in length
				if (instruction.RawForm.info.length != 6)
					continue;

				// The instruction has to have 2 operands
				// The first one (operands[0]) is the register being moved into
				// The second one (operands[1]) is the address
				if (instruction.RawForm.info.operand_count != 2)
					continue;

				ZydisDecodedOperand& first_operand = instruction.RawForm.operands[0];
				ZydisDecodedOperand& second_operand = instruction.RawForm.operands[1];

				// We have to be moving INTO a register, not FROM a register
				if (first_operand.type != ZYDIS_OPERAND_TYPE_REGISTER)
					continue;

				// We have to be moving from a memory location, not from a register
				if (second_operand.type != ZYDIS_OPERAND_TYPE_MEMORY)
					continue;

				// There has to be an offset... duh
				if (!second_operand.mem.disp.has_displacement)
					continue;

				// Calculate the absolute address
				ZyanU64 call_address = 0;
				ZydisCalcAbsoluteAddress(
					&instruction.RawForm.info,
					&second_operand,
					instruction.RawForm.runtime_address,
					&call_address
				);

				// It's a pointer to a pointer, we dereference it once to   
				// get the actual pointer to the first element in the array
				*FunctionsArray = reinterpret_cast<RFunction**>(call_address);
				return AURIE_SUCCESS;
			}

			return AURIE_OBJECT_NOT_FOUND;
		}

		AurieStatus GmpFindFunctionsArrayX64(
			IN const YYRunnerInterface& Interface,
			OUT RFunction*** FunctionsArray
		)
		{
			if (!Interface.Code_Function_Find)
				return AURIE_MODULE_INTERNAL_ERROR;

			// Disassemble this function
			std::vector<TargettedInstruction> instructions = GmpDisassemble(
				Interface.Code_Function_Find,
				0x200,
				0xFF
			);

			// It just so happens the first 7-byte long MOV  
			// instruction references the the_functions array.
			// It usually looks like mov <64bit register>, [the_functions]
			for (auto& instruction : instructions)
			{
				// The instruction has to be a mov
				if (instruction.RawForm.info.mnemonic != ZYDIS_MNEMONIC_MOV)
					continue;

				// The instruction has to be 7 bytes in length
				if (instruction.RawForm.info.length != 7)
					continue;

				// The instruction has to have 2 operands
				// The first one (operands[0]) is the register being moved into
				// The second one (operands[1]) is the address
				if (instruction.RawForm.info.operand_count != 2)
					continue;

				ZydisDecodedOperand& first_operand = instruction.RawForm.operands[0];
				ZydisDecodedOperand& second_operand = instruction.RawForm.operands[1];

				// We have to be moving INTO a register, not FROM a register
				if (first_operand.type != ZYDIS_OPERAND_TYPE_REGISTER)
					continue;

				// Get the register we're moving into, and get the largest variant of that register
				ZydisRegister largest_enclosing = ZydisRegisterGetLargestEnclosing(
					ZYDIS_MACHINE_MODE_LONG_64,
					first_operand.reg.value
				);

				// If the register is already in its largest variant, we can continue
				// This is to filter out the_numb (the number of elements in the functions array),
				// which is being moved in the same way, just into a 32-bit register.

				if (largest_enclosing != first_operand.reg.value)
					continue;

				// We have to be moving from a memory location, not from a register
				if (second_operand.type != ZYDIS_OPERAND_TYPE_MEMORY)
					continue;

				// There has to be an offset... duh
				if (!second_operand.mem.disp.has_displacement)
					continue;

				// Calculate the absolute address
				ZyanU64 call_address = 0;
				ZydisCalcAbsoluteAddress(
					&instruction.RawForm.info,
					&second_operand,
					instruction.RawForm.runtime_address,
					&call_address
				);

				// It's a pointer to a pointer, we dereference it once to   
				// get the actual pointer to the first element in the array
				*FunctionsArray = reinterpret_cast<RFunction**>(call_address);
				return AURIE_SUCCESS;
			}

			return AURIE_OBJECT_NOT_FOUND;
		}

		AurieStatus GmpFindFunctionsArray(
			IN const YYRunnerInterface& Interface,
			OUT RFunction*** FunctionsArray
		)
		{
			USHORT architecture = 0;
			AurieStatus last_status = PpGetCurrentArchitecture(architecture);

			if (!AurieSuccess(last_status))
				return last_status;

			if (architecture == IMAGE_FILE_MACHINE_AMD64)
				return GmpFindFunctionsArrayX64(Interface, FunctionsArray);

			return GmpFindFunctionsArrayX86(Interface, FunctionsArray);
		}

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

		AurieStatus GmpFindRVArrayOffset(
			IN TRoutine F_ArrayEquals,
			IN YYRunnerInterface& RunnerInterface,
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
	}
}
