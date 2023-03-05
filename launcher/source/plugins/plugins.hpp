#ifndef YYTK_LAUNCHER_PLUGINS_PLUGINS_H_
#define YYTK_LAUNCHER_PLUGINS_PLUGINS_H_

#include <string>
#include <filesystem>

struct plugin_item_t
{
	std::filesystem::path path;
	float size_kb; // size in kilobytes
	bool active; // managed by the menu code

	plugin_item_t()
	{
		path = "";
		size_kb = 0;
		active = false;
	}
};

namespace plugins
{
	std::vector<plugin_item_t> get_plugins_from_folder(const std::filesystem::path& folder_path);
}

#endif // YYTK_LAUNCHER_PLUGINS_PLUGINS_H_