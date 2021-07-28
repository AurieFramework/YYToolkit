#include "Code_Execute.hpp"
#include "../../Features/AUMI_API/Exports.hpp"
#include "../../Features/Plugin_API/PluginAPI.hpp"
#include "../../Utils/Error.hpp"

namespace Hooks::Code_Execute
{
	bool Function(CInstance* pSelf, CInstance* pOther, CCode* Code, RValue* Res, int Flags)
	{
		if (Tool::API::g_pExecuteCallback)
		{
			int _Result = Tool::API::g_pExecuteCallback((PFUNC_CEXEC)pfnOriginal, pSelf, pOther, Code, Res, Flags);

			if (_Result != -1)
				return (bool)_Result;
		}

		return pfnOriginal(pSelf, pOther, Code, Res, Flags);
	}

	void* GetTargetAddress()
	{
		YYTKTrace(__FUNCTION__ "()", __LINE__);

		void* Buffer = nullptr;

		if (auto Status = AUMI_GetCodeExecuteAddress(&Buffer))
		{
			Utils::Error::Error(1, "Failed to get the Code_Execute() pointer.");
		}
		
		return Buffer;
	}
}