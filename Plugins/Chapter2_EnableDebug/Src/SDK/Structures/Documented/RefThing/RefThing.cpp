#include "RefThing.hpp"

RefString::RefString(const char* _Thing, int _Size, bool _NoAutoFree)
{
	m_Thing = new char[_Size + 1];
	m_Size = _Size;
	m_refCount = _NoAutoFree ? 0xDEAD : 0;

	if (_Thing && m_Thing)
	{
		strncpy_s(m_Thing, _Size + 1, _Thing, _Size);
	}

	this->Inc();
}

RefString::~RefString()
{
	this->Dec();

	if (m_refCount == 0xDEAD || m_refCount == 0)
		delete this;
}

void RefString::Inc()
{
	this->m_refCount++;
}

void RefString::Dec()
{
	this->m_refCount--;

	if (m_refCount == 0 || m_refCount == 0xDEAD)
	{
		if (m_Thing)
			delete[] m_Thing;

		m_Thing = nullptr;
	}
}

const char* RefString::Get() const
{
	return m_Thing;
}

int RefString::Size() const
{
	return m_Size;
}

RefString* RefString::Alloc(const char* _Thing, const int& _Size)
{
	return new RefString(_Thing, _Size, true);
}

RefString* RefString::Alloc(const char* _Thing, const int& _Size, bool _NoAutoFree)
{
	return new RefString(_Thing, _Size, _NoAutoFree);
}

RefString* RefString::Assign(RefString* _Other)
{
	if (_Other)
	{
		_Other->Inc();
	}
	return _Other;
}

RefString* RefString::Remove(RefString* _Other)
{
	if (_Other)
	{
		_Other->Dec();
	}
	return nullptr;
}

RefString* RefString::Destroy(RefString* _Other)
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
