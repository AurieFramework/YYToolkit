#include "Code_Execute.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Error.hpp"

namespace Hooks::Code_Execute
{
	bool Function(CInstance* pSelf, CInstance* pOther, CCode* Code, RValue* Res, int Flags)
	{
		YYTKEvent Event("CodeEvent", reinterpret_cast<void*>(pfnOriginal), {&pSelf, &pOther, &Code, &Res, &Flags});
		
		Plugins::RunCallback(&Event);

		if (!Event.CalledOriginal())
			return pfnOriginal(pSelf, pOther, Code, Res, Flags);
	}

	void* GetTargetAddress()
	{
		return reinterpret_cast<void*>(gAPIVars.Code_Execute);
	}
}