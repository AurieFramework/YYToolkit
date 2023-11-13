#ifndef YYTK_API_H_
#define YYTK_API_H_

#include "../Tool.hpp"
#include <vector>

namespace YYTK
{
	void CmWriteInfo(
		IN std::string_view Format,
		IN ...
	);

	void CmWriteOutput(
		IN CmColor Color,
		IN std::string_view Format,
		IN ...
	);

	void CmWriteWarning(
		IN std::string_view Format,
		IN ...
	);

	void CmWriteError(
		IN std::string_view Filepath,
		IN const int& Line,
		IN std::string_view Format,
		IN ...
	);

	std::string CmpParseVa(
		IN const char* Format,
		IN va_list Arguments
	);

	void CmpSetTextColor(
		IN CmColor color
	);

	// Creates a console for the tool to output stuff into
	void CmpCreateConsole();

	Aurie::AurieStatus GmpGetRunnerInterface(
		OUT YYRunnerInterface& Interface
	);

	std::vector<TargettedInstruction> GmpDisassemble(
		IN PVOID Address,
		IN size_t MaximumSize,
		IN size_t MaximumInstructionsWithoutFunction
	);

	size_t GmpCountInstructionReferences(
		IN const std::vector<TargettedInstruction>& Instructions
	);

	Aurie::AurieStatus GmpFindFunctionsArray(
		IN const YYRunnerInterface& Interface,
		OUT RFunction*** FunctionsArray
	);

	Aurie::AurieStatus GmpFindScriptData(
		IN const YYRunnerInterface& Interface,
		IN TRoutine CopyStatic,
		OUT FNScriptData* ScriptData
	);

	// Allows for multiple matches in a scanned region
	Aurie::AurieStatus GmpSigscanRegionEx(
		IN const unsigned char* RegionBase,
		IN const size_t RegionSize,
		IN const unsigned char* Pattern,
		IN const char* PatternMask,
		OUT std::vector<size_t>& Matches
	);
}


#endif // YYTK_API_H_