#define YYSDK_PLUGIN    // This is needed so the SDK knows not to include core-specific headers. Always define it before including the SDK!
#include "SDK/SDK.hpp"  // Include the SDK.
#include <Windows.h>    // Include Windows's mess.
#include "Bytecode.hpp" //

static YYTKStatus(*GetScriptByName)(const char* Name, CScript*& outScript);

static VMBuffer oscr_Debug = { 0 };
static VMBuffer oscr_Dogcheck = { 0 };
static VMBuffer oscr_DebugCh1 = { 0 };

// Helper function
void ReplaceWhileRunning(VMBuffer& Buffer, char* pNewBuffer, int nNewSize, YYTKScriptEvent* pCodeEvent)
{
    // Prepare variables with which the function was called.
    CScript* pScript; int argc; char* pStackPointer; VMExec* pVM; YYObjectBase* pLocals; YYObjectBase* pArguments;

    // Extract arguments from the tuple into individual objects. C++ rules apply.
    std::tie(pScript, argc, pStackPointer, pVM, pLocals, pArguments) = pCodeEvent->Arguments();

    // Save the original buffer
    Buffer = *pScript->s_code->i_pVM;

    // Modify the game buffer
    pScript->s_code->i_pVM->m_pBuffer = pNewBuffer;
    pScript->s_code->i_pVM->m_size = nNewSize;

    // Call the function
    pCodeEvent->Call(pScript, argc, pStackPointer, pVM, pLocals, pArguments);

    // Restore the original buffer
    pScript->s_code->i_pVM->m_pBuffer = Buffer.m_pBuffer;
    pScript->s_code->i_pVM->m_size = Buffer.m_size;
}

// Handles all events that happen inside the game.
// Previously, callbacks served this purpose, however in 0.0.3, an object-oriented design was implemented.
// If you want to modify a code entry, you're gonna need this function.
YYTKStatus PluginEventHandler(YYTKPlugin* pPlugin, YYTKEventBase* pEvent)
{
    // Check if the event currently raised is a rendering event (EndScene for DX9, Present for DX11)
    if (pEvent->GetEventType() == EventType::EVT_DOCALLSCRIPT)
    {
        // Convert the base event to the actual event object based on it's type.
        // Tip: Use dynamic_cast to catch issues with exceptions!
        YYTKScriptEvent* pCodeEvent = dynamic_cast<YYTKScriptEvent*>(pEvent);

        // Unpack the pScript variable from the tuple, ignore the rest.
        CScript* pScript;

        // Extract arguments from the tuple into individual objects. C++ rules apply.
        std::tie(pScript, std::ignore, std::ignore, std::ignore, std::ignore, std::ignore) = pCodeEvent->Arguments();

        // Check if values are valid
        if (pScript)
        {
            if (pScript->s_code)
            {
                if (strcmp(pScript->s_code->i_pName, "gml_Script_scr_debug") == 0)
                {
                    ReplaceWhileRunning(oscr_Debug, (char*)(g_scrDebugPatch), 72, pCodeEvent);
                }

                
                else if (strcmp(pScript->s_code->i_pName, "gml_Script_scr_dogcheck") == 0)
                {
                    ReplaceWhileRunning(oscr_Dogcheck, (char*)(g_scrDogcheckPatch), 164, pCodeEvent);
                }
                
                else if (strcmp(pScript->s_code->i_pName, "gml_Script_scr_debug_ch1") == 0)
                {
                    ReplaceWhileRunning(oscr_DebugCh1, (char*)(g_scrDebugChapter1Patch), 100, pCodeEvent);
                }
            }
        }

        // Go To Room port
        if (GetAsyncKeyState(VK_F3) & 1)
        {
            auto CallBuiltin = pPlugin->GetCoreExport<YYTKStatus(*)(const char* Name, int argc, YYRValue& _result, YYRValue* Args)>("Global_CallBuiltin");

            YYRValue Result, Result2;
            YYRValue Arguments[2] = { "Go to room (ported by Archie from UMT to YYToolkit)\nEnter the room ID you wish to teleport to.", 11.0 };

            CallBuiltin("get_integer", 2, Result, Arguments);

            CallBuiltin("room_goto", 1, Result2, &Result);
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

    printf("[Chapter2++] Loaded!\n");
    printf("[Chapter2++] - Press F3 to teleport to rooms (UndertaleModTool GoToRoom.csx script)\n");
    printf("[Chapter2++] - Dogcheck is still enabled, use F3 instead of save file editing!\n");

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