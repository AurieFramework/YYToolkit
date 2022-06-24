#include "../../../Dependencies/MinHook/MinHook.h"
#include "../RTK.hpp"
#include <memory>

namespace rtk
{
	void* MmAllocateMemory(size_t Size)
	{
		return operator new(Size, std::nothrow);
	}

	void MmFreeMemory(void* pBlock)
	{
		if (pBlock)
			operator delete(pBlock, std::nothrow);
	}

	void* MmCreateHook(void* pHook, void* pNative)
	{
		void* lpOriginal = nullptr;

		if (MH_CreateHook(pNative, pHook, &lpOriginal))
			return nullptr;

		if (MH_EnableHook(pNative))
			return nullptr;

		return lpOriginal;
	}

	bool MmRemoveHook(void* pHook)
	{
		if (MH_DisableHook(pHook))
			return false;

		if (MH_RemoveHook(pHook))
			return false;

		return true;
	}
}