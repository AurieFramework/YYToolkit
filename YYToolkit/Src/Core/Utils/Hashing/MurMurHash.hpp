#pragma once
//https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
#include <stdint.h>

namespace Utils
{
	namespace Hash
	{
		uint32_t MurMurHash(const unsigned char* key, int len, uint32_t seed);
	}
}