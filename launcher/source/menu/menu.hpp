#ifndef RTK_LAUNCHER_MENU_H_
#define RTK_LAUNCHER_MENU_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <filesystem>
#include "../network/network.hpp"

struct CMenu
{
public:
	static CMenu* get_instance()
	{
		static CMenu instance;

		return &instance;
	}

	void init(GLFWwindow* window);
	void run(GLFWwindow* window);
	void destroy(GLFWwindow* window);
	void set_style(GLFWwindow* window);

	CMenu(const CMenu&) = delete;
	void operator=(const CMenu&) = delete;

	GLFWwindow* main_window;
	std::vector<std::string> version_tags; // Stores all YYTK version tags, cached in main()
private:
	std::filesystem::path runner_filepath; // Absolute path to the game executable
	std::filesystem::path data_filepath; // Absolute path to the game data file - empty = use default

	int injection_delay; // Dictates how long to wait after the game is ready
	std::string selected_version; // Selected YYTK version tag
	int pid_override; // Optional override to inject into a running process

	bool use_early_launch;

	CMenu()
	{
		main_window = nullptr;
		injection_delay = 0;

		use_early_launch = false;

		runner_filepath.clear();
		data_filepath.clear();
	}
};

#endif // RTK_LAUNCHER_MENU_H_