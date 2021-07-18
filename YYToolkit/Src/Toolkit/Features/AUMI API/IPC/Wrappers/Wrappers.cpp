#include "../../Exports.hpp"
#include "../IPC.hpp"
#include <string>

void IpcTestCommunication(IPCMessage_t* Message, IPCReply_t* OutReply)
{
	OutReply->AUMIResult = YYTK_OK;
	strcpy_s(OutReply->Buffer, 128, "Hello from YYToolkit - If you can read this, the IPC Test was successful!\nNow let me just pad this reply out to 128 characters.");
}

void IpcGetFunctionByIndex(IPCMessage_t* Message, IPCReply_t* OutReply)
{
	AUMIFunctionInfo RFInformation;

	YYTKStatus result = AUMI_GetFunctionByIndex(*(int*)(Message->Buffer), &RFInformation);

	OutReply->AUMIResult = result;
	memcpy(OutReply->Buffer, &RFInformation, sizeof(AUMIFunctionInfo));
}

void IpcGetFunctionByName(IPCMessage_t* Message, IPCReply_t* Reply)
{
	AUMIFunctionInfo RFInformation;
	YYTKStatus result = AUMI_GetFunctionByName(Message->Buffer, &RFInformation);

	Reply->AUMIResult = result;
	memcpy(Reply->Buffer, &RFInformation, sizeof(AUMIFunctionInfo));
}

void IpcExecuteCode(IPCMessage_t* Message, IPCReply_t* OutReply)
{
	const struct BufferLayout
	{
		int CodeSize;
		int LocalsUsed;
		char Code[512 - (sizeof(int) * 2)];
	} *CodeBuffer = (BufferLayout*)Message->Buffer;

	CCode Code;
	YYTKStatus result;
	RValue Arguments; memset(&Arguments, 0, sizeof(RValue));

	YYObjectBase* g_pGlobal = NULL;

	if (result = AUMI_GetGlobalState(&g_pGlobal))
	{
		OutReply->AUMIResult = result;
		return;
	}

	if (result = AUMI_CreateCode(&Code, (void*)CodeBuffer->Code, CodeBuffer->CodeSize, CodeBuffer->LocalsUsed, "YYToolkit (AUMI API) Code Entry"))
	{
		OutReply->AUMIResult = result;
		return;
	}

	if (result = AUMI_ExecuteCode(g_pGlobal, g_pGlobal, &Code, &Arguments))
	{
		OutReply->AUMIResult = result;
		AUMI_FreeCode(&Code);
		return;
	}

	AUMI_FreeCode(&Code);
	OutReply->AUMIResult = YYTK_OK;
}