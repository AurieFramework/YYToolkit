#pragma once
#define YYSDK_NODEFS
#include "../Utils/SDK.hpp"
#include <d3d11.h>
#include <d3d9.h>

namespace Hooks
{
	// I'm NOT making a cpp file for just this method...
	// Nevermind I did it
	void Initialize();

	void Uninitialize();
}