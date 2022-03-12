#pragma once
#include "../../SDK/SDK.hpp"
#include <vector>

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
	inline CAPIVars gAPIVars;

	/// <summary>
	/// GetFunctionByName returns a pointer to a GM builtin function.
	/// </summary>
	/// <param name="Name:">
	/// Controls which function will be found. 
	/// See GML documentation for further information.
	/// </param>
	/// <param name="outRoutine:">
	/// A pointer to a TRoutine object, will be assigned when the function returns.
	/// You may use this pointer to call the builtin function.
	/// </param>
	/// <returns>
	/// Returns false if the function wasn't found, true otherwise.
	/// </returns>
	DllExport bool GetFunctionByName(
		const std::string& Name,
		TRoutine& outRoutine
	);

	/// <summary>
	/// Retrieves the string representing the SDK version of the Core Module.
	/// </summary>
	/// <returns>
	/// The version string of the core module.
	/// Can be used to check if your plugin is compiled for the same version.
	/// </returns>
	DllExport const char* GetSDKVersion();

	/// <summary>
	/// Returns a pointer to the global game instance (globalvar).
	/// </summary>
	/// <param name="outInstance:">
	/// A reference to a CInstance pointer, which will be set upon the function returning.
	/// </param>
	/// <returns>
	/// Returns false if the lookup failed. Otherwise, it returns true.
	/// </returns>
	DllExport bool GetGlobalInstance(
		CInstance*& outInstance
	);

	/// <summary>
	/// [Deprecated] 
	/// This function checks if the current game is compiled with YYC.
	/// Plugins should use IsGameYYC() instead.
	/// </summary>
	/// <returns></returns>
	DllExport bool IsYYC();

	/// <summary>
	/// Checks if the game is compiled with YoYoCompiler (YYC).
	/// YYC compilation means that code is in x86 assembly instead of VM bytecode.
	/// </summary>
	/// <returns>
	/// Returns true only if the game is compiled with YYC. Otherwise, it returns false (even on error!).
	/// </returns>
	DllExport bool IsGameYYC();

	/// <summary>
	/// Calls a builtin function with custom arguments.
	/// </summary>
	/// <param name="Result:">
	/// A reference to a YYRValue, to which the return value is stored.
	/// </param>
	/// <param name="Name:">
	/// The name of the builtin (ex. room_goto, game_set_speed)
	/// </param>
	/// <param name="Self:">
	/// The instance 'self' points to - leave both Self and Other nullptr to use the global instance.
	/// </param>
	/// <param name="Other:">
	/// The instance 'other' points to - leave both Self and Other nullptr to use the global instance.
	/// </param>
	/// <param name="Args:">
	/// A vector of YYRValues used as parameters to the builtin function.
	/// The contents of this vector aren't modified.
	/// </param>
	/// <returns>
	/// The function returns false on error, true otherwise.
	/// </returns>
	DllExport bool CallBuiltin(
		YYRValue& Result,
		const std::string& Name,
		CInstance* Self,
		CInstance* Other,
		const std::vector<YYRValue>& Args
	);

	/// <summary>
	/// Finds an array of bytes in memory.
	/// </summary>
	/// <param name="Pattern:">
	/// The bytes to find.
	/// </param>
	/// <param name="Mask:">
	/// Masks the Pattern array. Use '?' as a wildcard for "any byte", 'x' for a fixed byte.
	/// Example: "xxx??xxx"
	/// </param>
	/// <param name="Base:">
	/// The base address for the search. If both Base and Size are 0, the entire memory is searched.
	/// </param>
	/// <param name="Size:">
	/// The size of the search. The search is done from (Base) up to (Base + Size).
	/// </param>
	/// <returns>
	/// Returns the base address of the array of bytes (if found), 0 otherwise.
	/// </returns>
	DllExport unsigned long FindPattern(
		const char* Pattern,
		const char* Mask,
		unsigned long Base,
		unsigned long Size
	);

	namespace Internal
	{
		// PRIVATE FUNCTIONS
		YYTKStatus Initialize(HMODULE hMainModule);

		YYTKStatus Unload();

		// MEMORY MANAGER FUNCTIONS
		DllExport YYTKStatus MmGetModuleInformation(
			const char* szModuleName,
			CModule& outModule
		);

		DllExport YYTKStatus MmFindByteArray(
			const byte* pbArray,
			unsigned int uArraySize,
			unsigned long ulSearchRegionBase,
			unsigned int ulSearchRegionSize,
			const char* szMask,
			DWORD& dwOutBuffer
		);

		// Overloaded just so clang doesn't complain.
		YYTKStatus MmFindByteArray(
			const char* pszArray,
			unsigned int uArraySize,
			unsigned long ulSearchRegionBase,
			unsigned int ulSearchRegionSize,
			const char* szMask,
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
	}
}