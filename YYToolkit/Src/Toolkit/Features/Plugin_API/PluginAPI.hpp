#pragma once
#include "../../Utils/SDK.hpp"

using YYTK_CodeCallback = int(__cdecl*)(PFUNC_CEXEC Original, CInstance* pSelf, CInstance* pOther, CCode* Code, RValue* Res, int Flags);

struct PluginState { int m_nID; };

namespace Tool::API 
{ 
	inline YYTK_CodeCallback g_pExecuteCallback = nullptr; 
	inline std::vector<PluginState> PluginStates;
};

DllExport const PluginState& YYTK_CreateState();