#define YYSDK_PLUGIN    // This is needed so the SDK knows not to include core-specific headers. Always define it before including the SDK!
#include "SDK/SDK.hpp"  // Include the SDK.
#include <Windows.h>    // Include Windows's mess.

// Handles all events that happen inside the game.
// Previously, callbacks served this purpose, however in 0.0.3, an object-oriented design was implemented.
// If you want to modify a code entry, you're gonna need this function.
YYTKStatus PluginEventHandler(YYTKPlugin* pPlugin, YYTKEventBase* pEvent)
{
    // Check if the event currently raised is a code event
    if (pEvent->GetEventType() == EventType::EVT_CODE_EXECUTE)
    {
        // Convert the base event to the actual event object based on it's type.
        // Tip: Use dynamic_cast to catch issues with exceptions!
        YYTKCodeEvent* pCodeEvent = dynamic_cast<YYTKCodeEvent*>(pEvent);

        // Prepare variables with which the function was called.
        CInstance* Self; CInstance* Other; CCode* pCode; RValue* Args; int Flags;

        // Extract arguments from the tuple into individual objects. C++ rules apply.
        std::tie(Self, Other, pCode, Args, Flags) = pCodeEvent->Arguments();

        // Swap the arguments for Other and Self, effectively breaking any game.
        pCodeEvent->Call(Other, Self, pCode, Args, Flags);
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