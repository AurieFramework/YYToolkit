#ifndef YYTK_API_H_
#define YYTK_API_H_

#include "../Tool.hpp"
#include <vector>

namespace YYTK
{
	YYTKAPI void CmWriteInfo(
		IN std::string_view Format,
		IN ...
	);

	YYTKAPI void CmWriteOutput(
		IN CmColor Color,
		IN std::string_view Format,
		IN ...
	);

	YYTKAPI void CmWriteWarning(
		IN std::string_view Format,
		IN ...
	);

	YYTKAPI void CmWriteError(
		IN std::string_view Filepath,
		IN const int& Line,
		IN std::string_view Format,
		IN ...
	);

	namespace Internal
	{
		std::string CmpParseVa(
			IN const char* Format,
			IN va_list Arguments
		);

		void CmpSetTextColor(
			IN CmColor color
		);

		// Creates a console for the tool to output stuff into
		void CmpCreateConsole();

		YYTKStatus GmpGetRunnerInterface(
			OUT YYRunnerInterface& Interface
		);

		std::vector<PVOID> GmpGetFunctionChain(
			IN PVOID Address,
			IN size_t MaximumSize,
			IN size_t MaximumInstructionsWithoutFunction
		);

		// Allows for multiple matches in a scanned region
		YYTKStatus GmpSigscanRegionEx(
			IN const unsigned char* RegionBase,
			IN const size_t RegionSize,
			IN const unsigned char* Pattern,
			IN const char* PatternMask,
			OUT std::vector<size_t>& Matches
		);
	}
}


#endif // YYTK_API_H_