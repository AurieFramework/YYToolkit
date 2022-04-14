#include "Features/Features.hpp"
#include <Windows.h>    // Include Windows's mess.

YYTKStatus CodeHandler(YYTKEventBase* pEvent, void*)
{
	YYTKCodeEvent* pCodeEvent = dynamic_cast<decltype(pCodeEvent)>(pEvent);

	auto& [Self, Other, Code, Res, Flags] = pCodeEvent->Arguments();

	if (!Code->i_pName)
		return YYTK_INVALIDARG;

	// Remove Save points
	if (_stricmp(Code->i_pName, "gml_Object_obj_savepoint_Create_0") == 0)
	{
		Features::RemoveSavePoints(Self);
	}

	// Remove healing from Save points
	else if (_stricmp(Code->i_pName, "gml_Object_obj_savepoint_Other_10") == 0)
	{
		// Backup the HP
		YYRValue BeforeSaveHP = Features::CallBuiltinWrapper(Self, "variable_global_get", { "hp" });

		// Run the game code
		pCodeEvent->Call(Self, Other, Code, Res, Flags);

		// Restore the HP
		Features::CallBuiltinWrapper(nullptr, "variable_global_set", { "hp", BeforeSaveHP });
	}

	// Change enemy statistics
	else if (_stricmp(Code->i_pName, "gml_Object_obj_battlecontroller_Create_0") == 0)
	{
		pCodeEvent->Call(Self, Other, Code, Res, Flags);

		printf("Snowgrave: %d\n", Features::GetSnowGraveProgression());

		switch (Features::GetSnowGraveProgression())
		{
		case 0: // No SnowGrave
			Features::ChangeEnemyStats(Self, 0.8, 1.5, 1.25);
			break;
		case 1: // First IceShock kill
			Features::ChangeEnemyStats(Self, 1.0, 1.6, 1.35);
			break;
		case 2: // Got FreezeRing - buff HP a lot
			Features::ChangeEnemyStats(Self, 0.5, 4.0, 2.5);
			break;
		case 3: // Killed Berdly
			Features::ChangeEnemyStats(Self, 1.2, 2.25, 2.6);
			break;
		default:
			break;
		}
	}

	return YYTK_OK;
}

static CallbackAttributes_t* pRegisteredCallback = nullptr;

YYTKStatus PluginUnload()
{
	if (pRegisteredCallback)
		PmRemoveCallback(pRegisteredCallback);

	Features::CallBuiltinWrapper(nullptr, "gc_enable", { 1.0 });
	return YYTK_OK;
}

DllExport YYTKStatus PluginEntry(YYTKPlugin* pPlugin)
{
	Features::CallBuiltinWrapper(nullptr, "gc_enable", { 0.0 });
	pPlugin->PluginUnload = PluginUnload;

	PrintMessage(CLR_DEFAULT, "[Chapter 2 Hard Mode] Loaded for YYTK %s!", YYSDK_VERSION);

	PluginAttributes_t* MyAttributes = nullptr;
	PmGetPluginAttributes(pPlugin, MyAttributes);

	YYTKStatus CallbackSuccess = PmCreateCallback(MyAttributes, pRegisteredCallback, CodeHandler, EVT_CODE_EXECUTE, nullptr);

	if (CallbackSuccess)
	{
		PrintError(__FILE__, __LINE__, "[Chapter 2 Hard Mode] Creation of callback failed with 0x%X!", CallbackSuccess);
		return YYTK_FAIL;
	}

	return YYTK_OK;
}

BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	return 1;
}