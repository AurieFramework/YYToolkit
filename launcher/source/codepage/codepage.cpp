#include "codepage.hpp"
#include <Windows.h>

std::wstring cp::codepage_to_unicode(int code_page, const std::string& string)
{
	if (string.empty())
		return L"";

	// Get the space needed for our new string
	int space_needed = MultiByteToWideChar(
		code_page, 
		0, 
		string.c_str(), 
		string.length(), 
		nullptr, 
		0
	);

	if (!space_needed)
		return L"";

	std::wstring buffer; buffer.resize(space_needed + 1);

	// Null-terminate the string
	buffer.at(space_needed) = L'\x00';

	// Convert the string
	if (!MultiByteToWideChar(code_page, 0, string.c_str(), string.length(), buffer.data(), space_needed))
		return L"";

	return buffer;
}

std::string cp::unicode_to_codepage(int code_page, const std::wstring& string)
{
	if (string.empty())
		return "";

	// Get the space needed for our new string
	int space_needed = WideCharToMultiByte(
		code_page,
		0,
		string.c_str(),
		string.length(),
		nullptr,
		0,
		nullptr,
		nullptr
	);

	if (!space_needed)
		return "";

	std::string buffer; buffer.resize(space_needed + 1);

	// Null-terminate the string
	buffer.at(space_needed) = '\x00';

	// Convert the string
	if (!WideCharToMultiByte(code_page, 0, string.c_str(), string.length(), buffer.data(), space_needed, nullptr, nullptr))
		return "";

	return buffer;
}
