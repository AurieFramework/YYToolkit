#pragma once
#include <cstring>
struct RefString
{
	char* m_Thing;
	int m_refCount;
	int m_Size;

	RefString(const char* _Thing, int _Size, bool _NoAutoFree)
	{
		m_Thing = new char[_Size];
		m_Size = _Size;
		m_refCount = _NoAutoFree ? 0xDEAD : 0;
		
		if (_Thing && m_Thing)
		{
			strncpy(m_Thing, _Thing, _Size);
		}

		this->Inc();
	}

	void Inc()
	{
		this->m_refCount++;
	}

	void Dec()
	{
		this->m_refCount--;

		if (m_refCount == 0 || m_refCount == 0xDEAD)
		{
			delete[] m_Thing;
			m_Thing = nullptr;

			//delete this; // Suicide.
		}
	}

	const char* Get() const
	{
		return m_Thing;
	}

	int Size() const
	{
		return m_Size;
	}

	static RefString* Alloc(const char* _Thing, const int& _Size)
	{ 
		return new RefString(_Thing, _Size, true);
	}

	static RefString* Assign(RefString* _Other)
	{
		if (_Other) 
		{ 
			_Other->Inc(); 
		} 
		return _Other; 
	}

	static RefString* Remove(RefString* _Other)
	{ 
		if (_Other) 
		{
			_Other->Dec();
		} 
		return nullptr; 
	}

	static RefString* Destroy(RefString* _Other)
	{ 
		if (_Other) 
		{ 
			if (_Other->m_Thing) 
			{ 
				delete[] _Other->m_Thing; 
			} 
			delete _Other; 
		} 
		return nullptr; 
	}
};