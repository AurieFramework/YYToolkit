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
    return YYTK_OK;
}

DllExport YYTKStatus PluginEntry(YYTKPlugin* pPlugin)
{
    g_pPlugin = pPlugin; // Keep a pointer to our plugin object, just in case we need it.
    g_pPlugin->Unload = FreeBuffer;
    g_pPlugin->Callbacks[CTIDX_Drawing] = DrawingCallback;

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