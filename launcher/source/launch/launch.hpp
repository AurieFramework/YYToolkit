#ifndef RTK_LAUNCHER_LAUNCH_H_
#define RTK_LAUNCHER_LAUNCH_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN

#include <filesystem>
#include <Windows.h>
#include <vector>
#include <string>

struct launch_info_t
{
	// The path to the runner
	std::filesystem::path runner;

	// The arguments to pass into the game
	std::wstring arguments;

	// The preferred YYToolkit version to use - empty = latest
	std::string forced_tagname;

	// Injection delay, in milliseconds
	int injection_delay;

	// The filename to save the YYTK module as - default = "rtkmod.tmp"
	std::string forced_dllname; 

	// Force injection into this process instead of creating a new one
	long pid_override;

	// Use early launch
	bool early_launch;

	launch_info_t()
	{
		runner.clear();
		arguments.clear();
		forced_tagname.clear();
		injection_delay = 0;
		forced_dllname.clear();
		pid_override = 0;
		early_launch = false;
	}
};

struct window_info_t
{
	// The name of the window
	std::string name;

	// The process ID the window belongs to
	long process_id;
};

namespace launch
{
	bool wait_until_ready(HANDLE process);

	DWORD get_running_process_pid(const std::filesystem::path& process_path);

	HANDLE create_process(const std::filesystem::path& process_path, const std::wstring& arguments, bool suspended);

	void do_full_launch(const launch_info_t& launch_info, std::atomic<int>* progress_out);

	void do_full_launch_offline(const launch_info_t& launch_info, std::atomic<int>* progress_out);

	std::vector<window_info_t> get_open_windows();
}

#endif // RTK_LAUNCHER_LAUNCH_H_