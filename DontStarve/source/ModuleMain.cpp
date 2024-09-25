#include <map>
#include <cmath>
#include <YYToolkit/Shared.hpp>
using namespace Aurie;
using namespace YYTK;

static YYTKInterface* g_ModuleInterface = nullptr;
static int ari_hunger_value = 5;
static bool ari_is_hungry = false;
static bool is_tracked_time_interval = false;
static int time_of_last_tick = 0; // TODO: Set this on file load.
static int held_item_id = -1;
static std::string held_item_name;
static bool game_is_active = false;

static bool use_health_instead_of_stamina = false;
static int hunger_stamina_health_penatly = 1;

static std::map<std::string, int> item_name_to_stars_map = {
	// Forage
	{ "apple", 5 },
	{ "basil", 5 },
	{ "blackberry", 1 },
	{ "blueberry", 1 },
	{ "burdock_root", 8 },
	{ "cherry", 5 },
	{ "chestnut", 3 },
	{ "chickpea", 3 },
	{ "coconut", 6 },
	{ "dill", 5 },
	{ "fennel", 3 },
	{ "fiddlehead", 5 },
	{ "garlic", 6 },
	{ "glowberry", 1 },
	{ "horseradish", 3 },
	{ "ice_block", 6 },
	{ "jasmine", 3 },
	{ "lemon", 5 },
	{ "moon_fruit", 8 },
	{ "morel_mushroom", 5 },
	{ "nettle", 5 },
	{ "orange", 5 },
	{ "oregano", 5 },
	{ "oyster_mushroom", 5 },
	{ "peach", 5 },
	{ "pear", 5 },
	{ "pineshroom", 5 },
	{ "pomegranate", 5 },
	{ "rose", 3 },
	{ "rose_hip", 5 },
	{ "rosemary", 6 },
	{ "sage", 5 },
	{ "sesame", 5 },
	{ "thyme", 5 },
	{ "underseaweed", 5 },
	{ "water_chestnut", 3 },
	{ "wild_berries", 1 },
	{ "wild_grapes", 6 },
	{ "wild_leek", 3 },
	{ "wintergreen_berry", 3 },

	// Cooked Dishes
	{ "apple_honey_curry", 50 },
	{ "apple_juice", 10 },
	{ "apple_pie", 20 },
	{ "baked_potato", 10 },
	{ "baked_sweetroot", 10 },
	{ "beet_salad", 10 },
	{ "beet_soup", 50 },
	{ "berries_and_cream", 10 },
	{ "berry_bowl", 30 },
	{ "blackberry_jam", 10 },
	{ "blueberry_jam", 10 },
	{ "braised_burdock", 20 },
	{ "braised_carrots", 10 },
	{ "bread", 10 },
	{ "breaded_catfish", 20 },
	{ "broccoli_salad", 20 },
	{ "buttered_peas", 10 },
	{ "cabbage_slaw", 10 },
	{ "candied_lemon_peel", 10 },
	{ "candied_strawberries", 10 },
	{ "canned_sardines", 10 },
	{ "caramel_candy", 10 },
	{ "caramelized_moon_fruit", 10 },
	{ "cauliflower_curry", 30 },
	{ "cherry_cobbler", 10 },
	{ "cherry_smoothie", 20 },
	{ "cherry_tart", 20 },
	{ "chickpea_curry", 20 },
	{ "chili_coconut_curry", 40 },
	{ "chocolate_cake", 30 },
	{ "clam_chowder", 20 },
	{ "coconut_cream_pie", 40 },
	{ "coconut_milk", 10 },
	{ "cod_with_thyme", 20 },
	{ "crab_cakes", 40 },
	{ "cranberry_juice", 10 },
	{ "cranberry_orange_scone", 40 },
	{ "crayfish_etouffee", 20 },
	{ "crispy_fried_earthshroom", 20 },
	{ "crunchy_chickpeas", 10 },
	{ "crystal_berry_pie", 30 },
	{ "cucumber_salad", 20 },
	{ "cucumber_sandwich", 30 },
	{ "deep_sea_soup", 20 },
	{ "deluxe_curry", 30 },
	{ "deviled_eggs", 20 },
	{ "dried_squid", 10 },
	{ "fish_skewers", 10 },
	{ "fish_stew", 20 },
	{ "fish_tacos", 40 },
	{ "fried_rice", 50 },
	{ "garlic_bread", 10 },
	{ "gazpacho", 30 },
	{ "glowberry_cookies", 40 },
	{ "golden_cheesecake", 50 },
	{ "golden_cookies", 50 },
	{ "grape_juice", 10 },
	{ "green_tea", 10 },
	{ "grilled_cheese", 30 },
	{ "grilled_corn", 10 },
	{ "grilled_eel_rice_bowl", 30 },
	{ "hard_boiled_egg", 10 },
	{ "harvest_plate", 50 },
	{ "herb_butter_pasta", 30 },
	{ "herb_salad", 40 },
	{ "horseradish_salmon", 30 },
	{ "hot_chocolate", 30 },
	{ "ice_cream_sundae", 50 },
	{ "iced_coffee", 20 },
	{ "incredibly_hot_pot", 40 },
	{ "jam_sandwich", 20 },
	{ "jasmine_tea", 10 },
	{ "latte", 10 },
	{ "lemon_cake", 20 },
	{ "lemon_pie", 30 },
	{ "lemonade", 10 },
	{ "loaded_baked_potato", 30 },
	{ "lobster_roll", 40 },
	{ "mackerel_sashimi", 10 },
	{ "marmalade", 10 },
	{ "miner's_mushroom_stew", 10 },
	{ "mocha", 20 },
	{ "monster_cookies", 20 },
	{ "monster_mash", 30 },
	{ "mont_blanc", 50 },
	{ "moon_fruit_cake", 30 },
	{ "mushroom_rice", 20 },
	{ "mushroom_steak_dinner", 40 },
	{ "noodles", 10 },
	{ "omelet", 30 },
	{ "onion_soup", 20 },
	{ "orange_juice", 10 },
	{ "pan_fried_bream", 10 },
	{ "pan-fried_salmon", 10 },
	{ "pan-fried_snapper", 10 },
	{ "peaches_and_cream", 20 },
	{ "perch_risotto", 40 },
	{ "pineshroom_toast", 20 },
	{ "pizza", 20 },
	{ "poached_pear", 20 },
	{ "pomegranate_juice", 10 },
	{ "pomegranate_sorbet", 20 },
	{ "potato_soup", 20 },
	{ "pudding", 20 },
	{ "pumpkin_pie", 40 },
	{ "pumpkin_stew", 30 },
	{ "quiche", 20 },
	{ "red_snapper_sushi", 10 },
	{ "rice_ball", 20 },
	{ "roasted_cauliflower", 10 },
	{ "roasted_chestnuts", 10 },
	{ "roasted_rice_tea", 10 },
	{ "roasted_sweet_potato", 10 },
	{ "rose_hip_jam", 10 },
	{ "rose_tea", 10 },
	{ "rosemary_garlic_noodles", 30 },
	{ "salmon_sashimi", 10 },
	{ "salted_watermelon", 10 },
	{ "sauteed_snow_peas", 10 },
	{ "sea_bream_rice", 40 },
	{ "seafood_boil", 50 },
	{ "seafood_snow_pea_noodles", 50 },
	{ "seaweed_salad", 20 },
	{ "sesame_broccoli", 10 },
	{ "sesame_tuna_bowl", 30 },
	{ "simmered_daikon", 10 },
	{ "sliced_turnip", 10 },
	{ "smoked_trout_soup", 10 },
	{ "spell_fruit_parfait", 50 },
	{ "spicy_cheddar_biscuit", 30 },
	{ "spicy_corn", 20 },
	{ "spicy_crab_sushi", 30 },
	{ "spicy_water_chestnuts", 10 },
	{ "spring_galette", 50 },
	{ "spring_salad", 20 },
	{ "steamed_broccoli", 10 },
	{ "strawberries_and_cream", 10 },
	{ "strawberry_shortcake", 30 },
	{ "summer_salad", 30 },
	{ "sushi_platter", 50 },
	{ "sweet_potato_pie", 20 },
	{ "sweet_sesame_balls", 30 },
	{ "tea_with_lemon", 20 },
	{ "tide_salad", 30 },
	{ "toasted_sunflower_seeds", 20 },
	{ "tomato_soup", 20 },
	{ "trail_mix", 10 },
	{ "tuna_sashimi", 10 },
	{ "turnip_&_cabbage_salad", 20 },
	{ "turnip_&_potato_gratin", 30 },
	{ "vegetable_pot_pie", 40 },
	{ "vegetable_quiche", 40 },
	{ "vegetable_soup", 20 },
	{ "veggie_sub_sandwich", 50 },
	{ "water_chestnut_fritters", 20 },
	{ "wild_berry_jam", 10 },
	{ "wild_berry_pie", 20 },
	{ "wild_berry_scone", 20 },
	{ "winter_stew", 30 },
	{ "wintergreen_ice_cream", 20 },
};

bool GameIsPaused()
{
	CInstance* global_instance = nullptr;
	g_ModuleInterface->GetGlobalInstance(&global_instance);
	RValue paused = global_instance->at("__pause_status");
	return paused.m_i64 > 0;
}

void ObjectCallback(
	IN FWCodeEvent& CodeEvent
)
{
	auto& [self, other, code, argc, argv] = CodeEvent.Arguments();

	if (!self)
		return;

	if (!self->m_Object)
		return;

	if (!strstr(self->m_Object->m_Name, "obj_ari"))
		return;

	//--------------------------------------------------------
	// TODO: Move this to its own function.
	CInstance* global_instance = nullptr;
	g_ModuleInterface->GetGlobalInstance(&global_instance);

	CScript* script_object = nullptr;
	g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_modify_health@Ari@Ari",
		(PVOID*)&script_object
	);

	RValue* health_penalty = new RValue(-5.0);
	if (is_tracked_time_interval) {
		is_tracked_time_interval = false;
		if (ari_is_hungry) {
			RValue result;
			script_object->m_Functions->m_ScriptFunction(
				global_instance->at("__ari").m_Object,
				self,
				result,
				1,
				{ &health_penalty }
			);
		}
	}
	delete health_penalty;

	//--------------------------------------------------------

	if (use_health_instead_of_stamina) {
		use_health_instead_of_stamina = false;

		global_instance = nullptr;
		g_ModuleInterface->GetGlobalInstance(&global_instance);

		script_object = nullptr;
		g_ModuleInterface->GetNamedRoutinePointer(
			"gml_Script_modify_health@Ari@Ari",
			(PVOID*)&script_object
		);

		health_penalty = new RValue(-5.0);
		RValue result;
		script_object->m_Functions->m_ScriptFunction(
			global_instance->at("__ari").m_Object,
			self,
			result,
			1,
			{ &health_penalty }
		);
			
		delete health_penalty;
	}

}

RValue& GmlScriptGetMinutesCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_get_minutes"));

	if (game_is_active && time_of_last_tick == 0) {
		time_of_last_tick = 21600;
	}

	if (Arguments[0]->m_i64 % 1800 == 0 && !is_tracked_time_interval && (Arguments[0]->m_i64 - time_of_last_tick) >= 1800) {
		is_tracked_time_interval = true;
		time_of_last_tick = Arguments[0]->m_i64 + 0;
		if (ari_hunger_value > 0) {
			ari_hunger_value -= 5;
			ari_is_hungry = false;
		}
		else {
			ari_is_hungry = true;
		}
	}

	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptModifyStaminaCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_modify_stamina@Ari@Ari"));
	if (Arguments[0]->m_Real >= 0) {
		CScript* gml_script_item_id_to_name = nullptr;
		g_ModuleInterface->GetNamedRoutinePointer(
			"gml_Script_item_id_to_string",
			(PVOID*)&gml_script_item_id_to_name
		);

		RValue item_id_as_string;
		RValue* item_id = new RValue(held_item_id);
		gml_script_item_id_to_name->m_Functions->m_ScriptFunction(
			Self,
			Other,
			item_id_as_string,
			1,
			{ &item_id }
		);

		//g_ModuleInterface->Print(CM_YELLOW, "ItemIdToString: %d => %s", item_id, item_id_as_string.AsString().data());

		if (item_name_to_stars_map.count(item_id_as_string.AsString().data()) > 0)
		{
			ari_hunger_value += 2 * item_name_to_stars_map[item_id_as_string.AsString().data()]; // TODO: Testing 2x multiplier for balancing.
			if (ari_hunger_value > 100) {
				ari_hunger_value = 100;
			}
			g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Increased hunger meter by %d!", item_name_to_stars_map[item_id_as_string.AsString().data()]);
		}
		else {
			ari_hunger_value -= Arguments[0]->m_Real;
			g_ModuleInterface->Print(CM_LIGHTYELLOW, "[DontStarve] - Item not in hunger lookup dictionary: (%d) %s", item_id, item_id_as_string.AsString().data());
			g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Increased hunger meter by %d!", static_cast<int>(Arguments[0]->m_Real));

		}

		delete item_id;
	}
	else {
		//g_ModuleInterface->Print(CM_LIGHTYELLOW, "[DontStarve] - Stamina cost was: %d", static_cast<int>(Arguments[0]->m_Real));
		int temp = ari_hunger_value += Arguments[0]->m_Real;
		if (temp < 0) {
			use_health_instead_of_stamina = true;
			hunger_stamina_health_penatly = abs(temp);
			ari_hunger_value = 0;
			g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Hunger meter depleted! Decreased health by %d!", hunger_stamina_health_penatly);
		}
		else {
			ari_hunger_value += Arguments[0]->m_Real;
			if (ari_hunger_value < 0) {
				ari_hunger_value = 0;
			}
			g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Decreased hunger meter by %d!", static_cast<int>(Arguments[0]->m_Real));
		}
	}

	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptOnDrawGuiCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,	
	IN RValue** Arguments
)
{
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_on_draw_gui@Display@Display"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	if (game_is_active && !GameIsPaused())
	{
		// Hunger Bar Icon
		RValue hunger_bar_icon_sprite_index = g_ModuleInterface->CallBuiltin(
			"asset_get_index", {
				"spr_ui_hud_hunger_bar_icon"
			}
		);

		g_ModuleInterface->CallBuiltin(
			"draw_sprite", {
				hunger_bar_icon_sprite_index, 1, 10, 125 // Image is 32,32
			}
		);

		// Hunger Bar (Fill)
		g_ModuleInterface->CallBuiltin(
			"draw_set_color", {
			 4235519  // c_orange
			}
		);

		int x1 = 50 + 9;
		int y1 = 115 + 5;
		int x2 = x1 + ari_hunger_value * 2;
		int y2 = 115 + 40;
		g_ModuleInterface->CallBuiltin(
			"draw_rectangle", {
				x1, y1, x2, y2, false
			}
		);

		g_ModuleInterface->Print(CM_AQUA, "[DontStarve] - Orange Rectangle Coordinates: %d, %d, %d, %d", x1, y1, x2, y2);

		// Hunger Bar (Black)
		g_ModuleInterface->CallBuiltin(
			"draw_set_color", {
			 0  // c_black
			}
		);

		int _x1 = x2 + 1;
		int _y1 = 115 + 5;
		int _x2 = _x1 + ((100 - ari_hunger_value) * 2);
		int _y2 = 115 + 40;
		g_ModuleInterface->CallBuiltin(
			"draw_rectangle", {
				_x1, _y1, _x2, _y2, false
			}
		);

		g_ModuleInterface->Print(CM_AQUA, "[DontStarve] - Black Rectangle Coordinates: %d, %d, %d, %d", _x1, _y1, _x2, _y2);

		// Hunger Bar (Border)
		RValue hunger_bar_sprite_index = g_ModuleInterface->CallBuiltin(
			"asset_get_index", {
				"spr_ui_hud_hunger_bar"
			}
		);

		g_ModuleInterface->CallBuiltin(
			"draw_sprite", {
				hunger_bar_sprite_index, 1, 50, 115
			}
		);

		// Hunger Bar Label
		g_ModuleInterface->CallBuiltin(
			"draw_set_color", {
			 16777215  // c_white
			});

		g_ModuleInterface->CallBuiltin(
			"draw_set_font",
			{
				1
			}
		);

		g_ModuleInterface->CallBuiltin(
			"draw_text_transformed", {
				50 + 90, 115, std::to_string(ari_hunger_value) + "%", 3, 3, 0 // "Nutrition Level: " + std::to_string(hunger) + "%"
			}
		);
	}

	return Result;
}

RValue& GmlScriptHeldItemCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_held_item@Ari@Ari"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	if (Result.m_Kind != VALUE_UNDEFINED)
	{
		if (held_item_id != Result.at("item_id").m_i64)
		{
			held_item_id = Result.at("item_id").m_i64;
		}
	}

	return Result;
}

RValue& GmlScriptSetupMainScreenCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	game_is_active = false;
	time_of_last_tick = 0;
	ari_hunger_value = 5;

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_setup_main_screen@TitleMenu@TitleMenu"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptGetWeatherCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	game_is_active = true;

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_get_weather@WeatherManager@Weather"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& GmlScriptSetTimeCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	if (time_of_last_tick == 0)
	{
		time_of_last_tick = Arguments[0]->m_i64;
	}

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "gml_Script_set_time@Clock@Clock"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

RValue& TestHookCallback(
	IN CInstance* Self,
	IN CInstance* Other,
	OUT RValue& Result,
	IN int ArgumentCount,
	IN RValue** Arguments
)
{
	// end of day testing: gml_Script_end_day
	time_of_last_tick = 0;

	const PFUNC_YYGMLScript original = reinterpret_cast<PFUNC_YYGMLScript>(MmGetHookTrampoline(g_ArSelfModule, "TestHook"));
	original(
		Self,
		Other,
		Result,
		ArgumentCount,
		Arguments
	);

	return Result;
}

void CreateHookGmlScriptGetMinutes(AurieStatus &status)
{
	CScript* gml_script_get_minutes = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_get_minutes",
		(PVOID*)&gml_script_get_minutes
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Failed to get script (gml_Script_get_minutes)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_get_minutes",
		gml_script_get_minutes->m_Functions->m_ScriptFunction,
		GmlScriptGetMinutesCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Failed to hook script (gml_Script_get_minutes)!");
	}
}

void CreateHookGmlScriptModifyStamina(AurieStatus &status)
{
	CScript* gml_script_modify_stamina = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_modify_stamina@Ari@Ari",
		(PVOID*)&gml_script_modify_stamina
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Failed to get script (gml_Script_modify_stamina@Ari@Ari)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_modify_stamina@Ari@Ari",
		gml_script_modify_stamina->m_Functions->m_ScriptFunction,
		GmlScriptModifyStaminaCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Failed to hook script (gml_Script_modify_stamina@Ari@Ari)!");
	}
}

void CreateHookGmlScriptOnDrawGui(AurieStatus &status)
{
	CScript* gml_script_on_draw_gui = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_on_draw_gui@Display@Display",
		(PVOID*)&gml_script_on_draw_gui
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to get script (gml_Script_on_draw_gui@Display@Display)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_on_draw_gui@Display@Display",
		gml_script_on_draw_gui->m_Functions->m_ScriptFunction,
		GmlScriptOnDrawGuiCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to hook script (gml_Script_on_draw_gui@Display@Display)!");
	}
}

void CreateHookGmlScriptHeldItem(AurieStatus& status)
{
	CScript* gml_script_held_item = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_held_item@Ari@Ari",
		(PVOID*)&gml_script_held_item
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to get script (gml_Script_held_item@Ari@Ari)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_held_item@Ari@Ari",
		gml_script_held_item->m_Functions->m_ScriptFunction,
		GmlScriptHeldItemCallback,
		nullptr
	);


	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to hook script (gml_Script_held_item@Ari@Ari)!");
	}
}

void CreateHookGmlScriptSetupMainScreen(AurieStatus& status)
{
	CScript* gml_script_setup_main_screen = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_setup_main_screen@TitleMenu@TitleMenu",
		(PVOID*)&gml_script_setup_main_screen
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to get script (gml_Script_setup_main_screen@TitleMenu@TitleMenu)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_setup_main_screen@TitleMenu@TitleMenu",
		gml_script_setup_main_screen->m_Functions->m_ScriptFunction,
		GmlScriptSetupMainScreenCallback,
		nullptr
	);


	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to hook script (gml_Script_setup_main_screen@TitleMenu@TitleMenu)!");
	}
}

void CreateHookGmlScriptGetWeather(AurieStatus& status)
{
	CScript* gml_script_get_weather = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_get_weather@WeatherManager@Weather",
		(PVOID*)&gml_script_get_weather
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to get script (gml_Script_get_weather@WeatherManager@Weather)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_get_weather@WeatherManager@Weather",
		gml_script_get_weather->m_Functions->m_ScriptFunction,
		GmlScriptGetWeatherCallback,
		nullptr
	);


	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to hook script (gml_Script_get_weather@WeatherManager@Weather)!");
	}
}

void CreateHookGmlScriptSetTime(AurieStatus& status)
{
	CScript* gml_script_set_time = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		"gml_Script_set_time@Clock@Clock",
		(PVOID*)&gml_script_set_time
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to get script (gml_Script_set_time@Clock@Clock)!");
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"gml_Script_set_time@Clock@Clock",
		gml_script_set_time->m_Functions->m_ScriptFunction,
		GmlScriptSetTimeCallback,
		nullptr
	);


	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to hook script gml_Script_set_time@Clock@Clock)!");
	}
}

void CreateTestHook(AurieStatus& status, const char* script_name)
{
	CScript* gml_script = nullptr;
	status = g_ModuleInterface->GetNamedRoutinePointer(
		script_name,
		(PVOID*)&gml_script
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to get script (%s)!", script_name);
	}

	status = MmCreateHook(
		g_ArSelfModule,
		"TestHook",
		gml_script->m_Functions->m_ScriptFunction,
		TestHookCallback,
		nullptr
	);

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Failed to hook script (%s)!", script_name);
	}
}

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
)
{
	UNREFERENCED_PARAMETER(ModulePath);

	// Obtain the YYTK interface.
	AurieStatus status = AURIE_SUCCESS;

	status = ObGetInterface(
		"YYTK_Main",
		(AurieInterfaceBase*&)(g_ModuleInterface)
	);

	if (!AurieSuccess(status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	g_ModuleInterface->Print(CM_LIGHTGREEN, "[DontStarve] - Hello from PluginEntry!");

	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Failed to obtain YYTK interface!");
	}

	g_ModuleInterface->CreateCallback(
		g_ArSelfModule,
		EVENT_OBJECT_CALL,
		ObjectCallback,
		0
	);

	
	CreateHookGmlScriptGetMinutes(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptModifyStamina(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptOnDrawGui(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptHeldItem(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptSetupMainScreen(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptGetWeather(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Exiting due to failure on start!");
		return status;
	}

	CreateHookGmlScriptSetTime(status);
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Exiting due to failure on start!");
		return status;
	}

	CreateTestHook(status, "gml_Script_end_day");
	if (!AurieSuccess(status))
	{
		g_ModuleInterface->Print(CM_LIGHTRED, "[DontStarve] - Exiting due to failure on start!");
		return status;
	}

	return AURIE_SUCCESS;
}