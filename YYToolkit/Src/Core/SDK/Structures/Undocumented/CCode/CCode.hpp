#pragma once
#include "../RToken/RToken.hpp" // Includes YYRValue.hpp and Enums.hpp

struct VMBuffer;
struct YYObjectBase;

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

struct YYGMLFuncs
{
	const char* pName;
	TGMLRoutine pFunc;
	YYVAR* pFuncVar;
};

struct CCode
{
	int (**_vptr$CCode)(void);
	CCode* m_pNext;
	int i_kind;
	bool i_compiled;
	const char* i_str;
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