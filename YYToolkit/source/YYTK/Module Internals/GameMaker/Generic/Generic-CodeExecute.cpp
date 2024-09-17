#include "../../Module Internals.hpp"

using namespace Aurie;

namespace YYTK
{

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

}