#include "UnitTests.hpp"
#include "../API/Internal.hpp"
#include "../../Utils/Logging/Logging.hpp"
#include "../../SDK/Structures/Documented/YYRValue/YYRValue.hpp"

bool Tests::RunUnitTests()
{
	bool bPassedTests = true;

	Utils::Logging::Message(CLR_DEFAULT, ""); // newline

	// Code_Execute
	{
		uintptr_t dwCodeExecute = 0;
		YYTKStatus stFoundCodeExecute = API::Internal::MmFindCodeExecute(dwCodeExecute);

		if (stFoundCodeExecute || dwCodeExecute == 0)
		{
			Utils::Logging::Error(__FILE__, __LINE__, "Unit Test \"Code_Execute\" failed");
			bPassedTests = false;
		}
	}

	// Code_Function_GET_the_function
	{
		uintptr_t dwCodeFunction = 0;
		YYTKStatus stFoundCodeFunction = API::Internal::MmFindCodeFunction(dwCodeFunction);

		if (stFoundCodeFunction || dwCodeFunction == 0)
		{
			Utils::Logging::Error(__FILE__, __LINE__, "Unit Test \"Code_Function_GET_the_function\" failed");
			bPassedTests = false;
		}
	}

	// Script Array
	{
#ifndef _WIN64
		CDynamicArray<CScript*>* pScriptArray = nullptr;
		YYTKStatus stFoundScriptArray = API::Internal::MmGetScriptArrayPtr(pScriptArray, 32);

		if (stFoundScriptArray || pScriptArray == nullptr)
		{
			Utils::Logging::Error(__FILE__, __LINE__, "Unit Test \"Script Array\" failed");
			bPassedTests = false;
		}
#endif
	}

	// Function Lookup Test
	{
		uintptr_t dwDirectPointer = 0;
		uintptr_t dwAsmRefPointer = 0;

		YYTKStatus stDirectCall = API::Internal::VfGetFunctionPointer("instance_destroy", FPType_DirectPointer, dwDirectPointer);
		YYTKStatus stAsmRefCall = API::Internal::VfGetFunctionPointer("instance_destroy", FPType_AssemblyReference, dwAsmRefPointer);

		if (stDirectCall || stAsmRefCall)
		{
			Utils::Logging::Error(__FILE__, __LINE__, "Unit Test \"Function Lookup (Status Check)\" failed");
			bPassedTests = false;
		}
			

		if (!dwDirectPointer || !dwAsmRefPointer)
		{
			Utils::Logging::Error(__FILE__, __LINE__, "Unit Test \"Function Lookup (Null Check)\" failed");
			bPassedTests = false;
		}
			

		if (dwDirectPointer != dwAsmRefPointer)
		{
			Utils::Logging::Error(__FILE__, __LINE__, "Unit Test \"Function Lookup (Equality Check)\" failed");
			bPassedTests = false;
		}
	}
	

	// Global Instance
	{
		uintptr_t dwGlobalInstFunction = 0;
		YYTKStatus stFoundFunc = API::Internal::VfGetFunctionPointer("@@GlobalScope@@", EFPType::FPType_DirectPointer, dwGlobalInstFunction);

		if (stFoundFunc || dwGlobalInstFunction == 0)
		{
			Utils::Logging::Error(__FILE__, __LINE__, "Unit Test \"Global Instance (Function Lookup)\" failed");
			bPassedTests = false;
		}
		else
		{
			TRoutine Routine = (TRoutine)dwGlobalInstFunction;
			
			YYRValue Value = YYRValue();
			Routine(&Value.As<RValue>(), nullptr, nullptr, 0, nullptr);

			if (Value.As<CInstanceBase*>() == nullptr)
			{
				Utils::Logging::Error(__FILE__, __LINE__, "Unit Test \"Global Instance (Result Check)\" failed");
				bPassedTests = false;
			}
		}
	}

	if (!bPassedTests)
	{
		int nUserChoice = MessageBoxA(
			0,
			"YYToolkit's self-check failed to pass.\n"
			"This means that some features may be broken or completely unavailable.\n"
			"Plugins relying on this functionality may cease to work.\n\n"
			"Proceed anyway?",
			"YYToolkit",
			MB_TOPMOST | MB_YESNO | MB_ICONWARNING | MB_SETFOREGROUND | MB_DEFBUTTON2
		);

		if (nUserChoice == IDNO)
			exit(0);
	}

	return bPassedTests;
}