#pragma once
#include "../../Enums/Enums.hpp"
#include "../../FwdDecls/FwdDecls.hpp"
#include <string>
#include <tuple>

// what

template <typename _ReturnValue, typename _Function, typename... _FunctionArgs>
class YYTKEvent
{
private:
	// need somehow to store the args
	// maybe a tuple

protected:
	_ReturnValue s_tReturnValue;
	_Function s_tOriginal;
	bool s_CalledOriginal;
	

public:
	virtual _ReturnValue& Call(_FunctionArgs... Args) const
	{
		s_ReturnValue = reinterpret_cast<_ReturnValue>(pfnOriginal(Args...));
		s_CalledOriginal = true;
		return s_ReturnValue;
	}

	virtual _Function Function() const
	{
		return s_tpfnOriginal;
	}

	virtual EventType GetEventType() const = 0;

	YYTKEvent(_Function Original, _FunctionArgs...)
	{
		// construct stuff
	}
};

class YYTKCodeEvent : public YYTKEvent<bool, bool(*)(CInstance* pSelf, CInstance* pOther, CCode* pCode, YYRValue* Res, int Flags), CInstance*, CInstance*, CCode*, YYRValue*, int>
{
	virtual EventType GetEventType() const override
	{
		return EventType::EVT_CODE_EXECUTE;
	}
};

// add more events