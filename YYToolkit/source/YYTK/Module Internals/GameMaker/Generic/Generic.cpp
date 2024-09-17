#include "../../Module Internals.hpp"

using namespace Aurie;

namespace YYTK
{
	// ===== Shared, non-GameMaker-reliant code =====

	std::vector<TargettedInstruction> GmpDisassemble(
		IN PVOID Address,
		IN size_t MaximumSize,
		IN size_t MaximumInstructionsWithoutFunction
	)
	{
		// Query our current architecture to know what we disassemble as
		USHORT image_architecture = 0;
		if (!AurieSuccess(PpGetCurrentArchitecture(image_architecture)))
			return {};

		ZyanUPointer runtime_address = reinterpret_cast<ZyanUPointer>(Address);

		// Our current offset in the memory_data array
		ZyanUSize offset = 0;

		// Instructions since the last lea [reg], [mem] instruction
		size_t instructions_since_last_function = 0;

		// Vector storing all the instructions encountered
		std::vector<TargettedInstruction> instructions = {};

		// Loop until we either exceed the instruction limit or hit the maximum pointer limit
		while (
			(instructions_since_last_function < MaximumInstructionsWithoutFunction) &&
			(runtime_address < (reinterpret_cast<ZyanUPointer>(Address) + MaximumSize)
		))
		{
			ZydisDisassembledInstruction current_instruction;
			
			ZyanStatus disassembly_status = ZydisDisassembleIntel(
				(image_architecture == IMAGE_FILE_MACHINE_AMD64) ? ZYDIS_MACHINE_MODE_LONG_64 : ZYDIS_MACHINE_MODE_LEGACY_32,
				runtime_address,
				reinterpret_cast<PVOID>(runtime_address),
				MaximumSize - offset,
				&current_instruction
			);

			if (!ZYAN_SUCCESS(disassembly_status))
			{
				runtime_address++;
				offset++;
			}

			// If we succeded, we can continue disassembly
			TargettedInstruction chain_entry = {};
			chain_entry.RawForm = current_instruction;
			chain_entry.FunctionTarget = nullptr;

			// If we're loading the address of something, it may be a function
			if (current_instruction.info.mnemonic == ZYDIS_MNEMONIC_LEA)
			{
				// Loop the operands of the instruction
				for (size_t i = 0; i < current_instruction.info.operand_count; i++)
				{
					ZydisDecodedOperand& current_operand = current_instruction.operands[i];

					// Find one of type memory (a pointer)
					if (current_operand.type != ZYDIS_OPERAND_TYPE_MEMORY)
						continue;

					// Make sure the instruction has displacement (a value)
					if (!current_operand.mem.disp.has_displacement)
						continue;

					// Calculate the absolute address
					ZyanU64 call_address = 0;
					ZydisCalcAbsoluteAddress(
						&current_instruction.info,
						&current_operand,
						current_instruction.runtime_address,
						&call_address
					);

					chain_entry.FunctionTarget = reinterpret_cast<PVOID>(call_address);

					// Reset our counter
					instructions_since_last_function = 0;
				}
			}

			// Push our stuff back into the vector
			instructions.push_back(chain_entry);

			offset += current_instruction.info.length;
			runtime_address += current_instruction.info.length;
			instructions_since_last_function++;
		}

		return instructions;
	}

	size_t GmpCountInstructionReferences(
		IN const std::vector<TargettedInstruction>& Instructions
	)
	{
		size_t count = 0;
		for (const auto& instruction : Instructions)
		{
			if (instruction.FunctionTarget)
				count++;
		}

		return count;
	}

	AurieStatus GmpSigscanRegionEx(
		IN const unsigned char* RegionBase,
		IN const size_t RegionSize,
		IN const unsigned char* Pattern,
		IN const char* PatternMask,
		OUT std::vector<size_t>& Matches
	)
	{
		Matches.clear();

		size_t pattern_size = strlen(PatternMask);
		size_t region_base = reinterpret_cast<size_t>(RegionBase);
		size_t region_size_left = RegionSize;

		while (true)
		{
			// Scan for the pattern
			size_t current_match = MmSigscanRegion(
				reinterpret_cast<const unsigned char*>(region_base),
				region_size_left,
				Pattern,
				PatternMask
			);

			// Once a pattern is not found, we break out and exit
			if (!current_match)
				break;

			// If we found it, there might still be more instances of that pattern!
			Matches.push_back(current_match);

			// Shift the region base, and subtract the size remaining accordingly
			// We subtract the size first, since we need the unchanged region_base variable.
			region_size_left -= (current_match + pattern_size) - region_base;
			region_base = current_match + pattern_size;
		}

		return AURIE_SUCCESS;
	}

	AurieStatus GmpFindMnemonicPattern(
		IN const std::vector<YYTK::TargettedInstruction>& Instructions,
		IN const std::vector<ZydisMnemonic>& Mnemonics,
		OUT size_t& StartIndex,
		OPTIONAL IN size_t LoopStartIndex // = 0
	)
	{
		// Loop all instructions in the vector
		for (size_t start_index = LoopStartIndex; start_index < Instructions.size() - Mnemonics.size(); start_index++)
		{
			bool pattern_matches = true;

			// Loop all target mnemonics
			for (size_t in_pattern_index = 0; in_pattern_index < Mnemonics.size(); in_pattern_index++)
			{
				const ZydisMnemonic& actual_mnemonic = Instructions.at(start_index + in_pattern_index).RawForm.info.mnemonic;
				const ZydisMnemonic& target_mnemonic = Mnemonics.at(in_pattern_index);

				if (actual_mnemonic != target_mnemonic)
				{
					pattern_matches = false;
					break;
				}
			}

			// If we didn't set the pattern_matches flag to false, 
			// that means it matched the whole thing, and we got our start index!
			if (pattern_matches)
			{
				StartIndex = start_index;
				return AURIE_SUCCESS;
			}
		}

		return AURIE_OBJECT_NOT_FOUND;
	}
}