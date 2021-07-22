#include "Code_Execute.hpp"
#include "../../Features/AUMI_API/Exports.hpp"
#include "../../Utils/Error.hpp"

namespace Hooks::Code_Execute
{
	bool Function(CInstance* pSelf, CInstance* pOther, CCode* Code, RValue* Res, int Flags)
	{
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