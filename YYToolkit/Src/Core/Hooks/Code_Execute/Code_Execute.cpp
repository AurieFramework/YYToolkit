#include "Code_Execute.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Error.hpp"

namespace Hooks::Code_Execute
{
	bool Function(CInstance* pSelf, CInstance* pOther, CCode* Code, RValue* Res, int Flags)
	{
		auto yyvarsMap = gAPIVars.g_pGlobal->m_yyvarsMap;
		auto Buckets = yyvarsMap->m_pBuckets;
		auto pFirstElem = Buckets[0]; auto pSecondElem = Buckets[1];
		auto pThirdElem = Buckets[2];
		return pfnOriginal(pSelf, pOther, Code, Res, Flags);
	}

	void* GetTargetAddress()
	{
		return gAPIVars.Code_Execute;
	}
}