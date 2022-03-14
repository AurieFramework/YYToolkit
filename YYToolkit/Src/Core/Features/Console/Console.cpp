#include "../../Utils/Error/Error.hpp"
#include "../API/API.hpp"
#include "Console.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>

using std::vector;
using std::string;

static vector<string> Tokenize(const string& ref)
{
	vector<string> vResults;
	size_t _beginFuncCall = ref.find_first_of('(');

	if (_beginFuncCall == string::npos)
	{
		Utils::Error::Error(
			false,
			__FILE__,
			__LINE__,
			"No function call was found"
		);

		return {};
	}

	size_t _endFuncCall = ref.find_first_of(')');

	if (_endFuncCall == string::npos)
	{
		Utils::Error::Error(
			false,
			__FILE__,
			__LINE__,
			"No function call was found"
		);
		return {};
	}

	// Function name
	vResults.push_back(ref.substr(0, _beginFuncCall));

	std::stringstream ss(ref.substr(_beginFuncCall + 1, _endFuncCall - _beginFuncCall));
	string sCurItem;

	while (std::getline(ss, sCurItem, ','))
	{
		sCurItem.erase(std::remove_if(sCurItem.begin(), sCurItem.end(), ::isspace), sCurItem.end());
		if (sCurItem.find_first_of(')') != string::npos)
		{
			auto closePos = sCurItem.find_first_of(')');

			sCurItem = sCurItem.substr(0, closePos);
		}

		if (!sCurItem.empty())
			vResults.push_back(sCurItem);
	}

	return vResults;
}

void Console::DoCommand()
{
	// Run in global scope
	CInstance* pInstance = static_cast<CInstance*>(API::gAPIVars.Globals.g_pGlobalInstance);

	// Prepare arguments and result buffer
	YYRValue Buffer;

	if (!API::CallBuiltin(Buffer, "get_string", pInstance, nullptr, { "Please input your expression:", "" }))
	{
		Utils::Error::Error(
			false,
			__FILE__,
			__LINE__,
			"API::CallBuiltin returned false"
		);

		return;
	}
	
	string Command = (Buffer.operator string());

	if (Command.empty())
		return;

	// Regex preprocessing

	// Syntax: global.whatever = value
	// Processed: variable_global_set("whatever", value)
	std::regex regexAssignment("global\\.([a-zA-Z_]+) = (.*)");
	std::regex regexPeek("global\\.([a-zA-Z_]+)");

	if (std::regex_match(Command, regexAssignment))
	{
		Command = std::regex_replace(Command, regexAssignment, "variable_global_set(\"$1\", $2)");
	}

	// Syntax: global.whatever
	// Processed: variable_global_get("whatever")
	else if (std::regex_match(Command, regexPeek))
	{
		Command = std::regex_replace(Command, regexPeek, "variable_global_get(\"$1\")");
	}

	vector<string> vecTokens = Tokenize(Command);
	
	if (vecTokens.empty())
		return;

	TRoutine Routine;
	if (API::GetFunctionByName(vecTokens[0], Routine))
	{
		RValue Result{}; Result.Kind = VALUE_UNSET; Result.I64 = 0;

		YYRValue* pArgs = new YYRValue[vecTokens.size()]();

		// REGEX MESS START
		for (int n = 1; n < vecTokens.size(); n++)
		{
			auto& token = vecTokens[n];

			// Check if the string is actually a number
			{
				std::regex reg("^-?\\d+\\.?\\d*$");
				if (std::regex_match(token, reg))
				{
					pArgs[n - 1] = std::stod(token);
					continue;
				}

			}
			// Check if the string is... well a string
			{
				std::regex reg("\".*\"");
				if (std::regex_match(token, reg))
				{
					// Find the quotes (") - we know there's at least two of them, since the regex matched.

					pArgs[n - 1] = static_cast<std::string>(token.substr(1, token.length() - 2)); // 'Do you also cast your eggs to eggs before making dinner?'
					continue;
				}
			}

			Utils::Error::Error(
				false,
				__FILE__,
				__LINE__,
				"Unknown token: %s", 
				token.c_str()
			);
		}
		// REGEX MESS END

		Routine(&Result, pInstance, pInstance, vecTokens.size() - 1, reinterpret_cast<RValue*>(pArgs));

		Utils::Error::NoNewlineMessage(CLR_GOLD, "%s", Command.c_str());
		Utils::Error::NoNewlineMessage(CLR_DEFAULT, " -> ");

		if (Result.Kind == VALUE_REAL)
			Utils::Error::Message(CLR_BLUE, "%.2f", Result.Real);

		else if (Result.Kind == VALUE_BOOL)
			Utils::Error::Message(CLR_TANGERINE, "%s", (Result.Real > 0.5) ? "true" : "false");

		else if (Result.Kind == VALUE_STRING)
			Utils::Error::Message(CLR_YELLOW, "\"%s\"", Result.String->Get());

		else
			Utils::Error::Message(CLR_GRAY, "Undefined (type 0x%X)", Result.Kind);

		delete[] pArgs;
		return;
	}

	Utils::Error::Error(
		false,
		__FILE__,
		__LINE__,
		"Unrecognized function %s",
		vecTokens[0].c_str()
	);
}
