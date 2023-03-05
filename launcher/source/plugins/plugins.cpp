#include "plugins.hpp"

std::vector<plugin_item_t> plugins::get_plugins_from_folder(const std::filesystem::path& folder_path)
{
	std::vector<plugin_item_t> items;

	std::error_code ec;
	for (auto& file : std::filesystem::directory_iterator(folder_path, ec))
	{
		if (file.is_regular_file(ec) && !file.is_symlink(ec))
		{
			if (file.path().extension() == ".dll")
			{
				plugin_item_t item;
				item.path = file.path();
				item.size_kb = file.file_size(ec) / 1000.f;

				// Push only if we got no errors
				if (!ec)
					items.push_back(item);
			}
		}
	}

	return items;
}
