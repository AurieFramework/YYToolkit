#include "Interface.hpp"

namespace YYTK
{
	Aurie::AurieStatus YYTKInterfaceImpl::Create()
	{
		YYTKStatus last_status = YYTK_SUCCESS;

		last_status = YYTK::Internal::GmpGetRunnerInterface(
			m_RunnerInterface
		);

		if (last_status != YYTK_SUCCESS)
			return Aurie::AURIE_MODULE_INTERNAL_ERROR;

		last_status = YYTK::Internal::GmpFindFunctionsArray(
			m_RunnerInterface,
			&m_FunctionsArray
		);

		if (last_status != YYTK_SUCCESS)
			return Aurie::AURIE_MODULE_INTERNAL_ERROR;

		YYTK::CmWriteOutput(CM_LIGHTBLUE, "Welcome to YYTK Next!");
		YYTK::CmWriteOutput(CM_LIGHTBLUE, "m_Functions at %p!", m_FunctionsArray);

		return Aurie::AURIE_SUCCESS;
	}

	void YYTKInterfaceImpl::Destroy()
	{
		// Free stuff here?
	}

	void YYTKInterfaceImpl::QueryVersion(
		OUT short& Major, 
		OUT short& Minor, 
		OUT short& Patch
	)
	{
		Major = 0;
		Minor = 0;
		Patch = 0;
	}

	YYTKStatus YYTKInterfaceImpl::GetNamedRoutineIndex(
		IN const char* FunctionName, 
		OUT int* Index
	)
	{
		if (!m_RunnerInterface.Code_Function_Find)
			return YYTK_INTERFACE_UNAVAILABLE;

		if (!Index)
			return YYTK_INVALID_PARAMETER;

		if (m_RunnerInterface.Code_Function_Find(FunctionName, Index))
		{
			return YYTK_SUCCESS;
		}

		return YYTK_OBJECT_NOT_FOUND;
	}

	YYTKStatus YYTKInterfaceImpl::GetNamedRoutinePointer(
		IN const char* FunctionName, 
		OUT PVOID* FunctionPointer
	)
	{
		return YYTKStatus();
	}

	YYTKStatus YYTKInterfaceImpl::GetGlobalInstance(OUT CInstance** Instance)
	{
		return YYTKStatus();
	}

	RValue YYTKInterfaceImpl::CallBuiltin(
		IN const char* FunctionName, 
		IN const std::vector<RValue>& Arguments
	)
	{
		return {};
	}

	RValue YYTKInterfaceImpl::CallBuiltinEx(
		IN const char* FunctionName, 
		IN CInstance* SelfInstance, 
		IN CInstance* OtherInstance, 
		IN const std::vector<RValue>& Arguments
	)
	{
		RValue Result;

		// Use the cached result if possible
		if (m_FunctionCache.contains(FunctionName))
		{
			m_FunctionCache.at(FunctionName)(
				&Result, 
				SelfInstance, 
				OtherInstance, 
				static_cast<int>(Arguments.size()), 
				const_cast<RValue*>(Arguments.data())
			);

			return Result;
		}
		
		return {};
	}

	void YYTKInterfaceImpl::Print(
		IN CmColor Color, 
		IN const char* Format, 
		IN ...
	)
	{

	}

	void YYTKInterfaceImpl::PrintInfo(
		IN const char* Format, 
		IN ...
	)
	{

	}

	void YYTKInterfaceImpl::PrintWarning(
		IN const char* Format, 
		IN ...
	)
	{

	}

	void YYTKInterfaceImpl::PrintError(
		IN const char* Function, 
		IN const int Line, 
		IN const char* Format, 
		IN ...
	)
	{

	}

	YYTKStatus YYTKInterfaceImpl::CreateCallback(
		IN const Aurie::AurieModule* Module, 
		IN EventTriggers Trigger, 
		IN PVOID Routine
	)
	{
		return YYTKStatus();
	}

	YYTKStatus YYTKInterfaceImpl::RemoveCallback(
		IN const Aurie::AurieModule* Module, 
		IN PVOID Routine
	)
	{
		return YYTKStatus();
	}

	YYTKStatus YYTKInterfaceImpl::InvalidateAllCaches()
	{
		m_FunctionCache.clear();
		return YYTK_SUCCESS;
	}
}