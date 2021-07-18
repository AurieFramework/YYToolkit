#include "../../Exports.hpp"
#include "../IPC.hpp"
#include <cstring>
#include <cstdlib>

void IpcTestCommunication(IPCMessage_t* Message, IPCReply_t* OutReply)
{
	OutReply->AUMIResult = AUMI_OK;
	strncpy(OutReply->Buffer, "Hello from YYToolkit - If you can read this, the IPC Test was successful!\nNow let me just pad this reply out to 128 characters.", 128);
}

void IpcGetFunctionByIndex(IPCMessage_t* Message, IPCReply_t* OutReply)
{
	AUMIFunctionInfo RFInformation;

	AUMIResult result = AUMI_GetFunctionByIndex(*(int*)(Message->Buffer), &RFInformation);

	OutReply->AUMIResult = result;
	memcpy(OutReply->Buffer, &RFInformation, sizeof(AUMIFunctionInfo));
}

void IpcGetFunctionByName(IPCMessage_t* Message, IPCReply_t* Reply)
{
	AUMIFunctionInfo RFInformation;
	AUMIResult result = AUMI_GetFunctionByName(Message->Buffer, &RFInformation);

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
	AUMIResult result;
	RValue Arguments; memset(&Arguments, 0, sizeof(RValue));

	YYObjectBase* g_pGlobal = NULL;

	if (result = AUMI_GetGlobalState(&g_pGlobal))
	{
		OutReply->AUMIResult = result;
		return;
	}

	if (result = AUMI_CreateCode(&Code, (void*)CodeBuffer->Code, CodeBuffer->CodeSize, CodeBuffer->LocalsUsed, "AUMI-C Code Entry"))
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
	OutReply->AUMIResult = AUMI_OK;
}