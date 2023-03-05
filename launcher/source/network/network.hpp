#ifndef YYTK_LAUNCHER_NETWORK_NETWORK_H_
#define YYTK_LAUNCHER_NETWORK_NETWORK_H_

#include <string>
#include <vector>
#include <libcurl/curl.h>
#include <filesystem>

struct assets_t
{
	// x86 version of YYTK - should be available in all releases?
	std::string download_url_x86;

	// x64 version of YYTK - should be available in releases after 2.1.2
	std::string download_url_x64;
};

struct release_t
{
	// Version of YYTK
	// ex.: v2.1.2; v1.0.4
	std::string tag_name;

	// The release name 
	// ex.: Enable script management in x64; Fix hooks in newer runners
	std::string title_name;

	// The assets (downloadable files)
	assets_t assets;
};

namespace network
{
	CURLcode fetch_response(CURL* easy_handle, std::string_view url, std::string& out_response);

	CURLcode fetch_response_binary(CURL* easy_handle, std::string_view url, std::vector<char>& vector);

	CURLcode fetch_releases(CURL* easy_handle, std::string_view author_name, std::string_view project_name, std::vector<release_t>& releases);

	CURLcode download_file(CURL* easy_handle, std::string_view url, const std::filesystem::path& path);
}

#endif // YYTK_LAUNCHER_NETWORK_NETWORK_H_