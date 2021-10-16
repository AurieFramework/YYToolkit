#define YYSDK_PLUGIN    // This is needed so the SDK knows not to include core-specific headers. Always define it before including the SDK!
#include "SDK/SDK.hpp"  // Include the SDK.
#include <Windows.h>    // Include Windows's mess.
#include "Bytecode.hpp" // Include the bytecode header.
#include <vector>       // Include the STL vector.

static VMBuffer* poscr_Dogcheck = nullptr;
static VMBuffer oscr_DebugCh1 = { 0 };

YYRValue EasyGMLCall(YYTKPlugin* pPlugin, const std::string& Name, const std::vector<YYRValue>& rvRef)
{
    // Modify this to be const, so the compiler doesn't complain
    auto CallBuiltin = pPlugin->GetCoreExport<YYTKStatus(*)(const char* Name, int argc, YYRValue& _result, const YYRValue* Args)>("Global_CallBuiltin");

    YYRValue Result;
    CallBuiltin(Name.c_str(), rvRef.size(), Result, rvRef.data());

    return Result;
}

void SpoofScriptCall(double NewReturnValue, VMBuffer*& oldBuffer, YYTKScriptEvent* pCodeEvent)
{
    // Prepare variables with which the function was called.
    CScript* pScript; int argc; char* pStackPointer; VMExec* pVM; YYObjectBase* pLocals; YYObjectBase* pArguments;

    // Extract arguments from the tuple into individual objects. C++ rules apply.
    std::tie(pScript, argc, pStackPointer, pVM, pLocals, pArguments) = pCodeEvent->Arguments();
    
    // If the VM wasn't patched already
    if (pScript->s_code->i_pVM)
    {
        // Back-up VM
        oldBuffer = pScript->s_code->i_pVM;

        // Deactivate VM
        pScript->s_code->i_pVM = nullptr;
    }

    // Call original   
    auto pReturnValue = pCodeEvent->Call(pScript, argc, pStackPointer, pVM, pLocals, pArguments);

    // Get pointer to return value
    RValue* pValue = reinterpret_cast<RValue*>(&pReturnValue[-1]);


    if (pValue)
    {
        // Change value
        pValue->Real = NewReturnValue;
        pValue->Kind = VALUE_REAL;
        pValue->Flags = 0;
    }
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
        CScript* pScript = nullptr;

        // Extract arguments from the tuple into individual objects. C++ rules apply.
        std::tie(pScript, std::ignore, std::ignore, std::ignore, std::ignore, std::ignore) = pCodeEvent->Arguments();

        // Check if values are valid
        if (pScript)
        {
            if (pScript->s_code)
            {
                if (strcmp(pScript->s_code->i_pName, "gml_Script_scr_dogcheck") == 0)
                {
                    SpoofScriptCall(0.00, poscr_Dogcheck, pCodeEvent);
                }
                
                else if (strcmp(pScript->s_code->i_pName, "gml_Script_scr_debug") == 0)
                {
                    // Patch debug for ch2
                    EasyGMLCall(pPlugin, "variable_global_set", { "debug", 1.00 });
                }

                else if (strcmp(pScript->s_code->i_pName, "gml_Script_scr_debug_ch1") == 0)
                {
                    // Unpack the rest of the variables, since we need that.
                    int argc; char* pStackPointer; VMExec* pVM; YYObjectBase* pLocals; YYObjectBase* pArguments;

                    std::tie(std::ignore, argc, pStackPointer, pVM, pLocals, pArguments) = pCodeEvent->Arguments();

                    // Save the original buffer
                    oscr_DebugCh1 = *pScript->s_code->i_pVM;

                    // Modify the game buffer
                    pScript->s_code->i_pVM->m_pBuffer = (char*)(g_scrDebugChapter1Patch);
                    pScript->s_code->i_pVM->m_size = 100;

                    // Call the function
                    pCodeEvent->Call(pScript, argc, pStackPointer, pVM, pLocals, pArguments);
                }
            }
        }

        // Go To Room port
        if (GetAsyncKeyState(VK_F3) & 1)
        {
            YYRValue CurrentRoom = EasyGMLCall(pPlugin, "variable_global_get", { "currentroom" });

            YYRValue Result = EasyGMLCall(pPlugin, "get_integer", { "Go to room (ported by Archie from UMT to YYToolkit).\nEnter the room ID you wish to teleport to.", CurrentRoom });

            EasyGMLCall(pPlugin, "room_goto", { Result });
        }
    }
    return YYTK_OK;
}

DllExport YYTKStatus PluginUnload(YYTKPlugin* pPlugin)
{
    // Release properly
    if (poscr_Dogcheck)
    {
        // Get the script
        YYTKStatus(*GetScriptByName)(const char* Name, CScript * &outScript);
        GetScriptByName = pPlugin->GetCoreExport<decltype(GetScriptByName)>("GetScriptByName");

        CScript* Script = nullptr;
        GetScriptByName("scr_dogcheck", Script);

        if (!Script)
        {
            printf("[Chapter2++] - Failed to find scr_dogcheck!");
            return YYTK_FAIL;
        }

        // Patch the VM to the original
        Script->s_code->i_pVM = poscr_Dogcheck;
    }

    EasyGMLCall(pPlugin, "variable_global_set", { "debug", 0.0 });

    return YYTK_OK;
}

// Create an entry routine - it has to be named exactly this, and has to accept these arguments.
// It also has to be declared DllExport (notice how the other functions are not).
DllExport YYTKStatus PluginEntry(YYTKPlugin* pPlugin)
{
    // Set 'PluginEventHandler' as the function to call when we a game event happens.
    // This is not required if you don't need to modify code entries / draw with D3D / anything else that requires precise timing.
    pPlugin->PluginHandler = PluginEventHandler;
    pPlugin->PluginUnload = PluginUnload;
    
    printf("[Chapter2++] - Press F3 to teleport to rooms (UndertaleModTool GoToRoom.csx script)\n");
    printf("[Chapter2++] - Dogcheck disabled using Direct VM hook method!\n");
    printf("[Chapter2++] - Debug enabled for both chapters!\n");
    printf("[Chapter2++] - Loaded for version %s!\n", YYSDK_VERSION);

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