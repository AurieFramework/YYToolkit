#include "YYSDK.hpp"
#include <Windows.h>
#include <string>

static YYTKPlugin* g_pPlugin = nullptr;

void CodeCallback(CInstance*& pSelf, CInstance*& pOther, CCode*& Code, RValue*& Res, int& Flags)
{
    // If we're executing obj_time_Draw_64
    if (std::string(Code->i_pName).find("obj_time_Draw_64") != std::string::npos)
    {
        // Prepare all our variables, including GML functions
        YYRValue RoomString, Result;
        TRoutine room_get_name = g_pPlugin->LookupFunction<TRoutine(*)(const char*)>("GetBuiltin")("room_get_name");
        TRoutine variable_global_get = g_pPlugin->LookupFunction<TRoutine(*)(const char*)>("GetBuiltin")("variable_global_get");
        TRoutine draw_set_color = g_pPlugin->LookupFunction<TRoutine(*)(const char*)>("GetBuiltin")("draw_set_color");
        TRoutine draw_text = g_pPlugin->LookupFunction<TRoutine(*)(const char*)>("GetBuiltin")("draw_text");

        // Create our string
        g_pPlugin->LookupFunction<YYTKStatus(*)(RValue&, const char*)>("CreateString")(RoomString, "room");

        {
            RValue Args = 0xFFFF;
            draw_set_color(&Result, pSelf, pOther, 1, &Args);
        }

        variable_global_get(&Result, pSelf, pOther, 1, &RoomString);

        // draw_text(10, 30, room);
        {
            YYRValue Args[3];
            Args[0] = 10;
            Args[1] = 30;
            Args[2] = Result;
            draw_text(&Result, pSelf, pOther, 3, Args);
        }

        // draw_text(50, 30, room_get_name(room))
        {
            YYRValue Args[3];
            Args[0] = 50;
            Args[1] = 30;
            room_get_name(&Args[2], pSelf, pOther, 1, &Result); // room_get_name(room)
            draw_text(&Result, pSelf, pOther, 3, Args);
        }
    }
}

DllExport YYTKStatus PluginEntry(YYTKPlugin* pPlugin)
{
    g_pPlugin = pPlugin; // Keep a pointer to our plugin object, just in case we need it.
    g_pPlugin->Callbacks[CTIDX_CodeExecute] = CodeCallback;

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