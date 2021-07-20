#pragma once
#include "StackTrace.hpp"

namespace Utils::Error
{
	void Error(bool critical, const char* fmt, ...);
}