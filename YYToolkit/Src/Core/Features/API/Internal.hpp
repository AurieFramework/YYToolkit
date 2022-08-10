#pragma once
#include <Windows.h>
#include "../../SDK/FwdDecls/FwdDecls.hpp"
#include "../../SDK/Enums/Enums.hpp"

struct CModule
{
	uintptr_t Base;
	DWORD Size;
	uintptr_t EntryPoint;
};

// Function pointer types
enum EFPType : int
{
	FPType_AssemblyReference,
	FPType_DirectPointer,
};

namespace API
{
	extern CAPIVars gAPIVars;

	namespace Internal
	{
		// PRIVATE FUNCTIONS

		// Initializes completely, beginning to end - runs unit tests, etc.
		YYTKStatus __Initialize__(HMODULE hMainModule);

		// Initialize only the global variables (gAPIVars)
		YYTKStatus __InitializeGlobalVars__();

		// Opens up the console and prints the version to it
		YYTKStatus __InitializeConsole__();

		YYTKStatus __Unload__();

		// MEMORY MANAGER FUNCTIONS
		DllExport YYTKStatus MmGetModuleInformation(
			const char* szModuleName,
			CModule& outModule
		);

		DllExport YYTKStatus MmFindByteArray(
			const unsigned char* pbArray,
			size_t uArraySize,
			uintptr_t ulSearchRegionBase,
			uintptr_t ulSearchRegionSize,
			const char* szMask,
			bool bStringSearch,
			uintptr_t& dwOutBuffer
		);

		// Overloaded just so clang doesn't complain.
		YYTKStatus MmFindByteArray(
			const char* pszArray,
			size_t uArraySize,
			uintptr_t ulSearchRegionBase,
			uintptr_t ulSearchRegionSize,
			const char* szMask,
			bool bStringSearch,
			uintptr_t& dwOutBuffer
		);

		DllExport YYTKStatus MmFindCodeExecute(
			uintptr_t& dwOutBuffer
		);

		DllExport YYTKStatus MmFindCodeFunction(
			uintptr_t& dwOutBuffer
		);

		DllExport YYTKStatus MmGetScriptArrayPtr(
			CDynamicArray<CScript*>*& outArray,
			const int& nMaxInstructions
		);

		DllExport YYTKStatus MmGetScriptData(
			CScript*& outScript, 
			int index
		);

		// VARIABLE FUNCTIONS
		DllExport YYTKStatus VfGetFunctionPointer(
			const char* szFunctionName,
			EFPType ePointerType,
			uintptr_t& pOutBuffer
		);

		DllExport YYTKStatus VfGetFunctionEntryFromGameArray(
			int nIndex, // Required
			TRoutine* pOutRoutine, // Optional
			int* pOutArgumentCount, // Optional
			char** pOutNameBuffer // Optional
		);

		DllExport YYTKStatus VfLookupFunction(
			const char* szFunctionName,
			TRoutine& outRoutine,
			int* pOptOutIndex
		);

		DllExport YYTKStatus VfGetIdByName(
			YYObjectBase* pObject,
			const char* szName,
			int& outId
		);

		DllExport YYTKStatus VfGetAPIState(
			CAPIVars*& outState
		);
	}
}