#pragma once
#include <d3d11.h>

namespace Utils
{
	namespace D3D11
	{
		IDXGISwapChain* GetSwapChain();

		void* GetVMTEntry(int index);
	}
}