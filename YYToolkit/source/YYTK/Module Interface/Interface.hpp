#ifndef YYTK_INTERFACE_H_
#define YYTK_INTERFACE_H_
#include "../Tool.hpp"
#include <map>

using namespace Aurie;

namespace YYTK
{
	class YYTKInterfaceImpl : public YYTKInterface
	{
	public:
		// Dictates whether the first stage of initializing completed already.
		bool m_FirstInitComplete = false;

		// Dictates whether the second stage of initializing completed already.
		bool m_SecondInitComplete = false;
	private:
		// The runner interface stolen by disassembling Extension_PrePrepare()
		YYRunnerInterface m_RunnerInterface = {};

		// A pointer to the functions array in memory
		RFunction** m_FunctionsArray = nullptr;

		// A pointer to the Script_Data() engine function.
		FNScriptData m_GetScriptData = nullptr;

		// The function cache used for faster lookups
		std::map<std::string, TRoutine> m_FunctionCache = {};

		// The size of one entry in the RFunction array
		// GameMaker LTS still uses a 64-byte char array in the RFunction struct directly
		// New runners (2023.8) use a const char* in the array
		size_t m_FunctionEntrySize = 0;

		void InternalExtractFunctionEntry(
			IN size_t Index,
			OUT std::string& FunctionName,
			OUT TRoutine& FunctionRoutine,
			OUT int32_t& ArgumentCount
		);

		size_t InternalDetermineFunctionEntrySize();

	public:
		// === Interface Functions ===
		virtual AurieStatus Create() override final;

		virtual void Destroy() override final;

		virtual void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		);

		virtual AurieStatus GetNamedRoutineIndex(
			IN const char* FunctionName,
			OUT int* FunctionIndex
		) override final;

		virtual AurieStatus GetNamedRoutinePointer(
			IN const char* FunctionName,
			OUT PVOID* FunctionPointer
		) override final;

		virtual AurieStatus GetGlobalInstance(
			OUT CInstance** Instance
		) override final;

		virtual RValue CallBuiltin(
			IN const char* FunctionName,
			IN std::vector<RValue> Arguments
		) override final;

		virtual AurieStatus CallBuiltinEx(
			OUT RValue& Result,
			IN const char* FunctionName,
			IN CInstance* SelfInstance,
			IN CInstance* OtherInstance,
			IN std::vector<RValue> Arguments
		) override final;

		virtual void Print(
			IN CmColor Color,
			IN const char* Format,
			IN ...
		) override final;

		virtual void PrintInfo(
			IN const char* Format,
			IN ...
		) override final;

		virtual void PrintWarning(
			IN const char* Format,
			IN ...
		) override final;

		virtual void PrintError(
			IN const char* Function,
			IN const int Line,
			IN const char* Format,
			IN ...
		) override final;

		virtual AurieStatus CreateCallback(
			IN const Aurie::AurieModule* Module,
			IN EventTriggers Trigger,
			IN PVOID Routine
		) override final;

		virtual AurieStatus RemoveCallback(
			IN const Aurie::AurieModule* Module,
			IN PVOID Routine
		) override final;

		virtual AurieStatus InvalidateAllCaches() override final;
	};

	inline YYTKInterfaceImpl g_ModuleInterface;
}

#endif // YYTK_INTERFACE_H_