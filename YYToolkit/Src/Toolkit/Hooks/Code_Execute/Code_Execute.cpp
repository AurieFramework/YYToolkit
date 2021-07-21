#include "../Hooks.hpp"
#include "../../Utils/SDK.hpp"

namespace Hooks
{
	bool Code_Execute(CInstance* pSelf, CInstance* pOther, CCode* Code, RValue* Res, int Flags)
	{
		return oCode_Execute(pSelf, pOther, Code, Res, Flags);
	}
}