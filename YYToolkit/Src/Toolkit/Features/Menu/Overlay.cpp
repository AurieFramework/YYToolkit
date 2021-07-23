#include "Menu.hpp"
#include "../../Utils/ImGui/imgui_internal.h"
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>

void Tool::Menu::Run()
{
	ImGui::SetNextWindowSize(ImVec2(550 * flGuiScale, 520 * flGuiScale));

	constexpr ImGuiWindowFlags Flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
	const static ImVec4 vTextColor = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

	const ImVec4& WindowColor = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
	const ImColor HoverColor = ImColor(0.086275f, 0.749020f, 1.0f, 0.8f);

	auto HoverBehavior = [](const ImVec4& color, bool highlight)
	{
		if (highlight)
		{
			ImGuiWindow* Window = ImGui::GetCurrentWindow();
			ImDrawList* DrawList = Window->DrawList;

			auto RectMin = ImGui::GetItemRectMin();
			auto RectMax = ImGui::GetItemRectMax();

			DrawList->AddRectFilled(ImVec2(RectMin.x + 4.0f, RectMax.y), ImVec2(RectMax.x - 4.0f, RectMax.y), ImColor(color), 2, ImDrawFlags_RoundCornersAll);
		}
	};

	auto DrawDelimiter = [HoverColor](float Thickness = 4.0f)
	{
		ImGuiWindow* Window = ImGui::GetCurrentWindow();
		ImDrawList* DrawList = Window->DrawList;

		ImVec2 WindowTop = Window->OuterRectClipped.Min;

		float Width = Window->OuterRectClipped.GetWidth();
		float Roundness = 10.0f;

		const ImDrawFlags DrawFlags = ImDrawFlags_RoundCornersTop;

		DrawList->AddRectFilled(WindowTop, ImVec2(WindowTop.x + Width, WindowTop.y + Thickness), HoverColor, Roundness, DrawFlags);
	};

	auto RenderWatermark = [Flags, DrawDelimiter]()
	{
		if (!Vars::bEnableWatermark)
			return;

		ImGui::SetNextWindowPos(ImVec2(10, 10));
		ImGui::SetNextWindowSize(ImVec2(140, 20));

		if (!ImGui::Begin("##Overlay", 0, Flags))
			return;

		DrawDelimiter(3);

		ImGui::PushFont(pSmallerFont);

		ImGui::Text("YYToolkit | FPS %.1f", ImGui::GetIO().Framerate);

		ImGui::PopFont();
		ImGui::End();
	};

	ImGui::PushStyleColor(ImGuiCol_Button, WindowColor); // No button edge
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, WindowColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, WindowColor);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
	
	RenderWatermark();

	if (!ImGui::Begin("MainMenu", 0, Flags | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		return ImGui::End();

	// Draw the window delimiter (the line at the top)
	{
		DrawDelimiter();
	}
	
	ImGui::BeginChild("##menuBar", ImVec2(550 * flGuiScale, 25 * flGuiScale), false, Flags);
	{
		// YYToolkit Header
		ImGui::PushFont(Menu::pBiggerFont);
		{
			ImGui::TextColored(ImVec4(0.101f, 0.404f, 0.98f, 1.0f), "YYToolkit");
			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
		}
		ImGui::PopFont();

		// Horizontal Tabs
		{
			if (ImGui::ButtonColored(vTextColor, "Undertale", ImVec2(0, 25 * flGuiScale)))
			{
				nTabPicked = 0;
				nSubtabPicked = 0;
			}
			HoverBehavior(HoverColor, nTabPicked == 0);

			ImGui::SameLine();

			if (ImGui::ButtonColored(vTextColor, "Deltarune", ImVec2(0, 25 * flGuiScale)))
			{
				nTabPicked = 1;
				nSubtabPicked = 0;
			}
			HoverBehavior(HoverColor, nTabPicked == 1);

			ImGui::SameLine();

			if (ImGui::ButtonColored(vTextColor, "Scripting", ImVec2(0, 25 * flGuiScale)))
			{
				nTabPicked = 2;
				nSubtabPicked = 0;
			}
			HoverBehavior(HoverColor, nTabPicked == 2);

			ImGui::SameLine();

			if (ImGui::ButtonColored(vTextColor, "Misc.", ImVec2(0, 25 * flGuiScale)))
			{
				nTabPicked = 3;
				nSubtabPicked = 0;
			}
			HoverBehavior(HoverColor, nTabPicked == 3);
		}

		ImGui::SameLine();
		ImGui::Dummy(ImVec2(42, 1));
		ImGui::SameLine();

		// Time Header
		{
			auto Time = std::time(nullptr);
			auto LocalTime = *std::localtime(&Time);
			std::ostringstream StringStream; StringStream << std::put_time(&LocalTime, "%H:%M:%S");

			ImGui::SameLine();
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
			ImGui::SameLine();
			ImGui::TextColored(vTextColor, "%s", StringStream.str().c_str());
		}
	}
	ImGui::EndChild();

	ImGui::Separator();

	ImGui::BeginChild("##subtabVBar", ImVec2(140 * flGuiScale, 460 * flGuiScale), false, Flags);
	{
		if (nTabPicked == 0) // Undertale
		{
			ImGui::ButtonColored(vTextColor, "Tweaks", ImVec2(0, 30));
			HoverBehavior(HoverColor, nSubtabPicked == 0);
			ImGui::ButtonColored(vTextColor, "Fixes", ImVec2(0, 30));
			HoverBehavior(HoverColor, nSubtabPicked == 1);
		}

		else if (nTabPicked == 1) // Deltarune
		{
			ImGui::ButtonColored(vTextColor, "General", ImVec2(0, 30));
			HoverBehavior(HoverColor, nSubtabPicked == 0);
		}

		else if (nTabPicked == 2) // Scripting
		{
			ImGui::ButtonColored(vTextColor, "Lua Scripts", ImVec2(0, 30));
			HoverBehavior(HoverColor, nSubtabPicked == 0);
			ImGui::ButtonColored(vTextColor, "ANSI C Scripts", ImVec2(0, 30));
			HoverBehavior(HoverColor, nSubtabPicked == 1);
		}

		else if (nTabPicked == 3) // Misc.
		{
			ImGui::ButtonColored(vTextColor, "Cheat", ImVec2(0, 30));
			HoverBehavior(HoverColor, nSubtabPicked == 0);
			ImGui::ButtonColored(vTextColor, "About", ImVec2(0, 30));
			HoverBehavior(HoverColor, nSubtabPicked == 1);
		}
	}
	ImGui::EndChild();
	
	ImGui::SameLine();

	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

	ImGui::SameLine();

	ImGui::ShowDemoWindow();

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(3);

	ImGui::End();
}