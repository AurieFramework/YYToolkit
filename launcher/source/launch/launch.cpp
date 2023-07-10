#include "launch.hpp"
#include <TlHelp32.h>
#include <libcurl/curl.h>
#include "../network/network.hpp"
#include "../inject/inject.hpp"
#include <thread>
#include <PFD/portable-file-dialogs.h>
#include "../codepage/codepage.hpp"

struct process_info_t
{
	DWORD target_pid;
	bool found;
};

static BOOL CALLBACK wait_until_ready_callback(HWND hwnd, process_info_t* process_info)
{
	DWORD process_id = 0;
	GetWindowThreadProcessId(hwnd, &process_id);

	if (process_id == process_info->target_pid)
	{
		if (IsWindowVisible(hwnd))
		{
			process_info->found = true;
			return false;
		}
	}
	
	return true;
}

bool launch::wait_until_ready(HANDLE process)
{
	process_info_t process_info;
	process_info.target_pid = GetProcessId(process);
	process_info.found = false;

	EnumWindows(reinterpret_cast<WNDENUMPROC>(wait_until_ready_callback), reinterpret_cast<LPARAM>(&process_info));
	
	return process_info.found;
}

DWORD launch::get_running_process_pid(const std::filesystem::path& process_path)
{
	// Get a list of all processes
	HANDLE process_snapshot = CreateToolhelp32Snapshot(
		TH32CS_SNAPPROCESS,
		0
	);

	// ... and make sure it's valid
	if (process_snapshot == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	PROCESSENTRY32 process_entry; process_entry.dwSize = sizeof(process_entry);
	Process32First(process_snapshot, &process_entry);

	// Loop over all the modules and compare their names
	do
	{
		if (!_wcsicmp(process_path.filename().wstring().c_str(), process_entry.szExeFile))
		{
			CloseHandle(process_snapshot);
			return process_entry.th32ProcessID;
		}

	} while (Process32Next(process_snapshot, &process_entry));

	CloseHandle(process_snapshot);
	return 0;
}


bool is_process_running(const std::wstring& proc_name)
{
	// Get a list of all processes
	HANDLE process_snapshot = CreateToolhelp32Snapshot(
		TH32CS_SNAPPROCESS,
		0
	);

	// ... and make sure it's valid
	if (process_snapshot == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	PROCESSENTRY32 process_entry; process_entry.dwSize = sizeof(process_entry);
	Process32First(process_snapshot, &process_entry);

	// Loop over all the processes and compare their names
	do
	{
		// If we found it, close the handle and return true
		if (!_wcsicmp(proc_name.c_str(), process_entry.szExeFile))
		{
			CloseHandle(process_snapshot);
			return true;
		}

	} while (Process32Next(process_snapshot, &process_entry));

	CloseHandle(process_snapshot);
	return false;
}


HANDLE launch::create_process(const std::filesystem::path& process_path, const std::wstring& arguments, bool suspended)
{
	std::wstring command_line = L"\"" + process_path.wstring() + L"\" " + arguments;

	STARTUPINFO start_info = { 0 };
	start_info.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION process_info = { 0 };

	bool process_created = CreateProcessW(
		nullptr,
		command_line.data(),
		nullptr,
		nullptr,
		true,
		(suspended ? CREATE_SUSPENDED : 0),
		nullptr,
		process_path.has_parent_path() ? process_path.parent_path().wstring().c_str() : 0,
		&start_info,
		&process_info
	);

	if (!process_created)
	{
		printf("[create_process] failed with error %x\n", GetLastError());
	}

	CloseHandle(process_info.hThread);

	return process_info.hProcess;
}

void launch::do_full_launch(const launch_info_t& launch_info, std::atomic<int>* progress_out)
{
	// Yes, I'm breaking my naming scheme, but it's WinAPI stuff
	// ... so it doesn't count, right?
	using PNTResumeProcess = long(NTAPI*)(HANDLE ProcessHandle);
	constexpr auto mb_flags = MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND;

	// --- INITIALIZING ---
	
	progress_out->store(0);

	// Pre-init all our variables, so we don't gotta worry about goto causing errors
	// yes I know goto is considered evil, but in this case I don't see an issue using it
	CURL* curl_handle = nullptr;
	HMODULE ntdll_module = nullptr;
	HANDLE target_process = nullptr;
	PNTResumeProcess NtResumeProcess = nullptr;
	std::vector<release_t> yytk_versions = {};
	release_t selected_release;
	bool is_target_x64 = false;
	wchar_t temp_file_path[MAX_PATH] = { 0 };

	// Create our libcurl handle
	curl_handle = curl_easy_init();

	printf("[inject] Got cURL handle %p\n", curl_handle);

	// ... and make sure it's valid
	if (!curl_handle)
	{
		MessageBoxA(0, "Failed to acquire cURL handle.", "cURL error", mb_flags);
		goto thread_cleanup;
	}

	// Find NTDLL.dll
	ntdll_module = GetModuleHandleW(L"ntdll.dll");

	printf("[inject] ntdll %p\n", ntdll_module);

	// ... and make sure it's valid (x2)
	if (!ntdll_module)
	{
		MessageBoxA(0, "Failed to query the ntdll module.", "WinAPI error", mb_flags);
		goto thread_cleanup;
	}

	// Get the address of NtResumeProcess
	NtResumeProcess = reinterpret_cast<PNTResumeProcess>(GetProcAddress(ntdll_module, "NtResumeProcess"));

	printf("[inject] NtResumeProcess %p\n", NtResumeProcess);

	// ... and make sure it's valid (x3)
	if (!NtResumeProcess)
	{
		MessageBoxA(0, "Failed to query for NtResumeProcess.", "WinAPI error", mb_flags);
		goto thread_cleanup;
	}

	// --- FETCHING RELEASES ---
	progress_out->store(1);

	printf("[inject] Fetching releases (stage 1)\n");

	network::fetch_releases(curl_handle, "Archie-osu", "YYToolkit", yytk_versions);

	// Failed to fetch?
	if (yytk_versions.empty())
	{
		int selection = MessageBoxA(
			0,
			"Failed to fetch releases.\n"
			"Do you want to use a locally-stored DLL?",
			"cURL error",
			MB_YESNO | MB_SETFOREGROUND | MB_TOPMOST | MB_ICONWARNING
		);

		if (selection == IDYES)
		{
			curl_easy_cleanup(curl_handle);

			return do_full_launch_offline(launch_info, progress_out);
		}

		goto thread_cleanup;
	}

	// Select a release to use
	// The first release in the vector is the most up-to-date one
	selected_release = yytk_versions.front();

	// Check if we have an override, if so, use that
	if (!launch_info.forced_tagname.empty())
	{
		auto selected_release_iterator = std::find_if(
			std::begin(yytk_versions),
			std::end(yytk_versions),
			[launch_info](const release_t& val) -> bool
			{
				return val.tag_name == launch_info.forced_tagname;
			}
		);

		if (selected_release_iterator != std::end(yytk_versions))
			selected_release = *selected_release_iterator;
	}

	// --- PREPARING ENVIRONMENT ---
	progress_out->store(2);

	printf("[inject] Preparing environment (stage 2)\n");

	target_process = launch_info.pid_override ? 
		OpenProcess(PROCESS_ALL_ACCESS, FALSE, launch_info.pid_override) : 
		launch::create_process(launch_info.runner, launch_info.arguments, true);

	printf("[inject] target_process pid %d\n", GetProcessId(target_process));

	if (!target_process)
	{
		MessageBoxA(0, "Failed to initialize the game.", "WinAPI error", mb_flags);
		goto thread_cleanup;
	}

	// Check if our game is x64
	is_target_x64 = inject::is_x64(target_process);

	printf("[inject] is_target_x64 returns %d\n", is_target_x64);

	// Make sure the current release supports the architecture
	if (is_target_x64 && selected_release.assets.download_url_x64.empty())
	{
		int selection = MessageBoxA(
			0, 
			"The target game is not supported by the selected YYTK version.\n"
			"Try to use the latest YYTK version instead?", 
			"Architecture not supported", 
			MB_YESNO | MB_SETFOREGROUND | MB_TOPMOST | MB_ICONWARNING
		);

		if (selection == IDYES)
		{
			selected_release = yytk_versions.front();
		}
		else
		{
			// Make sure we don't leave a suspended process running in the background
			TerminateProcess(target_process, 0); 

			goto thread_cleanup;
		}
	}

	{
		std::error_code ec;
		printf("[inject] Temp path: %S", std::filesystem::temp_directory_path(ec).wstring().c_str());
		std::filesystem::path desired_path = std::filesystem::temp_directory_path(ec) / launch_info.forced_dllname;

		wcscpy_s(temp_file_path, desired_path.wstring().c_str());
	}
	
	// --- DOWNLOADING DATA ---
	progress_out->store(3);

	printf("[inject] Downloading data (stage 3)\n");

	if (is_target_x64)
	{
		CURLcode curl_result = network::download_file(
			curl_handle, 
			selected_release.assets.download_url_x64, 
			temp_file_path
		);

		if (curl_result)
		{
			MessageBoxA(0, "Failed to download YYToolkit.", "cURL error", mb_flags);

			// Make sure we don't leave a suspended process running in the background
			TerminateProcess(target_process, 0);

			goto thread_cleanup;
		}
	}
	else
	{
		CURLcode curl_result = network::download_file(
			curl_handle,
			selected_release.assets.download_url_x86,
			temp_file_path
		);

		if (curl_result)
		{
			MessageBoxA(0, "Failed to download YYToolkit.", "cURL error", mb_flags);

			// Make sure we don't leave a suspended process running in the background
			TerminateProcess(target_process, 0);

			goto thread_cleanup;
		}
	}

	// --- WAITING FOR GAME ---
	progress_out->store(4);

	printf("[inject] Waiting for game (stage 4)\n");

	if (!launch_info.early_launch)
	{
		NtResumeProcess(target_process);
		printf("[inject] early launch off, waiting for process %d\n", GetProcessId(target_process));

		while (!launch::wait_until_ready(target_process))
		{
			// If the process terminated, we might have a new one spawned
			// This often occurs with steam games if steamapi.dll exists in the game folder
			DWORD process_status = 0;
			if (GetExitCodeProcess(target_process, &process_status))
			{
				// If the process died
				if (process_status != STILL_ACTIVE)
				{
					printf("[inject] Process died while waiting for it to inject\n");

					// Sleep for a bit, maybe it will get relaunched as part of Steam or something
					std::this_thread::sleep_for(std::chrono::seconds(3));

					DWORD pid = launch::get_running_process_pid(launch_info.runner);

					// If we found a running process with the same PID
					if (pid)
					{
						printf("[inject] Found new alive PID %d\n", pid);

						CloseHandle(target_process);
						target_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

						if (!target_process)
						{
							MessageBoxA(0, "Failed to initialize the game.", "WinAPI error", mb_flags);
							goto thread_cleanup;
						}
					}
					else
					{
						MessageBoxA(0, "The target process died and did not respawn in time.", "Framework error", mb_flags);
						goto thread_cleanup;
					}
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		if (launch_info.injection_delay)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(launch_info.injection_delay));
		}
	}

	// --- INJECTING ---
	progress_out->store(5);

	printf("[inject] Injecting (stage 5)\n");

	if (!inject::inject(target_process, temp_file_path))
	{
		MessageBoxA(0, "Injection failed.", "Framework error", mb_flags);

		// Make sure we don't leave a suspended process running in the background
		TerminateProcess(target_process, 0);
	}


thread_cleanup:
	// --- CLEANING UP ---
	progress_out->store(6);

	// Cleanup
	curl_easy_cleanup(curl_handle);

	if (target_process)
		CloseHandle(target_process);

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

// Most of this function is copy-pasted from do_full_launch
// Just skips getting releases, and instead asks for a local file
void launch::do_full_launch_offline(const launch_info_t& launch_info, std::atomic<int>* progress_out)
{
	using PNTResumeProcess = long(NTAPI*)(HANDLE ProcessHandle);
	constexpr auto mb_flags = MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND;

	// --- INITIALIZING ---

	progress_out->store(0);

	// Pre-init all our variables, so we don't gotta worry about goto causing errors
	// yes I know goto is considered evil, but in this case I don't see an issue using it
	HMODULE ntdll_module = nullptr;
	HANDLE target_process = nullptr;
	PNTResumeProcess NtResumeProcess = nullptr;
	std::wstring selected_path;

	// Find NTDLL.dll
	ntdll_module = GetModuleHandleW(L"ntdll.dll");

	printf("[inject] ntdll %p\n", ntdll_module);

	// ... and make sure it's valid
	if (!ntdll_module)
	{
		MessageBoxA(0, "Failed to query the ntdll module.", "WinAPI error", mb_flags);
		goto thread_cleanup;
	}

	// Get the address of NtResumeProcess
	NtResumeProcess = reinterpret_cast<PNTResumeProcess>(GetProcAddress(ntdll_module, "NtResumeProcess"));

	printf("[inject] NtResumeProcess %p\n", NtResumeProcess);

	// ... and make sure it's valid (x2)
	if (!NtResumeProcess)
	{
		MessageBoxA(0, "Failed to query for NtResumeProcess.", "WinAPI error", mb_flags);
		goto thread_cleanup;
	}

	// --- PREPARING ENVIRONMENT ---
	progress_out->store(2);

	// Create our game process / open a running one
	target_process = launch_info.pid_override ?
		OpenProcess(PROCESS_ALL_ACCESS, FALSE, launch_info.pid_override) :
		launch::create_process(launch_info.runner, launch_info.arguments, true);

	printf("[inject] target_process pid %d\n", GetProcessId(target_process));

	if (!target_process)
	{
		MessageBoxA(0, "Failed to initialize the game.", "WinAPI error", mb_flags);
		goto thread_cleanup;
	}

	// Gotta put this in a separate scope 
	{
		auto dialog_results = pfd::open_file("Select DLL", "", { "Dynamic Link Libraries", "*.dll", "All Files", "*" }).result();

		// There can only be one element in the vector
		// but this is more compact than checking if it's empty etc.
		for (const std::string& result : dialog_results)
		{
			selected_path = cp::codepage_to_unicode(CP_UTF8, result);
		}
	}

	// --- WAITING FOR GAME ---
	progress_out->store(4);

	if (!launch_info.early_launch)
	{
		NtResumeProcess(target_process);
		printf("[inject] early launch off, waiting for process %d\n", GetProcessId(target_process));

		while (!launch::wait_until_ready(target_process))
		{
			// If the process terminated, we might have a new one spawned
			// This often occurs with steam games if steamapi.dll exists in the game folder
			DWORD process_status = 0;
			if (GetExitCodeProcess(target_process, &process_status))
			{
				// If the process died
				if (process_status != STILL_ACTIVE)
				{
					printf("[inject] Process died while waiting for it to inject\n");

					// Sleep for a bit, maybe it will get relaunched as part of Steam or something
					std::this_thread::sleep_for(std::chrono::seconds(3));

					DWORD pid = launch::get_running_process_pid(launch_info.runner);

					// If we found a running process with the same PID
					if (pid)
					{
						printf("[inject] Found new alive PID %d\n", pid);

						CloseHandle(target_process);
						target_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

						if (!target_process)
						{
							MessageBoxA(0, "Failed to initialize the game.", "WinAPI error", mb_flags);
							goto thread_cleanup;
						}
					}
					else
					{
						MessageBoxA(0, "The target process died and did not respawn in time.", "Framework error", mb_flags);
						goto thread_cleanup;
					}
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		if (launch_info.injection_delay)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(launch_info.injection_delay));
		}
	}

	// --- INJECTING ---
	progress_out->store(5);

	if (!inject::inject(target_process, selected_path))
	{
		MessageBoxA(0, "Injection failed.", "Framework error", mb_flags);

		// Make sure we don't leave a suspended process running in the background
		TerminateProcess(target_process, 0);
	}

thread_cleanup:
	// --- CLEANING UP ---
	progress_out->store(6);

	if (target_process)
		CloseHandle(target_process);

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

static BOOL CALLBACK get_open_windows_callback(HWND hwnd, std::vector<window_info_t>* window_info)
{
	DWORD process_id = 0;
	GetWindowThreadProcessId(hwnd, &process_id);

	if (IsWindowVisible(hwnd) && GetWindowTextLengthA(hwnd) > 0 && process_id != GetCurrentProcessId())
	{
		wchar_t process_name[128] = { 0 };
		GetWindowTextW(hwnd, process_name, 128);

		window_info_t info;
		info.name = cp::unicode_to_codepage(CP_UTF8, process_name);
		info.process_id = process_id;

		window_info->push_back(info);
	}

	return true;
}

std::vector<window_info_t> launch::get_open_windows()
{
	std::vector<window_info_t> window_info;
	EnumWindows(reinterpret_cast<WNDENUMPROC>(get_open_windows_callback), reinterpret_cast<LPARAM>(&window_info));

	return window_info;
}
