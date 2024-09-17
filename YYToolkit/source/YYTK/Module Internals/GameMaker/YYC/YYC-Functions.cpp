#include "../../Module Internals.hpp"

using namespace Aurie;

namespace YYTK
{
	namespace YYC
	{
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
	}
}