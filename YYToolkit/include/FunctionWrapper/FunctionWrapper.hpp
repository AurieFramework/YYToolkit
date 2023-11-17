#ifndef YYTK_FUNCTIONWRAPPER_H_
#define YYTK_FUNCTIONWRAPPER_H_

#include <functional>
#include <tuple>

#pragma pack(push, 1)
template <typename>
class FunctionWrapper;

template <typename TRet, typename ...TArgs>
class FunctionWrapper<TRet(TArgs...)>
{
private:
	std::tuple<TArgs...> m_Arguments;
	std::function<TRet(TArgs...)> m_Function;
	bool m_CalledOriginal;
	TRet m_Result;
public:
	FunctionWrapper(std::function<TRet(TArgs...)> Function, TArgs&... Arguments)
	{
		m_Function = Function;
		m_Arguments = std::make_tuple(Arguments...);
		m_CalledOriginal = false;
		m_Result = {};
	}
	 
	TRet Call()
	{
		m_CalledOriginal = true;
		m_Result = std::apply(m_Function, m_Arguments);
		return m_Result;
	}

	std::tuple<TArgs...>& Arguments()
	{
		return m_Arguments;
	}

	const std::tuple<TArgs...>& Arguments() const
	{
		return m_Arguments;
	}

	TRet Call(TArgs&... Arguments)
	{
		m_CalledOriginal = true;
		m_Result = m_Function(Arguments...);
		return m_Result;
	}

	bool CalledOriginal() const
	{
		return m_CalledOriginal;
	}

	TRet& Result()
	{
		return m_Result;
	}

	const TRet& Result() const
	{
		return m_Result;
	}

	void Override(const TRet& newValue)
	{
		m_CalledOriginal = true;
		m_Result = newValue;
	}
};

// Override for void return type
template <typename ...Args>
class FunctionWrapper<void(Args...)>
{
private:
	std::tuple<Args...> m_Arguments;
	std::function<void(Args...)> m_Function;
	bool m_CalledOriginal;
public:
	FunctionWrapper(std::function<void(Args...)> Function, Args... Arguments)
	{
		m_Function = Function;
		m_Arguments = std::make_tuple(Arguments...);
		m_CalledOriginal = false;
	}

	void Call()
	{
		m_CalledOriginal = true;
		std::apply(m_Function, m_Arguments);
	}

	std::tuple<Args...>& Arguments()
	{
		return m_Arguments;
	}

	const std::tuple<Args...>& Arguments() const
	{
		return m_Arguments;
	}

	void Call(Args... Arguments)
	{
		m_CalledOriginal = true;
		m_Function(Arguments...);
	}

	bool CalledOriginal() const
	{
		return m_CalledOriginal;
	}

	void Override()
	{
		m_CalledOriginal = true;
	}
};

#pragma pack(pop)
#endif // YYTK_FUNCTIONWRAPPER_H_
