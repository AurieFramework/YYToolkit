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
            return YYTK_INVALIDARG;

        // Remove Save points
        if (_stricmp(Code->i_pName, "gml_Object_obj_savepoint_Create_0") == 0)
        {
            Features::RemoveSavePoints(pPlugin, Self);
        }

        // Remove healing from Save points
        else if (_stricmp(Code->i_pName, "gml_Object_obj_savepoint_Other_10") == 0)
        {
            // Backup the HP
            YYRValue BeforeSaveHP = Features::CallBuiltinWrapper(pPlugin, Self, "variable_global_get", { "hp" });

            // Run the game code
            pCodeEvent->Call(Self, Other, Code, Res, Flags);

            // Restore the HP
            Features::CallBuiltinWrapper(pPlugin, nullptr, "variable_global_set", { "hp", BeforeSaveHP });
        }

        // Change enemy statistics
        else if (_stricmp(Code->i_pName, "gml_Object_obj_battlecontroller_Create_0") == 0)
        {
            pCodeEvent->Call(Self, Other, Code, Res, Flags);

            printf("Snowgrave: %d\n", Features::IsSnowGraveRoute(pPlugin));

            switch (Features::IsSnowGraveRoute(pPlugin))
            {
            case 0: // No SnowGrave
                Features::ChangeEnemyStats(pPlugin, Self, 0.8, 1.5, 1.25);
                break;
            case 1: // First IceShock kill
                Features::ChangeEnemyStats(pPlugin, Self, 1.0, 1.6, 1.35);
                break;
            case 2: // Got FreezeRing - buff HP a lot
                Features::ChangeEnemyStats(pPlugin, Self, 0.5, 4.0, 2.5);
                break;
            case 3: // Killed Berdly
                Features::ChangeEnemyStats(pPlugin, Self, 1.2, 2.25, 2.6);
                break;
            default:
                break;
            }

            /*
            if (Features::IsSnowGraveRoute(pPlugin))
                Features::ChangeEnemyStats(pPlugin, Self, 0.6, 2.0, 1.8);
            else
                Features::ChangeEnemyStats(pPlugin, Self, 0.8, 1.5, 1.2);
            */
        }
    }

    return YYTK_OK;
}

DllExport YYTKStatus PluginUnload(YYTKPlugin* pPlugin)
{
    Features::CallBuiltinWrapper(pPlugin, nullptr, "gc_enable", { 1.0 });
    return YYTK_OK;
}

// Create an entry routine - it has to be named exactly this, and has to accept these arguments.
// It also has to be declared DllExport (notice how the other functions are not).
DllExport YYTKStatus PluginEntry(YYTKPlugin* pPlugin)
{
    Features::CallBuiltinWrapper(pPlugin, nullptr, "gc_enable", { 0.0 });

    // Set 'PluginEventHandler' as the function to call when we a game event happens.
    // This is not required if you don't need to modify code entries / draw with D3D / anything else that requires precise timing.
    pPlugin->PluginHandler = PluginEventHandler;
    pPlugin->PluginUnload = PluginUnload;

    using FNPrintFunc = void(*)(const char* String, ...);
    FNPrintFunc PrintMessage = pPlugin->GetCoreExport<FNPrintFunc>("PrintMessage");

    if (!PrintMessage)
        printf("[Chapter2 Hard Mode] - Loaded for YYTK version %s\n", YYSDK_VERSION);
    else
        PrintMessage("[Chapter2 Hard Mode] - Loaded for YYTK version %s", YYSDK_VERSION);

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