#ifndef YYTK_API_H_
#define YYTK_API_H_

#include "../Tool.hpp"
#include <vector>

namespace YYTK
{
	// Shared, not directly GM related

	std::vector<TargettedInstruction> GmpDisassemble(
		IN PVOID Address,
		IN size_t MaximumSize,
		IN size_t MaximumInstructionsWithoutFunction
	);

	size_t GmpCountInstructionReferences(
		IN const std::vector<TargettedInstruction>& Instructions
	);

	Aurie::AurieStatus GmpSigscanRegionEx(
		IN const unsigned char* RegionBase,
		IN const size_t RegionSize,
		IN const unsigned char* Pattern,
		IN const char* PatternMask,
		OUT std::vector<size_t>& Matches
	);

	Aurie::AurieStatus GmpFindMnemonicPattern(
		IN const std::vector<YYTK::TargettedInstruction>& Instructions,
		IN const std::vector<ZydisMnemonic>& Mnemonics,
		OUT size_t& StartIndex,
		OPTIONAL IN size_t LoopStartIndex = 0
	);

	// Shared, directly GM related

	std::string GmResolveGameSymbolFromAddress(
		IN LPCVOID Address
	);

	Aurie::AurieStatus GmpGetRunnerInterface(
		OUT YYRunnerInterface& Interface
	);

	void GmpRunnerInterfaceHook(
		IN Aurie::ProcessorContext& Function
	);

	Aurie::AurieStatus GmpCreateHookOnInterfaceCreation(
		OPTIONAL OUT PVOID* Rip,
		IN Aurie::AurieMidHookFunction Handler
	);

	Aurie::AurieStatus GmpFindScriptData(
		IN const YYRunnerInterface& Interface,
		IN TRoutine CopyStatic,
		OUT FNScriptData* ScriptData
	);

	Aurie::AurieStatus GmpFindCurrentRoomData(
		IN FNSetVariable SV_BackgroundColor,
		OUT CRoom*** Run_Room
	);

	Aurie::AurieStatus GmpFindCodeExecute(
		OUT PVOID* CodeExecute
	);

	Aurie::AurieStatus GmpGetYYObjectBaseAdd(
		IN const YYRunnerInterface& Interface,
		OUT PFN_YYObjectBaseAdd* Function
	);

	Aurie::AurieStatus GmpGetFindAllocSlotFromName(
		IN PFN_YYObjectBaseAdd YYObjectBase_Add,
		OUT	PFN_FindAllocSlot* FindAllocSlot
	);

	// Implementations made specifically for VM runners
	namespace VM
	{
		Aurie::AurieStatus GmpGetBuiltinInformation(
			OUT int32_t*& BuiltinCount,
			OUT RVariableRoutine*& BuiltinArray
		);

		Aurie::AurieStatus GmpFindFunctionsArray(
			IN const YYRunnerInterface& Interface,
			OUT RFunction*** FunctionsArray
		);

		Aurie::AurieStatus GmpFindRoomData(
			IN TRoutine RoomInstanceClear,
			OUT FNRoomData* RoomData
		);

		Aurie::AurieStatus GmpFindRVArrayOffset(
			IN TRoutine F_ArrayEquals,
			IN YYRunnerInterface& Interface,
			OUT int64_t* ArrayOffset
		);
	}

	// YYC specific implementations
	namespace YYC
	{
		Aurie::AurieStatus GmpGetBuiltinInformation(
			OUT int32_t*& BuiltinCount,
			OUT RVariableRoutine*& BuiltinArray
		);

		Aurie::AurieStatus GmpFindFunctionsArray(
			IN const YYRunnerInterface& Interface,
			OUT RFunction*** FunctionsArray
		);

		Aurie::AurieStatus GmpFindRoomData(
			IN TRoutine RoomInstanceClear,
			OUT FNRoomData* RoomData
		);

		Aurie::AurieStatus GmpFindRVArrayOffset(
			IN TRoutine F_ArrayEquals,
			OUT int64_t* ArrayOffset
		);
	}

	namespace Hooks
	{
		HRESULT WINAPI HkPresent(
			IN IDXGISwapChain* _this,
			IN unsigned int Sync,
			IN unsigned int Flags
		);

		HRESULT WINAPI HkResizeBuffers(
			IN IDXGISwapChain* _this,
			IN UINT BufferCount,
			IN UINT Width,
			IN UINT Height,
			IN DXGI_FORMAT NewFormat,
			IN UINT SwapChainFlags
		);

		bool HkExecuteIt(
			IN CInstance* SelfInstance,
			IN CInstance* OtherInstance,
			IN CCode* CodeObject,
			IN RValue* Arguments,
			IN INT Flags
		);

		void HkYYError(
			IN const char* ErrorString,
			IN ...
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