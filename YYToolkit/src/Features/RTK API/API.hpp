#ifndef RTK_API_API_H_
#define RTK_API_API_H_

#include <cstdint>
#include "../../SDK/Tool/Enums/Enums.hpp"

struct CRTKObject;
struct CRTKString;
struct CRTKPlugin;

namespace rtk
{
	// --- Core routines ---

	// Initializes the API - maps plugins to memory, initializes MinHook
	void CrInitializeAPI();

	// Uninitialize the API - unmaps plugins from memory, releases hooks
	void CrStopAPI();

	// --- Object Manager ---

	bool OmIsObjectType(CRTKObject* pObject, EObjectType ExpectedType);

	EStatus OmInitString(CRTKString* pString, const wchar_t* pBuffer);

	void OmFreeString(CRTKString* pString);

	// --- Memory Manager ---

	// Allocates a buffer of memory
	void* MmAllocateMemory(size_t Size);

	// Frees a buffer of memory allocated by MmAllocateMemory
	void MmFreeMemory(void* pBlock);

	// Creates and enables a MH hook
	void* MmCreateHook(void* pHook, void* pNative);

	// Removes and disables a MH hook
	bool MmRemoveHook(void* pHook);

	// --- Plugin Manager ---

	// Loads plugins from the autoexec folder
	EStatus PmInitialize();

	// Unloads all plugins
	EStatus PmUninitialize();

	// Maps a plugin into our address space, checks the major version
	EStatus PmLoadPlugin(CRTKString* pPath, int MajorVersionToCheck);

	// Unloads a plugin and calls it's PluginUnload if possible
	EStatus PmUnloadPlugin(void* pBaseAddress);

	// Calls PluginPreloads (PluginInitialize) routines on all loaded plugins
	void PmRunInitializeRoutines();

	// Calls PluginEntry routines on all loaded plugins
	void PmRunEntryRoutines();

	// --- Early Launch ---

	// TODO
}

#endif // RTK_API_API_H_