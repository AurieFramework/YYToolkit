#include "../../Module Internals.hpp"

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
				break;
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
	}
}