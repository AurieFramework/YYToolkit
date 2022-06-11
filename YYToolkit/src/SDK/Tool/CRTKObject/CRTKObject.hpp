#ifndef RTK_SDK_TOOL_CRTKOBJECT_H_
#define RTK_SDK_TOOL_CRTKOBJECT_H_

#include <cstdint>
#include "../Enums/Enums.hpp"

struct CRTKObject
{
	virtual EObjectType GetObjectType() const = 0;

	virtual ~CRTKObject() {};
};

using PRTKObject = CRTKObject*;

#endif // RTK_SDK_TOOL_CRTKOBJECT_H_