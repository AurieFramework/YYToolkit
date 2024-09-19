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

		bool m_IsYYCRunner = false;

		// Set to true if VEH is used to get the runner interface.
		// If it is, the runner interface is unavailable during stage 1 init.
		bool m_IsUsingVeh = false;

		// A handle to an event which is signaled by the vectored exception handler.
		// Once signaled, the runner interface has been populated with values.
		HANDLE m_RunnerInterfacePopulatedEvent = nullptr;

		// The instruction pointer of the Extension_PrePrepare breakpoint.
		PVOID m_ExceptionRIP = nullptr;

		// Original bytes of Extension_PrePrepare's JS instruction.
		// Only used if VEH is used.
		std::vector<uint8_t> m_ExtensionPatchBytes;

		// The base address of the JS instruction.
		// Only used if VEH is used.
		PVOID m_ExtensionPatchBase;
	private:

		// A pointer to the functions array in memory
		RFunction** m_FunctionsArray = nullptr;

		// A pointer to the Script_Data() engine function.
		FNScriptData m_GetScriptData = nullptr;

		// A pointer to the Room_Data() engine function.
		FNRoomData m_GetRoomData = nullptr;

		// A pointer to the pointer to the running room
		CRoom** m_RunRoom = nullptr;

		// Cache used for lookups of builtin functions (room_goto, etc.)
		// key = name, value = function pointer
		std::map<std::string, TRoutine> m_BuiltinFunctionCache = {};

		// Cache used for lookups of builtin variables (xprevious, etc.)
		// key = name, value = index in the m_BuiltinArray
		std::map<std::string, size_t> m_BuiltinVariableCache = {};

		// D3D11 stuff
		IDXGISwapChain* m_EngineSwapchain = nullptr;
		HWND m_WindowHandle = nullptr;

		// The size of one entry in the RFunction array
		// GameMaker LTS still uses a 64-byte char array in the RFunction struct directly
		// New runners (2023.8) use a const char* in the array
		size_t m_FunctionEntrySize = 0;

		// Array of up to 500 builtins
		RVariableRoutine* m_BuiltinArray = nullptr;
		int* m_BuiltinCount = nullptr;

		// Needed for RValue array access
		// RValue* actual_array = (RValue**)(RValue.m_Pointer)[this_value / sizeof(RValue*)];
		int64_t m_RValueArrayOffset = 0;

		// Used to add or set a named value in a YYObjectBase structure.
		PFN_YYObjectBaseAdd m_AddToYYObjectBase = nullptr;

		// Used to turn a name into a hash to use in lookups in the internal hashmap.
		// While accessible through variable_struct_get_hash in newer runners,
		// some don't have the function.
		// In case the name isn't in the hashmap, the function allocates a new slot for it,
		// effectively creating the variable inside the object.
		PFN_FindAllocSlot m_FindAllocSlot = nullptr;
	public:
		// Stores plugin callbacks
		std::vector<ModuleCallbackDescriptor> m_RegisteredCallbacks;

		// === Internal functions ===
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
			IN ModuleCallbackDescriptor& Descriptor
		);

		ModuleCallbackDescriptor* YkFindDescriptor(
			IN const ModuleCallbackDescriptor& Descriptor
		);

		void YkRemoveCallbackFromList(
			IN Aurie::AurieModule* Module,
			IN PVOID Routine
		);

		bool YkCallbackExists(
			IN Aurie::AurieModule* Module,
			IN PVOID Routine
		);

		template <typename T>
		void YkDispatchCallbacks(
			IN EventTriggers Trigger,
			IN OUT FunctionWrapper<T>& Function
		)
		{
			// Calls all callbacks matching the Trigger

			for (auto& callback_descriptor : m_RegisteredCallbacks)
			{
				if (callback_descriptor.Trigger == Trigger)
					reinterpret_cast<void(*)(FunctionWrapper<T>&)>(callback_descriptor.Routine)(Function);
			}
		}

		Aurie::AurieStatus YkFetchD3D11Info(
			OUT ID3D11Device** DeviceObject,
			OUT IDXGISwapChain** Swapchain
		);

		size_t YkDetermineFunctionEntrySize();

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

		virtual Aurie::AurieStatus GetScriptData(
			IN int Index,
			OUT CScript*& Script
		) override final;

		virtual Aurie::AurieStatus GetBuiltinVariableIndex(
			IN std::string_view Name,
			OUT size_t& Index
		) override final;

		virtual Aurie::AurieStatus GetBuiltinVariableInformation(
			IN size_t Index,
			OUT RVariableRoutine*& VariableInformation
		) override final;

		virtual Aurie::AurieStatus GetBuiltin(
			IN std::string_view Name,
			IN CInstance* TargetInstance,
			OPTIONAL IN int ArrayIndex,
			OUT RValue& Value
		) override final;

		virtual Aurie::AurieStatus SetBuiltin(
			IN std::string_view Name,
			IN CInstance* TargetInstance,
			OPTIONAL IN int ArrayIndex,
			IN RValue& Value
		) override final;

		virtual Aurie::AurieStatus GetArrayEntry(
			IN RValue& Value,
			IN size_t ArrayIndex,
			OUT RValue*& ArrayElement
		) override final;

		virtual Aurie::AurieStatus GetArraySize(
			IN RValue& Value,
			OUT size_t& Size
		) override final;

		virtual Aurie::AurieStatus GetRoomData(
			IN int32_t RoomID,
			OUT CRoom*& Room
		) override final;

		virtual Aurie::AurieStatus GetCurrentRoomData(
			OUT CRoom*& CurrentRoom
		) override final;

		virtual Aurie::AurieStatus GetInstanceObject(
			IN int32_t InstanceID,
			OUT CInstance*& Instance
		) override final;

		virtual Aurie::AurieStatus InvokeWithObject(
			IN const RValue& Object,
			IN std::function<void(CInstance* Self, CInstance* Other)> Method
		) override final;

		virtual Aurie::AurieStatus GetVariableSlot(
			IN const RValue& Object,
			IN const char* VariableName,
			OUT int32_t& Hash
		) override final;
	};

	inline YYTKInterfaceImpl g_ModuleInterface;
}

#endif // YYTK_INTERFACE_H_