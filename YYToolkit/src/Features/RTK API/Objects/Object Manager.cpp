#include "../API.hpp"

#include "../../../SDK/Tool/CRTKObject/CRTKObject.hpp"
#include "../../../SDK/Tool/CRTKString/CRTKString.hpp"

#include <xstring>

namespace rtk
{
	bool OmIsObjectType(CRTKObject* pObject, EObjectType ExpectedType)
	{
		if (!pObject)
			return false;

		return pObject->GetObjectType() == ExpectedType;
	}

	EStatus OmInitString(CRTKString* pString, const wchar_t* pBuffer)
	{
		// We don't wanna write OOB
		if (!OmIsObjectType(pString, EObjectType::kString))
			return EStatus::kInvalidObject;

		// Reserve memory for both the string + the null terminator
		size_t BufferLength = wcslen(pBuffer) + 1;

		// Allocate memory
		pString->m_pBuffer = reinterpret_cast<wchar_t*>(MmAllocateMemory(BufferLength * sizeof(wchar_t)));

		// Memory allocation failed
		if (!pString->m_pBuffer)
			return EStatus::kOutOfMemory;

		pString->m_Size = BufferLength;

		// Copy string - wcsncpy was complaining about being unsafe, so I use wmemcpy
		wmemcpy(pString->m_pBuffer, pBuffer, BufferLength);

		return EStatus::kSuccess;
	}

	void OmFreeString(CRTKString* pString)
	{
		// We don't wanna free some random object
		if (!OmIsObjectType(pString, EObjectType::kString))
			return;

		MmFreeMemory(pString->m_pBuffer);
		pString->m_Size = 0;
	}
}
