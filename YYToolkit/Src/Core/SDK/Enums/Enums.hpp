#pragma once
enum RVKind
{
	VALUE_REAL = 0,				// Real value
	VALUE_STRING,				// String value
	VALUE_ARRAY,				// Array value
	VALUE_PTR,					// Ptr value
	VALUE_VEC3,					// Vec3 (x,y,z) value (within the RValue)
	VALUE_UNDEFINED,			// Undefined value
	VALUE_OBJECT,				// YYObjectBase* value 
	VALUE_INT32,				// Int32 value
	VALUE_VEC4,					// Vec4 (x,y,z,w) value (allocated from pool)
	VALUE_VEC44,				// Vec44 (matrix) value (allocated from pool)
	VALUE_INT64,				// Int64 value
	VALUE_ACCESSOR,				// Actually an accessor
	VALUE_NULL,					// JS Null
	VALUE_BOOL,					// Bool value
	VALUE_ITERATOR,				// JS For-in Iterator
	VALUE_REF,					// Reference value (uses the ptr to point at a RefBase structure)
	VALUE_UNSET = 0x0ffffff		// Unset value (never initialized)
};
