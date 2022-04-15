#pragma once
#include "../../Undocumented/YYObjectBase/YYObjectBase.hpp"
struct RValue;

template <typename T>
struct CDynamicArray
{
	int m_arrayLength;
	T* Elements;
};

template <typename T>
struct CDynamicArrayRef
{
	int m_refCount;
	CDynamicArray<T>* Array;
	RValue* pOwner;

	static CDynamicArrayRef<T>* Assign(CDynamicArrayRef<T>* _Other)
	{
		if (_Other)
		{
			_Other->m_refCount++;
		}

		return _Other;
	}

	static CDynamicArrayRef<T>* Remove(CDynamicArrayRef<T>* _Other)
	{
		if (_Other)
		{
			_Other->m_refCount--;
		}

		return nullptr;
	}
};

struct RefDynamicArrayOfRValue : YYObjectBase
{
	int m_refCount;
	int m_flags;
	RValue* m_Array;
	signed long long m_Owner;
	int visited;
	int length;
};
