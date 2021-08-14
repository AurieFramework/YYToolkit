<<<<<<< Updated upstream
#include "YYSDK.hpp"
#define IsStringEqual(x, y) _stricmp(x, y) == 0
#define ReplaceString(with) memset(g_Buffer, 0, 512); strcpy_s(g_Buffer, 512, with); str = g_Buffer;
static YYTKPlugin* g_pPlugin = nullptr;
static char Amogus[] = "Amogus";

void DrawingCallback(float& x, float& y, const char*& str, int& linesep, int& linewidth)
{
    if (strlen(str) > 1)
        str = Amogus;
}

YYTKStatus FreeBuffer(YYTKPlugin*)
{
=======
#define YYSDK_PLUGIN        // Tell the SDK that it's being loaded from a plugin
#include "SDK/SDK.hpp"      // Include the SDK
#include <Windows.h>        // Include Windows's functions

YYTKStatus MyUnload(YYTKPlugin* pPlugin) // The unload routine, doesn't have to be exported
{
    // Create a message box
    MessageBoxA(0, "Goodbye, cruel world!", "YYToolkit Plugin", MB_OK);

    // Tell the core everything went fine
>>>>>>> Stashed changes
    return YYTK_OK;
}

DllExport YYTKStatus PluginEntry(YYTKPlugin* pPlugin) // The init routine, HAS TO be exported
{
<<<<<<< Updated upstream
    g_pPlugin = pPlugin; // Keep a pointer to our plugin object, just in case we need it.
    g_pPlugin->Unload = FreeBuffer;
    g_pPlugin->Callbacks[CTIDX_Drawing] = DrawingCallback;
=======
    // Create a message box
    MessageBoxA(0, "Hello, World!", "YYToolkit Plugin", MB_OK);
    
    // Set up the unload routine, gets called on YYToolkit unload (pressing END)
    pPlugin->PluginUnload = MyUnload; 
>>>>>>> Stashed changes

    // Tell the core everything went fine
    return YYTK_OK;
}


// Boilerplate setup for a Windows DLL, we just return true to shut Windows up
BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    return TRUE;
}