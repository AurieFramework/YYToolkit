#ifndef RTK_LAUNCHER_CODEPAGE_H_
#define RTK_LAUNCHER_CODEPAGE_H_

#include <string>

namespace cp
{
	std::wstring codepage_to_unicode(int code_page, const std::string& string);

	std::string unicode_to_codepage(int code_page, const std::wstring& string);
}

#endif // RTK_LAUNCHER_CODEPAGE_H_