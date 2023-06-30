#pragma once
#include "../../../FwdDecls/FwdDecls.hpp"

struct YYGMLFuncs
{
	const char* pName;
	union
	{
		PFUNC_YYGMLScript pScriptFunc;
		PFUNC_YYGML pFunc;
	};
	YYVAR* pFuncVar;
};
