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

enum class EJSRetValBool {
	EJSRVB_FALSE,
	EJSRVB_TRUE,
	EJSRVB_TYPE_ERROR
};

enum YYTKStatus : int
{
	YYTK_OK = 0,				// The operation completed successfully.
	YYTK_FAIL = 1,				// Unspecified error occured, see documentation.
	YYTK_INVALIDARG = 2,		// One or more arguments were invalid, see documentation.
	YYTK_INVALIDRESULT = 3,		// A result of a function was invalid, see documentation.
	YYTK_NOMATCH = 4,			// A pattern couldn't be found, see documentation.
	YYTK_UNAVAILABLE = 5,		// The function isn't available in the current context.
	YYTK_NOT_FOUND = 6,			// The value wasn't found.
};

enum EventType : unsigned __int64
{
	EVT_CODE_EXECUTE = (1 << 0),					// The event represents a Code_Execute() call.
	EVT_YYERROR = (1 << 1),							// The event represents a YYError() call.
	EVT_ENDSCENE = (1 << 2),						// The event represents an LPDIRECT3DDEVICE9::EndScene() call.
	EVT_PRESENT = (1 << 3),							// The event represents an IDXGISwapChain::Present() call.
	EVT_RESIZEBUFFERS = (1 << 4),					// The event represents an IDXGISwapChain::ResizeBuffers() call.
	EVT_WNDPROC = (1 << 5),							// The event represents a window procedure call.
	EVT_DOCALLSCRIPT = (1 << 6),					// The event represents a DoCallScript() call.
};

enum Color : int
{
	CLR_BLACK = 0,
	CLR_DARKBLUE = 1,
	CLR_GREEN = 2,
	CLR_AQUA = 3,
	CLR_RED = 4,
	CLR_PURPLE = 5,
	CLR_GOLD = 6,
	CLR_DEFAULT = 7,
	CLR_GRAY = 8,
	CLR_BLUE = 9,
	CLR_MATRIXGREEN = 10,
	CLR_LIGHTBLUE = 11,
	CLR_TANGERINE = 12,
	CLR_BRIGHTPURPLE = 13,
	CLR_YELLOW = 14,
	CLR_WHITE = 15
};

enum EVariableType : int
{
	VAR_SELF = -1,
	VAR_OTHER = -2,
	VAR_ALL = -3,
	VAR_NOONE = -4,
	VAR_GLOBAL = -5,
	VAR_BUILTIN = -6,
	VAR_LOCAL = -7,
	VAR_STACKTOP = -9,
	VAR_ARGUMENT = -15,
};