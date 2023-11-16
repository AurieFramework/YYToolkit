#ifndef YYTK_INTERFACE_H_
#define YYTK_INTERFACE_H_
#include "../Tool.hpp"
#include <map>

namespace YYTK
{
	class YYTKInterfaceImpl : public YYTKInterface
	{
	public:
		// Dictates whether the first stage of initializing completed already.
		bool m_FirstInitComplete = false;

		// Dictates whether the second stage of initializing completed already.
		bool m_SecondInitComplete = false;

		// The runner interface stolen by disassembling Extension_PrePrepare()
		YYRunnerInterface m_RunnerInterface = {};
	private:

		// A pointer to the functions array in memory
		RFunction** m_FunctionsArray = nullptr;

		// A pointer to the Script_Data() engine function.
		FNScriptData m_GetScriptData = nullptr;

		// The function cache used for faster lookups
		std::map<std::string, TRoutine> m_FunctionCache = {};

		// Stores plugin callbacks
		std::map<Aurie::AurieModule*, std::vector<ModuleCallbackDescriptor>> m_RegisteredCallbacks;

		// D3D11 stuff
		IDXGISwapChain* m_EngineSwapchain = nullptr;
		ID3D11Device* m_EngineDevice = nullptr;
		ID3D11DeviceContext* m_EngineDeviceContext = nullptr;

		// The size of one entry in the RFunction array
		// GameMaker LTS still uses a 64-byte char array in the RFunction struct directly
		// New runners (2023.8) use a const char* in the array
		size_t m_FunctionEntrySize = 0;

		void YkExtractFunctionEntry(
			IN size_t Index,
			OUT std::string& FunctionName,
			OUT TRoutine& FunctionRoutine,
			OUT int32_t& ArgumentCount
		);

		ModuleCallbackDescriptor YkCreateCallbackDescriptor(
			IN Aurie::AurieModule* Module,
			IN EventTriggers Trigger,
			IN PVOID Routine,
			IN int32_t Priority
		);

		ModuleCallbackDescriptor* YkAddToCallbackList(
			IN Aurie::AurieModule* Module,
			IN ModuleCallbackDescriptor& Descriptor
		);

		void YkRemoveCallbackFromList(
			IN Aurie::AurieModule* Module,
			IN PVOID Routine
		);

		bool YkCallbackExists(
			IN Aurie::AurieModule* Module,
			IN PVOID Routine
		);

		size_t YkDetermineFunctionEntrySize();

	public:
		// === Interface Functions ===
		virtual Aurie::AurieStatus Create() override final;

		virtual void Destroy() override final;

		virtual void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		) override final;

		virtual Aurie::AurieStatus GetNamedRoutineIndex(
			IN const char* FunctionName,
			OUT int* FunctionIndex
		) override final;

		virtual Aurie::AurieStatus GetNamedRoutinePointer(
			IN const char* FunctionName,
			OUT PVOID* FunctionPointer
		) override final;

		virtual Aurie::AurieStatus GetGlobalInstance(
			OUT CInstance** Instance
		) override final;

		virtual RValue CallBuiltin(
			IN const char* FunctionName,
			IN std::vector<RValue> Arguments
		) override final;

		virtual Aurie::AurieStatus CallBuiltinEx(
			OUT RValue& Result,
			IN const char* FunctionName,
			IN CInstance* SelfInstance,
			IN CInstance* OtherInstance,
			IN std::vector<RValue> Arguments
		) override final;

		virtual void Print(
			IN CmColor Color,
			IN std::string_view Format,
			IN ...
		) override final;

		virtual void PrintInfo(
			IN std::string_view Format,
			IN ...
		) override final;

		virtual void PrintWarning(
			IN std::string_view Format,
			IN ...
		) override final;

		virtual void PrintError(
			IN std::string_view Filepath,
			IN const int Line,
			IN std::string_view Format,
			IN ...
		) override final;

		virtual Aurie::AurieStatus CreateCallback(
			IN Aurie::AurieModule* Module,
			IN EventTriggers Trigger,
			IN PVOID Routine,
			IN int32_t Priority
		) override final;

		virtual Aurie::AurieStatus RemoveCallback(
			IN Aurie::AurieModule* Module,
			IN PVOID Routine
		) override final;

		virtual Aurie::AurieStatus GetInstanceMember(
			IN RValue Instance,
			IN const char* MemberName,
			OUT RValue*& Member
		) override final;

		virtual Aurie::AurieStatus EnumInstanceMembers(
			IN RValue Instance,
			IN std::function<bool(IN const char* MemberName, RValue* Value)> EnumFunction
		) override final;

		virtual Aurie::AurieStatus RValueToString(
			IN const RValue& Value,
			OUT std::string& String
		) override final;

		virtual Aurie::AurieStatus StringToRValue(
			IN const std::string_view String,
			OUT RValue& Value
		) override final;

		virtual const YYRunnerInterface& GetRunnerInterface() override final;

		virtual void InvalidateAllCaches() override final;
	};

	inline YYTKInterfaceImpl g_ModuleInterface;
}

#endif // YYTK_INTERFACE_H_