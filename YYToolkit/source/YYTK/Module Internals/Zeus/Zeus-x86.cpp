// This file is only used in x64 compilations. For x86 compilations, see Zeus-x86.cpp
#if not _WIN64
#include "../Module Internals.hpp"
using namespace Aurie;

#ifndef PAGE_SIZE
#define PAGE_SIZE (0x1000)
#endif // PAGE_SIZE

AurieStatus YYTK::Zeus::FindRunnerInterfaceHookpoint(
	OUT PVOID* TargetInstruction
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
	std::vector<size_t> pattern_matches = Memory::DmSigscanGameEx(
		UTEXT(
			"\x85\xC0"					// test eax, eax
			"\x0F\x88\x00\x00\x00\x00"	// js ??
			"\x50"						// push eax
			"\xE8\x00\x00\x00\x00"		// call ??
			"\xC7"						// first byte of a mov
		),
		"xxxx????x"
	);

	// TODO: Loop all matches, see if there's a long chain of ZYDIS_MNEMONIC_MOV, where:
	//		- operands[0].type = ZYDIS_OPERAND_TYPE_MEMORY
	//		- operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE

	if (pattern_matches.empty())
		return AURIE_OBJECT_NOT_FOUND;

	memset(&g_ModuleInterface.m_RunnerInterface, 0, sizeof(YYRunnerInterface));
	std::vector<ZydisDisassembledInstruction> mov_instructions;

	for (const auto& match : pattern_matches)
	{
		// Now disassemble at the match, strip all instructions except movs that match the pattern above
		auto instructions = Memory::DmDisassembleInstructionByRange(
			reinterpret_cast<PVOID>(match),
			0x500
		);

		// Get every mov from the instructions
		for (const auto& instr : instructions)
		{
			// Skip anything that's not a mov
			if (instr.info.mnemonic != ZYDIS_MNEMONIC_MOV)
				continue;

			// Strip any movs that aren't moving to some memory
			if (instr.operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY)
				continue;

			// Strip any movs that aren't moving from an immediate
			if (instr.operands[1].type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
				continue;

			mov_instructions.push_back(instr);
		}

		if (mov_instructions.size() > 80 && mov_instructions.size() < 104)
		{
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
	char* interface_base = reinterpret_cast<char*>(&g_ModuleInterface.m_RunnerInterface);
	for (auto& instr : mov_instructions)
	{
		int64_t offset = instr.operands[0].mem.disp.value - interface_start_on_stack;

		if (offset >= sizeof(YYRunnerInterface))
		{
			DbgPrintEx(LOG_SEVERITY_WARNING, "YYRunnerInterface+0x%04llx = 0x%p, but sizeof(YYRunnerInterface) = 0x%x", offset, instr.operands[1].imm.value, sizeof(YYRunnerInterface));
			continue;
		}

		// Copy the function pointer to our interface copy
		memcpy(interface_base + offset, &instr.operands[1].imm.value, sizeof(PVOID));
	}

	return AURIE_SUCCESS;
}

void YYTK::Zeus::HandleRunnerInterfaceCreation(
	IN ProcessorContext& ProcessorContext
)
{
	return;
}

AurieStatus YYTK::Zeus::RegisterRunnerInterfaceHook(
	IN PVOID Hookpoint,
	IN AurieMidHookFunction TargetFunction
)
{
	// Runner interface is ready by now - it's been ready ever since stage 1 found it on the stack.
	// A hook is not needed.
	SetEvent(g_ModuleInterface.m_RunnerInterfacePopulatedEvent);

	return AURIE_SUCCESS;
}

AurieStatus YYTK::Zeus::FindCodeExecutionHookpoint(
	OUT PVOID* TargetInstruction
)
{
	AurieStatus last_status = AURIE_SUCCESS;

	// We're looking for a pattern in Code_Execute
	size_t pattern_match = Memory::DmSigscanGame(
		UTEXT(
			"\xE8\x00\x00\x00\x00"	// call <ExecuteIt>
			"\x8A\xD8"				// mov bl, al
			"\x83\xC4\x14"			// add esp, 14
		),
		"x????xxxxx"
	);

	if (!pattern_match)
		return AURIE_MODULE_INITIALIZATION_FAILED;

	auto instruction = Memory::DmDisassembleInstruction(reinterpret_cast<PVOID>(pattern_match));

	if (instruction.info.mnemonic != ZYDIS_MNEMONIC_CALL)
		return AURIE_MODULE_INITIALIZATION_FAILED;

	if (instruction.info.operand_count_visible < 1)
		return AURIE_MODULE_INITIALIZATION_FAILED;

	// Calculate the address of the function which we're calling (ExecuteIt)
	ZyanU64 execute_it_address = 0;
	if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
		&instruction.info,
		&instruction.operands[0],
		instruction.runtime_address,
		&execute_it_address
	)))
	{
		return AURIE_MODULE_INITIALIZATION_FAILED;
	}

	// We should've never gotten here if the pattern or translation fails.
	assert(execute_it_address != 0);

	*TargetInstruction = reinterpret_cast<PVOID>(execute_it_address);

	return AURIE_SUCCESS;
}

AurieStatus YYTK::Zeus::DetermineFunctionEntrySize(
	IN RFunction** FunctionArray,
	OUT size_t* Size
)
{
	if (!FunctionArray)
		return AURIE_INVALID_PARAMETER;

	auto first_function = *FunctionArray;

	if (!first_function)
		return AURIE_INVALID_PARAMETER;

	// This is either an ASCII string that I just interpreted as a pointer,
	// or it's a real pointer into somewhere in the main executable's .rdata section.
	// If it's the latter, we know sizeof(RFunction) == 24.
	const char* potential_reference = first_function->ReferentialEntry.m_Name;
	if (!potential_reference)
		return AURIE_INVALID_PARAMETER;

	AurieStatus last_status = AURIE_SUCCESS;

	// Get the offset and size of the .rdata section
	uint64_t rdata_offset = 0;
	size_t rdata_size = 0;
	last_status = Internal::PpiGetModuleSectionBounds(
		GetModuleHandleA(nullptr),
		".rdata",
		rdata_offset,
		rdata_size
	);

	if (!AurieSuccess(last_status))
		return last_status;

	// Get the offset and size of the .text section
	uint64_t text_offset = 0;
	size_t text_size = 0;
	last_status = Internal::PpiGetModuleSectionBounds(
		GetModuleHandleA(nullptr),
		".text",
		text_offset,
		text_size
	);

	if (!AurieSuccess(last_status))
		return last_status;

	// The section base is returned relative to the module base
	text_offset += reinterpret_cast<uint64_t>(GetModuleHandleA(nullptr));
	rdata_offset += reinterpret_cast<uint64_t>(GetModuleHandleA(nullptr));

	char* rdata_section_start = reinterpret_cast<char*>(rdata_offset);
	char* rdata_section_end = reinterpret_cast<char*>(rdata_offset + rdata_size);

	// The string should be somewhere in the .rdata section
	// If it's not, it's definitely not RFunctionStringRef (or the structure is corrupt)
	if (potential_reference >= rdata_section_start && potential_reference <= rdata_section_end)
	{
		*Size = sizeof(RFunctionStringRef);
		return AURIE_SUCCESS;
	}

	char* text_section_start = reinterpret_cast<char*>(text_offset);
	char* text_section_end = reinterpret_cast<char*>(text_offset + text_size);
	char* routine = reinterpret_cast<char*>(first_function->FullEntry.m_Routine);

	// The routine should be somewhere in the .text section
	// If it's not, it's definitely not RFunctionStringFull (or the structure is corrupt)

	if (routine >= text_section_start && routine <= text_section_end)
	{
		*Size = sizeof(RFunctionStringFull);
		return AURIE_SUCCESS;
	}

	// Unknown size - possibly wrong runner architecture?
	return AURIE_MODULE_INTERNAL_ERROR;
}

AurieStatus YYTK::Zeus::FindScriptData(
	IN const YYRunnerInterface& Interface,
	IN PVOID CopyStatic,
	OUT FNScriptData* GetScriptData
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


		===== WARNING =====
		Some time after the 2024.1 release, F_CopyStatic has been changed to make the method no longer work!

		The function has changed to instead forcibly create a prototype if none exists, something like:
		void F_CopyStatic(RValue& Result, CInstance* SelfInstance, CInstance* OtherInstance, int ArgumentCount, RValue* Arguments)
		{
			int32_t script_index = YYGetInt32(Arguments, 0);
			if (script_index >= 100000)
				script_index -= 100000;

			// If no prototype exists, create one.
			YYObjectBase* prototype = g_pCurrentExec->pCCode->i_pPrototype;
			if ( !i_pPrototype )
			{
				i_pPrototype = Code_CreateStatic();

				// ...
			}

			CScript* script = ScriptData(script_index);
			// ...
		}

		This breaks the first-call technique, since that will now instead fall into Code_CreateStatic!
	*/

	// Disassemble 256 bytes at the function
	auto instructions = Memory::DmDisassembleInstructionByRange(
		CopyStatic,
		0x100
	);

	for (auto& instruction : instructions)
	{
		// Skip any non-call instructions
		if (instruction.info.mnemonic != ZYDIS_MNEMONIC_CALL)
			continue;

		// We're searching for call [visible_operand]
		// Note: this might not be necessary to check
		if (instruction.info.operand_count_visible != 1)
			continue;

		// We're jumping to an immediate, not to a register
		if (instruction.operands[0].type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
			continue;

		ZyanU64 call_address = 0;
		ZydisCalcAbsoluteAddress(
			&instruction.info,
			&instruction.operands[0],
			instruction.runtime_address,
			&call_address
		);

		// Skip the first YYGetInt32 call
		if (call_address == reinterpret_cast<ZyanU64>(Interface.YYGetInt32))
			continue;

		// We have an unknown function that's called from F_CopyStatic and isn't YYGetInt32!
		// 
		// Analyze the first few instructions. There should be no more calls from ScriptData, 
		// but there are calls from all the other functions that falsely pass the above check.

		auto target_function_instructions = Memory::DmDecodeInstructionByRange(
			reinterpret_cast<PVOID>(call_address),
			0x30,
			nullptr
		);

		// If we find a call instruction anywhere...
		if (std::find_if(
			target_function_instructions.cbegin(),
			target_function_instructions.cend(),
			[](const ZydisDecodedInstruction& Instruction)
			{
				return Instruction.mnemonic == ZYDIS_MNEMONIC_CALL;
			}
		) != std::cend(target_function_instructions))
		{
			// ... it's not our function.
			continue;
		}

		// Determine the call instruction's target
		*GetScriptData = reinterpret_cast<FNScriptData>(call_address);
		break;
	}

	return AURIE_SUCCESS;
}

AurieStatus YYTK::Zeus::FindCurrentRoomData(
	IN FNSetVariable SetVariable,
	OUT CRoom*** RunRoom
)
{
	// Disassemble 32 bytes at the function
	auto instructions = Memory::DmDisassembleInstructionByRange(
		SetVariable,
		0x20
	);

	// Find the first cmp instruction
	size_t target_cmp_index = Memory::DmFindMnemonicPattern(
		instructions,
		{
			ZYDIS_MNEMONIC_CMP // cmp Run_Room, 0
		},
		0
	);

	if (target_cmp_index == SIZE_MAX)
	{
		return AURIE_OBJECT_NOT_FOUND;
	}

	const ZydisDisassembledInstruction& compare_instruction = instructions.at(target_cmp_index);

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

	*RunRoom = reinterpret_cast<CRoom**>(run_room_address);
	return AURIE_SUCCESS;
}

AurieStatus YYTK::Zeus::FindSlotAdditionFunction(
	IN const YYRunnerInterface& Interface,
	OUT PFN_YYObjectBaseAdd* Function
)
{
	// Make sure we have the required function
	if (!Interface.StructAddRValue)
		return AURIE_UNAVAILABLE;

	// Disassemble 32 bytes at the function
	auto instructions = Memory::DmDisassembleInstructionByRange(
		Interface.StructAddRValue,
		0x20
	);

	// Find either the first jump or the first call, and trace the target
	ZyanU64 function_address = 0;

	for (const auto& instr : instructions)
	{
		if (instr.info.mnemonic != ZYDIS_MNEMONIC_JMP && instr.info.mnemonic != ZYDIS_MNEMONIC_CALL)
			continue;

		ZydisCalcAbsoluteAddress(
			&instr.info,
			&instr.operands[0],
			instr.runtime_address,
			&function_address
		);
		break;
	}

	if (!function_address)
		return AURIE_OBJECT_NOT_FOUND;

	*Function = reinterpret_cast<PFN_YYObjectBaseAdd>(function_address);
	return AURIE_SUCCESS;
}

AurieStatus YYTK::Zeus::FindSlotAllocationFunction(
	IN PFN_YYObjectBaseAdd YYObjectBase_Add,
	OUT PFN_FindAllocSlot* FindAllocSlot
)
{
	// Code_Variable_FindAlloc_Slot_From_Name is usually the first call in YYObjectBase::Add.

	// Make sure we have the required function
	if (!YYObjectBase_Add)
		return AURIE_UNAVAILABLE;

	// Disassemble 48 bytes at the function
	auto instructions = Memory::DmDisassembleInstructionByRange(
		YYObjectBase_Add,
		0x30
	);

	// Find the first call, and trace the target
	ZyanU64 function_address = 0;

	for (const auto& instr : instructions)
	{
		if (instr.info.mnemonic != ZYDIS_MNEMONIC_CALL)
			continue;

		ZydisCalcAbsoluteAddress(
			&instr.info,
			&instr.operands[0],
			instr.runtime_address,
			&function_address
		);

		break;
	}

	if (!function_address)
		return AURIE_OBJECT_NOT_FOUND;

	*FindAllocSlot = reinterpret_cast<PFN_FindAllocSlot>(function_address);
	return AURIE_SUCCESS;
}

AurieStatus YYTK::Zeus::YYC::FindFunctionsArray(
	IN const YYRunnerInterface& Interface,
	OUT RFunction*** FunctionsArray
)
{
	if (!Interface.Code_Function_Find)
		return AURIE_MODULE_INTERNAL_ERROR;

	// Disassemble this function
	auto instructions = Memory::DmDisassembleInstructionByRange(
		Interface.Code_Function_Find,
		0x80
	);

	// It just so happens the first 7-byte long MOV  
	// instruction references the the_functions array.
	// It usually looks like mov <64bit register>, [the_functions]
	for (auto& instruction : instructions)
	{
		// The instruction has to be a mov
		if (instruction.info.mnemonic != ZYDIS_MNEMONIC_MOV)
			continue;

		// The instruction has to be 7 bytes in length
		if (instruction.info.length != 6)
			continue;

		// The instruction has to have 2 operands
		// The first one (operands[0]) is the register being moved into
		// The second one (operands[1]) is the address
		if (instruction.info.operand_count != 2)
			continue;

		ZydisDecodedOperand& first_operand = instruction.operands[0];
		ZydisDecodedOperand& second_operand = instruction.operands[1];

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
			&instruction.info,
			&second_operand,
			instruction.runtime_address,
			&call_address
		);

		// It's a pointer to a pointer, we dereference it once to   
		// get the actual pointer to the first element in the array
		*FunctionsArray = reinterpret_cast<RFunction**>(call_address);
		return AURIE_SUCCESS;
	}

	return AURIE_OBJECT_NOT_FOUND;
}

AurieStatus YYTK::Zeus::YYC::GetBuiltinInformation(
	OUT int32_t** BuiltinCount,
	OUT RVariableRoutine** BuiltinArray
)
{
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
	size_t pattern_match = Memory::DmSigscanGame(
		UTEXT(
			"\x3D\xF4\x01\x00\x00"	// cmp eax, 0x1F4
			"\x75\x00"				// jnz short ??
		),
		"xxxxxx?"
	);

	if (!pattern_match)
		return AURIE_OBJECT_NOT_FOUND;

	// We disassemble the function starting at the pattern
	auto instructions = Memory::DmDisassembleInstructionByRange(
		reinterpret_cast<PVOID>(pattern_match),
		0x20
	);

	// Now, scan for the first jnz instruction
	// This should be the second one (instructions[1]),
	// but I don't want to hardcode it...
	intptr_t jnz_instruction_index = -1;
	for (size_t i = 0; i < instructions.size(); i++)
	{
		const auto& instruction = instructions.at(i);

		if (instruction.info.mnemonic != ZYDIS_MNEMONIC_JNZ)
			continue;

		jnz_instruction_index = i;
	}

	ZyanU64 jnz_target = 0;

	// Follow the jnz instruction (ie. we "pass" the check for eax < 500)
	ZyanStatus zyan_status = ZydisCalcAbsoluteAddress(
		&instructions[jnz_instruction_index].info,
		&instructions[jnz_instruction_index].operands[0],
		instructions[jnz_instruction_index].runtime_address,
		&jnz_target
	);

	// Translation failed? This shouldn't happen.
	if (!ZYAN_SUCCESS(zyan_status) || !jnz_target)
		return AURIE_EXTERNAL_ERROR;

	// Now we disassemble again, but this time at the target of the jnz
	// ie. where the CPU jumps to if we pass the bounds check
	instructions = Memory::DmDisassembleInstructionByRange(
		reinterpret_cast<PVOID>(jnz_target),
		0x40
	);

	ZyanU64 array_base_address = 0;
	ZyanU64 array_numb_address = 0;

	for (const auto& instruction : instructions)
	{
		// Until we have the base address of the array, we have to check for LEA instructions
		// TODO: 2023-newer-IDA.i64 seems to use different format?
		// 48 89 83 00 FC 06 01		mov qword ptr ds:builtin_variables.f_name[rbx], rax

		// We're searching for two instructions that have the same format:
		// Instruction 1: lea register, memory
		// Instruction 2: movsxd register, memory
		// 
		// We can therefore check for this format up front, reducing code duplication
		if (instruction.info.operand_count != 2)
		{
			continue;
		}

		// Check that the operand types match
		if (instruction.operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
		{
			continue;
		}

		// Check that the operand types match (part 2)
		if (instruction.operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
		{
			continue;
		}

		// Until we find the base address of the builtin variable array,
		// we check any LEA instruction we encounter.
		if ((instruction.info.mnemonic == ZYDIS_MNEMONIC_LEA) && (array_base_address == 0))
		{
			array_base_address = instruction.operands[1].mem.disp.value;
		}

		// Until we have the address of the array "numb" (ie. the amount of elements used up)
		// we have to check for MOVSXD instructions. It's the first one we encounter after the initial jmp
		if ((instruction.info.mnemonic == ZYDIS_MNEMONIC_MOV) && (array_numb_address == 0))
		{
			array_numb_address = instruction.operands[1].mem.disp.value;
		}
	}

	if (!array_base_address || !array_numb_address)
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	*BuiltinCount = reinterpret_cast<int32_t*>(array_numb_address);
	*BuiltinArray = reinterpret_cast<RVariableRoutine*>(array_base_address);

	return AURIE_SUCCESS;
}

AurieStatus YYTK::Zeus::YYC::FindArrayOffsetFromRValue(
	IN PVOID ArrayEquals,
	OUT int64_t* OffsetFromBase
)
{
	// Disassemble F_ArrayEquals
	auto instructions = Memory::DmDisassembleInstructionByRange(
		ArrayEquals,
		0x100
	);

	// x86 runners have the two movs inside the F_ArrayEquals function.
	// This cuts out one point of failure, which we take advantage of here.
	// We look for two movs that move into different registers each, but move
	// the same offset.
	size_t two_movs_index = SIZE_MAX;
	while (instructions.size())
	{
		// Find a potential match
		two_movs_index = Memory::DmFindMnemonicPattern(
			instructions,
			{
				ZYDIS_MNEMONIC_MOV,
				ZYDIS_MNEMONIC_MOV
			},
			0
		);

		// If no matches exist, end the loop
		if (two_movs_index == SIZE_MAX)
			break;

		ZydisDisassembledInstruction& first_mov = instructions.at(two_movs_index);
		ZydisDisassembledInstruction& second_mov = instructions.at(two_movs_index + 1);

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
		std::vector<ZydisDisassembledInstruction> new_instructions(
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

	*OffsetFromBase = instructions.at(two_movs_index).operands[1].mem.disp.value;
	return AURIE_SUCCESS;
}

AurieStatus YYTK::Zeus::YYC::FindRoomData(
	IN PVOID RoomInstanceClear,
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
	auto instructions = Memory::DmDisassembleInstructionByRange(
		RoomInstanceClear,
		0x50
	);

	size_t target_call_index = Memory::DmFindMnemonicPattern(
		instructions,
		{
			ZYDIS_MNEMONIC_CALL,	// call <Room_Data>
			ZYDIS_MNEMONIC_ADD,		// add esp, 0xC
			ZYDIS_MNEMONIC_TEST		// test eax, eax
		},
		0
	);

	if (target_call_index == SIZE_MAX)
		return AURIE_OBJECT_NOT_FOUND;

	const ZydisDisassembledInstruction& call_instruction = instructions.at(target_call_index);

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

AurieStatus YYTK::Zeus::VM::FindFunctionsArray(
	IN const YYRunnerInterface& Interface,
	OUT RFunction*** FunctionsArray
)
{
	if (!Interface.Code_Function_Find)
		return AURIE_MODULE_INTERNAL_ERROR;

	// Disassemble this function
	auto instructions = Memory::DmDisassembleInstructionByRange(
		Interface.Code_Function_Find,
		0x200
	);

	// In YYC, the first 7-byte long mov references the functions array:
	// mov <64bit register>, [the_functions]
	// In VM, this technique results in finding the Extension array, 
	// since Extension_Function_GetId is inlined into Code_Function_Find...

	std::vector<RFunction**> potential_function_arrays;
	for (auto& instruction : instructions)
	{
		// The instruction has to be a mov
		if (instruction.info.mnemonic != ZYDIS_MNEMONIC_MOV)
			continue;

		// The instruction has to be 7 bytes in length
		if (instruction.info.length != 7)
			continue;

		// The instruction has to have 2 operands
		// The first one (operands[0]) is the register being moved into
		// The second one (operands[1]) is the address
		if (instruction.info.operand_count != 2)
			continue;

		ZydisDecodedOperand& first_operand = instruction.operands[0];
		ZydisDecodedOperand& second_operand = instruction.operands[1];

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
			&instruction.info,
			&second_operand,
			instruction.runtime_address,
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

AurieStatus YYTK::Zeus::VM::GetBuiltinInformation(
	OUT int32_t** BuiltinCount,
	OUT RVariableRoutine** BuiltinArray
)
{
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

	// Scan for all occurences of this pattern in memory
	// Note that in the below pattern, not having a register in the first opcode
	// is crucial! Some games, it's eax, others it's r8d or some other register.
	auto matches = Memory::DmSigscanGameEx(
		UTEXT(
			"\xF4\x01\x00\x00"  // 500 in hex, no opcode
			"\x75\x00"			// jnz <??>
		),
		"xxxxx?"
	);

	if (matches.empty())
		return AURIE_OBJECT_NOT_FOUND;

	for (const auto& match : matches)
	{
		// We disassemble the function starting at -20h offset from the pattern
		// This is because we're "in the middle" of some instruction with the pattern,
		// so we can't just start disassembling there, as we'd get garbage instructions.
		auto match_instructions = Memory::DmDisassembleInstructionByRange(
			reinterpret_cast<PVOID>(match - 0x20),
			0x40
		);

		// Now, scan for the first jnz instruction
		// This should be the second one (instructions[1])
		size_t jnz_instruction_index = Memory::DmFindMnemonicPattern(
			match_instructions,
			{
				ZYDIS_MNEMONIC_JNZ
			},
			0
		);

		// If no jnz instruction exists, skip to the next entry
		if (jnz_instruction_index == SIZE_MAX)
			continue;

		ZyanU64 jnz_target = 0;

		// Follow the jnz instruction (ie. we "pass" the check for eax < 500)
		ZyanStatus zyan_status = ZydisCalcAbsoluteAddress(
			&match_instructions[jnz_instruction_index].info,
			&match_instructions[jnz_instruction_index].operands[0],
			match_instructions[jnz_instruction_index].runtime_address,
			&jnz_target
		);

		// Translation failed? This shouldn't happen.
		if (!ZYAN_SUCCESS(zyan_status) || !jnz_target)
			continue;

		// Now we disassemble again, but this time at the target of the jnz
		// ie. where the CPU jumps to if we pass the bounds check
		match_instructions = Memory::DmDisassembleInstructionByRange(
			reinterpret_cast<PVOID>(jnz_target),
			0x40
		);

		ZyanU64 array_base_address = 0;
		ZyanU64 array_numb_address = 0;

		for (const auto& instruction : match_instructions)
		{
			// Until we have the base address of the array, we have to check for LEA instructions

			// We're searching for two instructions that have the same format:
			// Instruction 1: lea register, memory
			// Instruction 2: movsxd register, memory
			// 
			// We can therefore check for this format up front, reducing code duplication
			if (instruction.info.operand_count != 2)
			{
				continue;
			}

			// Check that the operand types match
			if (instruction.operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
			{
				continue;
			}

			// Check that the operand types match (part 2)
			if (instruction.operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
			{
				continue;
			}

			// Until we find the base address of the builtin variable array,
			// we check any LEA instruction we encounter.
			if ((instruction.info.mnemonic == ZYDIS_MNEMONIC_LEA) && (array_base_address == 0))
			{
				// Try to calculate the absolute address of the target
				// It doesn't matter if we fail here - if we do, we try the next LEA.
				ZydisCalcAbsoluteAddress(
					&instruction.info,
					&instruction.operands[1],
					instruction.runtime_address,
					&array_base_address
				);
			}

			// Until we have the address of the array "numb" (ie. the amount of elements used up)
			// we have to check for MOVSXD instructions. It's the first one we encounter after the initial jmp
			if ((instruction.info.mnemonic == ZYDIS_MNEMONIC_MOVSXD) && (array_numb_address == 0))
			{
				ZydisCalcAbsoluteAddress(
					&instruction.info,
					&instruction.operands[1],
					instruction.runtime_address,
					&array_numb_address
				);
			}
		}

		if (!array_base_address || !array_numb_address)
			continue;

		*BuiltinCount = reinterpret_cast<int32_t*>(array_numb_address);
		*BuiltinArray = reinterpret_cast<RVariableRoutine*>(array_base_address);

		return AURIE_SUCCESS;
	}

	return AURIE_OBJECT_NOT_FOUND;
}

AurieStatus YYTK::Zeus::VM::FindArrayOffsetFromRValue(
	IN PVOID ArrayEquals,
	OUT int64_t* OffsetFromBase
)
{
	// So there's one of two ways this is implemented:
	// Either it's a mov-call-test mnemonic pattern (and the function is called like normal)
	// In the other case, it's inlined into F_ArrayEquals, in which case we don't care >:(

	auto instructions = Memory::DmDisassembleInstructionByRange(
		ArrayEquals,
		0x100
	);

	// The first match should be it
	size_t pattern_index = Memory::DmFindMnemonicPattern(
		instructions,
		{
			ZYDIS_MNEMONIC_MOV,
			ZYDIS_MNEMONIC_CALL,
			ZYDIS_MNEMONIC_TEST
		},
		0
	);

	if (pattern_index != SIZE_MAX)
		return AURIE_OBJECT_NOT_FOUND;

	ZydisDisassembledInstruction& call_instruction = instructions.at(pattern_index + 1);

	assert(call_instruction.info.mnemonic == ZYDIS_MNEMONIC_CALL);

	// Get the ArrayEquals internal handler
	ZyanU64 array_equals_internal_address = 0;
	ZydisCalcAbsoluteAddress(
		&call_instruction.info,
		&call_instruction.operands[0],
		call_instruction.runtime_address,
		&array_equals_internal_address
	);

	if (!array_equals_internal_address)
		return AURIE_INVALID_PARAMETER;

	// The rest is just from the YYC handler
	instructions = Memory::DmDisassembleInstructionByRange(
		reinterpret_cast<PVOID>(array_equals_internal_address),
		0x100
	);

	size_t two_movs_index = SIZE_MAX;
	while (instructions.size())
	{
		// Find a potential match
		two_movs_index = Memory::DmFindMnemonicPattern(
			instructions,
			{
				ZYDIS_MNEMONIC_MOV,
				ZYDIS_MNEMONIC_MOV
			},
			0
		);

		// If no matches exist, end the loop
		if (two_movs_index == SIZE_MAX)
			break;

		ZydisDisassembledInstruction& first_mov = instructions.at(two_movs_index);
		ZydisDisassembledInstruction& second_mov = instructions.at(two_movs_index + 1);

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
		std::vector<ZydisDisassembledInstruction> new_instructions(
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

	*OffsetFromBase = instructions.at(two_movs_index).operands[1].mem.disp.value;

	return AURIE_SUCCESS;
}

AurieStatus YYTK::Zeus::VM::FindRoomData(
	IN PVOID RoomInstanceClear,
	OUT FNRoomData* RoomData
)
{
	static CRoom*** s_RoomEntry = nullptr;
	static auto s_GetRoomEntry = [](IN int Index) -> CRoom*
		{
			return (*s_RoomEntry)[Index];
		};

	// In F_RoomInstanceClear (VM), the Room_Data function is inlined.
	// At the beginning of the function, the Room pointer is computed:
	// mov rax, cs:g_RoomArray.Rooms ; move a pointer to the first element in the room array into rax
	// mov rbx, [rax+rbx*8]		     ; compute the offset of the room at index rbx (sizeof(CRoom*) == 8)
	// test rbx, rbx				 ; check the room is not null
	// jz <??>						 ; jump if null

	// Disassemble the function
	auto instructions = Memory::DmDisassembleInstructionByRange(
		RoomInstanceClear,
		0x100
	);

	// The first match should be it
	size_t pattern_index = Memory::DmFindMnemonicPattern(
		instructions,
		{
			ZYDIS_MNEMONIC_MOV,
			ZYDIS_MNEMONIC_MOV,
			ZYDIS_MNEMONIC_TEST,
			ZYDIS_MNEMONIC_JZ
		},
		0
	);

	// If we didn't get a match, something is wrong
	if (pattern_index == SIZE_MAX)
		return AURIE_OBJECT_NOT_FOUND;

	assert(instructions[pattern_index].info.mnemonic == ZYDIS_MNEMONIC_MOV);

	ZydisDisassembledInstruction& mov_instruction = instructions[pattern_index];

	// Make sure the mov has two operands
	if (mov_instruction.info.operand_count != 2)
		return AURIE_INVALID_SIGNATURE;

	// We're supposed to be moving to a register
	if (mov_instruction.operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
		return AURIE_INVALID_SIGNATURE;

	// We're supposed to be moving from memory
	if (mov_instruction.operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
		return AURIE_INVALID_SIGNATURE;

	// Reject stupidity like mov reg, [reg] - we have to have displacement
	if (!mov_instruction.operands[1].mem.disp.has_displacement)
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "No displacement in room data! Yell at Archie to stop being lazy.");
		return AURIE_INVALID_SIGNATURE;
	}

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

	s_RoomEntry = reinterpret_cast<CRoom***>(array_address);
	*RoomData = s_GetRoomEntry;
	return AURIE_SUCCESS;
}

Aurie::AurieStatus YYTK::Zeus::FindErrorSuppressionVariable(
	IN PVOID IsNaN,
	OUT bool** SuppressionVariable
)
{
	// Disassemble the function
	auto instructions = Memory::DmDisassembleInstructionByRange(
		IsNaN,
		0x100
	);

	// Loop all instructions.
	// We're looking for the first mov [mem32], 1 (imm)
	for (const auto& instruction : instructions)
	{
		if (instruction.info.mnemonic != ZYDIS_MNEMONIC_MOV)
			continue;

		// We have to be moving to a memory location
		if (instruction.operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY)
			continue;

		// We have to have some displacement (offset)
		if (!instruction.operands[0].mem.disp.has_displacement)
			continue;

		// The second operand is an immediate value
		if (instruction.operands[1].type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
			continue;

		// And the value is 1
		if (instruction.operands[1].imm.value.s != 1)
			continue;

		ZyanU64 suppression_variable_address = 0;
		ZydisCalcAbsoluteAddress(
			&instruction.info,
			&instruction.operands[0],
			instruction.runtime_address,
			&suppression_variable_address
		);

		if (!suppression_variable_address)
			return AURIE_OBJECT_NOT_FOUND;

		*SuppressionVariable = reinterpret_cast<bool*>(suppression_variable_address);
		return AURIE_SUCCESS;
	}

	return AURIE_OBJECT_NOT_FOUND;
}

void YYTK::Zeus::BuildApproximateSymbolTable(
	OUT std::vector<std::pair<uintptr_t, std::string>>& SymbolTable
)
{
	AurieStatus last_status = AURIE_SUCCESS;
	SymbolTable.clear();
	SymbolTable.reserve(1024);

	// Push all scripts...
	int script_index = 0;
	while (AurieSuccess(last_status))
	{
		CScript* current_script = nullptr;
		last_status = g_ModuleInterface.GetScriptData(script_index++, current_script);

		if (!current_script)
			continue;

		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(current_script->m_Functions->m_ScriptFunction), current_script->m_Name });
	}

	// Push all built-in function entries...
	last_status = AURIE_SUCCESS;
	int builtin_func_index = 0;
	while (AurieSuccess(last_status))
	{
		std::string function_name;
		TRoutine function_ptr = nullptr;
		int32_t argument_count = 0;

		g_ModuleInterface.YkExtractFunctionEntry(
			builtin_func_index++,
			function_name,
			function_ptr,
			argument_count
		);

		if (!function_ptr)
			break;

		// If we go out-of-bounds, this will return an error status, since the function name won't exist.
		int sanity_check_index = 0;
		last_status = g_ModuleInterface.GetNamedRoutineIndex(
			function_name.c_str(),
			&sanity_check_index
		);

		// Insert if we got a valid function
		if (AurieSuccess(last_status))
			SymbolTable.push_back({ reinterpret_cast<uintptr_t>(function_ptr), function_name });
	}

	// Push all GV and SV functions
	last_status = AURIE_SUCCESS;
	size_t builtin_var_index = 0;
	while (AurieSuccess(last_status))
	{
		RVariableRoutine* variable_information = nullptr;
		last_status = g_ModuleInterface.GetBuiltinVariableInformation(
			builtin_var_index++,
			variable_information
		);

		if (!variable_information)
			continue;

		if (variable_information->m_GetVariable)
		{
			std::string name = "GV_";
			name.append(variable_information->m_Name);

			SymbolTable.push_back({ reinterpret_cast<uintptr_t>(variable_information->m_GetVariable), name });
		}
		if (variable_information->m_SetVariable)
		{
			std::string name = "SV_";
			name.append(variable_information->m_Name);

			SymbolTable.push_back({ reinterpret_cast<uintptr_t>(variable_information->m_SetVariable), name });
		}
	}

	// Push runner interface functions...
	const YYRunnerInterface& runner_interface = g_ModuleInterface.GetRunnerInterface();
	{
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYGetString), "YYGetString" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DebugConsoleOutput), "DebugConsoleOutput" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.ReleaseConsoleOutput), "ReleaseConsoleOutput" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.ShowMessage), "ShowMessage" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYError), "YYError" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYAlloc), "YYAlloc" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYRealloc), "YYRealloc" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYFree), "YYFree" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYStrDup), "YYStrDup" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYGetBool), "YYGetBool" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYGetFloat), "YYGetFloat" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYGetReal), "YYGetReal" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYGetInt32), "YYGetInt32" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYGetUint32), "YYGetUint32" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYGetInt64), "YYGetInt64" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYGetPtr), "YYGetPtr" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYGetPtrOrInt), "YYGetPtrOrInt" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYGetString), "YYGetString" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.BOOL_RValue), "BOOL_RValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.REAL_RValue), "REAL_RValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.PTR_RValue), "PTR_RValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.INT64_RValue), "INT64_RValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.INT32_RValue), "INT32_RValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.HASH_RValue), "HASH_RValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.SET_RValue), "SET_RValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.GET_RValue), "GET_RValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.COPY_RValue), "COPY_RValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.KIND_RValue), "KIND_RValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.FREE_RValue), "FREE_RValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYCreateString), "YYCreateString" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYCreateArray), "YYCreateArray" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.Script_Find_Id), "Script_Find_Id" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.Script_Perform), "Script_Perform" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.Code_Function_Find), "Code_Function_Find" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.HTTP_Get), "HTTP_Get" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.HTTP_Post), "HTTP_Post" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.HTTP_Request), "HTTP_Request" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.ASYNCFunc_SpriteAdd), "ASYNCFunc_SpriteAdd" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.ASYNCFunc_SpriteCleanup), "ASYNCFunc_SpriteCleanup" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.CreateSpriteAsync), "CreateSpriteAsync" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.Timing_Time), "Timing_Time" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.Timing_Sleep), "Timing_Sleep" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYMutexCreate), "YYMutexCreate" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYMutexDestroy), "YYMutexDestroy" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYMutexLock), "YYMutexLock" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYMutexUnlock), "YYMutexUnlock" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.CreateAsyncEventWithDSMap), "CreateAsyncEventWithDSMap" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.CreateAsyncEventWithDSMapAndBuffer), "CreateAsyncEventWithDSMapAndBuffer" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.CreateDsMap), "CreateDsMap" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DsMapAddDouble), "DsMapAddDouble" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DsMapAddString), "DsMapAddString" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DsMapAddInt64), "DsMapAddInt64" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.BufferGetContent), "BufferGetContent" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.BufferWriteContent), "BufferWriteContent" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.CreateBuffer), "CreateBuffer" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DsListCreate), "DsListCreate" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DsMapAddList), "DsMapAddList" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DsListAddMap), "DsListAddMap" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DsMapClear), "DsMapClear" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DsListClear), "DsListClear" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DsListClear), "DsListClear" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.BundleFileExists), "BundleFileExists" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.BundleFileName), "BundleFileName" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.SaveFileExists), "SaveFileExists" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.SaveFileName), "SaveFileName" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.Base64Encode), "Base64Encode" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DsListAddInt64), "DsListAddInt64" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.AddDirectoryToBundleWhitelist), "AddDirectoryToBundleWhitelist" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.AddFileToBundleWhitelist), "AddFileToBundleWhitelist" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.AddDirectoryToSaveWhitelist), "AddDirectoryToSaveWhitelist" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.AddFileToSaveWhitelist), "AddFileToSaveWhitelist" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.KIND_NAME_RValue), "KIND_NAME_RValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DsMapAddBool), "DsMapAddBool" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DsMapAddRValue), "DsMapAddRValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DestroyDsMap), "DestroyDsMap" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.StructCreate), "StructCreate" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.StructAddBool), "StructAddBool" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.StructAddDouble), "StructAddDouble" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.StructAddInt), "StructAddInt" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.StructAddRValue), "StructAddRValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.StructAddString), "StructAddString" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.WhitelistIsDirectoryIn), "WhitelistIsDirectoryIn" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.WhiteListIsFilenameIn), "WhiteListIsFilenameIn" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.WhiteListAddTo), "WhiteListAddTo" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.DirExists), "DirExists" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.BufferGetFromGML), "BufferGetFromGML" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.BufferTELL), "BufferTELL" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.BufferGet), "BufferGet" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.FilePrePend), "FilePrePend" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.StructAddInt32), "StructAddInt32" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.StructAddInt64), "StructAddInt64" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.StructGetMember), "StructGetMember" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.StructGetKeys), "StructGetKeys" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.YYGetStruct), "YYGetStruct" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.extOptGetRValue), "extOptGetRValue" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.extOptGetString), "extOptGetString" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.extOptGetReal), "extOptGetReal" });
		SymbolTable.push_back({ reinterpret_cast<uintptr_t>(runner_interface.isRunningFromIDE), "isRunningFromIDE" });
	}

	// Sort the vector for std::upper_bound lookups later.
	std::sort(
		SymbolTable.begin(),
		SymbolTable.end(),
		[](auto& a, auto& b)
		{ return a.first < b.first; }
	);
}

std::string YYTK::Zeus::GuessSymbolFromGameInstructionAddress(
	IN LPCVOID InstructionPointer
)
{
	// Try to get the NT header
	const uintptr_t address = reinterpret_cast<uintptr_t>(InstructionPointer);

	uint64_t text_section_start = 0;
	uint64_t text_section_end = 0;
	AurieStatus last_status = AURIE_SUCCESS;

	last_status = Memory::DmGetSectionBounds(".text", &text_section_start, &text_section_end);

	if (!AurieSuccess(last_status))
		return "";

	if (address < text_section_start)
		return "";

	if (address > text_section_end)
		return "";

	std::wstring game_name_wstring;
	MdGetImageFilename(g_ArInitialImage, game_name_wstring);

	auto it = std::upper_bound(
		g_ModuleInterface.m_KnownGameSymbols.begin(),
		g_ModuleInterface.m_KnownGameSymbols.end(),
		address,
		[](uintptr_t Address, const auto& Pair) {
			return Address < Pair.first;
		}
	);

	// If the iterator points to the start of the vector (we're below the first entry)
	if (it == g_ModuleInterface.m_KnownGameSymbols.begin())
		return "";

	const std::string game_name_utf8(game_name_wstring.begin(), game_name_wstring.end());
	const auto symbol = --it;

	return std::format("{}!{}+0x{:X}", game_name_utf8, symbol->second, address - symbol->first);
}

#endif