#include "../Module Internals.hpp"
#include <cinttypes>

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
				// This should be the second one (instructions[1]),
				// but I don't want to hardcode it...
				// TODO: Use mnemonic scan
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

			// In YYC, the first 7-byte long mov references the functions array:
				// mov <64bit register>, [the_functions]
			// In VM, this technique results in finding the Extension array, 
			// since Extension_Function_GetId is inlined into Code_Function_Find...

			std::vector<RFunction**> potential_function_arrays;
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
				potential_function_arrays.push_back(reinterpret_cast<RFunction**>(call_address));
			}

			if (potential_function_arrays.empty())
				return AURIE_OBJECT_NOT_FOUND;

			// Return the lowest one in memory
			// TODO: Figure out how to actually do this
			RFunction** lowest_in_memory = reinterpret_cast<RFunction**>(MAXULONG_PTR);
			for (auto& array_pointer : potential_function_arrays)
			{
				if (reinterpret_cast<uintptr_t>(lowest_in_memory) > reinterpret_cast<uintptr_t>(array_pointer))
					lowest_in_memory = array_pointer;
			}

			*FunctionsArray = lowest_in_memory;
			return AURIE_SUCCESS;
		}

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

		AurieStatus GmpFindRVArrayOffset(
			IN TRoutine F_ArrayEquals,
			OUT int64_t* ArrayOffset
		)
		{
			return AURIE_NOT_IMPLEMENTED;
		}

		AurieStatus GmpFindDoCallScript(
			OUT PVOID* DoCallScript
		)
		{
			return AURIE_NOT_IMPLEMENTED;
		}
	}
}