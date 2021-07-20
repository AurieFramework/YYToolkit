#pragma once
#include <stack>
#include <string>

namespace Utils::Stack
{
	struct YYTKStackTrace
	{
		const char* FunctionName;
		int Line;
		
		YYTKStackTrace() : FunctionName(0), Line(-1) {};
		YYTKStackTrace(const char* Name, int Line) : FunctionName(Name), Line(Line) {};
	};

	inline std::stack<YYTKStackTrace> g_StackTrace;

	inline std::string Unwind()
	{
		std::string Buffer = std::string();
		while (!g_StackTrace.empty())
		{
			const auto& Frame = g_StackTrace.top();
			Buffer += Frame.FunctionName + std::string(" @ L") + std::to_string(Frame.Line) + '\n';
			g_StackTrace.pop();
		}

		return Buffer;
	}
}

struct YYTKTracer
{
	YYTKTracer() = delete;
	YYTKTracer(const char* Name, const int line)
	{
		using namespace Utils::Stack;
		g_StackTrace.push(YYTKStackTrace(Name, line));
	}

	~YYTKTracer()
	{
		using namespace Utils::Stack;

		// If the stack is empty, it means we unwinded the stack before the current tracer got destroyed.
		// This is completely fine, but not checking would cause an exception.
		if (!g_StackTrace.empty())
			g_StackTrace.pop();
	}
};

#define YYTKTrace(name, line) YYTKTracer __Tracer = YYTKTracer(name, line)
