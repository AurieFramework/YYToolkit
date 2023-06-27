#include "menu.hpp"
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_stdlib.h>
#include <PFD/portable-file-dialogs.h>
#include "../fonts/orkney.hpp"
#include "../launch/launch.hpp"
#include "../codepage/codepage.hpp"
#include "../plugins/plugins.hpp"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

// Universal window flags, added to every window
constexpr auto window_flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

static ImVec2 viewport_size = ImVec2(0, 0);

void CMenu::set_style(GLFWwindow* window)
{
	ImGuiStyle& io = ImGui::GetStyle();

	// Main
	io.WindowPadding = { 8, 8 };
	io.FramePadding = { 4, 3 };
	io.CellPadding = { 4, 2 };
	io.ItemSpacing = { 8, 4 };
	io.ItemInnerSpacing = { 4, 4 };
	io.TouchExtraPadding = { 0, 0 };
	io.IndentSpacing = 20;
	io.ScrollbarSize = 14;
	io.GrabMinSize = 6;

	// Borders
	io.WindowBorderSize = 1;
	io.ChildBorderSize = 1;
	io.PopupBorderSize = 1;
	io.FrameBorderSize = 0;
	io.TabBorderSize = 0;

	// Rounding
	io.WindowRounding = 0;
	io.ChildRounding = 6;
	io.FrameRounding = 10;
	io.PopupRounding = 5;
	io.ScrollbarRounding = 0;
	io.GrabRounding = 10;
	io.LogSliderDeadzone = 4;
	io.TabRounding = 5;

	// Alignment
	io.WindowTitleAlign = { 0.5, 0.5 };
	io.WindowMenuButtonPosition = ImGuiDir_Left;
	io.ColorButtonPosition = ImGuiDir_Right;
	io.ButtonTextAlign = { 0.5, 0.5 };
	io.SelectableTextAlign = { 0, 0 };

	// Safe Area Padding
	io.DisplaySafeAreaPadding = { 3, 3 };
}

// Prepare ImGui for use
void CMenu::init(GLFWwindow* window)
{
	// Initialize ImGui
	printf("[~] Created ImGui context: %p\n", ImGui::CreateContext());
	ImGui::StyleColorsDark();

	ImGuiIO& io = ImGui::GetIO();

	// Disable the creation of INI files
	io.IniFilename = nullptr;

	if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
		printf("[-] Failed to init ImGui GLFW impl\n");

	if (!ImGui_ImplOpenGL3_Init())
		printf("[-] Failed to init OpenGL3\n");
	
	// Tell ImGui the font doesn't belong to the atlas
	// meaning we don't want to free it on exit (it'd segfault)
	ImFontConfig config;
	config.FontDataOwnedByAtlas = false;

	ImVector<ImWchar> font_ranges;
	ImFontGlyphRangesBuilder range_builder;

	// Add our required ranges
	range_builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
	range_builder.AddText(u8"ÌìŠšÈèØøŽžÝýÁáÍíÉéÏï");
	range_builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
	range_builder.BuildRanges(&font_ranges);
	
	io.Fonts->AddFontFromMemoryTTF(
		Roboto_Regular_ttf,
		Roboto_Regular_ttf_len,
		16,
		&config,
		font_ranges.Data
	);

	io.Fonts->Build();

	selected_version = version_tags.empty() ? "<failed to fetch>" : version_tags.front();

	main_window = window;
}

namespace ImGui
{
	bool CloseButton(ImGuiID id, const ImVec2& pos);
}

// Draw the TextInputs + Select and Close buttons
static void draw_runner_data_controls(std::filesystem::path& runner_filepath, std::filesystem::path& data_filepath, std::string& runner_filename, std::string& data_filename)
{
	// === Runner handling ===
	if (!runner_filepath.empty())
	{
		runner_filename = cp::unicode_to_codepage(CP_UTF8, runner_filepath.filename().wstring());
	}

	ImGui::AlignTextToFramePadding();

	// Draw the "Runner" text and the path input field
	{
		ImGui::Text("Runner");

		ImGui::SameLine(120);

		ImGui::SetNextItemWidth(viewport_size.x * 0.525f);
		ImGui::InputText("##InputRunner", &runner_filename, ImGuiInputTextFlags_ReadOnly);

		ImGui::SameLine();
	}

	// Draw the runner select & clear buttons
	if (ImGui::Button("Select##Runner", ImVec2(100, 0)))
	{
		auto dialog_results = pfd::open_file("Select runner", "", { "PE32 Executables", "*.exe", "All Files", "*" }).result();

		// There can only be one element in the vector
		// but this is more compact than checking if it's empty etc.
		for (const std::string& result : dialog_results)
		{
			std::wstring fixed_path = cp::codepage_to_unicode(CP_UTF8, result);
			runner_filepath = fixed_path;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Clear##Runner", ImVec2(100, 0)))
	{
		runner_filepath.clear();
	}

	// === data.win handling ===

	if (!data_filepath.empty())
	{
		data_filename = cp::unicode_to_codepage(CP_UTF8, data_filepath.filename().wstring());
	}

	ImGui::AlignTextToFramePadding();
	ImGui::Text("data.win");
	ImGui::SameLine(120);

	ImGui::SetNextItemWidth(viewport_size.x * 0.525f);
	ImGui::InputText("##InputData", &data_filename, ImGuiInputTextFlags_ReadOnly);

	ImGui::SameLine();

	if (ImGui::Button("Select##Data", ImVec2(100, 0)))
	{
		auto dialog_results = pfd::open_file("Select data.win", "", { "GameMaker WAD Files", "*.win *.unx *.ios", "All Files", "*" }).result();

		// There can only be one element in the vector
		// but this is more compact than checking if it's empty etc.
		for (const std::string& result : dialog_results)
		{
			std::wstring fixed_path = cp::codepage_to_unicode(CP_UTF8, result);
			data_filepath = fixed_path;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Clear##Data", ImVec2(100, 0)))
	{
		data_filepath.clear();
	}
}

// Render code
void CMenu::run(GLFWwindow* window)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	viewport_size = ImGui::GetMainViewport()->Size;

	// Draw the draggable window header
	// draw_window_header(window);

	// Set the "menu window" to be as big as possible
	ImGui::SetNextWindowSize(viewport_size);
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
	
	// Draw the main window inside the app
	if (ImGui::Begin(
		"##Main", 
		nullptr,
		window_flags | 
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
	))
	{
		// Draw the runner selection items

		std::string runner_filename = "<none selected>";
		std::string data_filename = "<using default>";

		draw_runner_data_controls(runner_filepath, data_filepath, runner_filename, data_filename);

		ImGui::Separator();

		// Used later when positioning the plugin manager
		float ypos_after_settings = ImGui::GetCursorPosY();

		ImGui::PushItemWidth(viewport_size.x / 3.5);

		// Draw releases combo
		{
			if (ImGui::BeginCombo("Preferred version", selected_version.c_str()))
			{
				for (const auto& tag : version_tags)
				{
					bool is_selected = (selected_version == tag);
					if (ImGui::Selectable(tag.c_str(), is_selected))
						selected_version = tag;

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		
		ImGui::InputInt("Injection delay (ms)", &injection_delay, 10, 100);

		ImGui::PopItemWidth();

		// Clamp the injection delay to something reasonable
		injection_delay = std::clamp(injection_delay, 0, 3000);

		static std::atomic<int> injection_progress = 0;
		static std::thread injection_thread_obj;

		ImGui::Spacing();

		if (ImGui::Button("Launch with YYToolkit##Main", { viewport_size.x / 2.1f, 0 }))
		{
			std::wstring arguments = L"";

			// Craft the arguments required for the runner (-game "/path/to/file with spaces.win")
			if (!data_filepath.empty())
				arguments = L"-game \"" + data_filepath.wstring() + L"\"";

			// Prepare arguments for the injection thread
			launch_info_t info;
			info.runner = runner_filepath;
			info.arguments = arguments;
			info.forced_tagname = selected_version;
			info.injection_delay = injection_delay;
			info.forced_dllname = "rtkmod.tmp";
			info.pid_override = pid_override;
			info.early_launch = use_early_launch;

			// Start the injection thread
			injection_thread_obj = std::thread(launch::do_full_launch, info, &injection_progress);

			// Make the injection progress window centered
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
				ImGuiCond_Always,
				ImVec2(0.5f, 0.5f)
			);

			// Open the actual injection progress window
			ImGui::OpenPopup("Injection progress");

			// Make the launcher topmost, so the game doesn't get in the way
			SetWindowPos(
				glfwGetWin32Window(window),
				HWND_TOPMOST,
				0, 0,
				0, 0,
				SWP_NOMOVE | SWP_NOSIZE
			);
		}

		if (ImGui::Button("Inject into a running process##Main", { viewport_size.x / 2.1f, 0 }))
		{
			MessageBoxA(0, "Not yet implemented!", "Error", MB_OK);
		}

		ImGui::Checkbox("Use early launch", &use_early_launch);

		// I want the popup to be rounded!
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10);
		{
			if (ImGui::BeginPopupModal(
				"Injection progress",
				NULL,
				ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize
			))
			{
				const std::map<int, std::string> load_stages = {
					std::make_pair(0, "Initializing..."),
					std::make_pair(1, "Fetching updates..."),
					std::make_pair(2, "Preparing environment..."),
					std::make_pair(3, "Downloading DLLs..."),
					std::make_pair(4, "Waiting for game..."),
					std::make_pair(5, "Injecting..."),
					std::make_pair(6, "Done!")
				};

				// Get the description of our current loading stage
				const char* current_load_stage = load_stages.at(injection_progress.load()).c_str();

				// Fetch the size of the text and the size of the window
				ImVec2 text_size = ImGui::CalcTextSize(current_load_stage);
				ImVec2 window_size = ImGui::GetWindowSize();

				// Calculate the center of the window
				float center_x = window_size.x / 2;

				// Offset it by half the text's size, such that the text's center is in the window's center
				center_x -= text_size.x / 2;

				// Draw the text
				ImGui::SetCursorPosX(center_x);
				ImGui::Text(current_load_stage);

				// Draw our progress bar centered - the injection thread is running at this point
				ImGui::ProgressBar((injection_progress.load() * 20.0f) / 100.f, ImVec2(0.0f, 0.0f), "");

				// If we're finished
				if (injection_progress.load() == 6)
				{
					// Reset our thread and progress
					injection_thread_obj.join();

					injection_progress.store(0);

					// Make the launcher no longer topmost
					SetWindowPos(
						glfwGetWin32Window(window),
						HWND_NOTOPMOST,
						0, 0,
						0, 0,
						SWP_NOMOVE | SWP_NOSIZE
					);

					// Close the popup
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}
		ImGui::PopStyleVar();

		// Return our cursor's YPos to be just after the first separator
		ImGui::SetCursorPosX(viewport_size.x * 0.5f);
		ImGui::SetCursorPosY(ypos_after_settings);

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0);
		{
			std::vector<plugin_item_t> plugins;
			
			if (!runner_filepath.empty())
				plugins = plugins::get_plugins_from_folder(runner_filepath.parent_path() / "autoexec");

			if (ImGui::BeginTable(
				"Plugin Table##Plugin Manager",
				2,
				ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY,
				{ 0, viewport_size.y - ypos_after_settings - 60 } // it's -30 if there's no buttons below it
			))
			{
				ImGui::TableSetupScrollFreeze(0, 1);
				ImGui::TableSetupColumn("Filename");
				ImGui::TableSetupColumn("Size");
				ImGui::TableHeadersRow();

				for (int row = 0; row < plugins.size(); row++)
				{
					ImGui::TableNextRow();
					for (int column = 0; column < 2; column++)
					{
						ImGui::TableSetColumnIndex(column);

						if (column == 0)
						{
							ImGui::Text("%s", cp::unicode_to_codepage(CP_UTF8, plugins.at(row).path.filename().wstring()).c_str());
						}
						else
						{
							ImGui::Text("%.2f kb", plugins.at(row).size_kb);
						}
					}
				}
				ImGui::EndTable();
			}
		}
		ImGui::PopStyleVar();

		ImGui::SetCursorPosX(viewport_size.x * 0.5f);
		if (ImGui::Button("Open plugin folder", { viewport_size.x * 0.49f, 0 }))
		{
			if (!runner_filepath.empty())
			{
				auto path = (runner_filepath.parent_path() / "autoexec");

				try
				{
					if (!std::filesystem::exists(path))
						std::filesystem::create_directory(path);
				}
				catch (std::exception& e)
				{
					std::string error_string = "Error while creating plugin folder: ";
					error_string.append(e.what());

					MessageBoxA(0, error_string.c_str(), "Error", MB_ICONERROR | MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
				}

				ShellExecuteW(NULL, L"open", path.wstring().c_str(), NULL, NULL, SW_SHOWDEFAULT);
			}
			else
			{
				MessageBoxA(0, "Please select a runner to manage plugins.", "No runner selected", MB_ICONERROR | MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
			}
		}
		
		ImGui::SetCursorPosX(viewport_size.x * 0.5f);
		if (ImGui::Button("Add plugin", { viewport_size.x * 0.24f, 0 }))
		{
			if (!runner_filepath.empty())
			{
				auto dialog_results = pfd::open_file("Select plugin", "", { "YYTK plugins", "*.dll" }).result();

				// There can only be one element in the vector
				// but this is more compact than checking if it's empty etc.
				for (const std::string& result : dialog_results)
				{
					std::filesystem::path original_path = cp::codepage_to_unicode(CP_UTF8, result);
					std::filesystem::path new_path = runner_filepath.parent_path() / "autoexec" / original_path.filename();

					std::error_code ec;
					std::filesystem::copy_file(original_path, new_path, ec);
					if (ec)
					{
						std::wstring error_string = L"Failed to install plugin: ";
						error_string.append(original_path.filename().wstring());

						MessageBoxW(
							0,
							error_string.c_str(),
							L"Error",
							MB_ICONERROR | MB_OK | MB_TOPMOST | MB_SETFOREGROUND
						);
						continue;
					}
				}
			}
			else
			{
				MessageBoxA(0, "Please select a runner to manage plugins.", "No runner selected", MB_ICONERROR | MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
			}
		}	

		ImGui::SameLine(0, viewport_size.x * 0.01f);

		if (ImGui::Button("Remove plugin", { viewport_size.x * 0.24f, 0 }))
		{
			if (!runner_filepath.empty())
			{
				auto path = (runner_filepath.parent_path() / "autoexec");

				auto dialog_results = pfd::open_file("Select plugin", cp::unicode_to_codepage(CP_UTF8, path.wstring()), { "YYTK plugins", "*.dll" }, pfd::opt::force_path).result();

				// There can only be one element in the vector
				// but this is more compact than checking if it's empty etc.
				for (const std::string& result : dialog_results)
				{
					// We shouldn't allow the user to remove plugins outside of the current game's folder
					std::filesystem::path target_path = cp::codepage_to_unicode(CP_UTF8, result);
					if (target_path.parent_path() != runner_filepath.parent_path() / "autoexec")
					{
						MessageBoxA(
							0,
							"This plugin is currently not installed.\nYou can only remove plugins in the game's plugin folder.",
							"Error",
							MB_ICONERROR | MB_OK | MB_TOPMOST | MB_SETFOREGROUND
						);
						continue;
					}

					std::error_code ec;
					std::filesystem::remove(target_path, ec);
				}
			}
			else
			{
				MessageBoxA(0, "Please select a runner to manage plugins.", "No runner selected", MB_ICONERROR | MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
			}
		}
	}

	ImGui::End();

	// ImGui::ShowDemoWindow();

	ImGui::Render();
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Called on app exit
void CMenu::destroy(GLFWwindow* window)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
}