#pragma once
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
};