#include "Code_Execute.hpp"
#include "../../Features/API/API.hpp"
#include "../../Utils/Error/Error.hpp"
#include "../../SDK/Plugins/YYTKEvent/YYTKEvent.hpp"

namespace Hooks::Code_Execute
{
	bool Function(CInstance* pSelf, CInstance* pOther, CCode* Code, RValue* Res, int Flags)
	{
		YYTKCodeEvent Event = YYTKCodeEvent(pfnOriginal, pSelf, pOther, Code, Res, Flags);
		Plugins::RunHooks(&Event);

		if (Event.CalledOriginal())
			return Event.GetReturn();

		return pfnOriginal(pSelf, pOther, Code, Res, Flags);
	}

	void* GetTargetAddress()
	{
		return reinterpret_cast<void*>(gAPIVars.Code_Execute);
	}
}