#include "WinAPI.hpp"
#include "../../Features/API/Internal.hpp"
#include <cstdint>

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

void Utils::WinAPI::IterateProcesses(FNProcessIterationFunc IteratorFunction, void* pParameter)
{
	Utils::WinAPI::SYSTEM_PROCESS_INFORMATION* pSpi = nullptr;

	if (!Utils::WinAPI::GetSysProcInfo(&pSpi))
		return;

	void* pFreeAddress = pSpi;

	CModule GameModule{};
	YYTKStatus stMMGM = API::Internal::MmGetModuleInformation(nullptr, GameModule);

	if (stMMGM)
	{
		free(pFreeAddress);
		return;
	}
	
	while (true)
	{
		IteratorFunction(pSpi, pParameter);

		if (pSpi->NextEntryOffset == 0)
			break;

		pSpi = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(reinterpret_cast<PBYTE>(pSpi) + pSpi->NextEntryOffset);
	}

	free(pFreeAddress);
}

bool Utils::WinAPI::IsPreloaded()
{
	FNProcessIterationFunc Func = [](void* pProcessInformation, void* pParam)
	{
		// Cast the pProcessInformation pointer to its actual type
		namespace WinAPI = Utils::WinAPI;
		WinAPI::SYSTEM_PROCESS_INFORMATION* pSPI = reinterpret_cast<WinAPI::SYSTEM_PROCESS_INFORMATION*>(pProcessInformation);

		// If it's nullptr (we failed to capture processes or some corruption occured)
		if (pSPI == nullptr)
			return;

		// Get information about the game process
		CModule GameModule;
		API::Internal::MmGetModuleInformation(nullptr, GameModule);

		// Get the PID of the current process entry
		uintptr_t pSPI_PID = reinterpret_cast<uintptr_t>(pSPI->ProcessId);

		// Check if it's the game process, if not, go to the next one
		if (pSPI_PID != GetCurrentProcessId())
			return;

		for (int i = 0; i < pSPI->NumberOfThreads; i++)
		{
			// Open the current thread
			HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, reinterpret_cast<uintptr_t>(pSPI->Threads[i].ClientId.UniqueThread));

			// Get its start address
			unsigned long dwThreadStartAddress = 0;
			bool bSuccess = WinAPI::GetThreadStartAddr(hThread, dwThreadStartAddress);

			CloseHandle(hThread);

			// If we failed getting the thread's start address, go to the next thread
			if (!bSuccess)
				continue;

			// If the thread's supposed to start in main()
			if (dwThreadStartAddress == GameModule.EntryPoint)
			{
				// Check if the thread is waiting and is suspended
				if (pSPI->Threads[i].State != Utils::WinAPI::KTHREAD_STATE::Waiting)
					return;

				if (pSPI->Threads[i].WaitReason != Utils::WinAPI::KWAIT_REASON::Suspended)
					return;

				// If we didn't return, it's sleeping - return true.
				*reinterpret_cast<bool*>(pParam) = true;
			}
		}
	};

	bool bReturnValue = false;
	Utils::WinAPI::IterateProcesses(Func, &bReturnValue);

	return bReturnValue;
}

// Basically a big copy pasterino from the IsPreloaded function lol
void Utils::WinAPI::ResumeGameProcess()
{
	FNProcessIterationFunc Func = [](void* pProcessInformation, void* pParam)
	{
		CModule& GameModule = *reinterpret_cast<CModule*>(pParam);

		// Cast the pProcessInformation pointer to its actual type
		namespace WinAPI = Utils::WinAPI;
		WinAPI::SYSTEM_PROCESS_INFORMATION* pSPI = reinterpret_cast<WinAPI::SYSTEM_PROCESS_INFORMATION*>(pProcessInformation);

		// If it's nullptr (we failed to capture processes or some corruption occured)
		if (pSPI == nullptr)
			return;

		// Get the PID of the current process entry
		uintptr_t pSPI_PID = reinterpret_cast<uintptr_t>(pSPI->ProcessId);

		// Check if it's the game process, if not, go to the next one
		if (pSPI_PID != GetCurrentProcessId())
			return;

		for (int i = 0; i < pSPI->NumberOfThreads; i++)
		{
			// Open the current thread
			HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, reinterpret_cast<uintptr_t>(pSPI->Threads[i].ClientId.UniqueThread));

			// Get its start address
			unsigned long dwThreadStartAddress = 0;
			WinAPI::GetThreadStartAddr(hThread, dwThreadStartAddress);

			// If the thread's supposed to start in main(), resume it
			if (dwThreadStartAddress == GameModule.EntryPoint)
				ResumeThread(hThread);

			// Close the handle
			CloseHandle(hThread);
		}
	};

	// Get information about the game process
	CModule GameModule;
	API::Internal::MmGetModuleInformation(nullptr, GameModule);

	Utils::WinAPI::IterateProcesses(Func, &GameModule);
}
