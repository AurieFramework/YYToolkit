#pragma once
#define YYSDK_NODEFS
#include "SDK.hpp"
#include <string_view>

namespace Utils::Error
{
	void Error(bool critical, const char* fmt, ...);

	// Apparently this has to be inline and here, or else you get link errors
	// I don't understand the linker
	inline std::string_view YYTKStatus_ToString(YYTKStatus Status) 
	{
		switch (Status)
		{
		case YYTKStatus::YYTK_OK:
			return "YYTK_OK";
		case YYTKStatus::YYTK_FAIL:
			return "YYTK_FAIL";
		case YYTKStatus::YYTK_UNAVAILABLE:
			return "YYTK_UNAVAILABLE";
		case YYTKStatus::YYTK_NO_MEMORY:
			return "YYTK_NO_MEMORY";
		case YYTKStatus::YYTK_NOT_FOUND:
			return "YYTK_NOT_FOUND";
		case YYTKStatus::YYTK_NOT_IMPLEMENTED:
			return "YYTK_NOT_IMPLEMENTED";
		default:
			return "YYTK_INVALID";
		}
	}
}