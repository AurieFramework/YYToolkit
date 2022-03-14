#include "UnitTests.hpp"
#include "../API/API.hpp"
#include "../../Utils/Error/Error.hpp"

bool Tests::RunUnitTests()
{
	bool bPassedTests = true;

	// Code_Execute
	{
		DWORD dwCodeExecute = 0;
		YYTKStatus stFoundCodeExecute = API::Internal::MmFindCodeExecute(dwCodeExecute);

		if (stFoundCodeExecute || dwCodeExecute == 0)
		{
			Utils::Error::Error(0, ""); // todo
			bPassedTests = false;
		}
	}

	// Code_Function_GET_the_function
	{
		DWORD dwCodeFunction = 0;
		YYTKStatus stFoundCodeFunction = API::Internal::MmFindCodeFunction(dwCodeFunction);

		if (stFoundCodeFunction || dwCodeFunction == 0)
			bPassedTests = false;
	}

	// Script Array
	{
		CDynamicArray<CScript*>* pScriptArray = nullptr;
		YYTKStatus stFoundScriptArray = API::Internal::MmGetScriptArrayPtr(pScriptArray, 32);

		if (stFoundScriptArray || pScriptArray == nullptr)
			bPassedTests = false;
	}

	// Function Lookup Test
	{
		DWORD dwDirectPointer = 0;
		DWORD dwAsmRefPointer = 0;

		YYTKStatus stDirectCall = API::Internal::VfGetFunctionPointer("instance_destroy", FPType_DirectPointer, dwDirectPointer);
		YYTKStatus stAsmRefCall = API::Internal::VfGetFunctionPointer("instance_destroy", FPType_AssemblyReference, dwAsmRefPointer);

		if (stDirectCall || stAsmRefCall)
			bPassedTests = false;

		if (!dwDirectPointer || !dwAsmRefPointer)
			bPassedTests = false;

		if (dwDirectPointer != dwAsmRefPointer)
			bPassedTests = false;
	}
	
	return bPassedTests;
}
