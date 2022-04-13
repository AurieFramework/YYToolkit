#pragma once
#include <cstring>
struct RefString
{
	char* m_Thing;
	int m_refCount;
	int m_Size;

	RefString(const char* _Thing, int _Size, bool _NoAutoFree);

	~RefString();

	void Inc();

	void Dec();

	const char* Get() const;

	int Size() const;

	static RefString* Alloc(const char* _Thing, const int& _Size);
	static RefString* Alloc(const char* _Thing, const int& _Size, bool _NoAutoFree);

	static RefString* Assign(RefString* _Other);

	static RefString* Remove(RefString* _Other);

	static RefString* Destroy(RefString* _Other);
};