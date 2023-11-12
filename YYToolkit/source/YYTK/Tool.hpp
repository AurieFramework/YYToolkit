#ifndef YYTK_TOOL_H_
#define YYTK_TOOL_H_

#include <Zydis/Zydis.h>
#include "Shared.hpp"

namespace YYTK
{
	struct HTTP_REQ_CONTEXT;
	typedef int (*PFUNC_async)(HTTP_REQ_CONTEXT* _pContext, void* _pPayload, int* _pMap);
	typedef void (*PFUNC_cleanup)(HTTP_REQ_CONTEXT* _pContext);
	typedef void (*PFUNC_process)(HTTP_REQ_CONTEXT* _pContext);
	typedef void* HYYMUTEX;
	typedef void* HSPRITEASYNC;
	
	struct RFunction
	{
		const char* FunctionName;
		TRoutine Routine;
		int ArgumentCount;
	};

	constexpr YYTKStatus ConvertStatus(const Aurie::AurieStatus& Status)
	{
		return static_cast<YYTKStatus>(static_cast<int>(Status));
	}
	
	struct TargettedInstruction
	{
		ZydisDisassembledInstruction Instruction;
		PVOID FunctionTarget;
	};

	// https://github.com/YoYoGames/GMEXT-Steamworks/blob/main/source/Steamworks_vs/Steamworks/Extension_Interface.h#L106
	struct YYRunnerInterface
	{
		// basic interaction with the user
		void (*DebugConsoleOutput)(const char* fmt, ...); // hook to YYprintf
		void (*ReleaseConsoleOutput)(const char* fmt, ...);
		void (*ShowMessage)(const char* msg);

		// for printing error messages
		void (*YYError)(const char* _error, ...);

		// alloc, realloc and free
		void* (*YYAlloc)(int _size);
		void* (*YYRealloc)(void* pOriginal, int _newSize);
		void  (*YYFree)(const void* p);
		const char* (*YYStrDup)(const char* _pS);

		// yyget* functions for parsing arguments out of the arg index
		bool (*YYGetBool)(const RValue* _pBase, int _index);
		float (*YYGetFloat)(const RValue* _pBase, int _index);
		double (*YYGetReal)(const RValue* _pBase, int _index);
		int32_t (*YYGetInt32)(const RValue* _pBase, int _index);
		uint32_t (*YYGetUint32)(const RValue* _pBase, int _index);
		int64_t (*YYGetInt64)(const RValue* _pBase, int _index);
		void* (*YYGetPtr)(const RValue* _pBase, int _index);
		intptr_t (*YYGetPtrOrInt)(const RValue* _pBase, int _index);
		const char* (*YYGetString)(const RValue* _pBase, int _index);

		// typed get functions from a single rvalue
		bool (*BOOL_RValue)(const RValue* _pValue);
		double (*REAL_RValue)(const RValue* _pValue);
		void* (*PTR_RValue)(const RValue* _pValue);
		int64_t (*INT64_RValue)(const RValue* _pValue);
		int32_t (*INT32_RValue)(const RValue* _pValue);

		// calculate hash values from an RValue
		int (*HASH_RValue)(const RValue* _pValue);

		// copying, setting and getting RValue
		void (*SET_RValue)(RValue* _pDest, RValue* _pV, YYObjectBase* _pPropSelf, int _index);
		bool (*GET_RValue)(RValue* _pRet, RValue* _pV, YYObjectBase* _pPropSelf, int _index, bool fPrepareArray, bool fPartOfSet);
		void (*COPY_RValue)(RValue* _pDest, const RValue* _pSource);
		int (*KIND_RValue)(const RValue* _pValue);
		void (*FREE_RValue)(RValue* _pValue);
		void (*YYCreateString)(RValue* _pVal, const char* _pS);

		void (*YYCreateArray)(RValue* pRValue, int n_values, const double* values);

		// finding and running user scripts from name
		int (*Script_Find_Id)(const char* name);
		bool (*Script_Perform)(int ind, CInstance* selfinst, CInstance* otherinst, int argc, RValue* res, RValue* arg);

		// finding builtin functions
		bool  (*Code_Function_Find)(const char* name, int* ind);

		// http functions
		void (*HTTP_Get)(const char* _pFilename, int _type, PFUNC_async _async, PFUNC_cleanup _cleanup, void* _pV);
		void (*HTTP_Post)(const char* _pFilename, const char* _pPost, PFUNC_async _async, PFUNC_cleanup _cleanup, void* _pV);
		void (*HTTP_Request)(const char* _url, const char* _method, const char* _headers, const char* _pBody, PFUNC_async _async, PFUNC_cleanup _cleanup, void* _pV, int _contentLength);

		// sprite loading helper functions
		int (*ASYNCFunc_SpriteAdd)(HTTP_REQ_CONTEXT* _pContext, void* _p, int* _pMap);
		void (*ASYNCFunc_SpriteCleanup)(HTTP_REQ_CONTEXT* _pContext);
		HSPRITEASYNC(*CreateSpriteAsync)(int* _pSpriteIndex, int _xOrig, int _yOrig, int _numImages, int _flags);

		// timing
		int64_t(*Timing_Time)(void);
		void (*Timing_Sleep)(int64_t slp, bool precise);

		// mutex handling
		HYYMUTEX(*YYMutexCreate)(const char* _name);
		void (*YYMutexDestroy)(HYYMUTEX hMutex);
		void (*YYMutexLock)(HYYMUTEX hMutex);
		void (*YYMutexUnlock)(HYYMUTEX hMutex);

		// ds map manipulation for 
		void (*CreateAsyncEventWithDSMap)(int _map, int _event);
		void (*CreateAsyncEventWithDSMapAndBuffer)(int _map, int _buffer, int _event);
		int (*CreateDsMap)(int _num, ...);

		bool (*DsMapAddDouble)(int _index, const char* _pKey, double value);
		bool (*DsMapAddString)(int _index, const char* _pKey, const char* pVal);
		bool (*DsMapAddInt64)(int _index, const char* _pKey, int64_t value);

		// buffer access
		bool (*BufferGetContent)(int _index, void** _ppData, int* _pDataSize);
		int (*BufferWriteContent)(int _index, int _dest_offset, const void* _pSrcMem, int _size, bool _grow, bool _wrap);
		int (*CreateBuffer)(int _size, enum eBuffer_Format _bf, int _alignment);

		// variables
		volatile bool* pLiveConnection;
		int* pHTTP_ID;

		int (*DsListCreate)();
		void (*DsMapAddList)(int _dsMap, const char* _key, int _listIndex);
		void (*DsListAddMap)(int _dsList, int _mapIndex);
		void (*DsMapClear)(int _dsMap);
		void (*DsListClear)(int _dsList);

		bool (*BundleFileExists)(const char* _pszFileName);
		bool (*BundleFileName)(char* _name, int _size, const char* _pszFileName);
		bool (*SaveFileExists)(const char* _pszFileName);
		bool (*SaveFileName)(char* _name, int _size, const char* _pszFileName);

		bool (*Base64Encode)(const void* input_buf, size_t input_len, void* output_buf, size_t output_len);

		void (*DsListAddInt64)(int _dsList, int64_t _value);

		void (*AddDirectoryToBundleWhitelist)(const char* _pszFilename);
		void (*AddFileToBundleWhitelist)(const char* _pszFilename);
		void (*AddDirectoryToSaveWhitelist)(const char* _pszFilename);
		void (*AddFileToSaveWhitelist)(const char* _pszFilename);

		const char* (*KIND_NAME_RValue)(const RValue* _pV);

		void (*DsMapAddBool)(int _index, const char* _pKey, bool value);
		void (*DsMapAddRValue)(int _index, const char* _pKey, RValue* value);
		void (*DestroyDsMap)(int _index);

		void (*StructCreate)(RValue* _pStruct);
		void (*StructAddBool)(RValue* _pStruct, const char* _pKey, bool _value);
		void (*StructAddDouble)(RValue* _pStruct, const char* _pKey, double _value);
		void (*StructAddInt)(RValue* _pStruct, const char* _pKey, int _value);
		void (*StructAddRValue)(RValue* _pStruct, const char* _pKey, RValue* _pValue);
		void (*StructAddString)(RValue* _pStruct, const char* _pKey, const char* _pValue);

		bool (*WhitelistIsDirectoryIn)(const char* _pszDirectory);
		bool (*WhiteListIsFilenameIn)(const char* _pszFilename);
		void (*WhiteListAddTo)(const char* _pszFilename, bool _bIsDir);
		bool (*DirExists)(const char* filename);
		IBuffer* (*BufferGetFromGML)(int ind);
		int (*BufferTELL)(IBuffer* buff);
		unsigned char* (*BufferGet)(IBuffer* buff);
		const char* (*FilePrePend)(void);

		void (*StructAddInt32)(RValue* _pStruct, const char* _pKey, int32_t _value);
		void (*StructAddInt64)(RValue* _pStruct, const char* _pKey, int64_t _value);
		RValue* (*StructGetMember)(RValue* _pStruct, const char* _pKey);

		int (*StructGetKeys)(RValue* _pStruct, const char** _keys, int* _count);

		RValue* (*YYGetStruct)(RValue* _pBase, int _index);

		void (*extOptGetRValue)(RValue& result, const char* _ext, const  char* _opt);
		const char* (*extOptGetString)(const char* _ext, const  char* _opt);
		double (*extOptGetReal)(const char* _ext, const char* _opt);

		bool (*isRunningFromIDE)();
	};
}

// Private includes
#include "Module Internals/Module Internals.hpp"
#include "Module Interface/Interface.hpp"

#endif // YYTK_TOOL_H_
