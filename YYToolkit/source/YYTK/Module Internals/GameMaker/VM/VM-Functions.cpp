#include "../../Module Internals.hpp"

using namespace Aurie;

namespace YYTK
{
	namespace VM
	{
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
	}
}