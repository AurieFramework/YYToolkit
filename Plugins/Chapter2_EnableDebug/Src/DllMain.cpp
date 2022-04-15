#define YYSDK_PLUGIN
#include "SDK/SDK.hpp"
#include <Windows.h>

static CallbackAttributes_t* pRegisteredCallback = nullptr;

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

// Automatically cast it to ScriptEvent, too lazy to do it
YYTKStatus ScriptCallback(YYTKScriptEvent* pEvent, void*)
{
	auto& [Script, v2, v3, v4, v5, v6] = pEvent->Arguments();

	if (!Script)
		return YYTK_INVALIDARG;

	CCode* pCode = GetCodeFromScript(Script);

	if (!pCode || !pCode->i_pName)
		return YYTK_INVALIDARG;

	if (!_stricmp(pCode->i_pName, "gml_Script_scr_debug") || !_stricmp(pCode->i_pName, "gml_Script_scr_debug_ch1"))
	{
		pCode->i_pVM = nullptr;
		YYRValue* pReturnValue = reinterpret_cast<YYRValue*>(pEvent->Call(Script, v2, v3, v4, v5, v6));

		if (!pReturnValue)
			return YYTK_INVALIDRESULT;

		*pReturnValue = 1.0;
	}

	else if (strstr(pCode->i_pName, "scr_dogcheck"))
	{
		pCode->i_pVM = nullptr;
		YYRValue* pReturnValue = reinterpret_cast<YYRValue*>(pEvent->Call(Script, v2, v3, v4, v5, v6));

		if (!pReturnValue)
			return YYTK_INVALIDRESULT;

		*pReturnValue = 0.0;
	}

	if (GetAsyncKeyState(VK_F3) & 1)
	{
		YYRValue CurrentRoom = 30.0;

		YYRValue Result, Result2;
		CallBuiltin(Result, "get_integer", nullptr, nullptr, { "Teleport to room (ported from UMT to YYTK)\nEnter the desired room ID.", CurrentRoom });

		CallBuiltin(Result2, "room_goto", nullptr, nullptr, { Result });
	}

	return YYTK_OK;
}

YYTKStatus PluginUnload()
{
	// oops forgot to remove this
	PmRemoveCallback(pRegisteredCallback);

	return YYTK_OK;
}

DllExport YYTKStatus PluginEntry(
	YYTKPlugin* PluginObject
)
{
	// Register a callback for script events
	PluginAttributes_t* pAttributes = nullptr;
	PmGetPluginAttributes(PluginObject, pAttributes);

	YYTKStatus CallbackStatus = PmCreateCallback(pAttributes, pRegisteredCallback, reinterpret_cast<FNEventHandler>(ScriptCallback), EVT_DOCALLSCRIPT, nullptr);
	
	if (CallbackStatus)
	{
		PrintError(__FILE__, __LINE__, "[Chapter 2 Debug] Creation of callback failed with 0x%X!", CallbackStatus);
		return YYTK_FAIL;
	}

	PrintMessage(CLR_DEFAULT, "[Chapter 2 Debug] Loaded for YYTK version %s", YYSDK_VERSION);

	return YYTK_OK;
}

BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
)
{
	return TRUE;
}