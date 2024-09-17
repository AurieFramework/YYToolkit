#include "../../Module Internals.hpp"

using namespace Aurie;

namespace YYTK
{
	// x64 version of GmpGetRunnerInterface.
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
				"\x33\xC9"						// xor ??, ??
				"\x48\x89\x8D\x00\x00\x00\x00"	// mov [rbp+??], ??
				"\x48\x8D\x0D\x00\x00\x00\x00"	// lea [??+??]
			),
			"x?xx?????xx?????",
			pattern_matches
		);

		// Loop through all the matches, and check if they have a long function chain
		std::vector<TargettedInstruction> instructions = {};
		for (const size_t& match : pattern_matches)
		{
			// Magic numbers, look at the disassembly in that huge comment above
			instructions = GmpDisassemble(
				reinterpret_cast<PVOID>(match),
				0x1000,
				4
			);

			size_t function_count = GmpCountInstructionReferences(instructions);

			// If there's less than 80 functions, it's probably not the interface
			// YYRunnerInterface has 96 functions in LTS + 2 variables = 98 lea matches
			// 2022.3 YYC has 88 functions
			if (function_count > 87 && function_count < 104)
				break;

			instructions.clear();
		}

		if (instructions.empty())
			return AURIE_OBJECT_NOT_FOUND;

		CmWriteWarning(
			"Found %lld functions in %lld assembly instructions!",
			GmpCountInstructionReferences(instructions),
			instructions.size()
		);

		// Function entries are not sorted by their offset in the YYRunnerInterface structure
		// We have to somehow get the runner interface base on the stack, and then fill out based off that...
		// Fucking hell this is hard..

		// The register that's XORed with itself in the first instruction is our target register.
		// Find that register.
		ZydisRegister target_register = ZYDIS_REGISTER_NONE;
		for (size_t i = 0; i < instructions[0].RawForm.info.operand_count; i++)
		{
			ZydisDecodedOperand& operand = instructions[0].RawForm.operands[i];

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

		for (size_t i = 0; i < instructions.size(); i++)
		{
			const TargettedInstruction& instruction = instructions[i];

			if (instruction.RawForm.info.mnemonic != ZYDIS_MNEMONIC_MOV)
				continue;

			// TODO: Check if this instruction references the target_register

			for (size_t j = 0; j < instructions[i].RawForm.info.operand_count; j++)
			{
				const ZydisDecodedOperand& operand = instructions[i].RawForm.operands[j];

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

		for (const TargettedInstruction& instruction : instructions)
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
}