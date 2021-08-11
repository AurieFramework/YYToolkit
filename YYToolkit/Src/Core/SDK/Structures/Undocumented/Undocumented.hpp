#pragma once
#include "../../FwdDecls/FwdDecls.hpp"

typedef void (*TRoutine)(YYRValue* Result, YYObjectBase* Self, YYObjectBase* Other, int argc, YYRValue* Args);
typedef void (*TGMLRoutine)(YYObjectBase* Self, YYObjectBase* Other);
typedef bool (*TCodeExecuteRoutine)(YYObjectBase* Self, YYObjectBase* Other, CCode* code, YYRValue* res, int flags);
typedef void (*TGetTheFunctionRoutine)(int id, char** bufName, void** bufRoutine, int* bufArgs, void* unused);


struct YYGMLFuncs
{
	const char* pName;
	TGMLRoutine pFunc;
	YYVAR* pFuncVar;
};

struct VMBuffer
{
	void** vTable;
	int m_size;
	int m_numLocalVarsUsed;
	int m_numArguments;
	char* m_pBuffer;
	void** m_pConvertedBuffer;
	char* m_pJumpBuffer;
};

struct CCode
{
	int (**_vptr$CCode)(void);
	CCode* m_pNext;
	int i_kind;
	int i_compiled;
	String i_str;
	RToken i_token;
	RValue i_value;
	VMBuffer* i_pVM;
	VMBuffer* i_pVMDebugInfo;
	char* i_pCode;
	const char* i_pName;
	int i_CodeIndex;
	YYGMLFuncs* i_pFunc;
	bool i_watch;
	int i_offset;
	int i_locals;
	int i_args;
	int i_flags;
	YYObjectBase* i_pPrototype;
};
