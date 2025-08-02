#include "../Module Internals.hpp"

ZydisDisassembledInstruction YYTK::Memory::DmDisassembleInstruction(
	IN PVOID InstructionBase
)
{
	uint16_t image_arch = 0;
	auto last_status = Aurie::PpGetCurrentArchitecture(image_arch);

	if (!Aurie::AurieSuccess(last_status))
		return {};

	ZydisDisassembledInstruction instruction = {};
	ZyanStatus status = ZydisDisassembleIntel(
		image_arch == IMAGE_FILE_MACHINE_AMD64 ? ZYDIS_MACHINE_MODE_LONG_64 : ZYDIS_MACHINE_MODE_LEGACY_32,
		reinterpret_cast<ZyanU64>(InstructionBase),
		InstructionBase,
		ZYDIS_MAX_INSTRUCTION_LENGTH,
		&instruction
	);

	return instruction;
}

ZydisDecodedInstruction YYTK::Memory::DmDecodeInstruction(
	IN PVOID InstructionBase
)
{
	uint16_t image_arch = 0;
	auto last_status = Aurie::PpGetCurrentArchitecture(image_arch);

	if (!Aurie::AurieSuccess(last_status))
		return {};

	ZydisDecoder decoder;
	ZyanStatus status = ZydisDecoderInit(
		&decoder,
		image_arch == IMAGE_FILE_MACHINE_AMD64 ? ZYDIS_MACHINE_MODE_LONG_64 : ZYDIS_MACHINE_MODE_LEGACY_32,
		image_arch == IMAGE_FILE_MACHINE_AMD64 ? ZYDIS_STACK_WIDTH_64 : ZYDIS_STACK_WIDTH_32
	);

	if (!ZYAN_SUCCESS(status))
		return {};

	ZydisDecodedInstruction instruction = {};
	status = ZydisDecoderDecodeInstruction(
		&decoder,
		nullptr,
		InstructionBase,
		ZYDIS_MAX_INSTRUCTION_LENGTH,
		&instruction
	);

	return instruction;
}

std::vector<ZydisDecodedInstruction> YYTK::Memory::DmDecodeInstructionByRange(
	IN PVOID InstructionBase,
	IN SIZE_T InstructionRange,
	OPTIONAL OUT ZyanU64* LastInstruction
)
{
	uint16_t image_arch = 0;
	auto last_status = Aurie::PpGetCurrentArchitecture(image_arch);

	if (!Aurie::AurieSuccess(last_status))
		return {};

	ZydisDecoder decoder;
	ZyanStatus status = ZydisDecoderInit(
		&decoder,
		image_arch == IMAGE_FILE_MACHINE_AMD64 ? ZYDIS_MACHINE_MODE_LONG_64 : ZYDIS_MACHINE_MODE_LEGACY_32,
		image_arch == IMAGE_FILE_MACHINE_AMD64 ? ZYDIS_STACK_WIDTH_64 : ZYDIS_STACK_WIDTH_32
	);

	// Reserve some space for the instructions.
	// Assume each instruction is 4-bytes long.
	std::vector<ZydisDecodedInstruction> instructions;
	instructions.reserve(InstructionRange / 4);

	ZyanU64 current_address = reinterpret_cast<ZyanU64>(InstructionBase);
	while (current_address < (reinterpret_cast<ZyanU64>(InstructionBase) + InstructionRange))
	{
		ZydisDecodedInstruction instruction = {};
		status = ZydisDecoderDecodeInstruction(
			&decoder,
			nullptr,
			reinterpret_cast<LPCVOID>(current_address),
			ZYDIS_MAX_INSTRUCTION_LENGTH,
			&instruction
		);

		if (!ZYAN_SUCCESS(status))
		{
			current_address++;
			continue;
		}

		current_address += instruction.length;
		instructions.push_back(instruction);
	}

	if (LastInstruction)
		*LastInstruction = current_address;

	return instructions;
}

std::vector<ZydisDisassembledInstruction> YYTK::Memory::DmDisassembleInstructionByCount(
	IN PVOID InstructionBase, 
	IN SIZE_T InstructionCount
)
{
	uint16_t image_arch = 0;
	auto last_status = Aurie::PpGetCurrentArchitecture(image_arch);

	if (!Aurie::AurieSuccess(last_status))
		return {};

	std::vector<ZydisDisassembledInstruction> instructions;
	instructions.resize(InstructionCount);

	ZyanU64 current_address = reinterpret_cast<ZyanU64>(InstructionBase);
	size_t current_index = 0;
	while (current_index < InstructionCount)
	{
		ZyanStatus status = ZydisDisassembleIntel(
			image_arch == IMAGE_FILE_MACHINE_AMD64 ? ZYDIS_MACHINE_MODE_LONG_64 : ZYDIS_MACHINE_MODE_LEGACY_32,
			current_address,
			reinterpret_cast<PVOID>(current_address),
			ZYDIS_MAX_INSTRUCTION_LENGTH,
			&instructions[current_index]
		);

		if (!ZYAN_SUCCESS(status))
		{
			current_address++;
			continue;
		}

		current_address += instructions[current_index].info.length;
		current_index++;
	}

	return instructions;
}

std::vector<ZydisDisassembledInstruction> YYTK::Memory::DmDisassembleInstructionByRange(
	IN PVOID InstructionBase, 
	IN SIZE_T InstructionRange
)
{
	uint16_t image_arch = 0;
	auto last_status = Aurie::PpGetCurrentArchitecture(image_arch);

	if (!Aurie::AurieSuccess(last_status))
		return {};

	std::vector<ZydisDisassembledInstruction> instructions;

	ZyanU64 current_address = reinterpret_cast<ZyanU64>(InstructionBase);
	while (current_address < (reinterpret_cast<ZyanU64>(InstructionBase) + InstructionRange))
	{
		ZydisDisassembledInstruction disassembled_instruction;
		ZyanStatus status = ZydisDisassembleIntel(
			image_arch == IMAGE_FILE_MACHINE_AMD64 ? ZYDIS_MACHINE_MODE_LONG_64 : ZYDIS_MACHINE_MODE_LEGACY_32,
			current_address,
			reinterpret_cast<PVOID>(current_address),
			ZYDIS_MAX_INSTRUCTION_LENGTH,
			&disassembled_instruction
		);

		if (!ZYAN_SUCCESS(status))
		{
			current_address++;
			continue;
		}

		current_address += disassembled_instruction.info.length;
		instructions.push_back(disassembled_instruction);
	}

	return instructions;
}

Aurie::AurieStatus YYTK::Memory::DmGetSectionBounds(
	IN const char* SectionName,
	OUT uint64_t* SectionStart,
	OUT uint64_t* SectionEnd
)
{
	uint64_t text_section_offset = 0;
	size_t text_section_size = 0;

	// Query the game's base address
	const uint64_t game_base = reinterpret_cast<uint64_t>(GetModuleHandleW(nullptr));

	// Get the .text section address for the game executable
	auto status = Aurie::Internal::PpiGetModuleSectionBounds(
		GetModuleHandleW(nullptr),
		".text",
		text_section_offset,
		text_section_size
	);

	// On success, populate the SectionStart and SectionEnd buffers.
	if (Aurie::AurieSuccess(status))
	{
		*SectionStart = game_base + text_section_offset;
		*SectionEnd = game_base + text_section_offset + text_section_size;
	}

	return status;
}

uintptr_t YYTK::Memory::DmCalculateInstructionAddress(
	IN const std::vector<ZydisDecodedInstruction>& Instructions,
	IN int64_t Index, 
	IN uintptr_t PostLastInstructionAddress
)
{
	if (Instructions.size() < static_cast<uint64_t>(Index))
		return UINTPTR_MAX;

	uintptr_t address = PostLastInstructionAddress;

	for (intptr_t i = (Instructions.size() - 1); i >= Index; i--)
		address -= Instructions[i].length;

	return address;
}

SIZE_T YYTK::Memory::DmFindMnemonicPattern(
	IN const std::vector<ZydisDecodedInstruction>& Instructions, 
	IN const std::vector<ZydisMnemonic>& Mnemonics, 
	OPTIONAL IN SIZE_T LoopStartIndex
)
{
	// Not possible to find n-long pattern in m instructions if n > m.
	if (Instructions.size() < Mnemonics.size())
		return SIZE_MAX;

	// Loop all instructions in the vector
	for (size_t start_index = LoopStartIndex; start_index < Instructions.size() - Mnemonics.size(); start_index++)
	{
		bool pattern_matches = true;

		// Loop all target mnemonics
		for (size_t in_pattern_index = 0; in_pattern_index < Mnemonics.size(); in_pattern_index++)
		{
			const ZydisMnemonic& actual_mnemonic = Instructions.at(start_index + in_pattern_index).mnemonic;
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
			return start_index;
		}
	}

	return SIZE_MAX;
}

SIZE_T YYTK::Memory::DmFindMnemonicPattern(
	IN const std::vector<ZydisDisassembledInstruction>& Instructions,
	IN const std::vector<ZydisMnemonic>& Mnemonics,
	OPTIONAL IN SIZE_T LoopStartIndex
)
{
	// Not possible to find n-long pattern in m instructions if n > m.
	if (Instructions.size() < Mnemonics.size())
		return SIZE_MAX;

	// Loop all instructions in the vector
	for (size_t start_index = LoopStartIndex; start_index < Instructions.size() - Mnemonics.size(); start_index++)
	{
		bool pattern_matches = true;

		// Loop all target mnemonics
		for (size_t in_pattern_index = 0; in_pattern_index < Mnemonics.size(); in_pattern_index++)
		{
			const ZydisMnemonic& actual_mnemonic = Instructions.at(start_index + in_pattern_index).info.mnemonic;
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
			return start_index;
		}
	}

	return SIZE_MAX;
}

size_t YYTK::Memory::DmSigscanGame(
	IN const unsigned char* Pattern,
	IN const char* Mask
)
{
	Aurie::AurieStatus last_status = Aurie::AURIE_SUCCESS;

	std::wstring game_name;
	last_status = Aurie::MdGetImageFilename(
		Aurie::g_ArInitialImage,
		game_name
	);

	return Aurie::MmSigscanModule(
		game_name.c_str(),
		Pattern,
		Mask
	);
}

std::vector<size_t> YYTK::Memory::DmSigscanGameEx(
	IN const unsigned char* Pattern, 
	IN const char* Mask
)
{
	uint64_t region_start = 0;
	uint64_t region_end = 0;
	
	Aurie::AurieStatus last_status = Aurie::AURIE_SUCCESS;
	
	last_status = DmGetSectionBounds(
		".text",
		&region_start,
		&region_end
	);

	if (!Aurie::AurieSuccess(last_status))
		return {};

	std::vector<size_t> matches;
	size_t last_match = 0;
	do
	{
		last_match = Aurie::MmSigscanRegion(
			reinterpret_cast<unsigned char*>(region_start),
			static_cast<size_t>(region_end - region_start),
			Pattern,
			Mask
		);

		if (last_match)
			matches.push_back(last_match);

		region_start = last_match + 1;
	} while (last_match != 0);
	
	return matches;
}
