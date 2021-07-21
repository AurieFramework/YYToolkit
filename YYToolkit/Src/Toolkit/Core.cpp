#include "Core.hpp"
#include "Utils/Error.hpp"
#include "Utils/SDK.hpp"
#include "Utils/UnitTests.hpp"
#include "Features/AUMI_API/Exports.hpp"
#include "Hooks/Hooks.hpp"
#include <Windows.h>

void Tool_Initialize()
{
	YYTKTrace(__FUNCTION__ "()", __LINE__);

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

	// Initialize libraries
	{
		
	}

	// Create hooks
	{
		Hooks::Initialize();
	}
}
