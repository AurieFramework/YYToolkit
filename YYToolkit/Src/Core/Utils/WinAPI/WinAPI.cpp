#include "WinAPI.hpp"
#include "../../Features/API/API.hpp"

#define STATUS_INFO_LENGTH_MISMATCH 0xc0000004

bool Utils::WinAPI::GetSysProcInfo(SYSTEM_PROCESS_INFORMATION** outInfo)
{
	using FN = NTSTATUS(NTAPI*)(SYSTEM_INFORMATION_CLASS Class, PVOID SystemInformation, ULONG SysInfoLength, PULONG ReturnLength);

	HMODULE hMod = GetModuleHandleA("ntdll.dll");

	// Couldn't find ntdll.dll (what?)
	if (hMod == NULL)
		return false;

	FN NtQuerySystemInformation = (FN)GetProcAddress(hMod, "NtQuerySystemInformation"); // oh no, internal functions!

	if (!NtQuerySystemInformation)
		return false;

	uint32_t size = sizeof(SYSTEM_PROCESS_INFORMATION);

	SYSTEM_PROCESS_INFORMATION* spi = (SYSTEM_PROCESS_INFORMATION*)malloc(sizeof(SYSTEM_PROCESS_INFORMATION));

	// We don't know how big the process list is, so we have to reallocate until we find it.
	NTSTATUS Status = 0;
	while ((Status = NtQuerySystemInformation(SystemProcessInformation, spi, size, NULL)) == STATUS_INFO_LENGTH_MISMATCH)
		spi = (SYSTEM_PROCESS_INFORMATION*)realloc(spi, size *= 2);

	if (NT_SUCCESS(Status))
	{
		*outInfo = spi;
		return true;
	}
	
	return false;
}

bool Utils::WinAPI::GetThreadStartAddr(HANDLE ThreadHandle, unsigned long& outAddr)
{
	using FN = NTSTATUS(NTAPI*)(HANDLE ThreadHandle, THREADINFOCLASS Class, PVOID ThreadInformation, ULONG Length, PULONG ReturnLength);

	HMODULE hMod = GetModuleHandleA("ntdll.dll");

	// Couldn't find ntdll.dll (what?)
	if (hMod == NULL)
		return false;

	FN NtQueryInformationThread = (FN)GetProcAddress(hMod, "NtQueryInformationThread"); // oh no, internal functions!

	if (!NtQueryInformationThread)
		return false;

	NTSTATUS Status = NtQueryInformationThread(ThreadHandle, (THREADINFOCLASS)9 /*ThreadQuerySetWin32StartAddress*/, &outAddr, sizeof(unsigned long), nullptr);

	return NT_SUCCESS(Status);
}