#ifndef YYTK_INTERFACE_H_
#define YYTK_INTERFACE_H_
#include "../Tool.hpp"
#include <map>

namespace YYTK
{
	class YYTKInterfaceImpl : public YYTKInterface
	{
	private:
		YYRunnerInterface m_RunnerInterface;
		RFunction** m_FunctionsArray;
		std::map<std::string, TRoutine> m_FunctionCache;

	public:
		virtual Aurie::AurieStatus Create() override final;

		virtual void Destroy() override final;

		virtual void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		);

		virtual YYTKStatus GetNamedRoutineIndex(
			IN const char* FunctionName,
			OUT int* FunctionIndex
		) override final;

		virtual YYTKStatus GetNamedRoutinePointer(
			IN const char* FunctionName,
			OUT PVOID* FunctionPointer
		) override final;

		virtual YYTKStatus GetGlobalInstance(
			OUT CInstance** Instance
		) override final;

		virtual RValue CallBuiltin(
			IN const char* FunctionName,
			IN const std::vector<RValue>& Arguments
		) override final;

		virtual RValue CallBuiltinEx(
			IN const char* FunctionName,
			IN CInstance* SelfInstance,
			IN CInstance* OtherInstance,
			IN const std::vector<RValue>& Arguments
		) override final;

		virtual void Print(
			IN CmColor Color,
			IN const char* Format,
			IN ...
		) override final;

		virtual void PrintInfo(
			IN const char* Format,
			IN ...
		) override final;

		virtual void PrintWarning(
			IN const char* Format,
			IN ...
		) override final;

		virtual void PrintError(
			IN const char* Function,
			IN const int Line,
			IN const char* Format,
			IN ...
		) override final;

		virtual YYTKStatus CreateCallback(
			IN const Aurie::AurieModule* Module,
			IN EventTriggers Trigger,
			IN PVOID Routine
		) override final;

		virtual YYTKStatus RemoveCallback(
			IN const Aurie::AurieModule* Module,
			IN PVOID Routine
		) override final;

		virtual YYTKStatus InvalidateAllCaches() override final;
	};

	inline YYTKInterfaceImpl g_ModuleInterface;
}

#endif // YYTK_INTERFACE_H_