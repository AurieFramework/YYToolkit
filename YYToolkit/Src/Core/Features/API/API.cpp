#include "../../Utils/Logging/Logging.hpp"
#include "Internal.hpp"
#include "API.hpp"
#include "../../Utils/PortableFileDialog/PFD.hpp"

bool API::GetFunctionByName(const std::string& Name, TRoutine& outRoutine)
{
	YYTKStatus Status = Internal::VfLookupFunction(Name.c_str(), outRoutine, nullptr);

	if (Status)
	{
		Utils::Logging::Error(
			__FILE__,
			__LINE__,
			"%s(\"%s\") returned %s",
			__FUNCTION__,
			Name.c_str(),
			Utils::Logging::YYTKStatus_ToString(Status).c_str()
		);
	}
	
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
		return false;

	// Functions should be callable
	if (!Function)
		return false;

	// Call it and save the result.
	Function(&Result, nullptr, nullptr, 0, nullptr);

	if (!Result.Instance)
		Utils::Logging::Error(__FILE__, __LINE__, "%s failed - Instance was nullptr!", __FUNCTION__);

	outInstance = Result.Instance;
	return true;
}

bool API::IsYYC()
{
	return IsGameYYC();
}

bool API::IsGameYYC()
{
	TRoutine Routine = nullptr;
	bool Success = GetFunctionByName("code_is_compiled", Routine);

	// The function shouldn't be nullptr, and it should exist.
	if (!Routine || !Success)
	{
		Utils::Logging::Error(
			__FILE__,
			__LINE__,
			"The code_is_compiled function couldn't be found"
		);
		return false;
	}

	// We can safely call it
	RValue Result;
	Routine(&Result, nullptr, nullptr, 0, nullptr);

	return Result.Real > 0.5;
}

bool API::CallBuiltin(YYRValue& Result, const std::string& Name, CInstance* Self, CInstance* Other, const std::vector<YYRValue>& Args)
{
	bool bShouldUseGlobalInstance = (!Self && !Other);

	TRoutine Function = nullptr;

	// The function should exist.
	if (!GetFunctionByName(Name, Function))
	{
		Utils::Logging::Error(
			__FILE__,
			__LINE__,
			"%s(\"%s\") returned %s",
			__FUNCTION__,
			Name.c_str(),
			"false"
		);
		return false;
	}
	
	// Builtins should have a function mapped to them, and that function shouldn't be a nullptr.
	if (!Function)
	{
		Utils::Logging::Error(
			__FILE__,
			__LINE__,
			"Function was nullptr"
		);
		return false;
	}
	
	// The only C-style casts in this API, since the C++ equivalent is awfully long.
	// reinterpret_cast<RValue*>(const_cast<YYRValue*>(Args.data()));

	if (bShouldUseGlobalInstance)
	{		
		// Call as if it was a 2.3 script, both are self and other refer to global.
		Function((RValue*)&Result, gAPIVars.Globals.g_pGlobalInstance, gAPIVars.Globals.g_pGlobalInstance, Args.size(), (RValue*)Args.data());
	}
	else
	{
		// Use argument-supplied instances
		Function((RValue*)&Result, Self, Other, Args.size(), (RValue*)Args.data());
	}

	return true;
}

uintptr_t API::FindPattern(const char* Pattern, const char* Mask, uintptr_t Base, uintptr_t Size)
{
	uintptr_t dwReturn = 0;

	if (auto Status = Internal::MmFindByteArray(Pattern, UINT_MAX, Base, Size, Mask, false, dwReturn))
	{
		Utils::Logging::Error(
			__FILE__,
			__LINE__,
			"%s(\"%s\", \"%s\") returned %s",
			__FUNCTION__,
			Pattern,
			Mask,
			Utils::Logging::YYTKStatus_ToString(Status).c_str()
		);
	}

	return dwReturn;
}

void API::PopToastNotification(const std::string& Text, const std::string& Caption, int IconType)
{
	pfd::notify(Caption, Text, static_cast<pfd::icon>(IconType));
}

void API::PopFileOpenDialog(const std::string& WindowTitle, const std::string& InitialPath, const std::vector<std::string>& Filters, bool AllowMultiselect, std::vector<std::string>& outSelected)
{
	outSelected = pfd::open_file(WindowTitle, InitialPath, Filters, (AllowMultiselect ? pfd::opt::multiselect : pfd::opt::none)).result();
}

void API::PrintMessage(Color color, const char* fmt, ...)
{
	va_list vaArgs;
	va_start(vaArgs, fmt);
	std::string Message = Utils::Logging::ParseVA(fmt, vaArgs);
	va_end(vaArgs);

	return Utils::Logging::Message(color, Message.c_str());
}

void API::PrintMessageNoNewline(Color color, const char* fmt, ...)
{
	va_list vaArgs;
	va_start(vaArgs, fmt);
	std::string Message = Utils::Logging::ParseVA(fmt, vaArgs);
	va_end(vaArgs);

	return Utils::Logging::NoNewlineMessage(color, Message.c_str());
}

void API::PrintError(const char* File, const int& Line, const char* fmt, ...)
{
	va_list vaArgs;
	va_start(vaArgs, fmt);
	std::string Message = Utils::Logging::ParseVA(fmt, vaArgs);
	va_end(vaArgs);

	return Utils::Logging::Error(File, Line, Message.c_str());
}