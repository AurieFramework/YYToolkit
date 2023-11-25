#include "network.hpp"
#include <libcurl/curl.h>
#include <json/json.hpp>
#include <regex>
#include <fstream>

// Turns "A StRiNg" into "a string"
static std::string lower_string(const std::string& string)
{
	std::string new_string;
	new_string.reserve(string.length());
	for (auto wch : string)
	{
		new_string.push_back(tolower(wch));
	}

	return new_string;
}

static size_t curl_parse_data(char* response_data, size_t, size_t response_size, std::string* data)
{
	char* buffer = new char[response_size + 1];
	memcpy_s(buffer, response_size, response_data, response_size);
	buffer[response_size] = '\x00';

	*data += buffer;

	delete[] buffer;

	return response_size;
}

static size_t curl_parse_data_binary(char* response_data, size_t, size_t response_size, std::vector<char>* data)
{
	for (size_t offset = 0; offset < response_size; offset++)
		data->push_back(response_data[offset]);

	return response_size;
}

CURLcode network::fetch_response_binary(CURL* easy_handle, std::string_view url, std::vector<char>& vector)
{
	curl_easy_setopt(easy_handle, CURLOPT_URL, url.data());
	curl_easy_setopt(easy_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36");
	curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, curl_parse_data_binary);
	curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &vector);
	curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1);
	// Fix for Windows versions other than 10
	curl_easy_setopt(easy_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_MAX_TLSv1_2);

#if _DEBUG
	curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 1);
#endif

	return curl_easy_perform(easy_handle);
}

CURLcode network::fetch_response(CURL* easy_handle, std::string_view url, std::string& out_response)
{
	curl_easy_setopt(easy_handle, CURLOPT_URL, url.data());
	curl_easy_setopt(easy_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/87.0.4280.88 Safari/537.36");
	curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, curl_parse_data);
	curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &out_response);
	curl_easy_setopt(easy_handle, CURLOPT_FOLLOWLOCATION, 1);

	// Fix for Windows versions other than 10
	curl_easy_setopt(easy_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_MAX_TLSv1_2);

#if _DEBUG
	curl_easy_setopt(easy_handle, CURLOPT_VERBOSE, 1);
#endif

	return curl_easy_perform(easy_handle);
}

CURLcode network::fetch_releases(CURL* easy_handle, std::string_view author_name, std::string_view project_name, std::vector<release_t>& releases)
{
	using json = nlohmann::json;

	// Craft the URL
	std::string url = "https://api.github.com/repos/";
	url += author_name;		// https://api.github.com/repos/OWNER
	url += "/";				// https://api.github.com/repos/OWNER/
	url += project_name;	// https://api.github.com/repos/OWNER/REPO
	url += "/releases";		// https://api.github.com/repos/OWNER/REPO/releases

	std::string response;
	CURLcode curl_result = fetch_response(easy_handle, url, response);

	// If we failed fetching a response
	if (curl_result)
		return curl_result;

	// Get the whole response as JSON
	json json_response = json::parse(response);

	// If it's empty we're done
	if (json_response.empty())
		return curl_result;

	// The response structure is like this:
	//	Response
	//		- Object 0
	//			- tag_name - "v2.1.2"
	//			- assets
	//				- Asset 0 - "YYToolkit.dll"
	//				- Asset 1 - "DownloadMe.zip"
	//		- Object 1
	//			- tag_name - "v2.1.1"
	//			- assets
	//				- Asset 0 - "YYToolkit.dll"
	//				- Asset 1 - "DownloadMe.zip"

	for (const auto& release : json_response)
	{
		release_t current_release;
		
		current_release.tag_name = release.at("tag_name").get<std::string>();
		current_release.title_name = release.at("name").get<std::string>();

		// Ignore YYTK Next releases
		if (current_release.tag_name._Starts_with("v3"))
			continue;

		if (!release.contains("assets"))
			continue;

		for (const auto& asset : release.at("assets"))
		{
			if (!asset.contains("name"))
				continue;

			if (!asset.at("name").is_string())
				continue;

			if (!asset.contains("browser_download_url"))
				continue;

			if (!asset.at("browser_download_url").is_string())
				continue;

			std::string asset_name = lower_string(asset.at("name").get<std::string>());
			std::string asset_durl = asset.at("browser_download_url").get<std::string>();

			// TODO: Use regexes here!
			if (asset_name == "yytoolkit.dll")
			{
				current_release.assets.download_url_x86 = asset_durl;
			}

			else if (asset_name == "yytoolkit-x64.dll")
			{
				current_release.assets.download_url_x64 = asset_durl;
			}
		}

		releases.push_back(current_release);
	}

	return curl_result;
}

CURLcode network::download_file(CURL* easy_handle, std::string_view url, const std::filesystem::path& path)
{
	std::vector<char> byte_buffer;
	CURLcode curl_result = fetch_response_binary(easy_handle, url, byte_buffer);

	// If we failed fetching a response
	if (curl_result)
		return curl_result;

	std::ofstream new_file(path.wstring(), std::ios_base::binary);

	// Relying on CURL errors here, even though this is not CURL's fault
	if (!new_file.is_open())
		return CURLE_WRITE_ERROR;

	new_file.write(byte_buffer.data(), byte_buffer.size());

	// should be OK?
	return curl_result; 
}