// Defines
#define WIN32_LEAN_AND_MEAN
#define GLFW_EXPOSE_NATIVE_WIN32
#define CURL_STATICLIB

// Libraries
#pragma comment(lib, "opengl32.lib")	// OpenGL for GLFW
#pragma comment(lib, "glfw3_mt.lib")	// GLFW
#pragma comment(lib, "libcurl_a.lib")	// Libcurl
#pragma comment(lib, "wldap32.lib")		// LDAP stuff
#pragma comment(lib, "crypt32.lib")		// Cryptography stuff
#pragma comment(lib, "ws2_32.lib")		// WinSock stuff
#pragma comment(lib, "normaliz.lib")	// International domain names to text

#pragma comment(linker, "/NODEFAULTLIB:LIBCMT") // Disable linking of linux functions lol

// Includes
#include <Windows.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "menu/menu.hpp"

constexpr int window_x = 720;
constexpr int window_y = 320;

int main()
{
	// Create the window
	if (!glfwInit())
		return 1;

	printf("[~] GLFW initialized successfully\n");

	if (curl_global_init(CURL_GLOBAL_DEFAULT))
		return 1;

	printf("[~] cURL initialized successfully\n");

	CURL* curl_handle = curl_easy_init();

	if (!curl_handle)
		return 0;

	printf("[~] Got cURL handle %p\n", curl_handle);

	std::vector<release_t> yytk_releases;
	network::fetch_releases(curl_handle, "Archie-osu", "YYToolkit", yytk_releases);

	printf("[~] Fetched %zd releases!\n", yytk_releases.size());

	for (const auto& release : yytk_releases)
		CMenu::get_instance()->version_tags.push_back(release.tag_name);

	curl_easy_cleanup(curl_handle);

	GLFWwindow* ui_window = glfwCreateWindow(
		window_x, 
		window_y, 
		"Archway beta",
		nullptr, 
		nullptr
	);

	printf("[~] Window created: %p\n", ui_window);

	if (!ui_window)
	{
		printf("[-] Window creation failed!\n");
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(ui_window);
	
	// Enable VSync
	glfwSwapInterval(1); 

	HWND window_handle = glfwGetWin32Window(ui_window);
	printf("[~] Got WinAPI window handle %p\n", window_handle);

	// Initialize the menu
	CMenu::get_instance()->init(ui_window);
	CMenu::get_instance()->set_style(ui_window);

	printf("[~] Entering main loop\n");

#if not _DEBUG
	glfwSetWindowSizeLimits(ui_window, window_x, window_y, window_x, window_y);

	HWND console_window = GetConsoleWindow();
	FreeConsole();
	CloseWindow(console_window);

	printf("[~] If you see this, Windows failed to close the console again...\n");
	printf("[~] I blame Microsoft.\n");
#endif // _DEBUG

	// Main window loop
	while (!glfwWindowShouldClose(ui_window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glfwPollEvents();

		CMenu::get_instance()->run(ui_window);

		glfwSwapBuffers(ui_window);
	}

	// On close
	printf("[~] Destroying window\n");
	CMenu::get_instance()->destroy(ui_window);

	printf("[~] Destroying cURL context\n");
	curl_global_cleanup();

	glfwTerminate();
	return 0;
}