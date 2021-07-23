#pragma once
#include <stack>
#include <string>
#include <vector>

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
		std::vector<YYTKStackTrace> Frames;
		while (!g_StackTrace.empty())
		{
			const auto& Frame = g_StackTrace.top();
			Frames.push_back(Frame);
			Buffer += Frame.FunctionName + std::string(" @ L") + std::to_string(Frame.Line) + '\n';
			g_StackTrace.pop();
		}

		// Fix for the stack being empty if multiple errors are raised.
		if (!Frames.empty())
		{
			for (auto& element : Frames)
				g_StackTrace.push(element);
		}
		
		return Buffer;
	}
}

struct YYTKTracer
{
	YYTKTracer() {}
	YYTKTracer(const char* Name, const int line)
	{
		using namespace Utils::Stack;
		YYTKStackTrace Trace(Name, line);
		g_StackTrace.push(Trace);
	}

	~YYTKTracer()
	{
		using namespace Utils::Stack;

		// If the stack is empty, it means we unwinded the stack before the current tracer got destroyed.
		// This is completely fine, but not checking would cause an exception.
		if (!g_StackTrace.empty() && g_StackTrace.size() != 0) // We have to check twice because idk
			g_StackTrace.pop();
	}
};

// Fuck's crashing due to a heap corruption, no idea why, just disabled it.
#define YYTKTrace(name, line)
