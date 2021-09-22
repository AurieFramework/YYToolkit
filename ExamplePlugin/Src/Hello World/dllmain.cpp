#define YYSDK_PLUGIN    // This is needed so the SDK knows not to include core-specific headers. Always define it before including the SDK!
#include "SDK/SDK.hpp"  // Include the SDK.
#include <Windows.h>    // Include Windows's mess.

bool bDebug = true;

// Handles all events that happen inside the game.
// Previously, callbacks served this purpose, however in 0.0.3, an object-oriented design was implemented.
// If you want to modify a code entry, you're gonna need this function.
YYTKStatus PluginEventHandler(YYTKPlugin* pPlugin, YYTKEventBase* pEvent)
{
    // Check if the event currently raised is a rendering event (EndScene for DX9, Present for DX11)
    if (pEvent->GetEventType() == EventType::EVT_ENDSCENE || pEvent->GetEventType() == EventType::EVT_PRESENT)
    {
        // If F3 was pressed
        if (GetAsyncKeyState(VK_F3) & 1)
        {
            auto CBF = pPlugin->GetCoreExport<YYTKStatus(*)(CInstance*, CInstance*, YYRValue&, int, const char*, YYRValue*)>("CallBuiltinFunction");

            YYRValue Result = 0.0; YYRValue Args[2] = { "debug", bDebug };

            CBF(0, 0, Result, 1, "variable_global_set", Args);

            printf("[DR Chapter 2 ToggleDebug] Debug mode %s!\n", bDebug ? "Enabled" : "Disabled");

            bDebug = !bDebug;
        }
    }
    return YYTK_OK;
}

// Create an entry routine - it has to be named exactly this, and has to accept these arguments.
// It also has to be declared DllExport (notice how the other functions are not).
DllExport YYTKStatus PluginEntry(YYTKPlugin* pPlugin)
{
    // Set 'PluginEventHandler' as the function to call when we a game event happens.
    // This is not required if you don't need to modify code entries / draw with D3D / anything else that requires precise timing.
    pPlugin->PluginHandler = PluginEventHandler;

    auto CBF = pPlugin->GetCoreExport<YYTKStatus(*)(CInstance*, CInstance*, YYRValue&, int, const char*, YYRValue*)>("CallBuiltinFunction");

    YYRValue Result = 0.0; YYRValue Args[2] = { "debug", true };

    CBF(0, 0, Result, 1, "variable_global_set", Args);

    printf("[DR Chapter 2 ToggleDebug] Let there be debug!\n");

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