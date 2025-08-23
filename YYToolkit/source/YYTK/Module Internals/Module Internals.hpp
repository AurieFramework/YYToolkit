#ifndef YYTK_API_H_
#define YYTK_API_H_

#include "../Tool.hpp"
#include <vector>

namespace YYTK
{
	namespace Zeus
	{
		/**
		 * \brief Determines the best address to hook at in order to find the runner interface.
		 * \param TargetInstruction A pointer to a buffer which receives the address of the instruction.
		 */
		Aurie::AurieStatus FindRunnerInterfaceHookpoint(
			OUT PVOID* TargetInstruction
		);

		/**
		 * \brief Registers the hook for the runner interface. Ran when the process is still suspended.
		 * \param Hookpoint The address determined by FindRunnerInterfaceHookpoint.
		 * \param TargetFunction An optional pointer to a function that will be called when the runner interface hook is hit.
		 */
		Aurie::AurieStatus RegisterRunnerInterfaceHook(
			IN PVOID Hookpoint,
			IN Aurie::AurieMidHookFunction TargetFunction
		);

		/**
		 * \brief Called by the game. Populated the m_RunnerInterface member in the module interface, and signals the population event. 
		 * \param ProcessorContext The context of the processor when the hook triggered.
		 */
		void HandleRunnerInterfaceCreation( 
			IN Aurie::ProcessorContext& ProcessorContext
		);

		/**
		 * \brief Determines the best address to hook at in order to manipulate code events in the runner.
		 * \param TargetInstruction A pointer to a buffer which receives the address of the instruction.
		 */
		Aurie::AurieStatus FindCodeExecutionHookpoint(
			OUT PVOID* TargetInstruction
		);

		Aurie::AurieStatus DetermineFunctionEntrySize(
			IN RFunction** FunctionArray,
			OUT size_t* Size
		);

		Aurie::AurieStatus FindScriptData(
			IN const YYRunnerInterface& Interface,
			IN PVOID CopyStatic,
			OUT FNScriptData* GetScriptData
		);

		Aurie::AurieStatus FindCurrentRoomData(
			IN FNSetVariable SetVariable,
			OUT CRoom*** RunRoom
		);

		Aurie::AurieStatus FindSlotAdditionFunction(
			IN const YYRunnerInterface& Interface,
			OUT PFN_YYObjectBaseAdd* Function
		);

		Aurie::AurieStatus FindSlotAllocationFunction(
			IN PFN_YYObjectBaseAdd YYObjectBase_Add,
			OUT	PFN_FindAllocSlot* FindAllocSlot
		);

		Aurie::AurieStatus FindErrorSuppressionVariable(
			IN PVOID IsNaN,
			OUT bool** SuppressionVariable
		);

		/**
		 * \brief Tries to determine what runner function owns instruction pointer based off known function pointers.
		 * \param InstructionPointer The instruction pointer to try look up.
		 */

		void BuildApproximateSymbolTable(
			OUT std::vector<std::pair<uintptr_t, std::string>>& SymbolTable
		);

		std::string GuessSymbolFromGameInstructionAddress(
			IN LPCVOID InstructionPointer
		);

		namespace YYC
		{
			Aurie::AurieStatus FindFunctionsArray(
				IN const YYRunnerInterface& Interface,
				OUT RFunction*** FunctionsArray
			);

			Aurie::AurieStatus GetBuiltinInformation(
				OUT int32_t** BuiltinCount,
				OUT RVariableRoutine** BuiltinArray
			);

			Aurie::AurieStatus FindArrayOffsetFromRValue(
				IN PVOID ArrayEquals,
				OUT int64_t* OffsetFromBase
			);

			Aurie::AurieStatus FindRoomData(
				IN PVOID RoomInstanceClear,
				OUT FNRoomData* RoomData
			);
		}

		namespace VM
		{
			Aurie::AurieStatus FindFunctionsArray(
				IN const YYRunnerInterface& Interface,
				OUT RFunction*** FunctionsArray
			);

			Aurie::AurieStatus GetBuiltinInformation(
				OUT int32_t** BuiltinCount,
				OUT RVariableRoutine** BuiltinArray
			);

			Aurie::AurieStatus FindArrayOffsetFromRValue(
				IN PVOID ArrayEquals,
				OUT int64_t* OffsetFromBase
			);

			Aurie::AurieStatus FindRoomData(
				IN PVOID RoomInstanceClear,
				OUT FNRoomData* RoomData
			);
		}
	}

	namespace Generic
	{
		/**
		 * \brief Prints information about the game loaded in memory into the log at TRACE_LEVEL.
		 */
		void MiPrintLoadInfo();
	}

	namespace Memory
	{

		/**
		 * \brief Fully disassembles an instruction using the current architecture.
		 * \param InstructionBase The address of the instruction.
		 * \return The disassembled instruction.
		 */
		ZydisDisassembledInstruction DmDisassembleInstruction(
			IN PVOID InstructionBase
		);

		/**
		 * \brief Decodes an instruction using the current architecture.
		 * \param InstructionBase The address of the instruction.
		 * \return The decoded instruction.
		 */
		ZydisDecodedInstruction DmDecodeInstruction(
			IN PVOID InstructionBase
		);

		/**
		 * \brief Decodes a set number of instructions using the current architecture.
		 * \param InstructionBase The address of the initial instruction.
		 * \param InstructionRange The maximum offset from InstructionBase.
		 * \param LastInstruction An optional pointer to a buffer which receives the address at which disassembly ended.
		 * \return The decoded instructions.
		 */
		std::vector<ZydisDecodedInstruction> DmDecodeInstructionByRange(
			IN PVOID InstructionBase,
			IN SIZE_T InstructionRange,
			OPTIONAL OUT ZyanU64* LastInstruction
		);

		/**
		 * \brief Disassembles a set number of instructions using the current architecture.
		 * \param InstructionBase The address of the initial instruction.
		 * \param InstructionCount The number of instructions to disassemble.
		 * \return The disassembled instructions.
		 */
		std::vector<ZydisDisassembledInstruction> DmDisassembleInstructionByCount(
			IN PVOID InstructionBase,
			IN SIZE_T InstructionCount
		);

		/**
		 * \brief Disassembles a set number of instructions using the current architecture.
		 * \param InstructionBase The address of the initial instruction.
		 * \param InstructionRange The maximum offset from InstructionBase.
		 * \return The disassembled instructions.
		 */
		std::vector<ZydisDisassembledInstruction> DmDisassembleInstructionByRange(
			IN PVOID InstructionBase,
			IN SIZE_T InstructionRange
		);

		/**
		 * \brief Retrieves the start and end addresses for a given section inside the game's executable.
		 * \param SectionName The name of the section.
		 * \param SectionStart A buffer which will get populated with the starting address of the section.
		 * \param SectionEnd A buffer which will get populated with the ending address of the section.
		 * \return A status code.
		 */
		Aurie::AurieStatus DmGetSectionBounds(
			IN const char* SectionName,
			OUT uint64_t* SectionStart,
			OUT uint64_t* SectionEnd
		);

		/**
		 * \brief Calculates the address of a decoded instruction given the IP past all the other instructions.
		 * \param Instructions The decoded instructions.
		 * \param Index The index of the instruction whose address is to be computed.
		 * \param PostLastInstructionAddress The instruction pointer past the final instruction.
		 * \return The address of the instruction.
		 */
		uintptr_t DmCalculateInstructionAddress(
			IN const std::vector<ZydisDecodedInstruction>& Instructions,
			IN int64_t Index,
			IN uintptr_t PostLastInstructionAddress
		);

		SIZE_T DmFindMnemonicPattern(
			IN const std::vector<ZydisDecodedInstruction>& Instructions,
			IN const std::vector<ZydisMnemonic>& Mnemonics,
			OPTIONAL IN SIZE_T LoopStartIndex
		);

		SIZE_T DmFindMnemonicPattern(
			IN const std::vector<ZydisDisassembledInstruction>& Instructions,
			IN const std::vector<ZydisMnemonic>& Mnemonics,
			OPTIONAL IN SIZE_T LoopStartIndex
		);

		size_t DmSigscanGame(
			IN const unsigned char* Pattern,
			IN const char* Mask
		);

		std::vector<size_t> DmSigscanGameEx(
			IN const unsigned char* Pattern,
			IN const char* Mask
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
		Aurie::AurieStatus InitializeStage1Hooks();

		// Meant for Stage 2 of loading in g_ModuleInterface
		Aurie::AurieStatus InitializeStage2Hooks(
			IN HWND WindowHandle,
			IN IDXGISwapChain* EngineSwapChain
		);

		Aurie::AurieStatus HkUninitialize(
			IN HWND WindowHandle
		);
	}
}


#endif // YYTK_API_H_