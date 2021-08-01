#include "YYSDK.hpp"

YYTKStatus MyUnloadRoutine(YYTKPlugin* pPlugin)
{
    // Create a messagebox notifying the user that the plugin is unloading.
    MessageBoxA(0, "Goodbye, world!", "Example plugin is unloading", MB_OK | MB_ICONWARNING);

    FreeLibraryAndExitThread((HMODULE)pPlugin->PluginModule, 0); // Detach our plugin from the process.

    return YYTK_OK;
}

DllExport YYTKStatus PluginEntry(YYTKPlugin* pPlugin)
{
    // Create a messagebox notifying the user that the plugin has loaded successfully.
    MessageBoxA(0, "Hello, world!", "Example plugin is loaded", MB_OK | MB_ICONWARNING);

    pPlugin->Unload = MyUnloadRoutine; // Set up our unload routine, so when we wanna unload / get unloaded, the core knows how to handle it.

    return YYTK_OK;
}

// Boilerplate setup for a Windows DLL, can be empty.
BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    return 1;
}

