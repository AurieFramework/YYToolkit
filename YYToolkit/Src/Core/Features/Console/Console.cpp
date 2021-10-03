#include "../../Utils/Error.hpp"
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

static vector<string> StripOutArguments(const string& ref)
{
	vector<string> vResults;
	size_t _beginFuncCall = ref.find_first_of('(');

	if (_beginFuncCall == string::npos)
	{
		Utils::Error::Error(false, "CONOUT - Failed parsing - no opening parentheses");
		return {};
	}

	size_t _endFuncCall = ref.find_first_of(')');

	if (_endFuncCall == string::npos)
	{
		Utils::Error::Error(false, "CONOUT - Failed parsing - no closing parentheses");
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
	CInstance* pInstance = static_cast<CInstance*>(gAPIVars.g_pGlobal);
	
	// Prepare arguments and result buffer
	YYRValue Buffer, Args[2];
	
	Args[0] = "YYToolkit Console Window\nInput your desired command.";
	Args[1] = "";

	if (auto Status = API::CallBuiltinFunction(pInstance, nullptr, Buffer, 2, "get_string", Args))
	{
		Utils::Error::Error(false, "CONOUT - get_string returned %s", Utils::Error::YYTKStatus_ToString(Status));
		return;
	}
	
	string Command = (Buffer.operator string());

	if (Command.empty())
		return;

	vector<string> vecTokens = StripOutArguments(Command);
	
	if (vecTokens.empty())
		return;

	if (TRoutine Routine = API::GetBuiltin(vecTokens[0].c_str()))
	{
		RValue Result;
		YYRValue* pArgs = new YYRValue[vecTokens.size()];

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

			Utils::Error::Error(false, "CONOUT - Unrecognized token %s", token.c_str());
		}
		// REGEX MESS END

		Routine(&Result, pInstance, pInstance, vecTokens.size() - 1, reinterpret_cast<RValue*>(pArgs));

		delete[] pArgs;
		return;
	}

	Utils::Error::Error(false, "CONOUT - Unrecognized function %s", vecTokens[0].c_str());
}
