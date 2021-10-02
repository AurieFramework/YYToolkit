#include "Menu.hpp"
#include "Framework/Framework.hpp"
#include <functional>
#include <iostream>

void Menu::Draw()
{
	static bool bShowConsole = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Windows"))
		{
			ImGui::MenuItem("Console", nullptr, &bShowConsole);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Game modifications"))
		{
			ImGui::MenuItem("DEFAULT STRING");

			ImGui::EndMenu();
		}
		
		ImGui::EndMainMenuBar();
	}

	if (bShowConsole)
		;
}