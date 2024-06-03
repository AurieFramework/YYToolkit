#include "../Module Internals.hpp"
#include <cinttypes>

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

		ZyanU8* memory_data = reinterpret_cast<ZyanU8*>(MmAllocateMemory(g_ArSelfModule, MaximumSize));
		if (!memory_data)
			return {};

		// Copy the bytes from the .text section to our buffer
		ZyanUPointer runtime_address = reinterpret_cast<ZyanUPointer>(Address);
		memcpy(memory_data, Address, MaximumSize);

		// Our current offset in the memory_data array
		ZyanUSize offset = 0;

		// Instructions since the last lea [reg], [mem] instruction
		ZyanI32 instructions_since_last_function = 0;

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
				memory_data + offset,
				MaximumSize - offset,
				&current_instruction
			);

			if (!ZYAN_SUCCESS(disassembly_status))
			{
				// Just an invalid opcode, skip to the next one and continue
				if (disassembly_status == ZYDIS_STATUS_DECODING_ERROR)
				{
					runtime_address++;
					offset++;
					continue;
				}
				
				// If it's another, more serious error, break out
				break;
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

		MmFreeMemory(g_ArSelfModule, memory_data);
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
		IN const std::vector<TargettedInstruction>& Instructions,
		IN const std::vector<ZydisMnemonic>& Mnemonics,
		OUT size_t& StartIndex
	)
	{
		// Loop all instructions in the vector
		for (size_t start_index = 0; start_index < Instructions.size() - Mnemonics.size(); start_index++)
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

	// ===== Shared, GameMaker-reliant code =====

	// x86 version of GmpGetRunnerInterface.
	AurieStatus GmpGetRunnerInterfaceX86(
		OUT YYRunnerInterface& Interface
	)
	{
		// In x86, the pattern for finding the runner interface is incredibly simple.
		/*
			Loop Hero (GM 2.3.6)

				85 C0                              test    eax, eax
				0F 88 4E 03 00 00                  js      loc_15D0F23
				50                                 push    eax
				E8 C5 35 00 00                     call    sub_15D41A0
				C7 84 24 A0 00 00 00 00 00 00 00   mov     [esp+0DB0h+var_D10], 0
				C7 44 24 24 F0 BF 54 01            mov     [esp+0DB0h+var_D8C], offset sub_154BFF0
				C7 44 24 28 C0 0F 5D 01            mov     [esp+0DB0h+var_D88], offset sub_15D0FC0
				C7 44 24 2C C0 DE 66 01            mov     [esp+0DB0h+var_D84], offset sub_166DEC0
				C7 44 24 30 A0 B1 54 01            mov     [esp+0DB0h+var_D80], offset sub_154B1A0
				...

			Deltarune, Chapter 2 (GM 2022.2)
				85 C0                              test    eax, eax
				0F 88 A6 03 00 00                  js      loc_4F7FDB
				50                                 push    eax
				E8 B5 FB FC FF                     call    sub_4C77F0
				C7 84 24 A0 00 00 00 00 00 00 00   mov     [esp+0DCCh+var_D2C], 0
				C7 44 24 24 40 05 58 00            mov     [esp+0DCCh+var_DA8], offset sub_580540
				C7 44 24 28 90 80 4F 00            mov     [esp+0DCCh+var_DA4], offset sub_4F8090
				C7 44 24 2C E0 72 43 00            mov     [esp+0DCCh+var_DA0], offset sub_4372E0
				C7 44 24 30 60 F5 57 00            mov     [esp+0DCCh+var_D9C], offset sub_57F560
				C7 44 24 34 E0 85 56 00            mov     [esp+0DCCh+var_D98], offset sub_5685E0

			... Diffing to see what's similar
				85 C0                              test    eax, eax
				0F 88 ?? ?? ?? ??                  js      ??
				50                                 push    eax
				E8 ?? ?? ?? ??                     call    ??

				then the C7s which are inconsistent lengths apart because of the first instruction
		*/

		// Find the required pattern in the game
		// There may be multiple that match, but only one is correct.
		uint64_t text_section_base = 0;
		size_t text_section_size = 0;

		// Get the .text section address for the game executable
		AurieStatus last_status = Internal::PpiGetModuleSectionBounds(
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
				"\x85\xC0"					// test eax, eax
				"\x0F\x88\x00\x00\x00\x00"	// js ??
				"\x50"						// push eax
				"\xE8\x00\x00\x00\x00"		// call ??
				"\xC7"						// first byte of a mov
			),
			"xxxx????x",
			pattern_matches
		);

		// TODO: Loop all matches, see if there's a long chain of ZYDIS_MNEMONIC_MOV, where:
		//		- operands[0].type = ZYDIS_OPERAND_TYPE_MEMORY
		//		- operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE

		if (pattern_matches.empty())
			return AURIE_OBJECT_NOT_FOUND;

		memset(&Interface, 0, sizeof(Interface));
		std::vector<ZydisDisassembledInstruction> mov_instructions;

		for (const auto& match : pattern_matches)
		{
			// Now disassemble at the match, strip all instructions except movs that match the pattern above
			std::vector<TargettedInstruction> instructions = GmpDisassemble(
				reinterpret_cast<PVOID>(match),
				0x4FF,
				140
			);

			// Get every mov from the instructions
			for (const auto& instr : instructions)
			{
				// Skip anything that's not a mov
				if (instr.RawForm.info.mnemonic != ZYDIS_MNEMONIC_MOV)
					continue;

				// Strip any movs that aren't moving to some memory
				if (instr.RawForm.operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY)
					continue;

				// Strip any movs that aren't moving from an immediate
				if (instr.RawForm.operands[1].type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
					continue;

				mov_instructions.push_back(instr.RawForm);
			}

			if (mov_instructions.size() > 80 && mov_instructions.size() < 104)
			{
				CmWriteWarning(
					"Found %d functions in %d assembly instructions!",
					mov_instructions.size(),
					instructions.size()
				);

				break;
			}

			mov_instructions.clear();
		}

		// Loop all compatible mov instructions, find the stack base
		int64_t interface_start_on_stack = INT_MAX;
		for (auto& instr : mov_instructions)
		{
			if (instr.operands[0].mem.disp.value < interface_start_on_stack)
				interface_start_on_stack = instr.operands[0].mem.disp.value;
		}

		// Now loop everything again and fill the struct
		char* interface_base = reinterpret_cast<char*>(&Interface);
		for (auto& instr : mov_instructions)
		{
			int64_t offset = instr.operands[0].mem.disp.value - interface_start_on_stack;

			if (offset >= sizeof(YYRunnerInterface))
			{
				// TODO: instr.operands[1].imm.value might be relative?
				CmWriteWarning(
					"YYRunnerInterface+0x%04llx = 0x%p, but sizeof(YYRunnerInterface) = 0x%x",
					offset,
					instr.operands[1].imm.value,
					sizeof(YYRunnerInterface)
				);

				continue;
			}

			// Copy the function pointer to our interface copy
			memcpy(interface_base + offset, &instr.operands[1].imm.value, sizeof(PVOID));
		}

		return AURIE_SUCCESS;
	}

	// Finds the Code_Variable_FindAlloc_Slot_From_Name function
	AurieStatus GmpGetFindAllocSlotFromName(
		IN PFN_YYObjectBaseAdd YYObjectBase_Add,
		OUT	PFN_FindAllocSlot* FindAllocSlot
	)
	{
		// Code_Variable_FindAlloc_Slot_From_Name is usually the first call.
		// See comment above GmpGetYYObjectBaseAdd for decomp...

		// Make sure we have the required function
		if (!YYObjectBase_Add)
			return AURIE_UNAVAILABLE;

		// Disassemble 48 bytes at the function
		std::vector<TargettedInstruction> instructions = GmpDisassemble(
			YYObjectBase_Add,
			48,
			0xFF
		);

		// Find the first call, and trace the target
		ZyanU64 function_address = 0;

		for (const auto& instr : instructions)
		{
			if (instr.RawForm.info.mnemonic != ZYDIS_MNEMONIC_CALL)
				continue;

			ZydisCalcAbsoluteAddress(
				&instr.RawForm.info,
				&instr.RawForm.operands[0],
				instr.RawForm.runtime_address,
				&function_address
			);
			break;
		}

		if (!function_address)
			return AURIE_OBJECT_NOT_FOUND;

		*FindAllocSlot = reinterpret_cast<PFN_FindAllocSlot>(function_address);
		return AURIE_SUCCESS;
	}

	std::pair<std::vector<TargettedInstruction>, size_t> GmpLookupChainedInstructions(
		IN const std::vector<size_t>& PotentialChainBaseAddresses,
		IN const size_t MinimumChainSize,
		IN const size_t MaximumChainSize,
		IN const size_t MaximumInstructionsWithoutFunctionReference,
		IN const size_t DisassemblySize
	)
	{
		size_t chain_size = 0;
		std::vector<TargettedInstruction> instructions = {};
		for (const size_t& base_address : PotentialChainBaseAddresses)
		{
			// Magic numbers, look at the disassembly in that huge comment above
			instructions = GmpDisassemble(
				reinterpret_cast<PVOID>(base_address),
				DisassemblySize,
				MaximumInstructionsWithoutFunctionReference
			);

			chain_size = GmpCountInstructionReferences(instructions);

			// If there's less than 80 functions, it's probably not the interface
			// YYRunnerInterface has 96 functions in LTS + 2 variables = 98 lea matches
			// 2022.3 YYC has 88 functions
			if (chain_size > MinimumChainSize && chain_size < MaximumChainSize)
				break;

			instructions.clear();
			chain_size = 0;
		}

		return std::make_pair(instructions, chain_size);
	}

	AurieStatus GmpGetRunnerInterfaceX64(
		OUT YYRunnerInterface& Interface
	)
	{
		AurieStatus last_status = AURIE_SUCCESS;

		// Scan the memory for this pattern:
		/*

			In Runner 2023.8 symbols
				E8 CF 30 18 00        call    class DLL_RFunction * __ptr64 __cdecl DLL_GetFunc(int)
				33 C9                 xor     ecx, ecx
				48 89 8D 88 00 00 00  mov     [rbp+7C0h+Interface.Script_Perform], rcx
				48 8D 0D CF 9F 1D 00  lea     rcx, void __cdecl YYprintf(char const * __ptr64,...)
				48 89 4D 90           mov     [rbp+7C0h+Interface.DebugConsoleOutput], rcx
				48 8D 0D B4 05 00 00  lea     rcx, void __cdecl ReleaseConsoleOutput(char const * __ptr64,...)
				... every field assigned here...
			In Risk of Rain Returns
				E8 9F 2D 00 00        call    DLL_GetFunc
				33 C9                 xor     ecx, ecx
				48 89 8D 88 00 00 00  mov     [rbp+136], rcx
				48 8D 0D 4F 30 F1 FF  lea     rcx, YYprintf
				48 89 4D 90           mov     [rbp-112], rcx
				48 8D 0D B4 05 00 00  lea     rcx, ReleaseConsoleInput
				48 89 4D 98           mov     [rbp-104], rcx
				48 8D 0D B9 93 01 00  lea     rcx, ShowMessage
				48 89 4D A0           mov     [rbp+7C0h+Interface.ShowMessage], rcx
				48 8D 0D EE 1C F1 FF  lea     rcx, sub_1412C0D70
				48 89 4D A8           mov     [rbp+7C0h+Interface.YYError], rcx

			Diffing to see what's similar (non-matches replaced with ??)

				E8 ?? ?? ?? ??	      call    DLL_GetFunc
				33 C9                 xor     ecx, ecx
				48 89 8D 88 00 00 00  mov     [rbp+136], rcx
				48 8D 0D ?? ?? ?? ??  lea     [rip+??]
				48 89 4D 90           mov     [rbp-112], rcx
				... all the way down ...

			Turns out in Will You Snail, this changes

				33 C0				  xor     eax, eax
				48 89 85 88 00 00 00  mov     [rbp+7D0h+var_840.Script_Perform], rax
				48 8D 05 9B 97 0F 00  lea     rax, DebugConsoleOutput
		*/

		// Find the required pattern in the game
		// There may be multiple that match, but only one is correct.
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
				"\x33\x00"						// xor ??, ??
				"\x48\x89\x00\x00\x00\x00\x00"	// mov [rsp+??], ??
				"\x48\x8D\x0D\x00\x00\x00\x00"	// lea [??+??]
			),
			"x?xx?????xx?????",
			pattern_matches
		);

		// If there's less than 80 functions, it's probably not the interface
		// YYRunnerInterface has 96 functions in LTS + 2 variables = 98 lea matches
		// 2022.3 YYC has 88 functions

		// Loop through all the matches, and check if they have a long function chain following them
		auto [chain_instructions, chain_size] = GmpLookupChainedInstructions(
			pattern_matches,
			84, /* 2022.3 YYC has 88 functions, so leaving 4 less is probably okay */
			104, /* LTS runner has 96 functions, and two variables = 98 chainlinks - leave a few free */
			4, /* There should be at most 4 instructions preceding the first mov-lea pair */
			0x1000 /* Chosen such that the chain's disassembly fits completely in the buffer */
		);
		
		// In Crashlands 2's demo, the mov instruction seems different.
		// The mov is relative to RBP instead of RSP, meaning it's 4-byte instead of 6-byte.
		// If this is the case in the current runner, we fail the check below.
		/*
			E8 6E 3F 00 00        call    sub_7FF718FE5280
			33 C9                 xor     ecx, ecx
			48 89 4D 58           mov     [rbp+58h], rcx
			48 8D 0D 51 C7 F5 FF  lea     rcx, sub_7FF718F3DA70
			48 89 4C 24 60        mov     [rsp+60h], rcx
			48 8D 0D A5 05 00 00  lea     rcx, sub_7FF718FE18D0
			48 89 4C 24 60        mov     [rsp+68h], rcx
		*/

		if (!chain_size)
		{
			// Overwrite pattern_matches with new base addresses
			GmpSigscanRegionEx(
				reinterpret_cast<const unsigned char*>((game_base + text_section_base)),
				text_section_size,
				UTEXT(
					"\x33\x00"						// xor ??, ??
					"\x48\x89\x4D\x00"				// mov [rbp+??], rcx
					"\x48\x8D\x0D\x00\x00\x00\x00"	// lea [??+??]
				),
				"x?xxx?xxx????",
				pattern_matches
			);

			// Rescan for the chain.
			std::tie(chain_instructions, chain_size) = GmpLookupChainedInstructions(
				pattern_matches,
				84, /* 2022.3 YYC has 88 functions, so leaving 4 less is probably okay */
				104, /* LTS runner has 96 functions, and two variables = 98 chainlinks - leave a few free */
				4, /* There should be at most 4 instructions preceding the first mov-lea pair */
				0x1000 /* Chosen such that the chain's disassembly fits completely in the buffer */
			);

			// If no chain exists, we give up.
			if (!chain_size)
				return AURIE_OBJECT_NOT_FOUND;
		}

		CmWriteWarning(
			"Found %lld functions in %lld assembly instructions!",
			chain_size,
			chain_instructions.size()
		);

		// Function entries are not sorted by their offset in the YYRunnerInterface structure
		// We have to somehow get the runner interface base on the stack, and then fill out based off that...
		// Fucking hell this is hard..

		// The register that's XORed with itself in the first instruction is our target register.
		// Find that register.
		ZydisRegister target_register = ZYDIS_REGISTER_NONE;
		for (size_t i = 0; i < chain_instructions[0].RawForm.info.operand_count; i++)
		{
			ZydisDecodedOperand& operand = chain_instructions[0].RawForm.operands[i];

			if (operand.type != ZYDIS_OPERAND_TYPE_REGISTER)
				continue;

			// We got the register. That's great. But given we're x64, we have to 
			// take care of the conversion between eg. ecx <=> rcx
			target_register = operand.reg.value;

			// Gets the bigger register, ie. converts ecx => rcx; bx => rbx
			ZydisRegister enclosing_register = ZydisRegisterGetLargestEnclosing(
				ZYDIS_MACHINE_MODE_LONG_64,
				target_register
			);

			// If the xor instruction is already xoring the biggest available register
			// for the current architecture, ZydisRegisterGetLargestEnclosing will
			// return ZYDIS_REGISTER_NONE. We don't want to overwrite target_register in that case.
			if (enclosing_register != ZYDIS_REGISTER_NONE)
				target_register = enclosing_register;

			break;
		}

		// If we didn't find the target register, shit went wrong.
		// If you're debugging this, check what disassembly is and compare it to 
		// the giant comment at the beginning of this function.
		if (target_register == ZYDIS_REGISTER_NONE)
			return AURIE_OBJECT_NOT_FOUND;

		CmWriteWarning(
			"Target register is %s!",
			ZydisRegisterGetString(target_register)
		);

		// Now we have to look for instructions that follow this pattern:
		// mov [rbp+offset], target_register
		// This is due to the interface not starting at [rbp], which means
		// we can't just take whatever offset is and memcpy(interface + offset, function, sizeof(function)).
		// We have to find where the structure actually starts on the stack.

		int64_t interface_start_on_stack = INT_MAX;

		// We find it by looping all instructions and looking for the mov that has the lowest offset.
		// The first elements are gonna be at [rbp+whatever], and that whatever is then gonna start
		// incrementing by sizeof(PVOID).

		for (size_t i = 0; i < chain_instructions.size(); i++)
		{
			const TargettedInstruction& instruction = chain_instructions[i];

			if (instruction.RawForm.info.mnemonic != ZYDIS_MNEMONIC_MOV)
				continue;

			// TODO: Check if this instruction references the target_register

			for (size_t j = 0; j < chain_instructions[i].RawForm.info.operand_count; j++)
			{
				const ZydisDecodedOperand& operand = chain_instructions[i].RawForm.operands[j];

				// Make sure the operand is of type memory
				if (operand.type != ZYDIS_OPERAND_TYPE_MEMORY)
					continue;

				// We're looking for instructions that are [rbp+whatever], not [rsp+whatever]
				if (operand.mem.base != ZYDIS_REGISTER_RBP)
					continue;

				int64_t displacement = operand.mem.disp.has_displacement ? operand.mem.disp.value : 0;

				if (displacement < interface_start_on_stack)
					interface_start_on_stack = displacement;

				break;
			}
		}

		CmWriteWarning(
			"Interface starts on stack offset %lld",
			interface_start_on_stack
		);

		// Now we look for every LEA instruction, and then find the most recent mov instruction.
		// For simplicity's sake, we first loop all instructions, and stuff each "mov" and "lea" into a vector.
		// We filter them by checking if one of their operands is a register, and comparing that register
		// to the target register we extracted earlier.
		std::vector<TargettedInstruction> movs_and_leas;

		for (const TargettedInstruction& instruction : chain_instructions)
		{
			switch (instruction.RawForm.info.mnemonic)
			{
			case ZYDIS_MNEMONIC_MOV:
			{
				bool has_correct_operands = false;

				for (size_t i = 0; i < instruction.RawForm.info.operand_count; i++)
				{
					const ZydisDecodedOperand& operand = instruction.RawForm.operands[i];

					// Make sure the operand is of type memory
					if (operand.type != ZYDIS_OPERAND_TYPE_MEMORY)
						continue;

					// We're looking for instructions that are [rbp+whatever], not [rsp+whatever]
					if (operand.mem.base != ZYDIS_REGISTER_RBP)
						continue;

					has_correct_operands = true;
					break;
				}

				// Only push if the mov operands are correct (matches "mov [rbp+whatever], register" pattern)
				if (has_correct_operands)
					movs_and_leas.push_back(instruction);

				break;
			}
			case ZYDIS_MNEMONIC_LEA:
			{
				movs_and_leas.push_back(instruction);
				break;
			}
			}
		}

		std::vector<std::pair<TargettedInstruction, TargettedInstruction>> associated_leas;

		// Now loop all the movs and leas, and associate them together
		for (size_t i = 0; i < movs_and_leas.size(); i++)
		{
			const TargettedInstruction& current_mov = movs_and_leas[i];

			// Skip any non-mov instructions (they come first, the pattern is mov-lea, mov-lea...)
			if (current_mov.RawForm.info.mnemonic != ZYDIS_MNEMONIC_MOV)
				continue;

			// Find the last mov instruction before the current lea
			TargettedInstruction last_lea = {};
			for (size_t j = 0; j < i; j++)
			{
				// Skip any non-mov instructions
				if (movs_and_leas[j].RawForm.info.mnemonic != ZYDIS_MNEMONIC_LEA)
					continue;

				last_lea = movs_and_leas[j];
			}

			associated_leas.push_back(
				std::make_pair(current_mov, last_lea)
			);
		}

		CmWriteWarning(
			"Associated %lld LEA + MOV instructions!",
			associated_leas.size()
		);

		// Now the moment we've all been waiting for. 
		// WE BUILD THE INTERFACE!
		char* interface_base = reinterpret_cast<char*>(&Interface);
		for (auto& [mov, lea] : associated_leas)
		{
			int64_t mov_displacement = 0;
			for (size_t i = 0; i < mov.RawForm.info.operand_count; i++)
			{
				const ZydisDecodedOperand& operand = mov.RawForm.operands[i];

				// Make sure the operand is of type memory
				if (operand.type != ZYDIS_OPERAND_TYPE_MEMORY)
					continue;

				// We're looking for instructions that are [rbp+whatever], not [rsp+whatever]
				if (operand.mem.base != ZYDIS_REGISTER_RBP)
					continue;

				mov_displacement = operand.mem.disp.has_displacement ? operand.mem.disp.value : 0;
				break;
			}

			// Shift the displacement of the mov (the offset from rbp) by where the structure starts
			// This effectively makes it so [rbp-70h] translates to 0 if the thing starts at [rbp-70h]
			mov_displacement -= interface_start_on_stack;

			// If we're trying to write past our YYRunnerInterface struct,
			// we just skip it. There's functions at the end that are in newer runners,
			// but not in LTS. Ask nik / nkrapivin for more info.
			if (mov_displacement >= sizeof(YYRunnerInterface))
			{
				CmWriteWarning(
					"YYRunnerInterface+0x%04llx = 0x%p, but sizeof(YYRunnerInterface) = 0x%llx",
					mov_displacement,
					lea.FunctionTarget,
					sizeof(YYRunnerInterface)
				);

				continue;
			}

			memcpy(
				interface_base + mov_displacement,
				&lea.FunctionTarget,
				sizeof(PVOID)
			);
		}

		return AURIE_SUCCESS;
	}

	// Locates the YYObjectBase:Add function, works in both x86 and x64, VM and YYC
	// Rough decomp is this:
	/*
		void YYObjectBase::Add(
			IN const char* Name,
			IN RValue* Value,
			IN int Flags
		)
		{
			if ((this->m_Flags & 1) == 0)
				return;

			int variable_slot = Code_Variable_FindAlloc_Slot_From_Name(this, Name);

			RValue* variable_slot_ptr = nullptr;
			if (this->m_YYVars)
				variable_slot_ptr = &this->m_YYVars[variable_slot];
			else
				variable_slot_ptr = this->InternalGetYYVar(variable_slot);

			// Deep-copy RValue from Value to variable_slot_ptr
			// ...
		}
	
	*/
	AurieStatus GmpGetYYObjectBaseAdd(
		IN const YYRunnerInterface& Interface,
		OUT PFN_YYObjectBaseAdd* Function
	)
	{
		// Make sure we have the required function
		if (!Interface.StructAddRValue)
			return AURIE_UNAVAILABLE;

		// Disassemble 32 bytes at the function
		std::vector<TargettedInstruction> instructions = GmpDisassemble(
			Interface.StructAddRValue,
			32, 
			0xFF
		);

		// Find either the first jump or the first call, and trace the target
		ZyanU64 function_address = 0;

		for (const auto& instr : instructions)
		{
			if (instr.RawForm.info.mnemonic != ZYDIS_MNEMONIC_JMP && instr.RawForm.info.mnemonic != ZYDIS_MNEMONIC_CALL)
				continue;
			
			ZydisCalcAbsoluteAddress(
				&instr.RawForm.info,
				&instr.RawForm.operands[0],
				instr.RawForm.runtime_address,
				&function_address
			);
			break;
		}

		if (!function_address)
			return AURIE_OBJECT_NOT_FOUND;

		*Function = reinterpret_cast<PFN_YYObjectBaseAdd>(function_address);
		return AURIE_SUCCESS;
	}

	// Calls whatever architecture is currently running
	AurieStatus GmpGetRunnerInterface(
		OUT YYRunnerInterface& Interface
	)
	{
		USHORT architecture = 0;
		AurieStatus last_status = PpGetCurrentArchitecture(architecture);

		if (!AurieSuccess(last_status))
			return last_status;

		if (architecture == IMAGE_FILE_MACHINE_AMD64)
			return GmpGetRunnerInterfaceX64(Interface);

		return GmpGetRunnerInterfaceX86(Interface);
	}

	AurieStatus GmpFindScriptData(
		IN const YYRunnerInterface& Interface,
		IN TRoutine CopyStatic,
		OUT FNScriptData* ScriptData
	)
	{
		if (!Interface.YYGetInt32)
			return AURIE_MODULE_INTERNAL_ERROR;

		if (!CopyStatic)
			return AURIE_MODULE_INTERNAL_ERROR;

		/*
			The F_CopyStatic function (registered as @@CopyStatic@@ in the runner) roughly decompiles to this:
			void F_CopyStatic(RValue& Result, CInstance* SelfInstance, CInstance* OtherInstance, int ArgumentCount, RValue* Arguments)
			{
				int32_t script_index = YYGetInt32(Arguments, 0);
				if (script_index >= 100000)
					script_index -= 100000;

				YYObjectBase* script_prototype = ScriptData(script_index)->m_Code->m_Prototype;
				if (script_prototype)
				{
					if (SelfInstance->m_Prototype)
						SelfInstance->m_Prototype->m_Prototype = script_prototype;
				}
			}

			We therefore know that the there are only two function calls, and we know the address of one of them.
			The following code disassembles F_CopyStatic, and looks for the first call that's not YYGetInt32.
			That will be our ScriptData() call.
		*/

		// Disassemble 256 bytes at the function
		std::vector<TargettedInstruction> instructions = GmpDisassemble(
			CopyStatic,
			0x100,
			0xFF
		);

		for (auto& instruction : instructions)
		{
			ZydisDisassembledInstruction& raw_instruction = instruction.RawForm;

			// Skip any non-call instructions
			if (raw_instruction.info.mnemonic != ZYDIS_MNEMONIC_CALL)
				continue;

			// We're searching for call [visible_operand]
			// Note: this might not be necessary to check
			if (raw_instruction.info.operand_count_visible != 1)
				continue;

			// We're jumping to an immediate, not to a register
			if (raw_instruction.operands[0].type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
				continue;

			ZyanU64 call_address = 0;
			ZydisCalcAbsoluteAddress(
				&raw_instruction.info,
				&raw_instruction.operands[0],
				raw_instruction.runtime_address,
				&call_address
			);

			// Skip the first YYGetInt32 call
			if (call_address == reinterpret_cast<ZyanU64>(Interface.YYGetInt32))
				continue;

			// Determine the call instruction's target
			*ScriptData = reinterpret_cast<FNScriptData>(call_address);
			break;
		}

		return AURIE_SUCCESS;
	}

	AurieStatus GmpFindCodeExecuteX64(
		OUT PVOID* CodeExecute
	)
	{
		AurieStatus last_status = AURIE_SUCCESS;

		// Get the name of the game executable
		std::wstring game_name;
		last_status = MdGetImageFilename(
			g_ArInitialImage,
			game_name
		);

		if (!AurieSuccess(last_status))
			return last_status;

		// We're looking for a pattern in Code_Execute
		size_t pattern_match = MmSigscanModule(
			game_name.c_str(),
			UTEXT(
				"\xE8\x00\x00\x00\x00"	// call <ExecuteIt>
				"\x0F\xB6\xD8"			// movzx ebx, al
				"\x3C\x01"				// cmp al, 1
			),
			"x????xxxxx"
		);

		if (!pattern_match)
			return AURIE_MODULE_INITIALIZATION_FAILED;

		std::vector<TargettedInstruction> instructions = GmpDisassemble(
			reinterpret_cast<PVOID>(pattern_match),
			0x10,
			0xFF
		);

		// Get the first instruction at that address (the call instruction), and make sure it has the
		// parameters we expect it to have (ie. is a call, and has 1 visible operand - the address.)
		ZydisDisassembledInstruction& call_instruction = instructions.front().RawForm;
		if (call_instruction.info.mnemonic != ZYDIS_MNEMONIC_CALL)
			return AURIE_MODULE_INITIALIZATION_FAILED;

		if (call_instruction.info.operand_count_visible < 1)
			return AURIE_MODULE_INITIALIZATION_FAILED;

		// Calculate the address of the function which we're calling (ExecuteIt)
		ZyanU64 execute_it_address = 0;
		if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
			&call_instruction.info,
			&call_instruction.operands[0],
			call_instruction.runtime_address,
			&execute_it_address
		)))
		{
			return AURIE_MODULE_INITIALIZATION_FAILED;
		}

		// We should've never gotten here if the pattern or translation fails.
		assert(execute_it_address != 0);

		*CodeExecute = reinterpret_cast<PVOID>(execute_it_address);

		return AURIE_SUCCESS;
	}

	AurieStatus GmpFindCodeExecuteX86(
		OUT PVOID* CodeExecute
	)
	{
		AurieStatus last_status = AURIE_SUCCESS;

		// Get the name of the game executable
		std::wstring game_name;
		last_status = MdGetImageFilename(
			g_ArInitialImage,
			game_name
		);

		if (!AurieSuccess(last_status))
			return last_status;

		// We're looking for a pattern in Code_Execute
		size_t pattern_match = MmSigscanModule(
			game_name.c_str(),
			UTEXT(
				"\xE8\x00\x00\x00\x00"	// call <ExecuteIt>
				"\x8A\xD8"				// mov bl, al
				"\x83\xC4\x14"			// add esp, 14
			),
			"x????xxxxx"
		);

		if (!pattern_match)
			return AURIE_MODULE_INITIALIZATION_FAILED;

		std::vector<TargettedInstruction> instructions = GmpDisassemble(
			reinterpret_cast<PVOID>(pattern_match),
			0x10,
			0xFF
		);

		// Get the first instruction at that address (the call instruction), and make sure it has the
		// parameters we expect it to have (ie. is a call, and has 1 visible operand - the address.)
		ZydisDisassembledInstruction& call_instruction = instructions.front().RawForm;
		if (call_instruction.info.mnemonic != ZYDIS_MNEMONIC_CALL)
			return AURIE_MODULE_INITIALIZATION_FAILED;

		if (call_instruction.info.operand_count_visible < 1)
			return AURIE_MODULE_INITIALIZATION_FAILED;

		// Calculate the address of the function which we're calling (ExecuteIt)
		ZyanU64 execute_it_address = 0;
		if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
			&call_instruction.info,
			&call_instruction.operands[0],
			call_instruction.runtime_address,
			&execute_it_address
		)))
		{
			return AURIE_MODULE_INITIALIZATION_FAILED;
		}

		// We should've never gotten here if the pattern or translation fails.
		assert(execute_it_address != 0);

		*CodeExecute = reinterpret_cast<PVOID>(execute_it_address);

		return AURIE_SUCCESS;
	}

	AurieStatus GmpFindCodeExecute(
		OUT PVOID* CodeExecute
	)
	{
		USHORT architecture = 0;
		AurieStatus last_status = PpGetCurrentArchitecture(architecture);

		if (!AurieSuccess(last_status))
			return last_status;

		if (architecture == IMAGE_FILE_MACHINE_AMD64)
			return GmpFindCodeExecuteX64(CodeExecute);

		return GmpFindCodeExecuteX86(CodeExecute);
	}

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