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

	Aurie::AurieStatus GmpGetBuiltinInformation(
		OUT int32_t*& BuiltinCount,
		OUT RVariableRoutine*& BuiltinArray
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

	Aurie::AurieStatus GmpFindRoomData(
		IN TRoutine RoomInstanceClear,
		OUT FNRoomData* RoomData
	);

	Aurie::AurieStatus GmpFindCurrentRoomData(
		IN FNSetVariable SV_BackgroundColor,
		OUT CRoom*** Run_Room
	);

	Aurie::AurieStatus GmpFindRVArrayOffset(
		IN TRoutine F_ArrayEquals,
		OUT int64_t* ArrayOffset
	);

	Aurie::AurieStatus GmpFindDoCallScript(
		OUT PVOID* DoCallScript
	);

	Aurie::AurieStatus GmpFindCodeExecute(
		OUT PVOID* CodeExecute
	);

	// Allows for multiple matches in a scanned region
	Aurie::AurieStatus GmpSigscanRegionEx(
		IN const unsigned char* RegionBase,
		IN const size_t RegionSize,
		IN const unsigned char* Pattern,
		IN const char* PatternMask,
		OUT std::vector<size_t>& Matches
	);

	// Not game specific, kinda like MmSigscan* but for instructions
	Aurie::AurieStatus GmpFindMnemonicPattern(
		IN const std::vector<TargettedInstruction>& Instructions,
		IN const std::vector<ZydisMnemonic>& Mnemonics,
		OUT size_t& StartIndex
	);

	namespace Hooks
	{
		HRESULT WINAPI HkPresent(
			IN IDXGISwapChain* _this,
			IN unsigned int Sync,
			IN unsigned int Flags
		);

		bool HkExecuteIt(
			IN CInstance* SelfInstance,
			IN CInstance* OtherInstance,
			IN CCode* CodeObject,
			IN RValue* Arguments,
			IN INT Flags
		);

		PVOID HkDoCallScript(
			IN CScript* Script,
			IN int ArgumentCount,
			IN char* VmStackPointer,
			IN PVOID VmInstance,
			IN CInstance* Locals,
			IN CInstance* Arguments
		);

		// Meant for Stage 1 of loading in g_ModuleInterface
		Aurie::AurieStatus HkPreinitialize();

		// Meant for Stage 2 of loading in g_ModuleInterface
		Aurie::AurieStatus HkInitialize(
			IN HWND WindowHandle,
			IN IDXGISwapChain* EngineSwapChain
		);

		Aurie::AurieStatus HkUninitialize(
			IN HWND WindowHandle
		);
	}
}


#endif // YYTK_API_H_