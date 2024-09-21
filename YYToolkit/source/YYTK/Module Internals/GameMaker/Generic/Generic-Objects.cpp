#include "../../Module Internals.hpp"

using namespace Aurie;

namespace YYTK
{
	// Finds the Code_Variable_FindAlloc_Slot_From_Name function
	AurieStatus GmpGetFindAllocSlotFromName(
		IN PFN_YYObjectBaseAdd YYObjectBase_Add,
		OUT	PFN_FindAllocSlot* FindAllocSlot
	)
	{
		// Code_Variable_FindAlloc_Slot_From_Name is usually the first call.
		// See comment above GmpGetYYObjectBaseAdd for decomp...

		// Make sure we have the required function
		if (!YYObjectBase_Add)
			return AURIE_UNAVAILABLE;

		// Disassemble 48 bytes at the function
		std::vector<TargettedInstruction> instructions = GmpDisassemble(
			YYObjectBase_Add,
			48,
			0xFF
		);

		// Find the first call, and trace the target
		ZyanU64 function_address = 0;

		for (const auto& instr : instructions)
		{
			if (instr.RawForm.info.mnemonic != ZYDIS_MNEMONIC_CALL)
				continue;

			ZydisCalcAbsoluteAddress(
				&instr.RawForm.info,
				&instr.RawForm.operands[0],
				instr.RawForm.runtime_address,
				&function_address
			);
			break;
		}

		if (!function_address)
			return AURIE_OBJECT_NOT_FOUND;

		*FindAllocSlot = reinterpret_cast<PFN_FindAllocSlot>(function_address);
		return AURIE_SUCCESS;
	}

	// Locates the YYObjectBase:Add function, works in both x86 and x64, VM and YYC
	// Rough decomp is this:
	/*
		void YYObjectBase::Add(
			IN const char* Name,
			IN RValue* Value,
			IN int Flags
		)
		{
			if ((this->m_Flags & 1) == 0)
				return;

			int variable_slot = Code_Variable_FindAlloc_Slot_From_Name(this, Name);

			RValue* variable_slot_ptr = nullptr;
			if (this->m_YYVars)
				variable_slot_ptr = &this->m_YYVars[variable_slot];
			else
				variable_slot_ptr = this->InternalGetYYVar(variable_slot);

			// Deep-copy RValue from Value to variable_slot_ptr
			// ...
		}

	*/
	AurieStatus GmpGetYYObjectBaseAdd(
		IN const YYRunnerInterface& Interface,
		OUT PFN_YYObjectBaseAdd* Function
	)
	{
		// Make sure we have the required function
		if (!Interface.StructAddRValue)
			return AURIE_UNAVAILABLE;

		// Disassemble 32 bytes at the function
		std::vector<TargettedInstruction> instructions = GmpDisassemble(
			Interface.StructAddRValue,
			32,
			0xFF
		);

		// Find either the first jump or the first call, and trace the target
		ZyanU64 function_address = 0;

		for (const auto& instr : instructions)
		{
			if (instr.RawForm.info.mnemonic != ZYDIS_MNEMONIC_JMP && instr.RawForm.info.mnemonic != ZYDIS_MNEMONIC_CALL)
				continue;

			ZydisCalcAbsoluteAddress(
				&instr.RawForm.info,
				&instr.RawForm.operands[0],
				instr.RawForm.runtime_address,
				&function_address
			);
			break;
		}

		if (!function_address)
			return AURIE_OBJECT_NOT_FOUND;

		*Function = reinterpret_cast<PFN_YYObjectBaseAdd>(function_address);
		return AURIE_SUCCESS;
	}

	AurieStatus GmpFindScriptData(
		IN const YYRunnerInterface& Interface,
		IN TRoutine CopyStatic,
		OUT FNScriptData* ScriptData
	)
	{
		if (!Interface.YYGetInt32)
			return AURIE_MODULE_INTERNAL_ERROR;

		if (!CopyStatic)
			return AURIE_MODULE_INTERNAL_ERROR;

		/*
			The F_CopyStatic function (registered as @@CopyStatic@@ in the runner) roughly decompiles to this:
			void F_CopyStatic(RValue& Result, CInstance* SelfInstance, CInstance* OtherInstance, int ArgumentCount, RValue* Arguments)
			{
				int32_t script_index = YYGetInt32(Arguments, 0);
				if (script_index >= 100000)
					script_index -= 100000;

				YYObjectBase* script_prototype = ScriptData(script_index)->m_Code->m_Prototype;
				if (script_prototype)
				{
					if (SelfInstance->m_Prototype)
						SelfInstance->m_Prototype->m_Prototype = script_prototype;
				}
			}

			We therefore know that the there are only two function calls, and we know the address of one of them.
			The following code disassembles F_CopyStatic, and looks for the first call that's not YYGetInt32.
			That will be our ScriptData() call.


			===== WARNING =====
			Some time after the 2024.1 release, F_CopyStatic has been changed to make the method no longer work!

			The function has changed to instead forcibly create a prototype if none exists, something like:
			void F_CopyStatic(RValue& Result, CInstance* SelfInstance, CInstance* OtherInstance, int ArgumentCount, RValue* Arguments)
			{
				int32_t script_index = YYGetInt32(Arguments, 0);
				if (script_index >= 100000)
					script_index -= 100000;

				// If no prototype exists, create one.
				YYObjectBase* prototype = g_pCurrentExec->pCCode->i_pPrototype;
				if ( !i_pPrototype )
				{
					i_pPrototype = Code_CreateStatic();

					// ...
				}

				CScript* script = ScriptData(script_index);
				// ...
			}

			This breaks the first-call technique, since that will now instead fall into Code_CreateStatic!
		*/

		// Disassemble 256 bytes at the function
		std::vector<TargettedInstruction> instructions = GmpDisassemble(
			CopyStatic,
			0x100,
			0xFF
		);

		for (auto& instruction : instructions)
		{
			ZydisDisassembledInstruction& raw_instruction = instruction.RawForm;

			// Skip any non-call instructions
			if (raw_instruction.info.mnemonic != ZYDIS_MNEMONIC_CALL)
				continue;

			// We're searching for call [visible_operand]
			// Note: this might not be necessary to check
			if (raw_instruction.info.operand_count_visible != 1)
				continue;

			// We're jumping to an immediate, not to a register
			if (raw_instruction.operands[0].type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
				continue;

			ZyanU64 call_address = 0;
			ZydisCalcAbsoluteAddress(
				&raw_instruction.info,
				&raw_instruction.operands[0],
				raw_instruction.runtime_address,
				&call_address
			);

			// Skip the first YYGetInt32 call
			if (call_address == reinterpret_cast<ZyanU64>(Interface.YYGetInt32))
				continue;

			// We have an unknown function that's called from F_CopyStatic and isn't YYGetInt32!
			// 
			// Analyze the first few instructions. There should be no more calls from ScriptData, 
			// but there are calls from all the other functions that falsely pass the above check.

			auto target_function_instructions = GmpDisassemble(
				reinterpret_cast<PVOID>(call_address),
				0x30,
				0xFFF
			);

			// If we find a call instruction anywhere...
			if (std::find_if(
				target_function_instructions.cbegin(),
				target_function_instructions.cend(),
				[](const TargettedInstruction& Instruction)
				{
					return Instruction.RawForm.info.mnemonic == ZYDIS_MNEMONIC_CALL;
				}
			) != std::cend(target_function_instructions))
			{
				// ... it's not our function.
				continue;
			}

			// Determine the call instruction's target
			*ScriptData = reinterpret_cast<FNScriptData>(call_address);
			break;
		}

		return AURIE_SUCCESS;
	}
}