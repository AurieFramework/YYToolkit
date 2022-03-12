#include "API.hpp"
#include "../../Utils/Error/Error.hpp"

bool API::GetFunctionByName(const std::string& Name, TRoutine& outRoutine)
{
	YYTKStatus Status = Internal::VfLookupFunction(Name.c_str(), outRoutine, nullptr);

	return Status == YYTK_OK;
}

const char* API::GetSDKVersion()
{
	return YYSDK_VERSION;
}

bool API::GetGlobalInstance(CInstance*& outInstance)
{
	TRoutine Function;
	RValue Result;

	// This function returns the global scope
	if (!GetFunctionByName("@@GlobalScope@@", Function))
	{
		Utils::Error::Error(false, "API Error -> %s() : L%d : ", __FUNCTION__, __LINE__);
		return false;
	}

	// Functions should be callable
	if (!Function)
	{
		Utils::Error::Error(false, "API Error -> %s() : L%d : ", __FUNCTION__, __LINE__);
		return false;
	}

	// Call it and save the result.
	Function(&Result, nullptr, nullptr, 0, nullptr);

	outInstance = Result.Instance;
	return true;
}

DllExport bool API::IsYYC()
{
	return IsGameYYC();
}

DllExport bool API::IsGameYYC()
{
	TRoutine Routine = nullptr;
	bool Success = GetFunctionByName("code_is_compiled", Routine);

	// The function shouldn't be nullptr, and it should exist.
	if (!Routine || !Success)
	{
		Utils::Error::Error(false, "API Error -> %s() : L%d", __FUNCTION__, __LINE__);
		return false;
	}

	// We can safely call it
	RValue Result;
	Routine(&Result, nullptr, nullptr, 0, nullptr);

	return Result.Real > 0.5;
}

DllExport bool API::CallBuiltin(YYRValue& Result, const std::string& Name, CInstance* Self, CInstance* Other, const std::vector<YYRValue>& Args)
{
	bool bShouldUseGlobalInstance = (!Self && !Other);

	TRoutine Function = nullptr;

	// The function should exist.
	if (!GetFunctionByName(Name, Function))
	{
		Utils::Error::Error(false, "API Error -> %s(\"%s\") : L%d", __FUNCTION__, Name.c_str(), __LINE__);
		return false;
	}
	
	// Builtins should have a function mapped to them, and that function shouldn't be a nullptr.
	if (!Function)
	{
		Utils::Error::Error(false, "API Error -> %s(\"%s\") : L%d", __FUNCTION__, Name.c_str(), __LINE__);
		return false;
	}
	
	// The only C-style casts in this API, since the C++ equivalent is awfully long.
	// reinterpret_cast<RValue*>(const_cast<YYRValue*>(Args.data()));

	if (bShouldUseGlobalInstance)
	{
		CInstance* g_pGlobal = nullptr;
		if (!GetGlobalInstance(g_pGlobal))
		{
			Utils::Error::Error(false, "API Error -> %s(\"%s\") : L%d", __FUNCTION__, Name.c_str(), __LINE__);
			return false;
		}
		
		// Call as if it was a 2.3 script, both are self and other refer to global.
		Function((RValue*)&Result, g_pGlobal, g_pGlobal, Args.size(), (RValue*)Args.data());
	}
	else
	{
		// Use argument-supplied instances
		Function((RValue*)&Result, Self, Other, Args.size(), (RValue*)Args.data());
	}

	return true;
}

DllExport unsigned long API::FindPattern(const char* Pattern, const char* Mask, unsigned long Base, unsigned long Size)
{
	DWORD dwReturn = 0;

	if (auto Status = Internal::MmFindByteArray(Pattern, UINT_MAX, Base, Size, Mask, dwReturn))
		Utils::Error::Error(false, "API Error -> %s(\"%s\", \"%s\") -> %s : L%d", __FUNCTION__, Pattern, Mask, Utils::Error::YYTKStatus_ToString(Status).c_str(), __LINE__);

	return dwReturn;
}
