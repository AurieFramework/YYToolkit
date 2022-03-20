#pragma once
#include "../../SDK/FwdDecls/FwdDecls.hpp"
#include "../../SDK/Structures/Documented/APIVars/APIVars.hpp"
#include <string>
#include <vector>

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
}