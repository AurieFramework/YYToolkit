#pragma once
#include "../../Hooks/Hooks.hpp" //g_pCurrentPlugin
#include "../../ImGui/imgui_cpp.hpp"
#include <vector>

namespace Framework
{
	namespace Wrappers
	{
		std::string RValueKind_ToString(const YYRValue& _Value);
		YYRValue GetGlobal(const char* _Name);
		void SetGlobal(const char* Name, const YYRValue& ref);
		std::vector<std::string> GetVariableNames(long InstanceID);
	}

	namespace GUI
	{
		constexpr auto WF_Basic = ImGuiWindowFlags_NoSavedSettings;
		void ApplyStylingDark();
		void StyleFonts(ImGuiIO& refIO);
	}
}