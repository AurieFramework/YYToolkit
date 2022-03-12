#include "Code_Execute.hpp"
#include "../../Features/API/API.hpp"
#include "../../Features/PluginManager/PluginManager.hpp"
#include "../../Utils/Error/Error.hpp"
#include "../../SDK/Plugins/YYTKEvent/YYTKEvent.hpp"

namespace Hooks
{
	namespace Code_Execute
	{
		bool Function(CInstance* pSelf, CInstance* pOther, CCode* Code, RValue* Res, int Flags)
		{
			YYTKCodeEvent Event = YYTKCodeEvent(pfnOriginal, pSelf, pOther, Code, Res, Flags);
			API::PluginManager::RunHooks(&Event);

			if (Event.CalledOriginal())
				return Event.GetReturn();

			return pfnOriginal(pSelf, pOther, Code, Res, Flags);
		}

		void* GetTargetAddress()
		{
			return reinterpret_cast<void*>(API::gAPIVars.Functions.Code_Execute);
		}
	}
}