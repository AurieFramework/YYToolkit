#include "API.hpp"
#include "../../Utils/Error/Error.hpp"
#include "../PluginManager/PluginManager.hpp"
#ifdef min
#undef min
#endif

YYTKStatus API::Internal::Initialize(HMODULE hMainModule)
{
	// Don't forget this!
	gAPIVars.Globals.g_hMainModule = hMainModule;

	// Allocate console
	AllocConsole();
	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	SetConsoleTitleA("YYTK Log Window");

	if (GetAsyncKeyState(VK_F9))
	{
		SetConsoleTitleA("YYToolkit Console (Developer Mode)");
		gAPIVars.Globals.g_bDebugMode = true;
	}

	// Print the version into the log
#if _DEBUG
	Utils::Error::Message(CLR_GOLD, "YYToolkit %s (Debug) by Archie#8615", YYSDK_VERSION);
#else
	Utils::Error::Message(CLR_LIGHTBLUE, "YYToolkit %s (Release) by Archie#8615", YYSDK_VERSION);
#endif

	// Initialize Code_Function_GET_the_function
	{
		YYTKStatus Status = API::Internal::MmFindCodeFunction(reinterpret_cast<DWORD&>(gAPIVars.Functions.Code_Function_GET_the_function));

		if (Status)
			Utils::Error::Message(CLR_TANGERINE, "[Warning] API::Internal::MmFindCodeFunction() failed. Error: %s", 
				Utils::Error::YYTKStatus_ToString(Status).c_str());
	}

	// Initialize Code_Execute
	{
		YYTKStatus Status = API::Internal::MmFindCodeExecute(reinterpret_cast<DWORD&>(gAPIVars.Functions.Code_Execute));

		if (Status)
			Utils::Error::Message(CLR_TANGERINE, "[Warning] API::Internal::MmFindCodeExecute() failed. Error: %s",
				Utils::Error::YYTKStatus_ToString(Status).c_str());
	}

	// Initialize g_hwWindowHandle
	{
		CInstance tmp;
		memset(&tmp, 0, sizeof(tmp));

		YYRValue Result;
		bool Success = API::CallBuiltin(Result, "window_handle", &tmp, &tmp, {});

		if (!Success)
			Utils::Error::Message(CLR_TANGERINE, "[Warning] API::CallBuiltin(\"window_handle\") failed.");

		API::gAPIVars.Globals.g_hwWindowHandle = reinterpret_cast<HWND>(Result.As<RValue>().Pointer);
	}

	// Initialize g_pGlobalInstance
	{
		CInstance tmp;
		memset(&tmp, 0, sizeof(tmp));

		YYRValue Result;
		bool Success = API::CallBuiltin(Result, "@@GlobalScope@@", &tmp, &tmp, {});

		if (!Success)
			Utils::Error::Message(CLR_TANGERINE, "[Warning] API::CallBuiltin(\"@@GlobalScope@@\") failed.");

		API::gAPIVars.Globals.g_pGlobalObject = Result;
	}
	
	// Initialize g_WindowDevice
	{
		YYRValue Result;
		bool Success = API::CallBuiltin(Result, "window_device", nullptr, nullptr, {});

		if (!Success)
			Utils::Error::Message(CLR_TANGERINE, "[Warning] API::CallBuiltin(\"window_device\") failed.");

		API::gAPIVars.Globals.g_pWindowDevice = Result.As<RValue>().Pointer;

		// TODO: Assign the variant of g_pWindowDevice
	}

	PluginManager::Initialize(); // Remove me to turn off plugin functionality.

	return YYTK_OK;
}

YYTKStatus API::Internal::Unload()
{
	PluginManager::Uninitialize();

	FreeConsole();
	return YYTK_OK;
}

DllExport YYTKStatus API::Internal::MmGetModuleInformation(const char* szModuleName, CModule& outModule)
{
	using Fn = int(__stdcall*)(HANDLE, HMODULE, CModule*, DWORD);

	static HMODULE Module = GetModuleHandleA("kernel32.dll");
	static Fn K32GetModuleInformation = reinterpret_cast<Fn>(GetProcAddress(Module, "K32GetModuleInformation"));

	HMODULE hModule = GetModuleHandleA(szModuleName);

	if (hModule == nullptr)
		return YYTK_NOT_FOUND;

	K32GetModuleInformation(GetCurrentProcess(), hModule, &outModule, sizeof(CModule));
	
	return YYTK_OK;
}

YYTKStatus API::Internal::MmFindByteArray(const byte* pbArray, unsigned int uArraySize, unsigned long ulSearchRegionBase, unsigned int ulSearchRegionSize, const char* szMask, DWORD& dwOutBuffer)
{
	if (gAPIVars.Globals.g_bDebugMode)
		Utils::Error::Message(CLR_DEFAULT, "%s(%p, %s);", __FUNCTION__, pbArray, szMask);

	dwOutBuffer = 0x00;

	if (!ulSearchRegionBase && !ulSearchRegionSize)
	{
		CModule CurrentModuleInformation;

		if (YYTKStatus _Status = MmGetModuleInformation(nullptr, CurrentModuleInformation))
			return _Status;

		ulSearchRegionBase = CurrentModuleInformation.Base;
		ulSearchRegionSize = CurrentModuleInformation.Size;
	}

	size_t PatternSize = std::min(strlen(szMask), uArraySize);

	for (unsigned i = 0; i < ulSearchRegionSize - PatternSize; i++)
	{
		int found = 1;
		for (unsigned j = 0; j < PatternSize; j++)
		{
			found &= szMask[j] == '?' || pbArray[j] == *(const byte*)(ulSearchRegionBase + i + j);
		}

		if (found)
		{
			dwOutBuffer = (ulSearchRegionBase + i);
			return YYTK_OK;
		}
	}

	return YYTK_NOMATCH;
	
}

YYTKStatus API::Internal::MmFindByteArray(const char* pszArray, unsigned int uArraySize, unsigned long ulSearchRegionBase, unsigned int ulSearchRegionSize, const char* szMask, DWORD& dwOutBuffer)
{
	if (gAPIVars.Globals.g_bDebugMode)
		Utils::Error::Message(CLR_DEFAULT, "%s(%p, %s);", __FUNCTION__, pszArray, szMask);

	return MmFindByteArray(reinterpret_cast<byte*>(const_cast<char*>(pszArray)), uArraySize, ulSearchRegionBase, ulSearchRegionSize, szMask, dwOutBuffer);
}

YYTKStatus API::Internal::MmFindCodeExecute(DWORD& dwOutBuffer)
{
	if (gAPIVars.Globals.g_bDebugMode)
		Utils::Error::Message(CLR_DEFAULT, "%s();", __FUNCTION__);

	unsigned long dwPattern = 0;
	
	if (YYTKStatus _Status = MmFindByteArray(
		"\x8A\xD8\x83\xC4\x14\x80\xFB\x01\x74",
		UINT_MAX,
		0,
		0,
		"xxxxxxxxx",
		dwPattern
	))
		return _Status;

	if (dwPattern == 0)
		return YYTK_INVALIDRESULT;

	while (*reinterpret_cast<WORD*>(dwPattern - 2) != 0xCCCC)
		dwPattern -= 1;

	dwOutBuffer = dwPattern;

	return YYTK_OK;
}

YYTKStatus API::Internal::MmFindCodeFunction(DWORD& dwOutBuffer)
{
	if (gAPIVars.Globals.g_bDebugMode)
		Utils::Error::Message(CLR_DEFAULT, "%s();", __FUNCTION__);

	return MmFindByteArray("\x8B\x44\x24\x04\x3B\x05\x00\x00\x00\x00\x7F", UINT_MAX, 0, 0, "xxxxxx????x", dwOutBuffer);
}

YYTKStatus API::Internal::VfGetFunctionPointer(const char* szFunctionName, EFPType ePointerType, DWORD& dwOutBuffer)
{
	if (gAPIVars.Globals.g_bDebugMode)
		Utils::Error::Message(CLR_DEFAULT, "%s(%s, %d);", __FUNCTION__, szFunctionName, ePointerType);

	if (!szFunctionName)
		return YYTK_INVALIDARG;

	bool bShouldFindAssemblyReference = (ePointerType == FPType_AssemblyReference);

	if (bShouldFindAssemblyReference)
	{
		DWORD dwStringReference = 0;

		std::string Mask(strlen(szFunctionName), 'x');

		if (YYTKStatus _Status = MmFindByteArray(
			reinterpret_cast<const byte*>(szFunctionName), 
			UINT_MAX, 
			0,
			0,
			Mask.c_str(),
			dwStringReference
		))
			return _Status;

		if (dwStringReference == 0)
			return YYTK_INVALIDRESULT;

		byte* pbNewPattern = new byte[5];
		pbNewPattern[0] = 0x68;

		memcpy(pbNewPattern + 1, &dwStringReference, sizeof(DWORD));

		if (YYTKStatus _Status = MmFindByteArray(
			const_cast<const byte*>(pbNewPattern),
			5,
			0,
			0,
			"xxxxx",
			dwOutBuffer
		))
		{
			delete[] pbNewPattern;
			return _Status;
		}
			

		if (dwOutBuffer == 0)
		{
			delete[] pbNewPattern;
			return YYTK_INVALIDRESULT;
		}

		dwOutBuffer = *reinterpret_cast<DWORD*>(dwOutBuffer - sizeof(DWORD));

		delete[] pbNewPattern;
		return YYTK_OK;
	}
	
	else
	{
		TRoutine pRoutine = nullptr;
		auto bSuccess = API::GetFunctionByName(szFunctionName, pRoutine);

		if (!bSuccess)
			return YYTK_FAIL;

		if (!pRoutine)
			return YYTK_INVALIDRESULT;

		dwOutBuffer = reinterpret_cast<DWORD>(pRoutine);
	}

	return YYTK_OK;
}

YYTKStatus API::Internal::VfGetFunctionEntryFromGameArray(int nIndex, TRoutine* pOutRoutine, int* pOutArgumentCount, char** pOutNameBuffer)
{
	if (!pOutRoutine)
		return YYTK_INVALIDARG;

	if (nIndex < 0)
		return YYTK_INVALIDARG;

	bool bShouldPopulateRoutine = (pOutRoutine != nullptr);
	bool bShouldPopulateArgc = (pOutArgumentCount != nullptr);
	bool bShouldPopulateName = (pOutNameBuffer != nullptr);
	

	if (!gAPIVars.Functions.Code_Function_GET_the_function)
		return YYTK_UNAVAILABLE;

	void*	pTempRoutine		= nullptr;
	char*	pTempNameBuffer		= nullptr;
	int		nTempArgumentCount	= 0;
	void*	pTempUnknown		= nullptr;

	gAPIVars.Functions.Code_Function_GET_the_function(nIndex, &pTempNameBuffer, &pTempRoutine, &nTempArgumentCount, &pTempUnknown);

	if (!pTempNameBuffer) // If the function doesn't have a name assigned at all
		return YYTK_INVALIDRESULT;

	if (*pTempNameBuffer == '\0') // Builtins shouldn't have no names
		return YYTK_INVALIDRESULT; 
	
	if (bShouldPopulateRoutine)
		*pOutRoutine = reinterpret_cast<decltype(*pOutRoutine)>(pTempRoutine);

	if (bShouldPopulateArgc)
		*pOutArgumentCount = nTempArgumentCount;

	if (bShouldPopulateName)
		*pOutNameBuffer = pTempNameBuffer;

	return YYTK_OK;
}

YYTKStatus API::Internal::VfLookupFunction(const char* szFunctionName, TRoutine& outRoutine, int* pOptOutIndex)
{
	if (gAPIVars.Globals.g_bDebugMode)
		Utils::Error::Message(CLR_DEFAULT, "%s(%s, %p);", __FUNCTION__, szFunctionName, pOptOutIndex);

	if (!szFunctionName)
		return YYTK_INVALIDARG;

	if (*szFunctionName == '\0')
		return YYTK_INVALIDARG;

	TRoutine		pTempRoutine	= nullptr;
	const char*		pTempName		= nullptr;

	int nIndex = 0;

	while (VfGetFunctionEntryFromGameArray(nIndex, &pTempRoutine, nullptr, const_cast<char**>(&pTempName)) == YYTK_OK)
	{
		if (_strnicmp(pTempName, szFunctionName, 64) == 0)
		{
			outRoutine = pTempRoutine;

			if (pOptOutIndex)
				*pOptOutIndex = nIndex;

			return YYTK_OK;
		}

		nIndex++;
	}

	return YYTK_NOT_FOUND;
}