#ifndef RTK_SDK_TOOL_CRTKSTRING_H_
#define RTK_SDK_TOOL_CRTKSTRING_H_

#include "../CRTKObject/CRTKObject.hpp"

#include <iostream>

struct CRTKString : CRTKObject
{
	virtual EObjectType GetObjectType() const override
	{
		return EObjectType::kString;
	}

	CRTKString()
	{
		m_pBuffer = nullptr;
		m_Size = 0;
	}

	CRTKString(const CRTKString& other) = delete;

	wchar_t* m_pBuffer;
	size_t m_Size;
};

#endif // RTK_SDK_TOOL_CRTKSTRING_H_