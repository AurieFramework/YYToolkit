#include "IPC.hpp"
#include "../../Shared.hpp"
#include <Windows.h>

#define IPCID_TestCommunication		0x01
#define IPCID_GetFunctionByIndex	0x02
#define IPCID_GetFunctionByName		0x03
#define IPCID_ExecuteCode			0x04

static bool g_ShouldRun = true;
static HANDLE g_hPipeHandle = INVALID_HANDLE_VALUE;

//Windows-exclusive, sorry.
static bool InitIPCPipe()
{
	if (g_hPipeHandle != INVALID_HANDLE_VALUE && g_hPipeHandle != NULL)
		CloseHandle(g_hPipeHandle);

	g_hPipeHandle = CreateFileA(
		"\\\\.\\pipe\\AUMI-IPC",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	return g_hPipeHandle != INVALID_HANDLE_VALUE;
}

static BOOL IpcPostReply(struct IPCReply_t* pReply)
{
	DWORD dwBytesTransferred;
	return WriteFile(g_hPipeHandle, pReply, sizeof(struct IPCReply_t), &dwBytesTransferred, NULL);
}

void AUMI_RunIPC()
{
	while (g_ShouldRun)
	{
		if (InitIPCPipe())
		{
			IPCMessage_t MessageBuffer;
			IPCReply_t MessageReply;
			DWORD dwBytesTransferred;

			BOOL result = ReadFile(g_hPipeHandle, &MessageBuffer, sizeof(MessageBuffer), &dwBytesTransferred, NULL);
			if (result)
			{
				switch (MessageBuffer.FuncID)
				{
				case IPCID_TestCommunication:
					IpcTestCommunication(&MessageBuffer, &MessageReply);
					break;
				case IPCID_GetFunctionByIndex:
					IpcGetFunctionByIndex(&MessageBuffer, &MessageReply);
					break;
				case IPCID_GetFunctionByName:
					IpcGetFunctionByName(&MessageBuffer, &MessageReply);
					break;
				case IPCID_ExecuteCode:
					IpcExecuteCode(&MessageBuffer, &MessageReply);
					break;
				default:
					MessageReply.AUMIResult = AUMI_NOT_IMPLEMENTED;
					break;
				}

				IpcPostReply(&MessageReply);
			}
		}
		Sleep(5);
	}

		
}

void AUMI_StopIPC()
{
	g_ShouldRun = false;
}
