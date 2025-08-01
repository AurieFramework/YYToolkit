#include "../Module Internals.hpp"
using namespace Aurie;

void YYTK::Generic::MiPrintLoadInfo()
{
	std::wstring filename = L"<UNKNOWN>";
	MdGetImageFilename(g_ArInitialImage, filename);

	HMODULE game_base = GetModuleHandleA(nullptr);

	DbgPrintEx(
		LOG_SEVERITY_TRACE, 
		"YYToolkit %s loading - hello from MiPrintLoadInfo!", 
		YYTK_VERSION_STRING
	);

	DbgPrintEx(
		LOG_SEVERITY_TRACE,
		"Executable %S is loaded at %p", 
		filename.c_str(), game_base
	);

	// Get info about the game's .text section.
	uint64_t text_start = 0, text_end = 0;
	Memory::DmGetSectionBounds(
		".text",
		&text_start,
		&text_end
	);

	// Debug info.
	DbgPrintEx(
		LOG_SEVERITY_TRACE,
		".text section spans 0x%I64X-0x%I64X",
		text_start, text_end
	);
}
