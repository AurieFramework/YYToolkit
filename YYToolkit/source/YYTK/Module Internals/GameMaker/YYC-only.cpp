#include "../Module Internals.hpp"
#include <cinttypes>

using namespace Aurie;

namespace YYTK
{
	namespace YYC
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
			int64_t jnz_instruction_index = -1;
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

		AurieStatus GmpFindFunctionsArray(
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

		AurieStatus GmpFindRoomData(
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

		AurieStatus GmpFindRVArrayOffset(
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
				return AURIE_MODULE_INITIALIZATION_FAILED;

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
				return AURIE_MODULE_INITIALIZATION_FAILED;

			*ArrayOffset = instructions.at(two_movs_index).RawForm.operands[1].mem.disp.value;

			return AURIE_SUCCESS;
		}

		AurieStatus GmpFindDoCallScript(
			OUT PVOID* DoCallScript
		)
		{
			/*
			- DoCallScript:
				Only used in VM games, but YYTK Next still provides a way to hook,
					mainly for compatibility with both VM and YYC compile targets.
					This sig is auto-generated by Sigmaker, as I haven't found a more consistent way yet.
					It *is* possible to hook at DoCallGML (second call), but the function is inconsistent
					across runner versions, and often changes signatures completely.

				The pattern we scan for is this (in DoCall):
					In Will You Snail (2022.4):
						.text:000000014028BE32 4D 8B CC        mov     r9, r12
						.text:000000014028BE35 4C 8B C6        mov     r8, rsi
						.text:000000014028BE38 41 8B D2        mov     edx, r10d
						.text:000000014028BE3B 48 8B 5D AF     mov     rbx, qword ptr [rbp+57h+var_A8]
						.text:000000014028BE3F 48 8B CB        mov     rcx, rbx
						.text:000000014028BE42 E8 79 F7 FF FF  call    DoCallScript
						.text:000000014028BE47 48 8B F0        mov     rsi, rax
						.text:000000014028BE4A 4D 85 FF        test    r15, r15
						.text:000000014028BE4D 74 12           jz      short loc_14028BE61
					In Risk of Rain Returns (2023.6):
						.text:00000001412B6F27 4D 8B CC        mov     r9, r12
						.text:00000001412B6F2A 4C 8B C6        mov     r8, rsi
						.text:00000001412B6F2D 41 8B D2        mov     edx, r10d
						.text:00000001412B6F30 48 8B 5D D7     mov     rbx, qword ptr [rbp+57h+var_80]
						.text:00000001412B6F34 48 8B CB        mov     rcx, rbx
						.text:00000001412B6F37 E8 54 2A 00 00  call    DoCallScript
						.text:00000001412B6F3C 48 8B F0        mov     rsi, rax
						.text:00000001412B6F3F 4D 85 FF        test    r15, r15
						.text:00000001412B6F42 74 16           jz      short loc_1412B6F5A
					In runner 2023.8 symbols, this changes:
						.text:000000000022594A 4D 8B CC        mov     r9, r12         ; Locals
						.text:000000000022594D 4C 8B C7        mov     r8, rdi         ; VmInstance
						.text:0000000000225950 41 8B D7        mov     edx, r15d       ; StackPointer
						.text:0000000000225953 48 8B 75 CF     mov     rsi, qword ptr [rbp+57h+var_88.anonymous_0]
						.text:0000000000225957 48 8B CE        mov     rcx, rsi        ; Script
						.text:000000000022595A E8 51 2B 00 00  call    DoCallScript
						.text:000000000022595F 48 8B F8        mov     rdi, rax
						.text:0000000000225962 4D 85 ED        test    r13, r13
						.text:0000000000225965 74 16           jz      short loc_22597D

					Diffing to see what's similar
							4D 8B CC        mov     r9, r12         ; Locals
							4C 8B ??        mov     r8, ??          ; VmInstance
							41 8B ??		mov		edx, ??
							48 8B ?? ??		mov     rsi, ??
							48 8B ??		mov     rcx, ??			; Script
							E8 ?? ?? ?? ??  call    ?? <DoCallScript>
							48 8B ??	    mov     ??, rax
							4D 85 ??        test	??, ??
							74 ??			jz		??

				You can find this function in IDA easily, simply search for the string "script call = %s\n" and xref.
			*/

			AurieStatus last_status = AURIE_SUCCESS;

			// Get the name of the game executable
			std::wstring game_name;
			last_status = MdGetImageFilename(
				g_ArInitialImage,
				game_name
			);

			if (!AurieSuccess(last_status))
				return last_status;

			// We're looking for a pattern in DoCall (different from DoCallGML)
			size_t pattern_match = MmSigscanModule(
				game_name.c_str(),
				UTEXT(
					"\x4D\x8B\xCC"			// mov r9, r12
					"\x4C\x8B\x00"			// mov r8, ??
					"\x41\x8B\x00"			// mov edx, ??
					"\x48\x8B\x00\x00"		// mov rsi, ??
					"\x48\x8B\x00"			// mov rcx, ??
					"\xE8\x00\x00\x00\x00"	// call <DoCallScript>
					"\x48\x8B\x00"			// mov ??, rax
					"\x4D\x85\x00"			// test ??, ??
					"\x74\x00"				// jz ??
				),
				"xxxxx?xx?xx??xx?x????xx?xx?x?"
			);

			if (!pattern_match)
				return AURIE_MODULE_INITIALIZATION_FAILED;

			std::vector<TargettedInstruction> instructions = GmpDisassemble(
				reinterpret_cast<PVOID>(pattern_match),
				0x20,
				0xFF
			);

			ZyanU64 docallscript_address = 0;
			for (auto& instruction : instructions)
			{
				ZydisDisassembledInstruction& raw_instruction = instruction.RawForm;

				if (raw_instruction.info.mnemonic != ZYDIS_MNEMONIC_CALL)
					continue;

				// Make sure the call instruction has an operand
				if (raw_instruction.info.operand_count_visible < 1)
					continue;

				// If we can't calculate the absolute address, just fail outright.
				if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
					&raw_instruction.info,
					&raw_instruction.operands[0],
					raw_instruction.runtime_address,
					&docallscript_address
				)))
				{
					return AURIE_MODULE_INITIALIZATION_FAILED;
				}

				// We found the first call instruction, we know where DoCallScript is.
				// No need to loop further.
				break;
			}

			if (!docallscript_address)
				return AURIE_MODULE_INITIALIZATION_FAILED;

			*DoCallScript = reinterpret_cast<PVOID>(docallscript_address);
			return AURIE_SUCCESS;
		}
	}
}
