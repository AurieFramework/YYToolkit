#define YYSDK_PLUGIN    // This is needed so the SDK knows not to include core-specific headers. Always define it before including the SDK!
#include "SDK/SDK.hpp"  // Include the SDK.
#include <Windows.h>    // Include Windows's mess.

static constexpr unsigned char g_PatchBytes[]
{ 
    0x04, 0x00, 0x00, 0xB6, 
    0x01, 0x00, 0x0F, 0x84, 
    0x00, 0x00, 0x52, 0x07, 
    0x00, 0x00, 0x05, 0x9C, 
    0x00, 0x00, 0x02, 0xC0, 
    0xB7, 0x8A, 0x01, 0x00, 
    0x00, 0x00, 0x52, 0x07, 
    0xFF, 0xFF, 0x0F, 0x84, 
    0x00, 0x00, 0x52, 0x07, 
    0x02, 0x00, 0x02, 0xD9, 
    0xA0, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x05, 0x86, 
    0xFF, 0xFF, 0x0F, 0x84, 
    0x00, 0x00, 0x55, 0x45, 
    0xFE, 0xB0, 0x01, 0x80, 
    0x00, 0x00, 0x05, 0x9E 
};

// Handles all events that happen inside the game.
// Previously, callbacks served this purpose, however in 0.0.3, an object-oriented design was implemented.
// If you want to modify a code entry, you're gonna need this function.
YYTKStatus PluginEventHandler(YYTKPlugin* pPlugin, YYTKEventBase* pEvent)
{
    // Check if the event currently raised is a code event
    if (pEvent->GetEventType() == EventType::EVT_DOCALLSCRIPT)
    {
        // Convert the base event to the actual event object based on it's type.
        // Tip: Use dynamic_cast to catch issues with exceptions!
        YYTKScriptEvent* pCodeEvent = dynamic_cast<YYTKScriptEvent*>(pEvent);

        // Prepare variables with which the function was called.
        CScript* pScript; int argc; char* pStackPointer; VMExec* pVM; YYObjectBase* pLocals; YYObjectBase* pArguments;

        // Extract arguments from the tuple into individual objects. C++ rules apply.
        std::tie(pScript, argc, pStackPointer, pVM, pLocals, pArguments) = pCodeEvent->Arguments();

        // Check if values are valid
        if (pScript)
        {
            if (pScript->s_code)
            {
                // Check if the code entry name is called scr_debug
                if (strcmp(pScript->s_code->i_pName, "gml_Script_scr_debug") == 0)
                {
                    // Modify the buffer
                    pScript->s_code->i_pVM->m_pBuffer = (char*)(g_PatchBytes);
                    pScript->s_code->i_pVM->m_size = 64;
                }
            }
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

    printf("[DR Chapter 2 Debug Enabler] I'm about to do what's called a pro-gamer move.\n");

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