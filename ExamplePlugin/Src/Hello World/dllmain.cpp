#define YYSDK_PLUGIN    // This is needed so the SDK knows not to include core-specific headers. Always define it before including the SDK!
#include "SDK/SDK.hpp"  // Include the SDK.
#include <Windows.h>    // Include Windows's mess.

// The unload function doesn't have to be exported, but still has to return YYTKStatus and accept a YYTKPlugin argument.
YYTKStatus MyUnloadFunc(YYTKPlugin* pPlugin)
{
    // Create a message box
    MessageBoxA(0, "Goodbye, cruel world!", "An example plugin", MB_OK); 

    // Tell the core everything went fine.
    return YYTK_OK;
}

// Create an entry routine - it has to be named exactly this, and has to accept these arguments.
// It also has to be declared DllExport (notice how the other functions are not).
DllExport YYTKStatus PluginEntry(YYTKPlugin* pPlugin)
{
    // Set 'MyUnloadFunc' as the function to call when we get unloaded / unload. 
    // This is not required if you don't need to cleanup after yourself (free buffers, unhook stuff, etc.)
    pPlugin->PluginUnload = MyUnloadFunc; 

    // Create a message box
    MessageBoxA(0, "Hello, nice world!", "An example plugin", MB_OK); 

    // Tell the core everything went fine.
    return YYTK_OK;
}

// Boilerplate setup for a Windows DLL, can just return TRUE.
// This has to be here or else you get linker errors (unless you disable the main method)
BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    return 1;
}