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
	EVT_CODE_EXECUTE = 0,	// The event represents a Code_Execute() call.
	EVT_YYERROR = 1,		// The event represents a YYError() call.
	EVT_ENDSCENE = 2,		// The event represents an LPDIRECT3DDEVICE9::EndScene() call.
	EVT_MESSAGEBOX = 3,		// The event represents a MessageBoxW() call.
	EVT_PRESENT = 4,		// The event represents an IDXGISwapChain::Present() call.
	EVT_RESIZEBUFFERS = 5,	// The event represents an IDXGISwapChain::ResizeBuffers() call.
	EVT_WNDPROC = 6,		// The event represents a window procedure call.
	EVT_DOCALLSCRIPT = 7,	// The event represents a DoCallScript() call.
	EVT_CUSTOM = 1337,		// The event represents a custom function call, raised by plugins.
};

enum Color : int
{
	CLR_BLACK = 0,
	CLR_DARKBLUE = 1,
	CLR_MATRIXGREEN = 2,
	CLR_AQUA = 3,
	CLR_RED = 4,
	CLR_PURPLE = 5,
	CLR_GOLD = 6,
	CLR_DEFAULT = 7,
	CLR_GRAY = 8,
	CLR_BLUE = 9,
	CLR_GREEN = 10,
	CLR_LIGHTBLUE = 11,
	CLR_TANGERINE = 12,
	CLR_PINK = 13,
	CLR_YELLOW = 14,
	CLR_WHITE = 15
};

enum EOpcode
{
	// todo
};