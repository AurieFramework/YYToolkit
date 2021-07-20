#include "UnitTests.hpp"
#include "SDK.hpp"
#include "../Features/AUMI_API/Exports.hpp"

namespace Utils::Tests
{
	int GetGlobalObjectTest()
	{
		YYTKTrace(__FUNCTION__ "()", __LINE__);

		// Correct call
		{
			YYObjectBase* g_pGlobal = nullptr;
			YYTKStatus Status = YYTK_OK;

			Status = AUMI_GetGlobalState(&g_pGlobal);

			if (!g_pGlobal || Status)
				return UT_GetGlobal_PointerFail;
		}
		
		// Incorrect call, should error
		YYTKStatus Status = YYTK_OK;
		Status = AUMI_GetGlobalState(nullptr);
		return Status != YYTK_OK ? UT_OK : UT_GetGlobal_NullFail;
	}

	int FunctionIndexTest()
	{
		YYTKTrace(__FUNCTION__ "()", __LINE__);

		// Correct call
		{
			AUMIFunctionInfo Info;
			YYTKStatus Status = YYTK_OK;

			Status = AUMI_GetFunctionByIndex(1, &Info);

			if (!Info.Function || Status)
				return UT_FuncIndex_PointerFail;
		}

		// Incorrect call, should error
		YYTKStatus Status = YYTK_OK;
		Status = AUMI_GetFunctionByIndex(-1, nullptr);
		return Status != YYTK_OK ? UT_OK : UT_FuncIndex_NullFail;
	}

	int FunctionPointerTest()
	{
		YYTKTrace(__FUNCTION__ "()", __LINE__);

		// Correct call
		{
			void* Function = nullptr;
			YYTKStatus Status = YYTK_OK;

			Status = AUMI_GetCodeExecuteAddress(&Function);

			if (!Function || Status)
				return UT_FuncPtr_CodeExec;
		}
		
		// Correct call
		{
			void* Function = nullptr;
			YYTKStatus Status = YYTK_OK;

			Status = AUMI_GetCodeFunctionAddress(&Function);

			if (!Function || Status)
				return UT_FuncPtr_CodeFunc;
		}

		return UT_OK;
	}
}