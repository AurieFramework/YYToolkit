#pragma once
#ifdef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#include "../../Enums/Enums.hpp"
#include "../../FwdDecls/FwdDecls.hpp"
#include <Windows.h>
#include <string>
#include <tuple>

// can we escape this template hell

class YYTKEventBase
{
public:
	virtual EventType GetEventType() const = 0;
};

template <typename _ReturnValue, typename _Function, EventType _Event, typename... _FunctionArgs>
class YYTKEvent : public YYTKEventBase
{
protected:
	std::tuple<_FunctionArgs...> s_tArguments;
	_ReturnValue s_tReturnValue;
	_Function s_tOriginal;
	bool s_CalledOriginal;

public:
	_ReturnValue& Call(_FunctionArgs... Args) const
	{
		s_ReturnValue = reinterpret_cast<_ReturnValue>(pfnOriginal(Args...));
		s_CalledOriginal = true;
		return s_ReturnValue;
	}

	_Function Function() const
	{
		return s_tpfnOriginal;
	}

	std::tuple<_FunctionArgs...>& Arguments()
	{
		return s_tArguments;
	}

	bool& CalledOriginal()
	{
		return s_CalledOriginal;
	}

	_ReturnValue& GetReturn()
	{
		return s_tReturnValue;
	}

	virtual EventType GetEventType() const override
	{
		return _Event;
	}

	YYTKEvent(_Function Original, _FunctionArgs... Args)
	{
		this->s_CalledOriginal = false;
		this->s_tReturnValue = 0; // Might be UB, who knows
		this->s_tArguments = std::make_tuple<_FunctionArgs>(Args);
		this->s_tOriginal = Original;
	}
};