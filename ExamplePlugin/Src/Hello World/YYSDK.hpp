#pragma once
#define WIN32_LEAN_AND_MEAN // Exclude all the Windows bloat that they bundle with Windows.h
#include <Windows.h>
#pragma warning(disable : 26812) // Warnings begone! (enum X is unscoped)

#define DllExport extern "C" __declspec(dllexport)
// Forward declaractions
struct YYTKPlugin;
struct FUNCTION_ENTRY;

enum YYTKStatus : int
{
	YYTK_OK = 0,				// The operation completed successfully.
	YYTK_FAIL = 1,				// Unspecified error occured, see source code.
	YYTK_UNAVAILABLE = 2,		// The called function is not available in the current context.
	YYTK_NO_MEMORY = 3,			// No more memory is available to the process.
	YYTK_NOT_FOUND = 4,			// The specified value could not be found.
	YYTK_NOT_IMPLEMENTED = 5,	// The specified function doesn't exist. (IPC error)
	YYTK_INVALID = 6			// One or more arguments were invalid.
};

constexpr int CALLBACK_TABLE_MAX_ENTRIES = 4;
constexpr int FUNCTION_TABLE_MAX_ENTRIES = 11;

// Indices into the CALLBACK_TABLE array
constexpr int CTIDX_CodeExecute = 0;
constexpr int CTIDX_EndScene = 1;
constexpr int CTIDX_Present = 2;
constexpr int CTIDX_Drawing = 3;

using PLUGIN_UNLOAD = YYTKStatus(*)(YYTKPlugin* pPlugin);
using PLUGIN_ENTRY = YYTKStatus(*)(YYTKPlugin* pPlugin);
using CALLBACK_TABLE = void*[CALLBACK_TABLE_MAX_ENTRIES];
using FUNCTION_TABLE = FUNCTION_ENTRY[FUNCTION_TABLE_MAX_ENTRIES];

struct FUNCTION_ENTRY
{
	const char* Name;
	void* Routine;
};

struct YYTKPlugin
{
	const char* Path;
	PLUGIN_ENTRY Entry;
	PLUGIN_UNLOAD Unload;
	CALLBACK_TABLE Callbacks;
	FUNCTION_TABLE* Functions;
	void* PluginModule;

	YYTKPlugin() = delete;
	
	template <typename Fn>
	inline Fn LookupFunction(const char* Name)
	{
		for (int n = 0; n < FUNCTION_TABLE_MAX_ENTRIES; n++)
		{
			if (_stricmp(Name, Functions[n]->Name) == 0)
				return (Fn)Functions[n]->Routine;
		}

		return nullptr;
	}
};

typedef void (*TPluginDrawTextRoutine)(float& x, float& y, const char*& str, int& linesep, int& linewidth);