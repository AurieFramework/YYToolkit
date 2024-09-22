// File: shared.hpp
// 
// Defines stuff shared between the Framework and its modules.
// Structs are meant to be opaque here, unless AURIE_INCLUDE_PRIVATE is defined.
// AURIE_INCLUDE_PRIVATE is set to 1 in the main Aurie Framework project through Visual Studio's Project Properties.

#ifndef AURIE_SHARED_H_
#define AURIE_SHARED_H_

// Includes
#include <cstdint>
#include <filesystem>

// Defines
#ifndef FORCEINLINE
#define FORCEINLINE __forceinline
#endif // FORCEINLINE

// Nameless structure / union
#pragma warning(disable : 4201)

#ifndef EXPORTED
#define EXPORTED extern "C" __declspec(dllexport)
#endif // EXPORTED

#ifndef IMPORTED
#define IMPORTED extern "C" __declspec(dllimport)
#endif // IMPORTED

#ifndef IN
#define IN
#endif // IN

#ifndef OUT
#define OUT	
#endif // OUT

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef AURIE_FWK_MAJOR
#define AURIE_FWK_MAJOR 1
#endif // AURIE_FWK_MAJOR

#ifndef AURIE_FWK_MINOR
#define AURIE_FWK_MINOR 0
#endif // AURIE_FWK_MINOR

#ifndef AURIE_FWK_PATCH
#define AURIE_FWK_PATCH 4
#endif // AURIE_FWK_PATCH


namespace Aurie
{
	namespace fs = ::std::filesystem;

	// Opaque
	struct AurieModule;
	struct AurieList;
	struct AurieObject;
	struct AurieMemoryAllocation;
	struct AurieHook;

	// Forward declarations (not opaque)
	struct AurieInterfaceBase;
	using PVOID = void*; // Allow usage of PVOID even without including PVOID

	enum AurieStatus : uint32_t
	{
		// The operation completed successfully.
		AURIE_SUCCESS = 0,
		// An invalid architecture was specified.
		AURIE_INVALID_ARCH,
		// An error occured in an external function call.
		AURIE_EXTERNAL_ERROR,
		// The requested file was not found.
		AURIE_FILE_NOT_FOUND,
		// The requested access to the object was denied.
		AURIE_ACCESS_DENIED,
		// An object with the same identifier / priority is already registered.
		AURIE_OBJECT_ALREADY_EXISTS,
		// One or more parameters were invalid.
		AURIE_INVALID_PARAMETER,
		// Insufficient memory is available.
		AURIE_INSUFFICIENT_MEMORY,
		// An invalid signature was detected.
		AURIE_INVALID_SIGNATURE,
		// The requested operation is not implemented.
		AURIE_NOT_IMPLEMENTED,
		// An internal error occured in the module.
		AURIE_MODULE_INTERNAL_ERROR,
		// The module failed to resolve dependencies.
		AURIE_MODULE_DEPENDENCY_NOT_RESOLVED,
		// The module failed to initialize.
		AURIE_MODULE_INITIALIZATION_FAILED,
		// The target file header, directory, or RVA could not be found or is invalid.
		AURIE_FILE_PART_NOT_FOUND,
		// The object was not found.
		AURIE_OBJECT_NOT_FOUND
	};

	enum AurieObjectType : uint32_t
	{
		// An AurieModule object
		AURIE_OBJECT_MODULE = 1,
		// An AurieInterfaceBase object
		AURIE_OBJECT_INTERFACE = 2,
		// An AurieMemoryAllocation object
		AURIE_OBJECT_ALLOCATION = 3,
		// An AurieHook object
		AURIE_OBJECT_HOOK = 4
	};

	enum AurieModuleOperationType : uint32_t
	{
		AURIE_OPERATION_UNKNOWN = 0,
		// The call is a ModulePreinitialize call
		AURIE_OPERATION_PREINITIALIZE = 1,
		// The call is a ModuleInitialize call
		AURIE_OPERATION_INITIALIZE = 2,
		// The call is a ModuleUnload call
		AURIE_OPERATION_UNLOAD = 3
	};

	constexpr inline bool AurieSuccess(const AurieStatus Status) noexcept
	{
		return Status == AURIE_SUCCESS;
	}

	constexpr inline const char* AurieStatusToString(const AurieStatus Status) noexcept
	{
		switch (Status)
		{
		case AURIE_SUCCESS:
			return "AURIE_SUCCESS";
		case AURIE_INVALID_ARCH:
			return "AURIE_INVALID_ARCH";
		case AURIE_EXTERNAL_ERROR:
			return "AURIE_EXTERNAL_ERROR";
		case AURIE_FILE_NOT_FOUND:
			return "AURIE_FILE_NOT_FOUND";
		case AURIE_ACCESS_DENIED:
			return "AURIE_ACCESS_DENIED";
		case AURIE_OBJECT_ALREADY_EXISTS:
			return "AURIE_OBJECT_ALREADY_EXISTS";
		case AURIE_INVALID_PARAMETER:
			return "AURIE_INVALID_PARAMETER";
		case AURIE_INSUFFICIENT_MEMORY:
			return "AURIE_INSUFFICIENT_MEMORY";
		case AURIE_INVALID_SIGNATURE:
			return "AURIE_INVALID_SIGNATURE";
		case AURIE_NOT_IMPLEMENTED:
			return "AURIE_NOT_IMPLEMENTED";
		case AURIE_MODULE_INTERNAL_ERROR:
			return "AURIE_MODULE_INTERNAL_ERROR";
		case AURIE_MODULE_DEPENDENCY_NOT_RESOLVED:
			return "AURIE_MODULE_DEPENDENCY_NOT_RESOLVED";
		case AURIE_MODULE_INITIALIZATION_FAILED:
			return "AURIE_MODULE_INITIALIZATION_FAILED";
		case AURIE_FILE_PART_NOT_FOUND:
			return "AURIE_FILE_PART_NOT_FOUND";
		case AURIE_OBJECT_NOT_FOUND:
			return "AURIE_OBJECT_NOT_FOUND";
		}

		return "AURIE_UNKNOWN_STATUS_CODE";
	}

	// All interfaces must inherit from the following class
	// You can add your own functions (make sure to open-source the interface class declaration)
	struct AurieInterfaceBase
	{
		// Interface "constructor"
		virtual AurieStatus Create() = 0;
		// Interface "destructor"
		virtual void Destroy() = 0;
		// Query interface version
		virtual void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		) = 0;
	};

	struct AurieOperationInfo
	{
		union
		{
			uint8_t Flags;
			struct
			{
				bool IsFutureCall : 1;
				bool Reserved : 7;
			};
		};

		PVOID ModuleBaseAddress;
	};

	// Always points to the initial Aurie image
	// Initialized in either ArProcessAttach or __aurie_fwk_init
	inline AurieModule* g_ArInitialImage = nullptr;

	using AurieEntry = AurieStatus(*)(
		AurieModule* Module,
		const fs::path& ModulePath
		);

	using AurieLoaderEntry = AurieStatus(*)(
		IN AurieModule* InitialImage,
		IN void* (*PpGetFrameworkRoutine)(IN const char* ImageExportName),
		IN OPTIONAL AurieEntry Routine,
		IN OPTIONAL const fs::path& Path,
		IN OPTIONAL AurieModule* SelfModule
		);

	using AurieModuleCallback = void(*)(
		IN AurieModule* AffectedModule,
		IN AurieModuleOperationType OperationType,
		OPTIONAL IN OUT AurieOperationInfo* OperationInfo
		);
}

#ifndef AURIE_INCLUDE_PRIVATE
#include <functional>
#include <Windows.h>

namespace Aurie
{
	inline AurieModule* g_ArSelfModule = nullptr;

	namespace Internal
	{
		// Points to aurie!PpGetFrameworkRoutine
		// Initialized in __aurie_fwk_init
		// Only present in loadable modules
		inline void* (*g_PpGetFrameworkRoutine)(
			IN const char* ImageExportName
			);

		EXPORTED inline int WINAPI DllMain(
			HINSTANCE,  // handle to DLL module
			DWORD,		// reason for calling function
			LPVOID		// reserved
		)
		{
			return TRUE;
		}

		EXPORTED inline AurieStatus __AurieFrameworkInit(
			IN AurieModule* InitialImage,
			IN void* (*PpGetFrameworkRoutine)(IN const char* ImageExportName),
			IN OPTIONAL AurieEntry Routine,
			IN OPTIONAL const fs::path& Path,
			IN OPTIONAL AurieModule* SelfModule
		)
		{
			if (!g_ArInitialImage)
				g_ArInitialImage = InitialImage;

			if (!g_ArSelfModule)
				g_ArSelfModule = SelfModule;

			if (!g_PpGetFrameworkRoutine)
				g_PpGetFrameworkRoutine = PpGetFrameworkRoutine;

			if (Routine)
				return Routine(SelfModule, Path);

			return AURIE_SUCCESS;
		}

		template <typename TFunction>
		class AurieApiDispatcher
		{
		private:
			using ReturnType = std::function<TFunction>::result_type;
		public:
			template <typename ...TArgs>
			ReturnType operator()(const char* FunctionName, TArgs&... Args)
			{
				auto Func = reinterpret_cast<TFunction*>(g_PpGetFrameworkRoutine(FunctionName));
				if (!Func)
				{
					std::string error_string = "Tried to call function ";
					error_string.append(FunctionName);
					error_string.append(", but PpGetFrameworkRoutine returns nullptr!\n\n");
					error_string.append("Is your Aurie installation up-to-date?");

					MessageBoxA(0, error_string.c_str(), "Aurie API Dispatcher", MB_OK | MB_ICONERROR);
					exit(0);
				}

				return Func(Args...);
			}

			ReturnType operator()(const char* FunctionName)
			{
				auto Func = reinterpret_cast<TFunction*>(g_PpGetFrameworkRoutine(FunctionName));
				if (!Func)
				{
					std::string error_string = "Tried to call function ";
					error_string.append(FunctionName);
					error_string.append(", but PpGetFrameworkRoutine returns nullptr!\n\n");
					error_string.append("Is your Aurie installation up-to-date?");

					MessageBoxA(0, error_string.c_str(), "Aurie API Dispatcher", MB_OK | MB_ICONERROR);
					exit(0);
				}

				return Func();
			}
		};
	}
}

#define AURIE_API_CALL(Function, ...) ::Aurie::Internal::AurieApiDispatcher<decltype(Function)>()(#Function, __VA_ARGS__)

namespace Aurie
{
	inline AurieStatus ElIsProcessSuspended(
		OUT bool& Suspended
	)
	{
		return AURIE_API_CALL(ElIsProcessSuspended, Suspended);
	}

	inline void MmGetFrameworkVersion(
		OUT OPTIONAL short* Major,
		OUT OPTIONAL short* Minor,
		OUT OPTIONAL short* Patch
	)
	{
		return AURIE_API_CALL(MmGetFrameworkVersion, Major, Minor, Patch);
	}

	inline PVOID MmAllocatePersistentMemory(
		IN size_t Size
	)
	{
		return AURIE_API_CALL(MmAllocatePersistentMemory, Size);
	}

	inline PVOID MmAllocateMemory(
		IN AurieModule* Owner,
		IN size_t Size
	)
	{
		return AURIE_API_CALL(MmAllocateMemory, Owner, Size);
	}

	inline AurieStatus MmFreePersistentMemory(
		IN PVOID AllocationBase
	)
	{
		return AURIE_API_CALL(MmFreePersistentMemory, AllocationBase);
	}

	inline AurieStatus MmFreeMemory(
		IN AurieModule* Owner,
		IN PVOID AllocationBase
	)
	{
		return AURIE_API_CALL(MmFreeMemory, Owner, AllocationBase);
	}

	inline size_t MmSigscanModule(
		IN const wchar_t* ModuleName,
		IN const unsigned char* Pattern,
		IN const char* PatternMask
	)
	{
		return AURIE_API_CALL(MmSigscanModule, ModuleName, Pattern, PatternMask);
	}

	inline size_t MmSigscanRegion(
		IN const unsigned char* RegionBase,
		IN const size_t RegionSize,
		IN const unsigned char* Pattern,
		IN const char* PatternMask
	)
	{
		return AURIE_API_CALL(MmSigscanRegion, RegionBase, RegionSize, Pattern, PatternMask);
	}

	inline AurieStatus MmCreateHook(
		IN AurieModule* Module,
		IN std::string_view HookIdentifier,
		IN PVOID SourceFunction,
		IN PVOID DestinationFunction,
		OUT OPTIONAL PVOID* Trampoline
	)
	{
		return AURIE_API_CALL(MmCreateHook, Module, HookIdentifier, SourceFunction, DestinationFunction, Trampoline);
	}

	inline AurieStatus MmHookExists(
		IN AurieModule* Module,
		IN std::string_view HookIdentifier
	)
	{
		return AURIE_API_CALL(MmHookExists, Module, HookIdentifier);
	}

	inline PVOID MmGetHookTrampoline(
		IN AurieModule* Module,
		IN std::string_view HookIdentifier
	)
	{
		return AURIE_API_CALL(MmGetHookTrampoline, Module, HookIdentifier);
	}

	inline AurieStatus MmRemoveHook(
		IN AurieModule* Module,
		IN std::string_view HookIdentifier
	)
	{
		return AURIE_API_CALL(MmRemoveHook, Module, HookIdentifier);
	}

	namespace Internal
	{
		inline bool MmpIsAllocatedMemory(
			IN AurieModule* Module,
			IN PVOID AllocationBase
		)
		{
			return AURIE_API_CALL(MmpIsAllocatedMemory, Module, AllocationBase);
		}

		inline AurieStatus MmpSigscanRegion(
			IN const unsigned char* RegionBase,
			IN const size_t RegionSize,
			IN const unsigned char* Pattern,
			IN const char* PatternMask,
			OUT uintptr_t& PatternBase
		)
		{
			return AURIE_API_CALL(MmpSigscanRegion, RegionBase, RegionSize, Pattern, PatternMask, PatternBase);
		}
	}

	inline AurieStatus MdMapImage(
		IN const fs::path& ImagePath,
		OUT AurieModule*& Module
	)
	{
		return AURIE_API_CALL(MdMapImage, ImagePath, Module);
	}

	inline bool MdIsImagePreinitialized(
		IN AurieModule* Module
	)
	{
		return AURIE_API_CALL(MdIsImagePreinitialized, Module);
	}

	inline bool MdIsImageInitialized(
		IN AurieModule* Module
	)
	{
		return AURIE_API_CALL(MdIsImageInitialized, Module);
	}

	inline bool MdIsImageRuntimeLoaded(
		IN AurieModule* Module
	)
	{
		return AURIE_API_CALL(MdIsImageRuntimeLoaded, Module);
	}


	inline AurieStatus MdMapFolder(
		IN const fs::path& FolderPath,
		IN bool Recursive
	)
	{
		return AURIE_API_CALL(MdMapFolder, FolderPath, Recursive);
	}

	inline AurieStatus MdGetImageFilename(
		IN AurieModule* Module,
		OUT std::wstring& Filename
	)
	{
		return AURIE_API_CALL(MdGetImageFilename, Module, Filename);
	}

	inline AurieStatus MdUnmapImage(
		IN AurieModule* Module
	)
	{
		return AURIE_API_CALL(MdUnmapImage, Module);
	}

	namespace Internal
	{
		inline AurieStatus MdpQueryModuleInformation(
			IN HMODULE Module,
			OPTIONAL OUT PVOID* ModuleBase,
			OPTIONAL OUT uint32_t* SizeOfModule,
			OPTIONAL OUT PVOID* EntryPoint
		)
		{
			return AURIE_API_CALL(MdpQueryModuleInformation, Module, ModuleBase, SizeOfModule, EntryPoint);
		}

		inline fs::path& MdpGetImagePath(
			IN AurieModule* Module
		)
		{
			return AURIE_API_CALL(MdpGetImagePath, Module);
		}

		inline AurieStatus MdpGetImageFolder(
			IN AurieModule* Module,
			OUT fs::path& Path
		)
		{
			return AURIE_API_CALL(MdpGetImageFolder, Module, Path);
		}

		inline AurieStatus MdpGetNextModule(
			IN AurieModule* Module,
			OUT AurieModule*& NextModule
		)
		{
			return AURIE_API_CALL(MdpGetNextModule, Module, NextModule);
		}

		inline PVOID MdpGetModuleBaseAddress(
			IN AurieModule* Module
		)
		{
			return AURIE_API_CALL(MdpGetModuleBaseAddress, Module);
		}

		inline AurieStatus MdpLookupModuleByPath(
			IN const fs::path& ModulePath,
			OUT AurieModule*& Module
		)
		{
			return AURIE_API_CALL(MdpLookupModuleByPath, ModulePath, Module);
		}
	}

	inline AurieStatus ObCreateInterface(
		IN AurieModule* Module,
		IN AurieInterfaceBase* Interface,
		IN const char* InterfaceName
	)
	{
		return AURIE_API_CALL(ObCreateInterface, Module, Interface, InterfaceName);
	}

	inline bool ObInterfaceExists(
		IN const char* InterfaceName
	)
	{
		return AURIE_API_CALL(ObInterfaceExists, InterfaceName);
	}

	inline AurieStatus ObDestroyInterface(
		IN AurieModule* Module,
		IN const char* InterfaceName
	)
	{
		return AURIE_API_CALL(ObDestroyInterface, Module, InterfaceName);
	}

	inline AurieStatus ObGetInterface(
		IN const char* InterfaceName,
		OUT AurieInterfaceBase*& Interface
	)
	{
		return AURIE_API_CALL(ObGetInterface, InterfaceName, Interface);
	}

	namespace Internal
	{
		inline void ObpSetModuleOperationCallback(
			IN AurieModule* Module,
			IN AurieModuleCallback CallbackRoutine
		)
		{
			return AURIE_API_CALL(ObpSetModuleOperationCallback, Module, CallbackRoutine);
		}

		inline AurieObjectType ObpGetObjectType(
			IN AurieObject* Object
		)
		{
			return AURIE_API_CALL(ObpGetObjectType, Object);
		}
	}

	inline AurieStatus PpQueryImageArchitecture(
		IN const fs::path& Path,
		OUT unsigned short& ImageArchitecture
	)
	{
		return AURIE_API_CALL(PpQueryImageArchitecture, Path, ImageArchitecture);
	}

	inline uintptr_t PpFindFileExportByName(
		IN const fs::path& Path,
		IN const char* ImageExportName
	)
	{
		return AURIE_API_CALL(PpFindFileExportByName, Path, ImageExportName);
	}

	inline void* PpGetFrameworkRoutine(
		IN const char* ExportName
	)
	{
		return AURIE_API_CALL(PpGetFrameworkRoutine, ExportName);
	}

	inline AurieStatus PpGetCurrentArchitecture(
		IN unsigned short& ImageArchitecture
	)
	{
		return AURIE_API_CALL(PpGetCurrentArchitecture, ImageArchitecture);
	}

	inline AurieStatus PpGetImageSubsystem(
		IN PVOID Image,
		OUT unsigned short& ImageSubsystem
	)
	{
		return AURIE_API_CALL(PpGetImageSubsystem, Image, ImageSubsystem);
	}

	namespace Internal
	{
		inline void* PpiFindModuleExportByName(
			IN const AurieModule* Image,
			IN const char* ImageExportName
		)
		{
			return AURIE_API_CALL(PpiFindModuleExportByName, Image, ImageExportName);
		}

		inline AurieStatus PpiQueryImageArchitecture(
			IN void* Image,
			OUT unsigned short& ImageArchitecture
		)
		{
			return AURIE_API_CALL(PpiQueryImageArchitecture, Image, ImageArchitecture);
		}

		inline AurieStatus PpiGetNtHeader(
			IN void* Image,
			OUT void*& NtHeader
		)
		{
			return AURIE_API_CALL(PpiGetNtHeader, Image, NtHeader);
		}

		inline AurieStatus PpiGetModuleSectionBounds(
			IN void* Image,
			IN const char* SectionName,
			OUT uint64_t& SectionBase,
			OUT size_t& SectionSize
		)
		{
			return AURIE_API_CALL(PpiGetModuleSectionBounds, Image, SectionName, SectionBase, SectionSize);
		}

		inline uint32_t PpiRvaToFileOffset(
			IN PIMAGE_NT_HEADERS ImageHeaders,
			IN uint32_t Rva
		)
		{
			return AURIE_API_CALL(PpiRvaToFileOffset, ImageHeaders, Rva);
		}
	}
}

#endif // AURIE_INCLUDE_PRIVATE
#endif // AURIE_SHARED_H_
