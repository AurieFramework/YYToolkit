// File: YYTK_Shared_Interface.hpp
// 
// Defines the interface, and a GetInterface() function, which returns the YYTKInterface instance.

#include "YYTK_Shared_Base.hpp"
#include "YYTK_Shared_Types.hpp"
#include <FunctionWrapper/FunctionWrapper.hpp>
#include <d3d11.h>

namespace YYTK
{
	// ExecuteIt
	using FWCodeEvent = FunctionWrapper<bool(CInstance*, CInstance*, CCode*, int, RValue*)>;
	// IDXGISwapChain::Present
	using FWFrame = FunctionWrapper<HRESULT(IDXGISwapChain*, UINT, UINT)>;
	// IDXGISwapChain::ResizeBuffers
	using FWResize = FunctionWrapper<HRESULT(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT)>;
	// WndProc calls
	using FWWndProc = FunctionWrapper<LRESULT(HWND, UINT, WPARAM, LPARAM)>;

	struct YYTKInterface : public Aurie::AurieInterfaceBase
	{
		// === Interface Functions ===
		virtual Aurie::AurieStatus Create() = 0;

		virtual void Destroy() = 0;

		virtual void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		) = 0;

		virtual Aurie::AurieStatus GetNamedRoutineIndex(
			IN const char* FunctionName,
			OUT int* FunctionIndex
		) = 0;

		virtual Aurie::AurieStatus GetNamedRoutinePointer(
			IN const char* FunctionName,
			OUT PVOID* FunctionPointer
		) = 0;

		virtual Aurie::AurieStatus GetGlobalInstance(
			OUT CInstance** Instance
		) = 0;

		virtual RValue CallBuiltin(
			IN const char* FunctionName,
			IN std::vector<RValue> Arguments
		) = 0;

		virtual Aurie::AurieStatus CallBuiltinEx(
			OUT RValue& Result,
			IN const char* FunctionName,
			IN CInstance* SelfInstance,
			IN CInstance* OtherInstance,
			IN std::vector<RValue> Arguments
		) = 0;

		virtual void Print(
			IN CmColor Color,
			IN std::string_view Format,
			IN ...
		) = 0;

		virtual void PrintInfo(
			IN std::string_view Format,
			IN ...
		) = 0;

		virtual void PrintWarning(
			IN std::string_view Format,
			IN ...
		) = 0;

		virtual void PrintError(
			IN std::string_view Filepath,
			IN const int Line,
			IN std::string_view Format,
			IN ...
		) = 0;

		virtual Aurie::AurieStatus CreateCallback(
			IN Aurie::AurieModule* Module,
			IN EventTriggers Trigger,
			IN PVOID Routine,
			IN int32_t Priority
		) = 0;

		virtual Aurie::AurieStatus RemoveCallback(
			IN Aurie::AurieModule* Module,
			IN PVOID Routine
		) = 0;

		virtual Aurie::AurieStatus GetInstanceMember(
			IN RValue Instance,
			IN const char* MemberName,
			OUT RValue*& Member
		) = 0;

		virtual Aurie::AurieStatus EnumInstanceMembers(
			IN RValue Instance,
			IN std::function<bool(IN const char* MemberName, RValue* Value)> EnumFunction
		) = 0;

		virtual Aurie::AurieStatus RValueToString(
			IN const RValue& Value,
			OUT std::string& String
		) = 0;

		virtual Aurie::AurieStatus StringToRValue(
			IN const std::string_view String,
			OUT RValue& Value
		) = 0;

		virtual const YYRunnerInterface& GetRunnerInterface() = 0;

		virtual void InvalidateAllCaches() = 0;

		virtual Aurie::AurieStatus GetScriptData(
			IN int Index,
			OUT CScript*& Script
		) = 0;

		virtual Aurie::AurieStatus GetBuiltinVariableIndex(
			IN std::string_view Name,
			OUT size_t& Index
		) = 0;

		virtual Aurie::AurieStatus GetBuiltinVariableInformation(
			IN size_t Index,
			OUT RVariableRoutine*& VariableInformation
		) = 0;

		virtual Aurie::AurieStatus GetBuiltin(
			IN std::string_view Name,
			IN CInstance* TargetInstance,
			OPTIONAL IN int ArrayIndex,
			OUT RValue& Value
		) = 0;

		virtual Aurie::AurieStatus SetBuiltin(
			IN std::string_view Name,
			IN CInstance* TargetInstance,
			OPTIONAL IN int ArrayIndex,
			IN RValue& Value
		) = 0;

		virtual Aurie::AurieStatus GetArrayEntry(
			IN RValue& Value,
			IN size_t ArrayIndex,
			OUT RValue*& ArrayElement
		) = 0;

		virtual Aurie::AurieStatus GetArraySize(
			IN RValue& Value,
			OUT size_t& Size
		) = 0;

		virtual Aurie::AurieStatus GetRoomData(
			IN int32_t RoomID,
			OUT CRoom*& Room
		) = 0;

		virtual Aurie::AurieStatus GetCurrentRoomData(
			OUT CRoom*& CurrentRoom
		) = 0;

		virtual Aurie::AurieStatus GetInstanceObject(
			IN int32_t InstanceID,
			OUT CInstance*& Instance
		) = 0;

		virtual Aurie::AurieStatus InvokeWithObject(
			IN const RValue& Object,
			IN std::function<void(CInstance* Self, CInstance* Other)> Method
		) = 0;

		virtual Aurie::AurieStatus GetVariableSlot(
			IN const RValue& Object,
			IN const char* VariableName,
			OUT int32_t& Hash
		) = 0;
	};

	inline YYTKInterface* GetInterface()
	{
		using namespace Aurie;
		static YYTKInterface* module_interface = nullptr;

		// Try getting the interface
		// If we error, we return nullptr.
		if (!module_interface)
		{
			AurieStatus last_status = ObGetInterface(
				"YYTK_Main",
				reinterpret_cast<AurieInterfaceBase*&>(module_interface)
			);

			if (!AurieSuccess(last_status))
				printf("[%s : %d] FATAL: Failed to get YYTK Interface (%s)!\n", __FILE__, __LINE__, AurieStatusToString(last_status));
		}

		return module_interface;
	}
}


