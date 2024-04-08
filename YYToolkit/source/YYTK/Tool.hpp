#ifndef YYTK_TOOL_H_
#define YYTK_TOOL_H_

#include <Zydis/Zydis.h>
#include "Shared.hpp"

namespace YYTK
{
	using FNScriptData = CScript*(*)(
		IN int Index
	);

	using FNRoomData = CRoom*(*)(
		IN int Index
	);

	using FNGetVariable = bool(*)(
		OPTIONAL IN CInstance* Instance,
		OPTIONAL IN int Index, 
		OUT RValue* Value
	);

	using FNSetVariable = bool(*)(
		OPTIONAL IN CInstance* Instance,
		OPTIONAL IN int Index, 
		IN RValue* Value
	);

	struct RFunctionStringFull
	{
		char m_Name[64];
		TRoutine m_Routine;
		int32_t m_ArgumentCount;
		int32_t m_UsageCount;
	};
#ifdef _WIN64
	static_assert(sizeof(RFunctionStringFull) == 80);
#endif // _WIN64
	struct RFunctionStringRef
	{
		const char* m_Name;
		TRoutine m_Routine;
		int32_t m_ArgumentCount;
		int32_t m_UsageCount;
	};
#ifdef _WIN64
	static_assert(sizeof(RFunctionStringRef) == 24);
#endif // _WIN64

	struct RFunction
	{
		union
		{
			RFunctionStringRef ReferentialEntry;
			RFunctionStringFull FullEntry;
		};

		RFunctionStringFull& GetIndexFull(size_t Index)
		{
			return *reinterpret_cast<RFunctionStringFull*>(reinterpret_cast<char*>(this) + (sizeof(RFunctionStringFull) * Index));
		}

		RFunctionStringRef& GetIndexReferential(size_t Index)
		{
			return *reinterpret_cast<RFunctionStringRef*>(reinterpret_cast<char*>(this) + (sizeof(RFunctionStringRef) * Index));
		}
	};
#ifdef _WIN64
	static_assert(sizeof(RFunction) == 80);
#endif // _WIN64

	struct RVariableRoutine
	{
		const char* m_Name;
		FNGetVariable m_GetVariable;
		FNSetVariable m_SetVariable;
		bool m_CanBeSet;
	};
#ifdef _WIN64
	static_assert(sizeof(RVariableRoutine) == 32);
#endif // _WIN64

	struct TargettedInstruction
	{
		ZydisDisassembledInstruction RawForm;
		PVOID FunctionTarget;
	};

	struct ModuleCallbackDescriptor
	{
		Aurie::AurieModule* OwnerModule;
		EventTriggers Trigger;
		int32_t Priority;
		PVOID Routine;
	};
}

// Private includes
#include "Module Internals/Module Internals.hpp"
#include "Module Interface/Interface.hpp"

#endif // YYTK_TOOL_H_
