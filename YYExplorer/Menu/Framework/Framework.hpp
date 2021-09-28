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
		std::vector<std::string> GetVariableNames(long InstanceID);
	}

	namespace GUI
	{
		constexpr auto WF_Basic = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;
		void ApplyStylingDark();
		void StyleFonts(ImGuiIO& refIO);
	}
}