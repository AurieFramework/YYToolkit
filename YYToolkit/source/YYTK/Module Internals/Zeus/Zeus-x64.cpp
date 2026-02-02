// This file is only used in x64 compilations. For x86 compilations, see Zeus-x86.cpp
#if _WIN64
#include "../Module Internals.hpp"
using namespace Aurie;

#ifndef PAGE_SIZE
#define PAGE_SIZE (0x1000)
#endif // PAGE_SIZE

AurieStatus YYTK::Zeus::FindRunnerInterfaceHookpoint(
	OUT PVOID* TargetInstruction
)
{
	// The core of YYToolkit.
	// The runner interface is a struct passed to extensions by the engine. 
	// 
	// The struct contains function pointers to engine functions that are meant to help extension-makers
	// make their extensions work across a large variety of engine versions without worrying about struct internals.
	//
	// This enables YYTK to act as part of the engine without relying on individual patterns for each function.
	//
	// The modern approach to finding this interface (from GM version 2023.8 up) is to scan for a mov-lea chain,
	// which is emitted as part of the Extension_PrePrepare function. This chain is responsible for populating
	// a stack-allocated YYRunnerInterface struct, which is then passed as a paramater to an extension.
	//
	// While we can't easily pin-point the start with a pattern, we can pinpoint the end.
	// In all known games, the chain is terminated by a call instruction (calling the extension function with the built interface).

	AurieStatus last_status = AURIE_SUCCESS;
	uint64_t text_start = 0, text_end = 0;

	// Get info about the game's .text section.
	last_status = Memory::DmGetSectionBounds(
		".text",
		&text_start,
		&text_end
	);

	// Don't fail getting the .text section.
	if (!AurieSuccess(last_status))
		return last_status;

	// I assume the .text section is page-aligned.
	_ASSERT((text_start & 0xFFF) == 0);

	// For all pages in the text section:
	for (size_t current_page = text_start; current_page < text_end; current_page += PAGE_SIZE)
	{
		// Decode instructions in the current page.
		ZyanU64 last_instruction = 0;
		auto decoded_instructions = Memory::DmDecodeInstructionByRange(
			reinterpret_cast<PVOID>(current_page),
			PAGE_SIZE,
			&last_instruction
		);

		// Look for these two patterns. One ends in a mov, one in a lea.
		// The LEA pattern is used for new games (DR Ch4, Fields of Mistria, etc.)
		auto lea_pattern_index = Memory::DmFindMnemonicPattern(
			decoded_instructions,
			{
				ZYDIS_MNEMONIC_MOV,
				ZYDIS_MNEMONIC_LEA,
				ZYDIS_MNEMONIC_MOV,
				ZYDIS_MNEMONIC_LEA,
				ZYDIS_MNEMONIC_MOV,
				ZYDIS_MNEMONIC_LEA,
				ZYDIS_MNEMONIC_MOV,
				ZYDIS_MNEMONIC_LEA,
				ZYDIS_MNEMONIC_CALL
			},
			0
		);

		// The MOV pattern is used in "A Little Bit S'more" - v4 seems to have it covered so it's only right for v5 to cover it too.
		auto mov_pattern_index = Memory::DmFindMnemonicPattern(
			decoded_instructions,
			{
				ZYDIS_MNEMONIC_LEA,
				ZYDIS_MNEMONIC_MOV,
				ZYDIS_MNEMONIC_LEA,
				ZYDIS_MNEMONIC_MOV,
				ZYDIS_MNEMONIC_LEA,
				ZYDIS_MNEMONIC_MOV,
				ZYDIS_MNEMONIC_LEA,
				ZYDIS_MNEMONIC_MOV,
				ZYDIS_MNEMONIC_CALL
			},
			0
		);

		// Continue looping if neither is found.
		if (lea_pattern_index == SIZE_MAX && mov_pattern_index == SIZE_MAX)
			continue;

		// If lea_pattern_index is found, use that. Else use mov_pattern_index.
		auto pattern_index = (lea_pattern_index != SIZE_MAX) ? lea_pattern_index : mov_pattern_index;

		// Offset from the base of the patterns:
		auto last_lea_index = pattern_index + 7;

		// Get the instruction address of the lea, since DmDecodeInstruction doesn't output it directly
		auto last_lea_address = Memory::DmCalculateInstructionAddress(decoded_instructions, static_cast<int64_t>(last_lea_index), last_instruction);

		// Fully disassemble two instructions starting at the LEA - that is the LEA itself, and the CALL after it.
		auto lea_call_pair = Memory::DmDisassembleInstructionByCount(
			reinterpret_cast<PVOID>(last_lea_address),
			2
		);

		// This check seems to be enough?
		if (lea_call_pair.back().operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY)
			continue;

		DbgPrintEx(LOG_SEVERITY_TRACE, "Using %s pattern for RI ending", (lea_pattern_index != SIZE_MAX) ? "LEA" : "MOV");
		DbgPrintEx(LOG_SEVERITY_DEBUG, "Found lea_call_pair at 0x%I64X", lea_call_pair.back().runtime_address);

		// Disassemble a page worth of instructions before our lea-call pair.
		auto pre_lea_call_instructions = Memory::DmDisassembleInstructionByRange(
			reinterpret_cast<PVOID>(last_lea_address - PAGE_SIZE),
			PAGE_SIZE
		);

		// Find the last sub rsp instruction.
		const auto& last_sub_rsp = std::find_if(
			pre_lea_call_instructions.crbegin(),
			pre_lea_call_instructions.crend(),
			[](const ZydisDisassembledInstruction& Value) -> bool
			{
				if (Value.info.mnemonic != ZYDIS_MNEMONIC_SUB)
					return false;

				if (Value.operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
					return false;

				if (Value.operands[0].reg.value != ZYDIS_REGISTER_RSP)
					return false;

				return true;
			}
		);

		// If none found:
		if (last_sub_rsp == pre_lea_call_instructions.crend())
		{
			DbgPrintEx(LOG_SEVERITY_ERROR, "No last_sub_rsp found matching 0x%I64X", lea_call_pair.back().runtime_address);
			return AURIE_MODULE_INTERNAL_ERROR;
		}

		// Go to the instruction just after the sub rsp one (it's a reverse iterator, so -1 brings you forward)
		const auto& instruction_just_after = *(last_sub_rsp - 1);
		
		// And we breakpoint that one
		*TargetInstruction = reinterpret_cast<PVOID>(instruction_just_after.runtime_address);

		// We're now almost done. While the following code might be better in some standalone function,
		// I keep it here, as we've already gathered all the relevant information from the earlier operations.
		//
		// We want to find the start of the MOV-LEA chain, such that the handler of the runner interface creation has an easier time reconstructing it.
		// To do this, we simply scan up the instructions, until we hit another call. While the runner interface construction
		// can be broken up by the occasional instruction, but that just happens to never be a call.
		for (size_t i = pre_lea_call_instructions.size() - 1; i-- > 0;)
		{
			if (pre_lea_call_instructions[i].info.mnemonic == ZYDIS_MNEMONIC_CALL)
			{
				// In the hook (HandleRunnerInterfaceCreation), we start disassembly at the call before the mov-lea chain.
				g_ModuleInterface.m_RunnerInterfaceSetupStart = pre_lea_call_instructions[i].runtime_address;
				DbgPrintEx(LOG_SEVERITY_TRACE, "Assumed RI base: 0x%I64X | %s", pre_lea_call_instructions[i].runtime_address, pre_lea_call_instructions[i].text);
				break;
			}
		}

		// And we end disassembly at the instruction that calls the extension.
		g_ModuleInterface.m_RunnerInterfaceSetupEnd = lea_call_pair.back().runtime_address;

		// Print information about the hookpoint and return successfully.
		DbgPrintEx(LOG_SEVERITY_TRACE, "RI hookpoint: 0x%I64X | %s", instruction_just_after.runtime_address, instruction_just_after.text);
		return AURIE_SUCCESS;
	}
	
	return AURIE_INVALID_SIGNATURE;
}

void YYTK::Zeus::HandleRunnerInterfaceCreation(
	IN ProcessorContext& ProcessorContext
)
{
	DbgPrintEx(LOG_SEVERITY_TRACE, "    rax=%016llx rbx=%016llx rcx=%016llx", ProcessorContext.RAX, ProcessorContext.RBX, ProcessorContext.RCX);
	DbgPrintEx(LOG_SEVERITY_TRACE, "    rdx=%016llx rsi=%016llx rdi=%016llx", ProcessorContext.RDX, ProcessorContext.RSI, ProcessorContext.RDI);
	DbgPrintEx(LOG_SEVERITY_TRACE, "    rip=%016llx rsp=%016llx rbp=%016llx", ProcessorContext.RIP, ProcessorContext.RSP, ProcessorContext.RBP);
	DbgPrintEx(LOG_SEVERITY_TRACE, "    r8=%016llx r9=%016llx r10=%016llx", ProcessorContext.R8, ProcessorContext.R9, ProcessorContext.R10);
	DbgPrintEx(LOG_SEVERITY_TRACE, "    r11=%016llx r12=%016llx r13=%016llx", ProcessorContext.R11, ProcessorContext.R12, ProcessorContext.R13);
	DbgPrintEx(LOG_SEVERITY_TRACE, "    r14=%016llx r15=%016llx tsp=%016llx", ProcessorContext.R14, ProcessorContext.R15, ProcessorContext.TrampolineRSP);

	int64_t stack_frame_size = ProcessorContext.RSP - ProcessorContext.RBP;

	auto disassembly_start = g_ModuleInterface.m_RunnerInterfaceSetupStart;
	auto disassembly_end = g_ModuleInterface.m_RunnerInterfaceSetupEnd;

	auto disassembled_instructions = Memory::DmDisassembleInstructionByRange(
		reinterpret_cast<PVOID>(disassembly_start),
		disassembly_end - disassembly_start
	);

	char* allocated_page = static_cast<char*>(
		MmAllocateMemory(g_ArSelfModule, PAGE_SIZE)
	);

	if (!allocated_page)
	{
		DbgPrintEx(LOG_SEVERITY_CRITICAL, "Failed to allocate fake stack memory!");
		return;
	}

	memset(allocated_page, 0, PAGE_SIZE);

	// Place RBP right in the middle of our allocated page
	char* my_rbp = allocated_page + 0x800;
	char* my_rsp = my_rbp + stack_frame_size;

	// Simulated registers. Key is register name, value is the register contents.
	std::map<std::string, uint64_t> simulated_registers;

	// The lowest address accessed by our simulated code.
	uint64_t* lowest_accessed_address = reinterpret_cast<uint64_t*>(UINT64_MAX);

	for (auto& instruction : disassembled_instructions)
	{
		// lea reg, [mem64]
		if (instruction.info.mnemonic == ZYDIS_MNEMONIC_LEA)
		{
			if (instruction.operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
				continue;

			if (instruction.operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
				continue;

			// Calculate the address referenced by the memory operand.
			ZyanU64 referenced_memory = 0;
			if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
				&instruction.info,
				&instruction.operands[1],
				instruction.runtime_address,
				&referenced_memory
			)))
			{
				DbgPrintEx(LOG_SEVERITY_WARNING, "Unknown LEA address at %I64x (%s)", instruction.runtime_address, instruction.text);
				continue;
			}

			// Get the name of the register needed 
			const char* register_name = ZydisRegisterGetString(instruction.operands[0].reg.value);

			// Save the value for this register
			simulated_registers[register_name] = referenced_memory;
		}

		// mov [mem64], register
		// We expect the "target" operand to be rsp-relative or rbp-relative.
		if (instruction.info.mnemonic == ZYDIS_MNEMONIC_MOV)
		{
			if (instruction.operands[0].type != ZYDIS_OPERAND_TYPE_MEMORY)
				continue;

			if (instruction.operands[1].type != ZYDIS_OPERAND_TYPE_REGISTER)
				continue;

			if (instruction.operands[0].mem.base != ZYDIS_REGISTER_RSP && instruction.operands[0].mem.base != ZYDIS_REGISTER_RBP)
				continue;

			const ZyanI64 target_disp = instruction.operands[0].mem.disp.value;
			const ZydisRegister source_register = instruction.operands[1].reg.value;
			const char* source_register_name = ZydisRegisterGetString(source_register);

			uint64_t* accessed_address = nullptr;

			switch (instruction.operands[0].mem.base)
			{
			case ZYDIS_REGISTER_RSP:
				accessed_address = reinterpret_cast<uint64_t*>(my_rsp + target_disp);
				break;
			case ZYDIS_REGISTER_RBP:
				accessed_address = reinterpret_cast<uint64_t*>(my_rbp + target_disp);
				break;
			}

			*accessed_address = simulated_registers[source_register_name];

			// Take note of the lowest accessed address. This is likely the base address of the runner interface.
			if (accessed_address < lowest_accessed_address)
				lowest_accessed_address = accessed_address;
		}
	}

	// Copy out the runner interface.
	g_ModuleInterface.m_RunnerInterface = *(YYRunnerInterface*)(lowest_accessed_address);

	// Free the fake stack allocation.
	MmFreeMemory(g_ArSelfModule, allocated_page);

	// Signal the event.
	SetEvent(g_ModuleInterface.m_RunnerInterfacePopulatedEvent);

	g_ModuleInterface.YkSetupLateInitialization();
}

AurieStatus YYTK::Zeus::RegisterRunnerInterfaceHook(
	IN PVOID Hookpoint,
	IN AurieMidHookFunction TargetFunction
)
{
	return MmCreateMidfunctionHook(
		g_ArSelfModule,
		"RunnerInterface",
		Hookpoint,
		TargetFunction
	);
}

AurieStatus YYTK::Zeus::FindCodeExecutionHookpoint(
	OUT PVOID* TargetInstruction
)
{
	// Look for the call to ExecuteIt, present inside the Code_Execute function.
	// This pattern dates back to YYTK v2 days.
	size_t code_execute_match = Memory::DmSigscanGame(
		UTEXT(
			"\xE8\x00\x00\x00\x00"	// call <ExecuteIt>
			"\x0F\xB6\xD8"			// movzx ebx, al
			"\x3C\x01"				// cmp al, 1
		),
		"x????xxxxx"
	);

	// If there's no match, it's possible that we have a 2024.14 VM runner
	if (!code_execute_match)
	{
		DbgPrintEx(LOG_SEVERITY_TRACE, "code_execute_match is null, maybe a 2024.14 VM runner?");
		code_execute_match = Memory::DmSigscanGame(
			UTEXT(
				"\xE8\x00\x00\x00\x00"	// call <ExecuteIt>
				"\x3C\x01"				// cmp al, 1
				"\x74\x00"				// jz ??
			),
			"x????xxx?"
		);

		// If no matches still, we fail.
		if (!code_execute_match)
			return AURIE_OBJECT_NOT_FOUND;
	}

	auto disassembled_instruction = Memory::DmDisassembleInstruction(
		reinterpret_cast<PVOID>(code_execute_match)
	);

	DbgPrintEx(LOG_SEVERITY_TRACE, "Zeus::FindCodeExecutionHookpoint => %s", disassembled_instruction.text);

	// Dereference to get the address of ExecuteIt.
	ZyanU64 execute_it_address = 0;
	if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
		&disassembled_instruction.info,
		&disassembled_instruction.operands[0],
		disassembled_instruction.runtime_address,
		&execute_it_address
	)))
	{
		return AURIE_MODULE_INITIALIZATION_FAILED;
	}

	// Write it to the buffer.
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

Aurie::AurieStatus YYTK::Zeus::FindScriptData(
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

Aurie::AurieStatus YYTK::Zeus::FindCurrentRoomData(
	IN FNSetVariable SetVariable, 
	OUT CRoom*** RunRoom
)
{
	// Disassemble 80 bytes at the function
	auto instructions = Memory::DmDisassembleInstructionByRange(
		SetVariable,
		0x50
	);

	// The first mov in this pattern is a mov reg, [memory]
	size_t target_mov_index = Memory::DmFindMnemonicPattern(
		instructions,
		{
			ZYDIS_MNEMONIC_MOV,
			ZYDIS_MNEMONIC_TEST,
			ZYDIS_MNEMONIC_JZ
		},
		0
	);
	
	if (target_mov_index == SIZE_MAX)
	{
		return AURIE_OBJECT_NOT_FOUND;
	}

	const ZydisDisassembledInstruction& move_instruction = instructions.at(target_mov_index);

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

	// Disassemble 96 bytes at the function
	auto instructions = Memory::DmDisassembleInstructionByRange(
		YYObjectBase_Add,
		0x60
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

// These functions are only present on x64. Having them on x86 would just clutter up the headers.
static AurieStatus FindCurrentFunction(
	IN const YYTK::YYRunnerInterface& Interface,
	OUT YYTK::RFunction*** g_pFunction
)
{
	using namespace YYTK;

	// Disassemble YYGetPtr.
	auto instructions = Memory::DmDisassembleInstructionByRange(
		Interface.YYGetPtr,
		0x50
	);

	// It just so happens the first mov that has a 
	// memory operand references the the_functions array.
	// It usually looks like mov <64bit register>, [the_functions]
	for (auto& instruction : instructions)
	{
		// The instruction has to be a mov
		if (instruction.info.mnemonic != ZYDIS_MNEMONIC_MOV)
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
		*g_pFunction = reinterpret_cast<RFunction**>(call_address);
		return AURIE_SUCCESS;
	}

	return AURIE_OBJECT_NOT_FOUND;
}

static AurieStatus FindFunctionsArrayWithScriptPerform(
	IN const YYTK::YYRunnerInterface& Interface,
	IN YYTK::RFunction** g_pFunctions,
	OUT YYTK::RFunction*** FunctionsArray
)
{
	using namespace YYTK;

	// Disassemble instructions at Script_Perform
	auto instructions = Memory::DmDisassembleInstructionByRange(
		Interface.Script_Perform,
		0x100
	);

	// Get the first MOV instruction that references g_pFunctions.
	auto current_function_iterator = std::find_if(
		instructions.begin(),
		instructions.end(),
		[g_pFunctions](IN const ZydisDisassembledInstruction& Instruction) -> bool
		{
			// Looking for a mov.
			if (Instruction.info.mnemonic != ZYDIS_MNEMONIC_MOV)
				return false;

			const ZydisDecodedOperand& first_operand = Instruction.operands[0];
			const ZydisDecodedOperand& second_operand = Instruction.operands[1];

			// Our mov should be moving to a register from a memory location.
			if (first_operand.type != ZYDIS_OPERAND_TYPE_REGISTER)
				return false;

			if (second_operand.type != ZYDIS_OPERAND_TYPE_MEMORY)
				return false;

			// Calculate the absolute address
			ZyanU64 call_address = 0;
			ZydisCalcAbsoluteAddress(
				&Instruction.info,
				&second_operand,
				Instruction.runtime_address,
				&call_address
			);

			// Return true if it's referencing g_pFunctions.
			return call_address == reinterpret_cast<ZyanU64>(g_pFunctions);
		}
	);

	if (current_function_iterator == instructions.end())
	{
		DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to find g_pFunction reference in Script_Perform!");
		return AURIE_OBJECT_NOT_FOUND;
	}

	// Get the index that current_function_iterator belongs to.
	ptrdiff_t current_function_index = std::distance(instructions.begin(), current_function_iterator);
	
	// Go back in the instructions list from the g_pFunctions-referencing instruction until
	// we find another MOV instruction referencing a memory address.
	//
	// This will be our the_functions reference.
	for (auto i = (current_function_index - 1); i > 0; i--)
	{
		// Looking for a mov.
		if (instructions[i].info.mnemonic != ZYDIS_MNEMONIC_MOV)
			continue;

		const ZydisDecodedOperand& first_operand = instructions[i].operands[0];
		const ZydisDecodedOperand& second_operand = instructions[i].operands[1];

		// Our mov should be moving to a register from a memory location.
		if (first_operand.type != ZYDIS_OPERAND_TYPE_REGISTER)
			continue;

		if (second_operand.type != ZYDIS_OPERAND_TYPE_MEMORY)
			continue;

		// Calculate the absolute address
		ZyanU64 call_address = 0;
		ZydisCalcAbsoluteAddress(
			&instructions[i].info,
			&second_operand,
			instructions[i].runtime_address,
			&call_address
		);

		if (call_address)
		{
			DbgPrintEx(LOG_SEVERITY_TRACE, "Found the_functions reference in Script_Perform (%s)", instructions[i].text);

			*FunctionsArray = reinterpret_cast<RFunction**>(call_address);
			return AURIE_SUCCESS;
		}
	}

	DbgPrintEx(LOG_SEVERITY_ERROR, "Failed to find the_functions reference in Script_Perform!");
	return AURIE_OBJECT_NOT_FOUND;
}

AurieStatus YYTK::Zeus::YYC::FindFunctionsArray(
	IN const YYRunnerInterface& Interface,
	OUT RFunction*** FunctionsArray
)
{
	if (!Interface.Code_Function_Find)
		return AURIE_MODULE_INTERNAL_ERROR;

	// If we have Script_Perform (and YYGetPtr, but we always seem to have that), 
	// chances are we're on a newer runner. We can use the v5-exclusive method of finding g_pFunction,
	// and then scanning Script_Perform for references to the true functions array.
	// 
	// This bypasses the 2024.14 changes that make referencing the functions array from Code_Function_Find impossible.
	if (Interface.Script_Perform && Interface.YYGetPtr)
	{
		// Get the g_pFunction pointer. It will contain nullptr at this point, but we don't care about it's actual value.
		RFunction** g_pFunction = nullptr;
		AurieStatus last_status = FindCurrentFunction(
			Interface,
			&g_pFunction
		);

		// Make sure we got it.
		if (!AurieSuccess(last_status))
			return last_status;

		last_status = FindFunctionsArrayWithScriptPerform(
			Interface,
			g_pFunction,
			FunctionsArray
		);

		return last_status;
	}

	// If the required functions are unavailable, we resort to the old method of scanning Code_Function_Find, and
	// either have it work or crash the runner.
	
	// Disassemble the Code_Function_Find function.
	auto instructions = Memory::DmDisassembleInstructionByRange(
		Interface.Code_Function_Find,
		0x200
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
		*FunctionsArray = reinterpret_cast<RFunction**>(call_address);
		return AURIE_SUCCESS;
	}

	return AURIE_OBJECT_NOT_FOUND;
}

Aurie::AurieStatus YYTK::Zeus::YYC::GetBuiltinInformation(
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
		break;
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
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	*BuiltinCount = reinterpret_cast<int32_t*>(array_numb_address);
	*BuiltinArray = reinterpret_cast<RVariableRoutine*>(array_base_address);

	return AURIE_SUCCESS;
}

Aurie::AurieStatus YYTK::Zeus::YYC::FindArrayOffsetFromRValue(
	IN PVOID ArrayEquals,
	OUT int64_t* OffsetFromBase
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
	auto instructions = Memory::DmDisassembleInstructionByRange(
		ArrayEquals,
		0x100
	);

	AurieStatus last_status = AURIE_SUCCESS;

	// Now look for this pattern.
	// The call instruction target is the ArrayEquals function.
	size_t start_index = Memory::DmFindMnemonicPattern(
		instructions,
		{
			ZYDIS_MNEMONIC_MOV,
			ZYDIS_MNEMONIC_MOV,
			ZYDIS_MNEMONIC_CALL
		},
		0
	);

	// Make sure we found that
	if (!AurieSuccess(last_status))
		return last_status;

	// We know the index of the call instruction is two away from the first mov
	size_t call_index = start_index + 2;

	const ZydisDisassembledInstruction& call_instruction = instructions.at(call_index);

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
		return AURIE_INVALID_PARAMETER;

	instructions = Memory::DmDisassembleInstructionByRange(
		reinterpret_cast<PVOID>(array_equals_internal_address),
		0x100
	);

	// Map to store counts of displacements: <DisplacementValue, Count>
	std::map<int64_t, int> displacement_counts;

	for (const auto& instr : instructions)
	{
		// End at the return of the current function
		if (instr.info.mnemonic == ZYDIS_MNEMONIC_RET)
			break;

		// We only care about MOV instructions
		if (instr.info.mnemonic != ZYDIS_MNEMONIC_MOV)
			continue;

		const auto& dest = instr.operands[0];
		const auto& src = instr.operands[1];

		// Filter for 64-bit general-purpose registers (excluding RSP) to target pointers.
		// This effectively filters out the 'length' member (int32_t) and stack operations.
		// Skip RSP because the values aren't on the stack. 
		if (dest.type != ZYDIS_OPERAND_TYPE_REGISTER || dest.size != 64 || dest.reg.value == ZYDIS_REGISTER_RSP)
			continue;

		if (src.type != ZYDIS_OPERAND_TYPE_MEMORY)
			continue;

		// Ignore direct pointer access with no offset
		if (!src.mem.disp.has_displacement)
			continue;

		displacement_counts[src.mem.disp.value]++;
	}

	// Now find the best candidate.
	// We look for a displacement that was accessed at least twice (for array1 and array2).
	// std::map sorts by key, so this will naturally find the lowest offset first.

	int64_t found_offset = -1;
	bool found = false;

	for (const auto& [disp, count] : displacement_counts)
	{
		if (count >= 2)
		{
			found_offset = disp;
			found = true;
			break;
		}
	}

	if (!found)
		return AURIE_OBJECT_NOT_FOUND;

	*OffsetFromBase = found_offset;

	return AURIE_SUCCESS;
}

Aurie::AurieStatus YYTK::Zeus::YYC::FindRoomData(
	IN PVOID RoomInstanceClear, 
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
			int room_id = YYGetRef(Arguments, 0, 0x1000003, 0, 0);
			CRoom* room_data = Room_Data(room_id); // <=== looking for this

			if (room_data)
				room_data->ClearStorageInstances();
		}

		It's the third call instruction.
	*/

	// Disassemble 80 bytes at the function
	auto instructions = Memory::DmDisassembleInstructionByRange(
		RoomInstanceClear,
		0x50
	);

	size_t target_call_index = Memory::DmFindMnemonicPattern(
		instructions,
		{
			ZYDIS_MNEMONIC_CALL,
			ZYDIS_MNEMONIC_TEST
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

Aurie::AurieStatus YYTK::Zeus::VM::GetBuiltinInformation(
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

Aurie::AurieStatus YYTK::Zeus::VM::FindArrayOffsetFromRValue(
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

	if (pattern_index == SIZE_MAX)
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

Aurie::AurieStatus YYTK::Zeus::VM::FindRoomData(
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

	size_t pattern_index = 0;
	ZydisDisassembledInstruction* mov_instruction = nullptr;

	while (pattern_index != SIZE_MAX)
	{
		pattern_index = Memory::DmFindMnemonicPattern(
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
		if (pattern_index == SIZE_MAX)
			return AURIE_OBJECT_NOT_FOUND;

		assert(instructions[pattern_index].info.mnemonic == ZYDIS_MNEMONIC_MOV);

		mov_instruction = &instructions[pattern_index];

		// Shift pattern index by 1, to prevent a "continue" going back to finding the same match twice.
		pattern_index++;

		// Make sure the mov has two operands
		if (mov_instruction->info.operand_count != 2)
			continue;

		// We're supposed to be moving to a register
		if (mov_instruction->operands[0].type != ZYDIS_OPERAND_TYPE_REGISTER)
			continue;

		// We're supposed to be moving from memory
		if (mov_instruction->operands[1].type != ZYDIS_OPERAND_TYPE_MEMORY)
			continue;

		// Reject stupidity like mov reg, [reg] - we have to have displacement
		if (!mov_instruction->operands[1].mem.disp.has_displacement)
		{
			DbgPrintEx(LOG_SEVERITY_WARNING, "No displacement in room data. Continuing...");
			continue;
		}

		// Break - we found it.
		break;
	}


	// Calculate the address of the room array
	ZyanU64 array_address = 0;
	if (!ZYAN_SUCCESS(ZydisCalcAbsoluteAddress(
		&mov_instruction->info,
		&mov_instruction->operands[1],
		mov_instruction->runtime_address,
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
	// We're looking for the first mov [mem64], 1 (imm)
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