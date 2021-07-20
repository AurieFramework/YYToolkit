#include "../Hooks.hpp"
#include "../../Utils/SDK.hpp"

namespace Hooks
{
	bool Code_Execute(CInstance* pSelf, CInstance* pOther, CCode* Code, RValue* Res, int Flags)
	{
		using Fn = decltype(&Code_Execute);

		return ((Fn)oCode_Execute)(pSelf, pOther, Code, Res, Flags);
	}
}