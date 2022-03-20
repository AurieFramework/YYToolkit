#pragma once
#include <Windows.h>
#include "../../SDK/FwdDecls/FwdDecls.hpp"
#include "../../SDK/Enums/Enums.hpp"

struct CModule
{
	DWORD Base;
	DWORD Size;
	DWORD EntryPoint;
};

// Function pointer types
enum EFPType : int
{
	FPType_AssemblyReference,
	FPType_DirectPointer,
};

// Lookup types
enum ELType : int
{
	LType_FunctionName,
	LType_Index,
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

		// Initialize preload-only features, like plugins
		YYTKStatus __InitializePreload__();

		YYTKStatus __Unload__();

		// MEMORY MANAGER FUNCTIONS
		DllExport YYTKStatus MmGetModuleInformation(
			const char* szModuleName,
			CModule& outModule
		);

		DllExport YYTKStatus MmFindByteArray(
			const unsigned char* pbArray,
			unsigned int uArraySize,
			unsigned long ulSearchRegionBase,
			unsigned int ulSearchRegionSize,
			const char* szMask,
			bool bStringSearch,
			DWORD& dwOutBuffer
		);

		// Overloaded just so clang doesn't complain.
		YYTKStatus MmFindByteArray(
			const char* pszArray,
			unsigned int uArraySize,
			unsigned long ulSearchRegionBase,
			unsigned int ulSearchRegionSize,
			const char* szMask,
			bool bStringSearch,
			DWORD& dwOutBuffer
		);

		DllExport YYTKStatus MmFindCodeExecute(
			DWORD& dwOutBuffer
		);

		DllExport YYTKStatus MmFindCodeFunction(
			DWORD& dwOutBuffer
		);

		// VARIABLE FUNCTIONS
		DllExport YYTKStatus VfGetFunctionPointer(
			const char* szFunctionName,
			EFPType ePointerType,
			DWORD& pOutBuffer
		);

		/// <summary>
		/// Gets the RFunction object of a builtin function by it's index. 
		/// This function is not to be used by plugins, as it is entirely game-dependant.
		/// Plugins should use GetFunctionByName or CallBuiltin.
		/// </summary>
		/// <param name="nIndex:">
		/// The index of the function.
		/// </param>
		/// <param name="pOutRoutine:">
		/// A pointer to a buffer which will receive a pointer to the routine. 
		/// This argument is optional.
		/// </param>
		/// <param name="pOutArgumentCount:">
		/// A pointer to a buffer which will receive the number of arguments required by the routine.
		/// This argument is optional.
		/// </param>
		/// <param name="pOutNameBuffer:">
		/// A pointer to a buffer which will receive the name of the routine. 
		/// This argument is optional.
		/// </param>
		/// <returns>
		/// Returns YYTK_INVALIDARG if one or more arguments are invalid.
		/// Returns YYTK_UNAVAILABLE if the Code_Function_GET_the_function isn't set.
		/// Returns YYTK_INVALIDRESULT if the index doesn't exist or the function has no name.
		/// Returns YYTK_OK on success.
		/// </returns>
		DllExport YYTKStatus VfGetFunctionEntryFromGameArray(
			int nIndex, // Required
			TRoutine* pOutRoutine, // Optional
			int* pOutArgumentCount, // Optional
			char** pOutNameBuffer // Optional
		);

		// Wrapper around VfGetFunctionEntryFromGameArray, loops until it found the matching name
		// Pretty much the internal 
		DllExport YYTKStatus VfLookupFunction(
			const char* szFunctionName,
			TRoutine& outRoutine,
			int* pOptOutIndex
		);

		DllExport YYTKStatus MmGetScriptArrayPtr(
			CDynamicArray<CScript*>*& outArray,
			const int& nMaxInstructions
		);
	}
}