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

enum eGMLKind : unsigned int
{
	eGMLK_NONE = 0x0,
	eGMLK_ERROR = 0xFFFF0000,
	eGMLK_DOUBLE = 0x1,
	eGMLK_STRING = 0x2,
	eGMLK_INT32 = 0x4,
};

enum YYTKStatus : int
{
	YYTK_OK = 0,				// The operation completed successfully.
	YYTK_FAIL = 1,				// Unspecified error occured, see source code.
	YYTK_UNAVAILABLE = 2,		// The called function is not available in the current context.
	YYTK_NO_MEMORY = 3,			// No more memory is available to the process.
	YYTK_NOT_FOUND = 4,			// The specified value could not be found.
	YYTK_NOT_IMPLEMENTED = 5,	// The specified function doesn't exist. (IPC error)
	YYTK_INVALID = 6			// One or more arguments were invalid.
};

enum EventType : int
{
	EVT_CODE_EXECUTE = 0,
	EVT_DRAWING = 1,
	EVT_ENDSCENE = 2,
	EVT_MESSAGEBOX = 3,
	EVT_PRESENT = 4,
	EVT_RESIZEBUFFERS = 5,
	EVT_WNDPROC = 6,
};