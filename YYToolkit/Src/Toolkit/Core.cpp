#include "Core.hpp"
#include "Utils/Error.hpp"
#include "Utils/SDK.hpp"
#include "Utils/UnitTests.hpp"
#include "Hooks/Hooks.hpp"
#include <Windows.h>

void Tool_Initialize()
{
	// Unit tests
	{
		if (int Result = Utils::Tests::FunctionIndexTest())
		{
			Utils::Error::Error(0, "Undefined behavior detected.\nError Code: %i", Result);
		}

		if (int Result = Utils::Tests::FunctionPointerTest())
		{
			Utils::Error::Error(0, "Undefined behavior detected.\nError Code: %i", Result);
		}

		if (int Result = Utils::Tests::GetGlobalObjectTest())
		{
			Utils::Error::Error(0, "Undefined behavior detected.\nError Code: %i", Result);
		}
	}

	// Initialize API
	{
		
	}

	// Create hooks
	{
		Hooks::Initialize();
	}
}
