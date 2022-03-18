#pragma once
#include "../../Documented/CCode/CCode.hpp"
#include "../../Documented/YYRValue/YYRValue.hpp"
#include "../../Documented/VMBuffer/VMBuffer.hpp"

struct VMExec
{
	VMExec* pPrev;
	VMExec* pNext;
	char* pStack;
	int localCount;
	YYObjectBase* pLocals;
	YYObjectBase* pSelf;
	YYObjectBase* pOther;
	CCode* pCCode;
	YYRValue* pArgs;
	int argumentCount;
	const char* pCode;
	char* pBP;
	VMBuffer* pBuffer;
	int line;
	const char* pName;
	VMBuffer* pDebugInfo;
	const char* pScript;
	int stackSize;
	int offs;
	int boffs;
	int retCount;
	int bufferSize;
	int prevoffs;
	void** buff;
	int* jt;
};
