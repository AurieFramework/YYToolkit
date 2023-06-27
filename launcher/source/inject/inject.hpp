#ifndef RTK_LAUNCHER_INJECT_H_
#define RTK_LAUNCHER_INJECT_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif // WIN32_LEAN_AND_MEAN
#include <string>

namespace inject
{
	std::wstring get_process_module_path(HANDLE process_handle, const std::wstring& module_name);

	uintptr_t get_process_module_base(HANDLE process_handle, const std::wstring& module_name);

	bool is_x64();

	bool is_x64(HANDLE process_handle);

	bool inject(HANDLE process_handle, const std::wstring& path_to_dll);

	uintptr_t get_arch_module_base(const std::wstring& module_name, bool should_be_x64);
}

#endif // RTK_LAUNCHER_INJECT_H_