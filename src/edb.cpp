#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

using json = nlohmann::json;


namespace edb
{
	std::map<int, std::string> formatted_events_choices;

	const std::string first = "CarrotJuicer\\cje";
	const std::string second = "db.json";

	void init()
	{
		std::string cjedb_path = first + second;
		
		if (!std::filesystem::exists(cjedb_path))
		{
			spdlog::info("[edb] Skipping loading {}.", cjedb_path);
			return;
		}

		try
		{
			json j;
			std::ifstream i(cjedb_path);
			i >> j;

			const auto& events = j.at("events");
			for (auto it = events.begin(); it < events.end(); ++it)
			{
				const auto& v = it.value();
				std::stringstream formatted;

				for (const auto& choice : v.at("choices"))
				{
					formatted << "\n" << choice.at("title").get<std::string>() << "\n"
						<< choice.at("text").get<std::string>() << "\n";
				}
				formatted << "\n";

				formatted_events_choices[v.at("storyId")] = formatted.str();
			}

			spdlog::info("[edb] {} opened, read {} events.", cjedb_path, formatted_events_choices.size());
		}
		catch (std::exception& e)
		{
			spdlog::info("[edb] Exception reading {}: {}", cjedb_path, e.what());
		}
	}

	void print_choices(const int story_id)
	{
		if (const auto search = formatted_events_choices.find(story_id); search != formatted_events_choices.end())
		{
			spdlog::info("[choices] {}", search->second);
		}
	}
}
