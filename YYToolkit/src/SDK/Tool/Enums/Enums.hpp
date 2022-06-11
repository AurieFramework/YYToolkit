#ifndef RTK_SDK_TOOL_ENUMS_H_
#define RTK_SDK_TOOL_ENUMS_H_

#include <cstdint>

// Used in RTKObject->GetObjectType();
enum class EObjectType : uint32_t
{
	kVector,
	kLinkedList,
	kString,
	kPlugin
};

#define RTK_Success(Status) ((Status) != EStatus::kSuccess)

enum class EStatus : uint32_t
{
	kSuccess,			// No error
	kFail,				// Unspecified error
	kInvalidObject,		// Invalid object type passed into a function
	kOutOfMemory,		// A memory allocation failed
	kExportNotFound,	// A library export wasn't found
	kFileNotFound,		// The file specified doesn't exist
};

#endif // RTK_SDK_TOOL_ENUMS_H_