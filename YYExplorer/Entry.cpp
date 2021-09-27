#include "Hooks/Hooks.hpp"
#ifndef YYSDK_PLUGIN
#define YYSDK_PLUGIN
#endif // YYSDK_PLUGIN

YYTKStatus EventHandler(YYTKPlugin* _Plugin, YYTKEventBase* _evt)
{
	// Handle all events properly
	switch (_evt->GetEventType())
	{
	case EVT_CODE_EXECUTE:
		YYTKCodeEvent* pCodeEvent = dynamic_cast<YYTKCodeEvent*>(_evt);
		CInstance* pSelf, *pOther; CCode* pCode; RValue* pArguments;
		
		std::tie(pSelf, pOther, pCode, pArguments, std::ignore) = pCodeEvent->Arguments();

		Hooks::Code_Execute(pCodeEvent, pSelf, pOther, pCode, pArguments);
		break;

	case EVT_DOCALLSCRIPT:
		YYTKScriptEvent* pScriptEvent = dynamic_cast<YYTKScriptEvent*>(_evt);
		CScript* pScript; int argc; char* pStackPointer; VMExec* pVM;

		std::tie(pScript, argc, pStackPointer, pVM, std::ignore, std::ignore) = pScriptEvent->Arguments();

		Hooks::DoCallScript(pScriptEvent, pScript, argc, pStackPointer, pVM);
		break;

	case EVT_PRESENT:
		YYTKPresentEvent* pPresentEvent = dynamic_cast<YYTKPresentEvent*>(_evt);
		IDXGISwapChain* pSwapChain; UINT Sync, Flags;

		std::tie(pSwapChain, Sync, Flags) = pPresentEvent->Arguments();

		Hooks::Present(pPresentEvent, pSwapChain, Sync, Flags);
		break;

	case EVT_ENDSCENE:
		YYTKEndSceneEvent* pEndSceneEvent = dynamic_cast<YYTKEndSceneEvent*>(_evt);
		break;

	case EVT_WNDPROC:
		YYTKWindowProcEvent* pWindowProcEvent = dynamic_cast<YYTKWindowProcEvent*>(_evt);
		break;

	default:
		break;
	}

	return YYTK_OK;
}

YYTKStatus PluginUnload(YYTKPlugin* _Plugin)
{
	return YYTK_OK;
}

DllExport YYTKStatus PluginEntry(YYTKPlugin* _Plugin)
{
	_Plugin->PluginUnload = PluginUnload;
	_Plugin->PluginHandler = EventHandler;

	return YYTK_OK;
}