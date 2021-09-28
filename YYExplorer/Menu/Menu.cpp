#include "Menu.hpp"
#include "Framework/Framework.hpp"

void Menu::ShowObjectTree()
{
	// Get all globals
	auto vGlobalObjects = Framework::Wrappers::GetVariableNames(-5);

    if (ImGui::TreeNode("Global Scope"))
    {

		for (auto& Name : vGlobalObjects)
		{
            if (ImGui::TreeNode(Name.c_str()))
            {
                ImGui::Text("Name: %s", Name.c_str());
				
				ImGui::TreePop();
            }
		}
        ImGui::TreePop();
    }

	ImGui::ShowDemoWindow();
}

void Menu::Draw()
{
	static bool bShowObjectTree = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("View"))
		{
			ImGui::MenuItem("Object Tree", nullptr, &bShowObjectTree);

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (bShowObjectTree)
		ShowObjectTree();
}