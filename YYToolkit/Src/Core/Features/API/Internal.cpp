#include "../PluginManager/PluginManager.hpp"
#include "../../Utils/Logging/Logging.hpp"
#include "../../Utils/MH/hde/hde32.h"
#include "../UnitTests/UnitTests.hpp"
#include "Internal.hpp"
#include "API.hpp"

#ifdef min
#undef min
#endif

YYTKStatus API::Internal::__Initialize__(HMODULE hMainModule)
{
	if (gAPIVars.Globals.g_bWasPreloaded)
	{
		uintptr_t dwGetDevice = 0;
		YYTKStatus Status = VfGetFunctionPointer("window_device", EFPType::FPType_AssemblyReference, dwGetDevice);

		if (Status || !dwGetDevice)
			Utils::Logging::Critical(
				__FILE__,
				__LINE__, 
				"VfGetFunctionPointer(\"window_device\") failed with %s", 
				Utils::Logging::YYTKStatus_ToString(Status).c_str()
			);

		TRoutine pfnGetDevice = reinterpret_cast<TRoutine>(dwGetDevice);

		void* hwWindow = nullptr;

		// Wait until the runner has a window before we initialize the API
		while (!hwWindow)
		{
			// stfu clang
			if (!pfnGetDevice)
				break;

			RValue Result = RValue();
			pfnGetDevice(&Result, nullptr, nullptr, 0, nullptr);

			hwWindow = Result.Pointer;
		}
	}

	__InitializeGlobalVars__();
	Tests::RunUnitTests();
	PluginManager::RunPluginMains();

	return YYTK_OK;
}

YYTKStatus API::Internal::__InitializeGlobalVars__()
{
	// Initialize Code_Function_GET_the_function
	{
		YYTKStatus Status = API::Internal::MmFindCodeFunction(reinterpret_cast<uintptr_t&>(gAPIVars.Functions.Code_Function_GET_the_function));

		if (Status)
			Utils::Logging::Error(__FILE__, __LINE__, "API::Internal::MmFindCodeFunction() failed with %s",
				Utils::Logging::YYTKStatus_ToString(Status).c_str());
	}

	// Initialize Code_Execute
	{
		YYTKStatus Status = API::Internal::MmFindCodeExecute(reinterpret_cast<uintptr_t&>(gAPIVars.Functions.Code_Execute));

		if (Status)
			Utils::Logging::Error(__FILE__, __LINE__, "API::Internal::MmFindCodeExecute() failed with %s",
				Utils::Logging::YYTKStatus_ToString(Status).c_str());
	}

	// Initialize g_hwWindowHandle
	{
		YYRValue Result = {};
		API::CallBuiltin(Result, "window_handle", nullptr, nullptr, {});

		API::gAPIVars.Globals.g_hwWindowHandle = reinterpret_cast<HWND>(Result.As<RValue>().Pointer);
	}

	// Initialize g_pGlobalInstance
	{
		YYRValue Result = {};
		API::CallBuiltin(Result, "@@GlobalScope@@", nullptr, nullptr, {});

		API::gAPIVars.Globals.g_pGlobalObject = Result;
	}

	// Initialize g_WindowDevice
	{
		YYRValue Result = {};
		API::CallBuiltin(Result, "window_device", nullptr, nullptr, {});

		API::gAPIVars.Globals.g_pWindowDevice = Result.As<RValue>().Pointer;
	}

	return YYTK_OK;
}

YYTKStatus API::Internal::__InitializeConsole__()
{
	AllocConsole();
	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);

	// Set console title scope
	{
		std::string sTitleString = std::string("YYToolkit Log (v") + YYSDK_VERSION + ")";
#ifdef _WIN64
		sTitleString.append(" - x64");
#endif

		SetConsoleTitleA(sTitleString.c_str());
	}

	// Disable the "left-click to select" autism which pauses the entire tool
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	DWORD dwMode;
	GetConsoleMode(hInput, &dwMode);
	SetConsoleMode(hInput, ENABLE_EXTENDED_FLAGS | (dwMode & ~ENABLE_QUICK_EDIT_MODE));


#if _DEBUG
	Utils::Logging::Message(CLR_GOLD, "YYToolkit %s (Debug) by Archie#3274", YYSDK_VERSION);
#else
	Utils::Logging::Message(CLR_LIGHTBLUE, "YYToolkit by Archie#3274", YYSDK_VERSION);
#endif

	return YYTK_OK;
}

YYTKStatus API::Internal::__Unload__()
{
	PluginManager::Uninitialize();

	ShowWindow(GetConsoleWindow(), SW_HIDE);
	FreeConsole();

	API::PopToastNotification("YYToolkit has been unloaded.", "Goodbye!", 0);

	return YYTK_OK;
}

YYTKStatus API::Internal::MmGetModuleInformation(const char* szModuleName, CModule& outModule)
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

YYTKStatus API::Internal::MmFindByteArray(const unsigned char* pbArray, size_t uArraySize, uintptr_t ulSearchRegionBase, uintptr_t ulSearchRegionSize, const char* szMask, bool bStringSearch, uintptr_t& dwOutBuffer)
{
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
			found &= szMask[j] == '?' || pbArray[j] == *(const unsigned char*)(ulSearchRegionBase + i + j);
		}

		if (found)
		{
			if (bStringSearch && (*reinterpret_cast<char*>(ulSearchRegionBase + i - 1) != '\x00'))
				continue;

			dwOutBuffer = (ulSearchRegionBase + i);
			return YYTK_OK;
		}
	}

	return YYTK_NOMATCH;
}

YYTKStatus API::Internal::MmFindByteArray(const char* pszArray, size_t uArraySize, uintptr_t ulSearchRegionBase, uintptr_t ulSearchRegionSize, const char* szMask, bool bStringSearch, uintptr_t& dwOutBuffer)
{
	return MmFindByteArray(reinterpret_cast<unsigned char*>(const_cast<char*>(pszArray)), uArraySize, ulSearchRegionBase, ulSearchRegionSize, szMask, bStringSearch, dwOutBuffer);
}

YYTKStatus API::Internal::MmFindCodeExecute(uintptr_t& dwOutBuffer)
{
	uintptr_t dwPattern = 0;
#ifdef _WIN64

	// New method for 2023.x
	// Is inconsistent, not using it for now...
	/*
	if (!MmFindByteArray(
		"\xE8\x00\x00\x00\x00\x3C\x01\x74\x15",
		UINT_MAX,
		0,
		0,
		"x????xxxx",
		false,
		dwPattern
	))
	{
		dwOutBuffer = dwPattern + 5 + *(int32_t*)(dwPattern + 1);
		return YYTK_OK;
	}
	*/

	// direct ref
	// address in opcode
	if (!MmFindByteArray(
		"\xE8\x00\x00\x00\x00\x0F\xB6\xD8\x3C\x01",
		UINT_MAX,
		0,
		0,
		"x????xxxxx",
		false,
		dwPattern
	))
	{
		if (dwPattern)
		{
			unsigned long Relative = *reinterpret_cast<int32_t*>(dwPattern + 1);
			dwOutBuffer = (dwPattern + 5) + Relative; // eip = instruction base + 5 + relative offset

			if (dwOutBuffer)
				return YYTK_OK;
		}
	}

	if (!MmFindByteArray(
		"\x4C\x8B\x50\x08\x75\x18",
		UINT_MAX,
		0,
		0,
		"xxxxxx",
		false,
		dwPattern
	))
	{
		dwOutBuffer = dwPattern;
		return YYTK_OK;
	}
	
	return YYTK_NOMATCH;
#else

	if (YYTKStatus _Status = MmFindByteArray(
		"\x8A\xD8\x83\xC4\x14\x80\xFB\x01\x74",
		UINT_MAX,
		0,
		0,
		"xxxxxxxxx",
		false,
		dwPattern
	))
		return _Status;

	if (dwPattern == 0)
		return YYTK_INVALIDRESULT;

	while (*reinterpret_cast<WORD*>(dwPattern - 2) != 0xCCCC)
		dwPattern -= 1;

	dwOutBuffer = dwPattern;

	return YYTK_OK;
#endif
}


struct RFunction
{
	union {
		unsigned char padding[64];
		char* name;
	};

	void* function;
	int32_t argc;
	int32_t padding2;
};

static RFunction** the_functions_array = nullptr;

static void dummy_find_function(int id, char** bufName, void** bufRoutine, int* bufArgs, void* unused)
{
	*bufName = (char*)(&(*the_functions_array)[id].name);
	*bufRoutine = (*the_functions_array)[id].function;
	*bufArgs = (*the_functions_array)[id].argc;
}

YYTKStatus API::Internal::MmFindCodeFunction(uintptr_t& dwOutBuffer)
{
#if _WIN64
	YYTKStatus Status = MmFindByteArray("\x3B\x0D\x00\x00\x00\x00\x7F\x3A", UINT_MAX, 0, 0, "xx????xx", false, dwOutBuffer);

	// If we found it using the old method
	if (!Status)
		return YYTK_OK;

	// I literally can't find better words to describe this
	// Finds the_functions array in FINALIZE_Code_Function
	uintptr_t holy_shit_result = 0;
	YYTKStatus holy_shit_pattern = MmFindByteArray(
		"\x48\x8B\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x33\xC0\x48\x89\x05\x00\x00\x00\x00\x89\x05\x00\x00\x00\x00",
		UINT_MAX,
		0,
		0,
		"xxx????x????xxxxx????xx????",
		false,
		holy_shit_result
	);

	// We failed anyway...
	if (holy_shit_pattern || holy_shit_result == 0)
	{
		return holy_shit_pattern;
	}

	the_functions_array = (RFunction**)(holy_shit_result + 7ULL + *(int32_t*)(holy_shit_result + 3));
	dwOutBuffer = (uintptr_t)dummy_find_function;
	return YYTK_OK;
#else
	return MmFindByteArray("\x8B\x44\x24\x04\x3B\x05\x00\x00\x00\x00\x7F", UINT_MAX, 0, 0, "xxxxxx????x", false, dwOutBuffer);
#endif
}

YYTKStatus API::Internal::VfGetFunctionPointer(const char* szFunctionName, EFPType ePointerType, uintptr_t& dwOutBuffer)
{
	if (!szFunctionName)
		return YYTK_INVALIDARG;

	// Assembly References aren't possible on x64 because the address is relative to RIP
#ifndef _WIN64
	bool bShouldFindAssemblyReference = (ePointerType == FPType_AssemblyReference);

	if (bShouldFindAssemblyReference)
	{
		uintptr_t dwStringReference = 0;

		std::string Mask(strlen(szFunctionName) + 1, 'x');

		if (YYTKStatus _Status = MmFindByteArray(
			reinterpret_cast<const unsigned char*>(szFunctionName), 
			UINT_MAX, 
			0,
			0,
			Mask.c_str(),
			true,
			dwStringReference
		))
			return _Status;

		if (dwStringReference == 0)
			return YYTK_INVALIDRESULT;

		unsigned char* pbNewPattern = new unsigned char[5];
		pbNewPattern[0] = 0x68;

		memcpy(pbNewPattern + 1, &dwStringReference, sizeof(DWORD));

		if (YYTKStatus _Status = MmFindByteArray(
			const_cast<const unsigned char*>(pbNewPattern),
			5,
			0,
			0,
			"xxxxx",
			false,
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
#endif
	{
		TRoutine pRoutine = nullptr;
		YYTKStatus stLookupResult = Internal::VfLookupFunction(szFunctionName, pRoutine, nullptr);

		if (stLookupResult)
			return stLookupResult;

		if (!pRoutine)
			return YYTK_INVALIDRESULT;

		dwOutBuffer = reinterpret_cast<uintptr_t>(pRoutine);
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

YYTKStatus API::Internal::VfGetIdByName(YYObjectBase* pObject, const char* szName, int& outId)
{
#ifndef _DEBUG
	return YYTK_UNAVAILABLE;
#endif

	uintptr_t dwGlobalGet = 0;
	uintptr_t dwCallAddress = 0;

	// Find function scope
	{
		YYTKStatus Status = VfGetFunctionPointer("variable_global_get", EFPType::FPType_DirectPointer, dwGlobalGet);

		if (Status)
			return Status;
		else if (dwGlobalGet == 0)
			return YYTK_INVALIDRESULT;
	}

	// Find byte array scope
	{
		YYTKStatus Status = MmFindByteArray("\xE8\x00\x00\x00\x00\x6A\x00\x6A\x00", UINT_MAX, dwGlobalGet, 0xFF, "x????xxxx", false, dwCallAddress);

		if (Status)
			return Status;
		else if (dwCallAddress == 0)
			return YYTK_INVALIDRESULT;
	}

	uintptr_t Relative = *reinterpret_cast<uintptr_t*>(dwCallAddress + 1);
	Relative += (dwCallAddress + 5); // eip = instruction base + 5 + relative offset

	if (Relative == 0)
		return YYTK_INVALIDRESULT;

	using FNCodeVariableFindSlot = int(*)(YYObjectBase* _pObj, const char* _name);

	FNCodeVariableFindSlot fn = reinterpret_cast<FNCodeVariableFindSlot>(Relative);

	outId = fn(pObject, szName);

	return YYTK_OK;
}

DllExport YYTKStatus API::Internal::VfGetAPIState(CAPIVars*& outState)
{
	outState = &gAPIVars;
	return YYTK_OK;
}

YYTKStatus API::Internal::MmGetScriptArrayPtr(CDynamicArray<CScript*>*& outArray, const int& nMaxInstructions)
{
#if _WIN64
	return YYTK_UNAVAILABLE;
#else
	uintptr_t dwScriptExists = 0;

	if (YYTKStatus Status = VfGetFunctionPointer("script_exists", EFPType::FPType_DirectPointer, dwScriptExists))
		return Status;

	if (dwScriptExists == 0)
		return YYTK_INVALIDRESULT;

	// call script_exists
	// xor ecx, ecx
	// add esp, 0Ch
	uintptr_t pPattern = 0;
	YYTKStatus Status = MmFindByteArray("\xE8\x00\x00\x00\x00\x00\xC9\x83\xC4\x0C", UINT_MAX, dwScriptExists, 0xFF, "x?????xxxx", false, pPattern);

	if (Status)
		return Status;

	if (!pPattern)
		return YYTK_NOMATCH;

	// Get 4 bytes from the JMP opcode (the relative offset)
	uintptr_t pJmpOffset = *reinterpret_cast<uintptr_t*>(pPattern + 1);

	// Calculate the jumped-to address, it's relative from the end of the jmp instruction, the size of which is 5 bytes.
	uintptr_t pFunction = (pPattern + 5) + pJmpOffset;

	hde32s hsInstruction;
	memset(&hsInstruction, 0, sizeof(hde32s));

	for (int nInstr = 0; nInstr < nMaxInstructions; nInstr++)
	{
		int nInstructionSize = hde32_disasm(reinterpret_cast<const void*>(pFunction), &hsInstruction);

		// If the opcodes match what we're searching for
		if ((hsInstruction.opcode == 0xA1 || (hsInstruction.opcode == 0x8B && hsInstruction.modrm)) && nInstructionSize > 4)
		{
			if (hsInstruction.imm.imm32)
				outArray = reinterpret_cast<CDynamicArray<CScript*>*>(hsInstruction.imm.imm32 - sizeof(long*));
			else
				outArray = reinterpret_cast<CDynamicArray<CScript*>*>(hsInstruction.disp.disp32 - sizeof(long*));

			return YYTK_OK;
		}
	
		// Skip to the next instruction
		pFunction += nInstructionSize;
	}

	return YYTK_NOT_FOUND;
#endif
}

DllExport YYTKStatus API::Internal::MmGetScriptData(CScript*& outScript, int index)
{
#ifdef _WIN64
	using FNScriptData = CScript*(*)(int);
	
	if (index < 0)
		return YYTK_INVALIDARG;

	uintptr_t FuncCallPattern = FindPattern("\xE8\x00\x00\x00\x00\x33\xC9\x0F\xB7\xD3", "x????xxxxx", 0, 0);

	if (!FuncCallPattern)
		return YYTK_INVALIDRESULT;

	uintptr_t Relative = *reinterpret_cast<uint32_t*>(FuncCallPattern + 1);
	Relative = (FuncCallPattern + 5) + Relative;

	if (!Relative)
		return YYTK_INVALIDRESULT;

	outScript = reinterpret_cast<FNScriptData>(Relative)(index);

	return YYTK_OK;
#else
	return YYTK_UNAVAILABLE;
#endif
}
