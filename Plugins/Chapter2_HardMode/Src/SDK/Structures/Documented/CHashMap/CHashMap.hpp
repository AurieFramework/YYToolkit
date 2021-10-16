#pragma once
#ifndef YYSDK_PLUGIN
#include "../../../../Utils/Hashing/MurMurHash.hpp"
#endif
struct YYObjectBase;

template <typename Key, typename Value>
struct CHashMap
{
	int m_curSize;
	int m_numUsed;
	int m_curMask;
	int m_growThreshold;
	struct CElement
	{
		Value v;
		Key k;
		unsigned int Hash;
	} *m_pBuckets;

	static unsigned int CalculateHash(int val)		{ return 1 - 0x61C8864F * val; }

	static unsigned int CalculateHash(void* val)	{ return ((signed int)val >> 8) + 1; }

	static unsigned int CalculateHash(YYObjectBase* val) { return 7 * ((signed int)val >> 6) + 1; }
#ifndef YYSDK_PLUGIN
	static unsigned int CalculateHash(const char* val, size_t Len)
	{
		return Utils::Hash::MurMurHash(val, Len, 0);
	}
#endif
};