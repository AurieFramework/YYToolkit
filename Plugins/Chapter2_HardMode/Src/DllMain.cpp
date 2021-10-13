#include "Features/Features.hpp"
#include <Windows.h>    // Include Windows's mess.
#include <vector>       // Include the STL vector.

YYTKStatus PluginEventHandler(YYTKPlugin* pPlugin, YYTKEventBase* pEvent)
{
    if (pEvent->GetEventType() == EVT_CODE_EXECUTE)
    {
        YYTKCodeEvent* pCodeEvent = dynamic_cast<decltype(pCodeEvent)>(pEvent);

        auto& [Self, Other, Code, Res, Flags] = pCodeEvent->Arguments();

        if (!Code->i_pName)
            return YYTK_INVALID;

        if (_stricmp(Code->i_pName, "gml_Object_obj_savepoint_Create_0") == 0)
        {
            Features::RemoveSavePoints(pPlugin, Self);
        }

        else if (_stricmp(Code->i_pName, "gml_Object_obj_savepoint_Other_10") == 0)
        {
            // Backup the HP
            YYRValue BeforeSaveHP = Features::CallBuiltinWrapper(pPlugin, Self, "variable_global_get", { "hp" });

            // Run the game code
            pCodeEvent->Call(Self, Other, Code, Res, Flags);

            // Restore the HP
            Features::CallBuiltinWrapper(pPlugin, Self, "variable_global_set", { "hp", BeforeSaveHP });
        }

        // TODO: When Spamton NEO gets angry af, you gotta no-hit or PERISH.
    }

    return YYTK_OK;
}

DllExport YYTKStatus PluginUnload(YYTKPlugin* pPlugin)
{
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

    printf("[DR One Shot] Loaded for version %s", YYSDK_VERSION);

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