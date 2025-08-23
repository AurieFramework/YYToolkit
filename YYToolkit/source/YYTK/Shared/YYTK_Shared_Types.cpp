#include "YYTK_Shared_Types.hpp"
#include "YYTK_Shared_Interface.hpp"

using namespace YYTK;
using namespace Aurie;

double YYTK::RValue::ToDouble() const
{
	return GetPrivateInterface()->RV_ToDouble(this);
}

int32_t YYTK::RValue::ToInt32() const
{
	return GetPrivateInterface()->RV_ToInt32(this);
}

int64_t YYTK::RValue::ToInt64() const
{
	return GetPrivateInterface()->RV_ToInt64(this);
}

bool YYTK::RValue::ToBoolean() const
{
	return GetPrivateInterface()->RV_ToBoolean(this);
}

std::string YYTK::RValue::GetKindName() const
{
	return GetPrivateInterface()->RV_GetKindName(this);
}

YYObjectBase* YYTK::RValue::ToObject() const
{
	return ToPointer<YYObjectBase*>();
}

CInstance* YYTK::RValue::ToInstance() const
{
	return ToPointer<CInstance*>();
}

const char* YYTK::RValue::ToCString() const
{
	return GetPrivateInterface()->RV_ToCString(this);
}

std::string YYTK::RValue::ToString() const
{
	return GetPrivateInterface()->RV_ToCString(this);
}

std::u8string YYTK::RValue::ToUTF8String() const
{
	return GetPrivateInterface()->RV_ToU8String(this);
}

std::map<std::string, RValue*> YYTK::RValue::ToRefMap()
{
	return GetPrivateInterface()->RV_ToRefMap(this);
}

std::map<std::string, RValue> YYTK::RValue::ToMap() const
{
	return GetPrivateInterface()->RV_ToMap(this);
}

std::vector<RValue*> YYTK::RValue::ToRefVector()
{
	return GetPrivateInterface()->RV_ToRefVector(this);
}

std::vector<RValue> YYTK::RValue::ToVector() const
{
	return GetPrivateInterface()->RV_ToVector(this);
}

RValue* YYTK::RValue::GetRefMember(
	IN const char* MemberName
)
{
	return GetPrivateInterface()->RV_IndexByNameRef(this, MemberName);
}

RValue* YYTK::RValue::GetRefMember(
	IN const std::string& MemberName
)
{
	return GetPrivateInterface()->RV_IndexByNameRef(this, MemberName);
}

RValue YYTK::RValue::GetMember(
	IN const char* MemberName
) const
{
	return GetPrivateInterface()->RV_IndexByName(this, MemberName);
}

RValue YYTK::RValue::GetMember(
	IN const std::string& MemberName
) const
{
	return GetPrivateInterface()->RV_IndexByName(this, MemberName);
}

int32_t YYTK::RValue::GetMemberCount() const
{
	return GetPrivateInterface()->RV_GetMemberCount(this);
}

RValue* YYTK::RValue::ToArray()
{
	return GetPrivateInterface()->RV_ToCArray(this);
}

bool YYTK::RValue::IsUndefined() const
{
	return GetPrivateInterface()->RV_IsUndefined(this) || GetPrivateInterface()->RV_IsUnset(this);
}

bool YYTK::RValue::IsStruct() const
{
	return GetPrivateInterface()->RV_IsStruct(this);
}

bool YYTK::RValue::IsNumberConvertible() const
{
	return GetPrivateInterface()->RV_IsNumberCompatible(this);
}

bool YYTK::RValue::IsString() const
{
	return GetPrivateInterface()->RV_IsString(this);
}

bool YYTK::RValue::IsArray() const
{
	return GetPrivateInterface()->RV_IsArray(this);
}

void* YYTK::RValue::ToPointer() const
{
	return GetPrivateInterface()->RV_ToPointer(this);
}

YYTK::RValue::RValue()
{
	GetPrivateInterface()->RV_CreateEmpty(this);
}

YYTK::RValue::~RValue()
{
	GetPrivateInterface()->RV_Free(this);
}

YYTK::RValue::RValue(
	IN const std::vector<RValue>& Values
)
{
	GetPrivateInterface()->RV_CreateFromVector(this, Values);
}

YYTK::RValue::RValue(
	IN void* Pointer
)
{
	GetPrivateInterface()->RV_CreateFromPointer(this, Pointer);
}

RValue::RValue(
	IN std::string_view Value
)
{
	GetPrivateInterface()->RV_CreateFromAnsiString(this, Value);
}

YYTK::RValue::RValue(
	IN std::u8string_view Value
)
{
	GetPrivateInterface()->RV_CreateFromU8String(this, Value);
}

YYTK::RValue::RValue(
	IN const char* Value
)
{
	GetPrivateInterface()->RV_CreateFromAnsiString(this, Value);
}

YYTK::RValue::RValue(
	IN const char8_t* Value
)
{
	GetPrivateInterface()->RV_CreateFromU8String(this, Value);
}

YYTK::RValue::RValue(
	IN bool Value
)
{
	GetPrivateInterface()->RV_CreateFromBoolean(this, Value);
}

YYTK::RValue::RValue(
	IN const RValue& Other
)
{
	GetPrivateInterface()->RV_CreateEmpty(this);
	GetPrivateInterface()->RV_Copy(this, &Other);
}

RValue& YYTK::RValue::operator=(
	IN const RValue& Other
	)
{
	GetPrivateInterface()->RV_Free(this);
	GetPrivateInterface()->RV_Copy(this, &Other);

	return *this;
}

YYTK::RValue::RValue(
	IN const std::map<std::string, RValue>& Values
)
{
	GetPrivateInterface()->RV_CreateFromMap(this, Values);
}

RValue& YYTK::RValue::operator[](
	IN size_t Index
	)
{
	return *GetPrivateInterface()->RV_IndexByNumberRef(this, Index);
}

RValue YYTK::RValue::operator[](
	IN size_t Index
	) const
{
	return GetPrivateInterface()->RV_IndexByNumber(this, Index);
}

RValue& RValue::operator[](
	IN std::string_view Element
	)
{
	return *GetPrivateInterface()->RV_IndexByNameRef(this, Element);
}

RValue YYTK::RValue::operator[](
	IN std::string_view MemberName
	) const
{
	return GetPrivateInterface()->RV_IndexByName(this, MemberName);
}

bool YYTK::RValue::ContainsValue(
	IN std::string_view MemberName
) const
{
	RValue self = *this;
	return GetPrivateInterface()->RV_ContainsNestedValue(this, MemberName);
}

YYTK::RValue::operator bool()
{
	return this->ToBoolean();
}

YYTK::RValue::operator double()
{
	return this->ToDouble();
}

YYTK::RValue::operator std::string()
{
	return this->ToString();
}

YYTK::RValue::operator std::u8string()
{
	return this->ToUTF8String();
}

YYTK::RValue::operator int32_t()
{
	return this->ToInt32();
}

YYTK::RValue::operator int64_t()
{
	return this->ToInt64();
}

#if YYTK_DEFINE_INTERNAL
CInstanceInternal& YYTK::CInstance::GetMembers()
{
	return *GetPrivateInterface()->CInstance_GetInternalData(this);
}

bool YYObjectBase::Add(
	IN const char* Name,
	IN const RValue& Value,
	IN int Flags
)
{
	return GetPrivateInterface()->YYObjectBase_Add(this, Name, Value, Flags);
}

bool YYObjectBase::IsExtensible()
{
	return this->m_Flags & 1;
}

RValue* YYObjectBase::FindOrAllocValue(
	IN const char* Name
)
{
	return GetPrivateInterface()->YYObjectBase_FindOrAllocateValue(this, Name);
}

CRoomInternal& YYTK::CRoom::GetMembers()
{
	return *GetPrivateInterface()->CRoom_GetInternalData(this);
}

#endif // YYTK_DEFINE_INTERNAL

RValue YYTK::CInstance::ToRValue() const
{
	return RValue(this);
}

RValue* YYTK::CInstance::GetRefMember(
	IN const char* MemberName
)
{
	RValue self = this;
	return GetPrivateInterface()->RV_IndexByNameRef(&self, MemberName);
}

RValue* YYTK::CInstance::GetRefMember(
	IN const std::string& MemberName
)
{
	RValue self = this;
	return GetPrivateInterface()->RV_IndexByNameRef(&self, MemberName);
}

const RValue* YYTK::CInstance::GetRefMember(
	IN const char* MemberName
) const
{
	RValue self = this;
	return GetPrivateInterface()->RV_IndexByNameRef(&self, MemberName);
}

const RValue* YYTK::CInstance::GetRefMember(
	IN const std::string& MemberName
) const
{
	RValue self = this;
	return GetPrivateInterface()->RV_IndexByNameRef(&self, MemberName);
}

RValue YYTK::CInstance::GetMember(
	IN const char* MemberName
) const
{
	RValue self = this;
	return GetPrivateInterface()->RV_IndexByName(&self, MemberName);
}

RValue YYTK::CInstance::GetMember(
	IN const std::string& MemberName
) const
{
	RValue self = this;
	return GetPrivateInterface()->RV_IndexByName(&self, MemberName);
}

int32_t YYTK::CInstance::GetMemberCount() const
{
	RValue self = this;
	return GetPrivateInterface()->RV_GetMemberCount(&self);
}

bool YYTK::CInstance::ContainsValue(
	IN std::string_view MemberName
) const
{
	RValue self = this;
	return GetPrivateInterface()->RV_ContainsNestedValue(&self, MemberName);
}

CInstance* YYTK::CInstance::FromInstanceID(
	IN int32_t InstanceID
)
{
	return GetPrivateInterface()->CInstance_FromID(InstanceID);
}

const char* YYTK::CCode::GetName() const
{
	return GetPrivateInterface()->CCode_GetName(this);
}

const char* YYTK::CScript::GetName() const
{
	return GetPrivateInterface()->CScript_GetName(this);
}