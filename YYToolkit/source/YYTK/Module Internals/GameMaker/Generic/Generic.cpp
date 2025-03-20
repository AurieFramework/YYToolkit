#include "../../Module Internals.hpp"
using namespace Aurie;

namespace YYTK
{
	// ===== Shared, non-GameMaker-reliant code =====

	std::string GmResolveGameSymbolFromAddress(
		IN LPCVOID Address
	)
	{
		// Try to get the NT header
		const uintptr_t address = reinterpret_cast<uintptr_t>(Address);
		void* image_base = GetModuleHandleW(nullptr);
		void* nt_header_ptr = 0;
		if (!AurieSuccess(Internal::PpiGetNtHeader(image_base, nt_header_ptr)))
			return "";

		PIMAGE_NT_HEADERS nt_header = static_cast<PIMAGE_NT_HEADERS>(nt_header_ptr);
		if (address < reinterpret_cast<uintptr_t>(image_base))
			return "";

		if (address > reinterpret_cast<uintptr_t>(image_base) + nt_header->OptionalHeader.SizeOfImage)
			return "";

		std::map<uintptr_t, std::string> symbols;
		AurieStatus last_status = AURIE_SUCCESS;

		// Push all scripts...
		int script_index = 0;
		while (AurieSuccess(last_status))
		{
			CScript* current_script = nullptr;
			last_status = g_ModuleInterface.GetScriptData(script_index++, current_script);

			if (!current_script)
				continue;

			symbols.insert({ reinterpret_cast<uintptr_t>(current_script->m_Functions->m_ScriptFunction), current_script->m_Name });
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
				symbols.insert({ reinterpret_cast<uintptr_t>(function_ptr), function_name });
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

				symbols.insert({ reinterpret_cast<uintptr_t>(variable_information->m_GetVariable), name });
			}
			if (variable_information->m_SetVariable)
			{
				std::string name = "SV_";
				name.append(variable_information->m_Name);

				symbols.insert({ reinterpret_cast<uintptr_t>(variable_information->m_SetVariable), name });
			}
		}

		// Push runner interface functions...
		const YYRunnerInterface& runner_interface = g_ModuleInterface.GetRunnerInterface();
		{
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYGetString), "YYGetString" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DebugConsoleOutput), "DebugConsoleOutput" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.ReleaseConsoleOutput), "ReleaseConsoleOutput" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.ShowMessage), "ShowMessage" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYError), "YYError" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYAlloc), "YYAlloc" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYRealloc), "YYRealloc" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYFree), "YYFree" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYStrDup), "YYStrDup" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYGetBool), "YYGetBool" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYGetFloat), "YYGetFloat" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYGetReal), "YYGetReal" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYGetInt32), "YYGetInt32" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYGetUint32), "YYGetUint32" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYGetInt64), "YYGetInt64" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYGetPtr), "YYGetPtr" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYGetPtrOrInt), "YYGetPtrOrInt" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYGetString), "YYGetString" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.BOOL_RValue), "BOOL_RValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.REAL_RValue), "REAL_RValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.PTR_RValue), "PTR_RValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.INT64_RValue), "INT64_RValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.INT32_RValue), "INT32_RValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.HASH_RValue), "HASH_RValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.SET_RValue), "SET_RValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.GET_RValue), "GET_RValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.COPY_RValue), "COPY_RValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.KIND_RValue), "KIND_RValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.FREE_RValue), "FREE_RValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYCreateString), "YYCreateString" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYCreateArray), "YYCreateArray" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.Script_Find_Id), "Script_Find_Id" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.Script_Perform), "Script_Perform" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.Code_Function_Find), "Code_Function_Find" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.HTTP_Get), "HTTP_Get" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.HTTP_Post), "HTTP_Post" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.HTTP_Request), "HTTP_Request" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.ASYNCFunc_SpriteAdd), "ASYNCFunc_SpriteAdd" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.ASYNCFunc_SpriteCleanup), "ASYNCFunc_SpriteCleanup" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.CreateSpriteAsync), "CreateSpriteAsync" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.Timing_Time), "Timing_Time" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.Timing_Sleep), "Timing_Sleep" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYMutexCreate), "YYMutexCreate" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYMutexDestroy), "YYMutexDestroy" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYMutexLock), "YYMutexLock" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYMutexUnlock), "YYMutexUnlock" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.CreateAsyncEventWithDSMap), "CreateAsyncEventWithDSMap" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.CreateAsyncEventWithDSMapAndBuffer), "CreateAsyncEventWithDSMapAndBuffer" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.CreateDsMap), "CreateDsMap" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DsMapAddDouble), "DsMapAddDouble" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DsMapAddString), "DsMapAddString" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DsMapAddInt64), "DsMapAddInt64" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.BufferGetContent), "BufferGetContent" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.BufferWriteContent), "BufferWriteContent" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.CreateBuffer), "CreateBuffer" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DsListCreate), "DsListCreate" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DsMapAddList), "DsMapAddList" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DsListAddMap), "DsListAddMap" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DsMapClear), "DsMapClear" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DsListClear), "DsListClear" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DsListClear), "DsListClear" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.BundleFileExists), "BundleFileExists" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.BundleFileName), "BundleFileName" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.SaveFileExists), "SaveFileExists" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.SaveFileName), "SaveFileName" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.Base64Encode), "Base64Encode" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DsListAddInt64), "DsListAddInt64" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.AddDirectoryToBundleWhitelist), "AddDirectoryToBundleWhitelist" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.AddFileToBundleWhitelist), "AddFileToBundleWhitelist" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.AddDirectoryToSaveWhitelist), "AddDirectoryToSaveWhitelist" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.AddFileToSaveWhitelist), "AddFileToSaveWhitelist" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.KIND_NAME_RValue), "KIND_NAME_RValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DsMapAddBool), "DsMapAddBool" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DsMapAddRValue), "DsMapAddRValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DestroyDsMap), "DestroyDsMap" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.StructCreate), "StructCreate" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.StructAddBool), "StructAddBool" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.StructAddDouble), "StructAddDouble" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.StructAddInt), "StructAddInt" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.StructAddRValue), "StructAddRValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.StructAddString), "StructAddString" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.WhitelistIsDirectoryIn), "WhitelistIsDirectoryIn" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.WhiteListIsFilenameIn), "WhiteListIsFilenameIn" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.WhiteListAddTo), "WhiteListAddTo" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.DirExists), "DirExists" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.BufferGetFromGML), "BufferGetFromGML" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.BufferTELL), "BufferTELL" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.BufferGet), "BufferGet" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.FilePrePend), "FilePrePend" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.StructAddInt32), "StructAddInt32" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.StructAddInt64), "StructAddInt64" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.StructGetMember), "StructGetMember" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.StructGetKeys), "StructGetKeys" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.YYGetStruct), "YYGetStruct" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.extOptGetRValue), "extOptGetRValue" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.extOptGetString), "extOptGetString" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.extOptGetReal), "extOptGetReal" });
			symbols.insert({ reinterpret_cast<uintptr_t>(runner_interface.isRunningFromIDE), "isRunningFromIDE" });
		}

		symbols.insert({ reinterpret_cast<uintptr_t>(g_ModuleInterface.m_CodeExecute), "Code_Execute" });

		if (g_ModuleInterface.m_ExceptionRIP)
			symbols.insert({ reinterpret_cast<uintptr_t>(g_ModuleInterface.m_ExceptionRIP), "Extension_PrePrepare" });


		std::wstring game_name_wstring;
		MdGetImageFilename(g_ArInitialImage, game_name_wstring);

		const std::string game_name_utf8(game_name_wstring.begin(), game_name_wstring.end());
		const auto symbol = --symbols.upper_bound(address);

		return std::format("{}!{}+0x{:X}", game_name_utf8, symbol->second, address - symbol->first);
	}

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