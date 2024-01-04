#include "../Module Internals.hpp"
#include <cinttypes>

using namespace Aurie;

namespace YYTK
{
	AurieStatus GmpGetRunnerInterface(
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
			if (function_count > 90 && function_count < 104)
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

		int64_t interface_start_on_stack = 0;

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
			}

			memcpy(
				interface_base + mov_displacement,
				&lea.FunctionTarget,
				sizeof(PVOID)
			);
		}

		return AURIE_SUCCESS;
	}

	Aurie::AurieStatus GmpGetBuiltinInformation(
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
			return AURIE_MODULE_INITIALIZATION_FAILED;

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
			return AURIE_MODULE_INITIALIZATION_FAILED;

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
			return AURIE_MODULE_INITIALIZATION_FAILED;

		BuiltinCount = reinterpret_cast<int32_t*>(array_numb_address);
		BuiltinArray = reinterpret_cast<RVariableRoutine*>(array_base_address);

		return AURIE_SUCCESS;
	}

	std::vector<TargettedInstruction> GmpDisassemble(
		IN PVOID Address,
		IN size_t MaximumSize,
		IN size_t MaximumInstructionsWithoutFunction
	)
	{
		using namespace Aurie;

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

		// Loop either until Zydis fails at disassembly, or until we get
		// a lot of instructions without a function reference (meaning we wondered off)
		ZydisDisassembledInstruction current_instruction;
		while (ZYAN_SUCCESS(ZydisDisassembleIntel(
			ZYDIS_MACHINE_MODE_LONG_64,
			runtime_address,
			memory_data + offset,
			MaximumSize - offset,
			&current_instruction
		)) && instructions_since_last_function < MaximumInstructionsWithoutFunction)
		{
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

	Aurie::AurieStatus GmpFindScriptData(
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

	Aurie::AurieStatus GmpFindRoomData(
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

	Aurie::AurieStatus GmpFindCurrentRoomData(
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

	Aurie::AurieStatus GmpFindRVArrayOffset(
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

	Aurie::AurieStatus GmpFindDoCallScript(
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

	Aurie::AurieStatus GmpFindCodeExecute(
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
				"\x0F\xB6\xD8"			// mozvx ebx, al
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

	Aurie::AurieStatus GmpFindMnemonicPattern(
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
}
