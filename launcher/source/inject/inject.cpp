#include "inject.hpp"
#include <cstdint>
#include <fstream>
#include <TlHelp32.h>

static bool inject_loadlib_internal(HANDLE process_handle, const std::wstring& path_to_dll, LPTHREAD_START_ROUTINE load_routine)
{
	// Allocate memory for our string inside the target process
	LPVOID allocated_memory = VirtualAllocEx(
		process_handle,
		nullptr,
		path_to_dll.length() * sizeof(wchar_t),
		MEM_COMMIT,
		PAGE_READWRITE
	);

	if (!allocated_memory)
	{
		return false;
	}

	// Write the target DLL's path into the process's memory
	BOOL writing_success = WriteProcessMemory(
		process_handle,
		allocated_memory,
		path_to_dll.data(),
		path_to_dll.length() * sizeof(wchar_t),
		nullptr
	);

	if (!writing_success)
	{
		return false;
	}

	// Create a thread and hope it works
	HANDLE thread_handle = CreateRemoteThread(
		process_handle,
		nullptr,
		0,
		load_routine,
		allocated_memory,
		CREATE_SUSPENDED,
		nullptr
	);

	if (!thread_handle)
	{
		return false;
	}

	// Wait until the thread finishes (LLW returns)
	// Then don't forget to close the handle
	ResumeThread(thread_handle);
	WaitForSingleObject(thread_handle, INFINITE);
	CloseHandle(thread_handle);

	BOOL free_success = VirtualFreeEx(
		process_handle,
		allocated_memory,
		0,
		MEM_RELEASE
	);

	if (!free_success)
	{
		return false;
	}

	return true;
}

static DWORD resolve_rva(PIMAGE_NT_HEADERS nt_header, DWORD virtual_address)
{
	PIMAGE_SECTION_HEADER pHeaders = IMAGE_FIRST_SECTION(nt_header);

	// Loop over all the sections of the file
	for (int n = 0; n < nt_header->FileHeader.NumberOfSections; n++)
	{
		// ... to check if the RVA points to within that section
		// the section begins at pHeaders[n].VirtualAddress and ends at pHeaders[n].VirtualAddress + pHeaders[n].SizeOfRawData
		if (virtual_address >= pHeaders[n].VirtualAddress && virtual_address < (pHeaders[n].VirtualAddress + pHeaders[n].SizeOfRawData))
		{
			// The RVA points into this section, so return the offset inside the section's data.
			return (virtual_address - pHeaders[n].VirtualAddress) + pHeaders[n].PointerToRawData;
		}
	}

	return 0;
}

static uintptr_t get_pe_export_offset(const std::wstring& path_to_pe, const std::string& function_name)
{
	// Try to open our file
	std::ifstream input_file(path_to_pe, std::ios::binary | std::ios::ate);
	if (!input_file.is_open() || !input_file.good())
	{
		return 0;
	}

	// Get the size of the open file (we opened ATE so we can use tellg to get the current position => filesize)
	std::streampos input_file_size = input_file.tellg();

	// I think we need more than this to fit stuff in?
	if (input_file_size <= 1024)
	{
		return 0;
	}

	char* file_in_memory = new char[static_cast<intptr_t>(input_file_size)];

	if (!file_in_memory)
	{
		return 0;
	}

	// Seek to the beginning of the file (offset 0 from the beginning)
	input_file.seekg(0, std::ios::beg);

	// Read the whole file into memory, and close the stream
	input_file.read(file_in_memory, input_file_size);
	input_file.close();

	// Check if the DOS header is valid and follow the NT header pointer.
	PIMAGE_DOS_HEADER dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(file_in_memory);
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
	{
		delete[] file_in_memory;
		return 0;
	}

	// We don't know if the OptionalHeader is x86 or x64 yet, unsafe to access it just yet!!!
	// Also verify the NT signature is fine.
	PIMAGE_NT_HEADERS nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(file_in_memory + dos_header->e_lfanew);
	if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
	{
		delete[] file_in_memory;
		return 0;
	}

	PIMAGE_EXPORT_DIRECTORY export_directory = nullptr;

	// I386
	if (nt_headers->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
	{
		// This NT_HEADERS object has the correct bitness, it is now safe to access the optional header
		PIMAGE_NT_HEADERS32 nt_headers_x86 = reinterpret_cast<PIMAGE_NT_HEADERS32>(nt_headers);

		// Handle object files
		if (!nt_headers_x86->FileHeader.SizeOfOptionalHeader)
		{
			delete[] file_in_memory;
			return 0;
		}

		// In case our file doesn't have an export header
		if (nt_headers_x86->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_EXPORT)
		{
			delete[] file_in_memory;
			return 0;
		}

		// Get the RVA - we can't just add this to file_in_memory because of section alignment and stuff...
		// We could if we had the file already LLA'd into memory, but that's impossible, since  the file's a different arch.
		DWORD export_dir_address = nt_headers_x86->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		if (!export_dir_address)
		{
			delete[] file_in_memory;
			return 0;
		}

		// Get the export directory from the VA 
		export_directory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
			file_in_memory + resolve_rva(
				nt_headers,
				export_dir_address
			)
		);
	}
	else if (nt_headers->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
	{
		PIMAGE_NT_HEADERS64 nt_headers_x64 = reinterpret_cast<PIMAGE_NT_HEADERS64>(nt_headers);

		// Handle object files
		if (!nt_headers_x64->FileHeader.SizeOfOptionalHeader)
		{
			delete[] file_in_memory;
			return 0;
		}

		// In case our file doesn't have an export header
		if (nt_headers_x64->OptionalHeader.NumberOfRvaAndSizes <= IMAGE_DIRECTORY_ENTRY_EXPORT)
		{
			delete[] file_in_memory;
			return 0;
		}

		// Get the RVA - we can't just add this to file_in_memory because of section alignment and stuff...
		// We could if we had the file already LLA'd into memory, but that's impossible, since  the file's a different arch.
		DWORD export_dir_address = nt_headers_x64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		if (!export_dir_address)
		{
			delete[] file_in_memory;
			return 0;
		}

		// Get the export directory from the VA 
		export_directory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
			file_in_memory + resolve_rva(
				nt_headers,
				export_dir_address
			)
			);
	}
	else
	{
		// Intel Itanium not supported, sorry...
		delete[] file_in_memory;
		return 0;
	}

	if (!export_directory)
	{
		delete[] file_in_memory;
		return 0;
	}

	// Get all our required arrays
	DWORD* function_names = reinterpret_cast<DWORD*>(
		file_in_memory +
		resolve_rva(
			nt_headers,
			export_directory->AddressOfNames
		)
		);

	WORD* function_name_ordinals = reinterpret_cast<WORD*>(
		file_in_memory +
		resolve_rva(
			nt_headers,
			export_directory->AddressOfNameOrdinals
		)
		);

	DWORD* function_addresses = reinterpret_cast<DWORD*>(
		file_in_memory +
		resolve_rva(
			nt_headers,
			export_directory->AddressOfFunctions
		)
		);

	// Loop over all the named exports
	for (DWORD n = 0; n < export_directory->NumberOfNames; n++)
	{
		// Get the name of the export
		const char* export_name = file_in_memory + resolve_rva(nt_headers, function_names[n]);

		// Get the function ordinal for array access
		short function_ordinal = function_name_ordinals[n];

		// Get the function offset
		uintptr_t function_offset = function_addresses[function_ordinal];

		// If it's our target export
		if (function_name == export_name)
		{
			delete[] file_in_memory;
			return function_offset;
		}
	}

	delete[] file_in_memory;
	return 0;
}

// Turns "A StRiNg" into "a string"
static std::wstring lower_string(const std::wstring& string)
{
	std::wstring new_string;
	new_string.reserve(string.length());
	for (auto wch : string)
	{
		new_string.push_back(towlower(wch));
	}

	return new_string;
}

static std::wstring get_process_module_path_fallback(HANDLE process_handle, const std::wstring& module_name)
{
	bool is_target_x64 = inject::is_x64(process_handle);

	wchar_t system_root_path[MAX_PATH] = { 0 };
	if (!GetEnvironmentVariableW(L"SystemRoot", system_root_path, MAX_PATH))
	{
		return L"";
	}

	// We need this wstring so we can concatenate stuff to it
	std::wstring process_module_path = system_root_path;

	// This function should only be called if the injector is x64 and the target is x32
	// but we can't be sure enough
	if (!is_target_x64)
	{
		return process_module_path + L"\\SysWoW64\\" + module_name;
	}

	// (continuation of last comment): this should never trigger
	return process_module_path + L"\\System32\\" + module_name;
}

std::wstring inject::get_process_module_path(HANDLE process_handle, const std::wstring& module_name)
{
	// Get a list of all the modules in our target process
	HANDLE process_snapshot = CreateToolhelp32Snapshot(
		TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
		GetProcessId(process_handle)
	);

	if (process_snapshot == INVALID_HANDLE_VALUE)
	{
		return get_process_module_path_fallback(process_handle, module_name);
	}

	MODULEENTRY32 module_entry; module_entry.dwSize = sizeof(module_entry);
	Module32First(process_snapshot, &module_entry);

	// Loop over all the modules and compare their names
	do
	{
		if (lower_string(module_name) == lower_string(module_entry.szModule))
		{
			// Close the handle and return the module's base path
			CloseHandle(process_snapshot);
			return module_entry.szExePath;
		}
	} while (Module32Next(process_snapshot, &module_entry));

	CloseHandle(process_snapshot);
	return get_process_module_path_fallback(process_handle, module_name);
}

uintptr_t inject::get_process_module_base(HANDLE process_handle, const std::wstring& module_name)
{
	// Get a list of all the modules in our target process
	HANDLE process_snapshot = CreateToolhelp32Snapshot(
		TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
		GetProcessId(process_handle)
	);

	if (process_snapshot == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	MODULEENTRY32 module_entry; module_entry.dwSize = sizeof(module_entry);
	Module32First(process_snapshot, &module_entry);

	// Loop over all the modules and compare their names
	do
	{
		if (lower_string(module_name) == lower_string(module_entry.szModule))
		{
			// Close the handle and return the module's base address
			CloseHandle(process_snapshot);
			return reinterpret_cast<uintptr_t>(module_entry.modBaseAddr);
		}
	} while (Module32Next(process_snapshot, &module_entry));

	CloseHandle(process_snapshot);
	return 0;
}

bool inject::is_x64()
{
	return sizeof(intptr_t) == sizeof(int64_t);
}

bool inject::is_x64(HANDLE process_handle)
{
	BOOL is_wow64 = FALSE;
	IsWow64Process(process_handle, &is_wow64);

	return !static_cast<bool>(is_wow64);
}

bool inject::inject(HANDLE process_handle, const std::wstring& path_to_dll)
{
	// Query target architectures
	bool is_injector_x64 = is_x64();
	bool is_injectee_x64 = is_x64(process_handle);

	// If both processes are the same arch, we can perform a regular injection
	if (is_injector_x64 == is_injectee_x64)
	{
		return inject_loadlib_internal(
			process_handle,
			path_to_dll,
			reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryW)
		);
	}

	// We can't inject because CreateToolhelp32Snapshot would return ERROR_PARTIAL_COPY
	// This only happens if we have a 32-bit injector AND a 64-bit target
	if (!is_injector_x64)
	{
		return false;
	}

	// If the injector is x32, and we try to access a DLL from System32 (ie. 64-bit ones) 
	// we get redirected to SysWoW64 variants - we don't want this when calculating offsets.
	Wow64EnableWow64FsRedirection(false);

	// Try to get the path of kernel32.dll used by the target process
	std::wstring target_kernel32_path = get_process_module_path(process_handle, L"kernel32.dll");
	if (target_kernel32_path.empty())
		return false;

	uintptr_t kernel32_base = get_process_module_base(process_handle, L"kernel32.dll");
	uintptr_t offset_to_llw = get_pe_export_offset(target_kernel32_path, "LoadLibraryW");

	// Re-enable the FS redirection as per MSDN recommendation
	Wow64EnableWow64FsRedirection(true);

	if (!kernel32_base)
		return false;

	if (!offset_to_llw)
		return false;

	return inject_loadlib_internal(
		process_handle,
		path_to_dll,
		reinterpret_cast<LPTHREAD_START_ROUTINE>(kernel32_base + offset_to_llw)
	);
}