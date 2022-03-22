#pragma once
#include <Windows.h>
#include <winternl.h>

using FNProcessIterationFunc = void(*)(void* pProcessInformation, void* pParameter);

namespace Utils
{
    namespace WinAPI
    {
        typedef enum _KWAIT_REASON
        {
            Executive = 0,
            FreePage = 1,
            PageIn = 2,
            PoolAllocation = 3,
            DelayExecution = 4,
            Suspended = 5,
            UserRequest = 6,
            WrExecutive = 7,
            WrFreePage = 8,
            WrPageIn = 9,
            WrPoolAllocation = 10,
            WrDelayExecution = 11,
            WrSuspended = 12,
            WrUserRequest = 13,
            WrEventPair = 14,
            WrQueue = 15,
            WrLpcReceive = 16,
            WrLpcReply = 17,
            WrVirtualMemory = 18,
            WrPageOut = 19,
            WrRendezvous = 20,
            Spare2 = 21,
            Spare3 = 22,
            Spare4 = 23,
            Spare5 = 24,
            WrCalloutStack = 25,
            WrKernel = 26,
            WrResource = 27,
            WrPushLock = 28,
            WrMutex = 29,
            WrQuantumEnd = 30,
            WrDispatchInt = 31,
            WrPreempted = 32,
            WrYieldExecution = 33,
            WrFastMutex = 34,
            WrGuardedMutex = 35,
            WrRundown = 36,
            MaximumWaitReason = 37
        } KWAIT_REASON;

        typedef enum _KTHREAD_STATE
        {
            Initialized,
            Ready,
            Running,
            Standby,
            Terminated,
            Waiting,
            Transition,
            DeferredReady,
            GateWaitObsolete,
            WaitingForProcessInSwap,
            MaximumThreadState
        } KTHREAD_STATE, * PKTHREAD_STATE;

        typedef struct _SYSTEM_THREAD {
            LARGE_INTEGER           KernelTime;
            LARGE_INTEGER           UserTime;
            LARGE_INTEGER           CreateTime;
            ULONG                   WaitTime;
            PVOID                   StartAddress;
            CLIENT_ID               ClientId;
            KPRIORITY               Priority;
            LONG                    BasePriority;
            ULONG                   ContextSwitchCount;
            KTHREAD_STATE           State;
            KWAIT_REASON            WaitReason;

        } SYSTEM_THREAD, * PSYSTEM_THREAD;

        typedef struct _VM_COUNTERS
        {
            SIZE_T PeakVirtualSize;
            SIZE_T VirtualSize;
            ULONG PageFaultCount;
            SIZE_T PeakWorkingSetSize;
            SIZE_T WorkingSetSize;
            SIZE_T QuotaPeakPagedPoolUsage;
            SIZE_T QuotaPagedPoolUsage;
            SIZE_T QuotaPeakNonPagedPoolUsage;
            SIZE_T QuotaNonPagedPoolUsage;
            SIZE_T PagefileUsage;
            SIZE_T PeakPagefileUsage;
        } VM_COUNTERS, * PVM_COUNTERS;

        typedef struct _SYSTEM_PROCESS_INFORMATION {
            ULONG                   NextEntryOffset;
            ULONG                   NumberOfThreads;
            LARGE_INTEGER           Reserved[3];
            LARGE_INTEGER           CreateTime;
            LARGE_INTEGER           UserTime;
            LARGE_INTEGER           KernelTime;
            UNICODE_STRING          ImageName;
            KPRIORITY               BasePriority;
            HANDLE                  ProcessId;
            HANDLE                  InheritedFromProcessId;
            ULONG                   HandleCount;
            ULONG                   Reserved2[2];
            ULONG                   PrivatePageCount;
            VM_COUNTERS             VirtualMemoryCounters;
            IO_COUNTERS             IoCounters;
            SYSTEM_THREAD           Threads[0];

        } SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

        bool GetSysProcInfo(SYSTEM_PROCESS_INFORMATION** outInfo);

        bool GetThreadStartAddr(HANDLE ThreadHandle, unsigned long& outAddr);

        void IterateProcesses(FNProcessIterationFunc IteratorFunction, void* Parameter);

        bool IsPreloaded();

        void ResumeGameProcess();
    }
}
