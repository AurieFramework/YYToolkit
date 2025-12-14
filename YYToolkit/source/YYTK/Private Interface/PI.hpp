#ifndef YYTK_PRIVATE_INTERFACE_H_
#define YYTK_PRIVATE_INTERFACE_H_
#include "../Tool.hpp"
#include <thread>

namespace YYTK
{
	struct YYTKPrivateInterfaceImpl : YYTK::YYTKPrivateInterface
	{
	private:
		bool YkIsLowerLevelInterfaceReady();

		void YkWaitForLowerLevelInit(
			IN const char* Function
		);

	public:
		/* Aurie Boilerplate */
		virtual Aurie::AurieStatus Create() override final;

		virtual void Destroy() override final;

		virtual void QueryVersion(
			OUT short& Major,
			OUT short& Minor,
			OUT short& Patch
		) override final;

		/* RValue conversions */

		virtual double RV_ToDouble(
			IN const RValue* Value
		) override final;

		virtual int32_t RV_ToInt32(
			IN const RValue* Value
		) override final;

		virtual int64_t RV_ToInt64(
			IN const RValue* Value
		) override final;

		virtual PVOID RV_ToPointer(
			IN const RValue* Value
		) override final;

		virtual bool RV_ToBoolean(
			IN const RValue* Value
		) override final;

		virtual const char* RV_GetKindName(
			IN const RValue* Value
		) override final;

		virtual const char* RV_GetObjectSpecificKind(
			IN const RValue* Value
		) override final;

		virtual YYObjectBase* RV_ToObject(
			IN const RValue* Value
		) override final;

		virtual CInstance* RV_ToInstance(
			IN const RValue* Value
		) override final;

		virtual const char* RV_ToCString(
			IN const RValue* Value
		) override final;

		virtual std::string RV_ToString(
			IN const RValue* Value
		) override final;

		virtual std::u8string RV_ToU8String(
			IN const RValue* Value
		) override final;

		virtual std::map<std::string, RValue> RV_ToMap(
			IN const RValue* Value
		) override final;

		virtual std::map<std::string, RValue*> RV_ToRefMap(
			IN RValue* Value
		) override final;

		virtual std::vector<RValue> RV_ToVector(
			IN const RValue* Value
		) override final;

		virtual std::vector<RValue*> RV_ToRefVector(
			IN RValue* Value
		) override final;

		virtual int32_t RV_GetMemberCount(
			IN const RValue* Value
		) override final;

		virtual RValue* RV_ToCArray(
			IN RValue* Value
		) override final;

		virtual RValue RV_IndexByNumber(
			IN const RValue* Value,
			IN size_t Index
		) override final;

		virtual RValue* RV_IndexByNumberRef(
			IN RValue* Value,
			IN size_t Index
		) override final;

		virtual RValue RV_IndexByName(
			IN const RValue* Value,
			IN std::string_view Index
		) override final;

		virtual RValue* RV_IndexByNameRef(
			IN RValue* Value,
			IN std::string_view Index
		) override final;

		virtual bool RV_ContainsNestedValue(
			IN const RValue* Value,
			IN std::string_view Index
		) override final;

		virtual bool RV_IsUndefined(
			IN const RValue* Value
		) override final;

		virtual bool RV_IsUnset(
			IN const RValue* Value
		) override final;

		virtual bool RV_IsStruct(
			IN const RValue* Value
		) override final;

		virtual bool RV_IsNumberCompatible(
			IN const RValue* Value
		) override final;

		virtual bool RV_IsString(
			IN const RValue* Value
		) override final;

		virtual bool RV_IsArray(
			IN const RValue* Value
		) override final;

		/* RValue initializers */

		virtual void RV_CreateEmpty(
			IN RValue* Value
		) override final;

		virtual void RV_CreateFromDouble(
			IN RValue* Value,
			IN double Contents
		) override final;

		virtual void RV_CreateFromInteger(
			IN RValue* Value,
			IN int64_t Contents
		) override final;

		virtual void RV_CreateFromPointer(
			IN RValue* Value,
			IN void* Contents
		) override final;

		virtual void RV_CreateFromObjectPointer(
			IN RValue* Value,
			IN void* Contents
		) override final;

		virtual void RV_CreateFromVector(
			IN RValue* Value,
			IN const std::vector<RValue>& Contents
		) override final;

		virtual void RV_CreateFromAnsiString(
			IN RValue* Value,
			IN const std::string_view Contents
		) override final;

		virtual void RV_CreateFromU8String(
			IN RValue* Value,
			IN const std::u8string_view Contents
		) override final;

		virtual void RV_CreateFromBoolean(
			IN RValue* Value,
			IN bool Contents
		) override final;

		virtual void RV_CreateFromMap(
			IN RValue* Value,
			IN const std::map<std::string, RValue>& Contents
		) override final;

		virtual void RV_Copy(
			IN RValue* Destination,
			IN const RValue* Source
		) override final;

		virtual void RV_Free(
			IN RValue* Value
		) override final;

		virtual const char* CCode_GetName(
			IN const CCode* Object
		) override final;

		virtual const char* CScript_GetName(
			IN const CScript* Object
		) override final;

		virtual CRoomInternal* CRoom_GetInternalData(
			IN CRoom* Object
		) override final;

		virtual bool YYObjectBase_Add(
			IN YYObjectBase* Object,
			IN const char* Name,
			IN const RValue& Value,
			IN int Flags
		) override final;

		virtual RValue* YYObjectBase_FindOrAllocateValue(
			IN YYObjectBase* Object,
			IN const char* Name
		) override final;

		virtual CInstanceInternal* CInstance_GetInternalData(
			IN CInstance* Instance
		) override final;

		virtual CInstance* CInstance_FromID(
			IN int32_t InstanceID
		) override final;

		virtual Aurie::AurieStatus YkSetRuntimeFlags(
			uint8_t NewFlags
		) override final;
	};

	inline YYTKPrivateInterfaceImpl g_PrivateInterface;
}

#endif // YYTK_PRIVATE_INTERFACE_H_