#define YYSDK_PLUGIN    // This is needed so the SDK knows not to include core-specific headers. Always define it before including the SDK!
#include "SDK/SDK.hpp"  // Include the SDK.
#include <Windows.h>    // Include Windows's mess.
#include <vector>       // Include the STL vector.

YYRValue EasyGMLCall(YYTKPlugin* pPlugin, const std::string& Name, const std::vector<YYRValue>& rvRef)
{
	// Get the callbuiltin function from the core API
	auto CallBuiltin = GetCoreAPIAddress<bool(*)(YYRValue& Result,
		const std::string& Name,
		CInstance* Self,
		CInstance* Other,
		const std::vector<YYRValue>& Args)>(pPlugin, "CallBuiltin");

	// Call it like normal
	YYRValue Result;
	CallBuiltin(Result, Name, nullptr, nullptr, rvRef);

	return Result;
}

// I have to do this because there's some misalignment between older and newer DR versions
CCode* GetCodeFromScript(CScript* pScript)
{
	// If the script is invalid
	if (!pScript)
		return nullptr;

	// Old runner versions (pre DR 1.10 / DR 1.09)
	if (pScript->s_code)
		return pScript->s_code;

	// New runners have objects shifted up by 1 (due to the s_text member missing) so cast it to CCode and call it a day.
	if (pScript->s_text)
		return (CCode*)pScript->s_text;

	return nullptr;
}

YYTKStatus ScriptHandler(YYTKEventBase* pEvent, void* Argument1)
{
	// Convert the base event to the actual event object.
	// We only registered this callback for scripts, so we know it's that type.
	YYTKScriptEvent* pCodeEvent = dynamic_cast<YYTKScriptEvent*>(pEvent);

	// Get the plugin from Argument1, as made in the call.
	YYTKPlugin* pPlugin = reinterpret_cast<YYTKPlugin*>(Argument1);

	// Extract arguments from the tuple into individual objects. C++ rules apply.
	auto& [Script, a2, a3, a4, a5, a6] = pCodeEvent->Arguments();

	if (!Script)
		return YYTK_FAIL;

	CCode* pCode = GetCodeFromScript(Script);

	if (!pCode)
		return YYTK_FAIL;

	if (!pCode->i_pName)
		return YYTK_FAIL;

	if (strcmp(pCode->i_pName, "gml_Script_scr_debug") == 0 ||
		strcmp(pCode->i_pName, "gml_Script_scr_debug_ch1") == 0)
	{

		// By doing it this way, we avoid the crash with the cutscenes
		// which is something that UMT's Ch2 Debug.csx is currently suffering from.
		pCode->i_pVM = nullptr;
		YYRValue* pReturn = (YYRValue*)pCodeEvent->Call(Script, a2, a3, a4, a5, a6);
		*pReturn = 1.0;
	}
	else if (strcmp(pCode->i_pName, "gml_Script_scr_dogcheck") == 0 ||
		strcmp(pCode->i_pName, "gml_Script_scr_dogcheck_ch1") == 0)
	{
		// Just say we called it LOL
		pCode->i_pVM = nullptr;
		YYRValue* pReturn = (YYRValue*)pCodeEvent->Call(Script, a2, a3, a4, a5, a6);
		*pReturn = 0.0;
	}

	// Go To Room port
	if (GetAsyncKeyState(VK_F3) & 1)
	{
		YYRValue CurrentRoom = 30.0;

		YYRValue Result = EasyGMLCall(pPlugin, "get_integer", { "Go to room (ported by Archie from UMT to YYToolkit).\nEnter the room ID you wish to teleport to.", CurrentRoom });

		EasyGMLCall(pPlugin, "room_goto", { Result });
	}
	return YYTK_OK;
}

DllExport YYTKStatus PluginUnload(YYTKPlugin* pPlugin)
{
	EasyGMLCall(pPlugin, "variable_global_set", { "debug", 0.0 });

	return YYTK_OK;
}

// Create an entry routine - it has to be named exactly this, and has to accept these arguments.
// It also has to be declared DllExport (notice how the other functions are not).
DllExport YYTKStatus PluginEntry(YYTKPlugin* PluginObject)
{
	using FNPrintMessage = void(*)(const char* String, ...);
	using FNGetObjectAttributes = PluginAttributes_t*(*)(YYTKPlugin*);
	using FNCreateCallback = CallbackAttributes_t*(*)(PluginAttributes_t*, FNEventHandler, EventType, void*);
	using FNRemoveCallback = YYTKStatus(*)(CallbackAttributes_t* CallbackAttributes);

	FNPrintMessage PrintMessage = GetCoreAPIAddress<FNPrintMessage>(PluginObject, "PrintMessage");
	FNCreateCallback CreateCallback = GetCoreAPIAddress<FNCreateCallback>(PluginObject, "PmCreateCallback");
	FNGetObjectAttributes GetObjectAttributes = GetCoreAPIAddress<FNGetObjectAttributes>(PluginObject, "PmGetObjectAttributes");
	FNRemoveCallback RemoveCallback = GetCoreAPIAddress<FNRemoveCallback>(PluginObject, "PmRemoveCallback");

	if (!PrintMessage || !CreateCallback || !GetObjectAttributes)
		return YYTK_FAIL;

	auto Attributes = CreateCallback(GetObjectAttributes(PluginObject), (FNEventHandler)ScriptHandler, EVT_DOCALLSCRIPT, PluginObject);
	RemoveCallback(Attributes);

	PrintMessage("[Chapter2++] - Plugin loaded for YYTK version %s", YYSDK_VERSION);
	PrintMessage("[Chapter2++] - Please report any bugs you encounter on GitHub or the Underminers Discord.");
   
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