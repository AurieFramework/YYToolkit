#pragma once
//Used in commanding AUMI to do something.
struct IPCMessage_t
{
	short FuncID;
	char Buffer[512];
};

//AUMI replies sent to the server.
//Buffer contents change depending on the function called.
//The only thing guaranteed to have safe values is the AUMIResult member.
struct IPCReply_t
{
	int AUMIResult;
	char Buffer[128];
};

void AUMI_RunIPC();

void AUMI_StopIPC();


extern void IpcTestCommunication(IPCMessage_t* Message, IPCReply_t* OutReply);

extern void IpcGetFunctionByIndex(IPCMessage_t* Message, IPCReply_t* OutReply);

extern void IpcGetFunctionByName(IPCMessage_t* Message, IPCReply_t* OutReply);

extern void IpcExecuteCode(IPCMessage_t* Message, IPCReply_t* OutReply);