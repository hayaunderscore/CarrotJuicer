// Handles all Discord RPC-related shenanigans

// Shut up man
#define _CRT_SECURE_NO_WARNINGS

#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <time.h>
#include <fmt/format.h>
#include <curl/curl.h>
#include <thread>
#include <utility>
#include <algorithm> 
#include <cctype>
#include <locale>

#include "config.hpp"
#include "mdb.hpp"
#include "edb.hpp"
#include "discord.hpp"

using namespace std::literals;
using json = nlohmann::json;

namespace discord
{
	const std::string APP_ID = "954453106765225995";
	
	static void HandleDiscordReady(const DiscordUser* connectedUser) { std::cout << "RPC: Connected to " << connectedUser->username << "\n"; }
	static void HandleDiscordDisconnected(int errcode, const char* message) { std::cout << "RPC: Disconnected code " << errcode << ": " << message << "\n"; }
	static void HandleDiscordError(int errcode, const char* message) { std::cout << "RPC: Error code " << errcode << ": " << message << "\n"; }
	static void HandleDiscordJoin(const char* secret) { }
	static void HandleDiscordSpectate(const char* secret) { }
	static void HandleDiscordJoinRequest(const DiscordUser* request) { }
	
	static int64_t start;
	bool in_training = false;
	
	// handle names for characters and races
	// TODO: Handle these via master.mdb over using umapyoi.net
	std::unordered_map<int, std::string> chara_dict = {};
	std::unordered_map<int, std::string> outfit_dict = {};
	std::unordered_map<int, std::string> race_dict = {};
	
	// Months and scenario dictionaries
	std::unordered_map<int, std::string> MONTH_DICT = 
	{
		{1, "January"}, {2, "February"}, {3, "March"}, {4, "April"},
		{5, "May"}, {6, "June"}, {7, "July"}, {8, "August"},
		{9, "September"}, {10, "October"}, {11, "November"}, {12, "December"}
	};
	std::unordered_map<int, std::string> SCENARIO_DICT = 
	{
		{1,	 "URA Finals"},
		{2,	 "Aoharu Cup"},
		{3,	 "Grand Live"},
		{4,	 "Make a New Track"},
		{5,	 "Grand Masters"},
		{6,	 "Project L'Arc"},
		{7,	 "U.A.F. Ready GO!"},
		{8,	 "Great Food Festival"},
		{9,	 "Run! Mecha Umamusume"},
		{10, "The Twinkle Legends"},
		{11, "Design Your Island"}
	};
	
	// default curl callback
	std::size_t callback(
			const char* in,
			std::size_t size,
			std::size_t num,
			std::string* out)
	{
		const std::size_t totalBytes(size * num);
		out->append(in, totalBytes);
		return totalBytes;
	}
	
	void get_dict_from_url(std::unordered_map<int, std::string>& dict, std::string url, std::string key_name, std::string value_name)
	{
		CURL* curl = curl_easy_init();
		if (curl == NULL) return;
		
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		
		long code = 0;
		std::unique_ptr<std::string> data(new std::string());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, data.get());
		
		curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
		
		if (code == 200)
		{
			std::cout << "HTTP 200 request to " << url << " successful!" << "\n";
			json j;
			try
			{
				j = json::parse(*data.get());
				std::cout << "Got JSON data!" << "\n";
				for (auto& [key, val] : j.items())
				{
					int k = val[key_name]; std::string v = val[value_name];
					dict[k] = v;
				}
			}
			catch (const json::parse_error& e)
			{
				std::cout << "json: parse_error: " << e.what() << "\n";
			}
		}
		
		curl_easy_cleanup(curl);
	}
	
	void get_chara_dict()
	{
		std::thread th(get_dict_from_url, std::ref(chara_dict), "https://umapyoi.net/api/v1/character/names", "game_id", "name");
		th.detach();
	}
	
	void get_outfit_dict()
	{
		std::thread th(get_dict_from_url, std::ref(outfit_dict), "https://umapyoi.net/api/v1/outfit", "id", "title");
		th.detach();
	}
	
	void get_race_dict()
	{
		std::thread th(get_dict_from_url, std::ref(race_dict), "https://umapyoi.net/api/v1/race_program", "id", "name");
		th.detach();
	}
	
	void init() 
	{
		// init curl and dictionaries
		// TODO: prefer mdb over this if possible
		curl_global_init(CURL_GLOBAL_DEFAULT);

		get_chara_dict();
		get_outfit_dict();
		get_race_dict();
		
		DiscordEventHandlers handlers;
		memset(&handlers, 0, sizeof(handlers));
			
		handlers.ready = HandleDiscordReady;
		handlers.disconnected = HandleDiscordDisconnected;
		handlers.errored = HandleDiscordError;
		handlers.joinGame = HandleDiscordJoin;
		handlers.spectateGame = HandleDiscordSpectate;
		handlers.joinRequest = HandleDiscordJoinRequest;
		
		start = time(0);
		
		Discord_Initialize(APP_ID.c_str(), &handlers, 1, NULL);
		
		// Use start up RPC here....
		DiscordRichPresence discordPresence;
		memset(&discordPresence, 0, sizeof(discordPresence));
		
		discordPresence.state = "Main Menu";
		//discordPresence.details = "Ready your umapyois!";
		discordPresence.startTimestamp = start;
		discordPresence.largeImageKey = "umaicon";
		discordPresence.largeImageText = "It's Special Week!";
		Discord_UpdatePresence(&discordPresence);
	}
	
	void deinit()
	{
		Discord_ClearPresence();
		Discord_Shutdown();
		curl_global_cleanup();
	}
	
	std::string opponent_list_sig =
		"\x81\xB3\x6F\x70\x70\x6F\x6E\x65\x6E\x74\x5F\x69\x6E\x66\x6F\x5F\x61\x72\x72\x61\x79\x93";
	std::string opponent_list_opponent_info_header = "\x88\xC0\x01";
	std::string opponent_list_opponent_info_header_fixed = "\x87";

	std::string load_index_sig_header = "\xA9\x63\x61\x72\x64\x5F\x6C\x69\x73\x74";
	std::string load_index_sig_footer = "\x01\xB1\x73\x75\x70\x70\x6F\x72\x74\x5F\x63\x61\x72\x64\x5F\x6C\x69\x73\x74";
	std::string load_index_card_list_header = "\x86\xC0\x01";
	std::string load_index_card_list_header_fixed = "\x85";

	json try_parse_msgpack(const std::string& data)
	{
		try
		{
			return json::from_msgpack(data);
		}
		catch (const json::parse_error& e)
		{
			if (e.id == 113)
			{
				// Try to fix team_stadium/opponent_list
				auto idx = data.find(opponent_list_sig);
				if (idx != std::string::npos)
				{
					std::string fixed = data;
					int cnt = 0;
					while (true)
					{
						idx = fixed.find(opponent_list_opponent_info_header, idx);
						if (idx == std::string::npos) break;
						fixed.replace(idx, opponent_list_opponent_info_header.length(),
									  opponent_list_opponent_info_header_fixed);
						idx += opponent_list_opponent_info_header_fixed.length();
						cnt += 1;
					}
					if (cnt != 3)
					{
						throw;
					}

					return json::from_msgpack(fixed);
				}

				// Try to fix load/index
				idx = data.find(load_index_sig_header);
				if (idx != std::string::npos)
				{
					auto idx_end = data.find(load_index_sig_footer, idx);
					std::string fixed = data;
					while (true)
					{
						idx = fixed.find(load_index_card_list_header, idx);
						if (idx == std::string::npos || idx > idx_end) break;
						fixed.replace(idx, load_index_card_list_header.length(),
									  load_index_card_list_header_fixed);
						idx += load_index_card_list_header_fixed.length();
						idx_end -= (load_index_card_list_header.length() - load_index_card_list_header_fixed.length());
					}

					return json::from_msgpack(fixed);
				}
			}

			throw;
		}
	}
	
	std::string turn_to_string(int turn) 
	{
		turn = turn-1;
		if (turn < 12) {
			double t = turn / 2.0 + 6;
			turn = std::floor(t);
		}

		bool second_half = (turn % 2 != 0);
		if (second_half) 
		{
			turn -= 1;
		}

		turn /= 2;

		int month = (turn % 12) + 1;
		int year = (turn / 12) + 1;
		std::string result = "Y" + std::to_string(year) + ", " + 
							 (second_half ? "Late " : "Early ") + 
							 MONTH_DICT[month];

		return result;
	}
	
	void update_presence_by_data(const std::string& data)
	{
		try
		{
			json j;
			try
			{
				j = try_parse_msgpack(data);
			}
			catch (const json::parse_error& e)
			{
				std::cout << "json: parse_error: " << e.what() << "\n";
				return;
			}

			try
			{
				if (!j.contains("data"))
				{
					return;
				}
				auto& data = j.at("data");
				
				DiscordRichPresence discordPresence;
				memset(&discordPresence, 0, sizeof(discordPresence));
				discordPresence.state = "Main Menu";
				// discordPresence.details = "Home";
				discordPresence.largeImageKey = "umaicon";
				discordPresence.largeImageText = "It's Special Week!";
				discordPresence.startTimestamp = start;
				bool should_update = false;
				
				// New loading behavior?
				if (data.contains("single_mode_load_common"))
				{
					for (auto& [key, val] : data["single_mode_load_common"].items())
					{
						data[key] = val;
					}
				}
				
				// Concert Theater
				if (data.contains("live_theater_save_info_array"))
				{
					discordPresence.state = "Concert Theater";
					discordPresence.details = "Vibing";
					should_update = true;
				}
				// Claw Machine
				if (data.contains("collected_plushies"))
				{
					discordPresence.details = "Playing the Claw Machine";
					discordPresence.largeImageKey = "claw_machine";
					discordPresence.largeImageText = "";
					auto query = mdb::find_query("SELECT chara_id FROM card_data c WHERE default_rarity != 0;");
					std::vector<int> total_charas;
					int64_t total_plushies = query.size();
					for (auto& it : query) {
						total_charas.push_back(std::get<0>(it));
					}
					size_t plushies = data.at("collected_plushies").size();
					// TODO: size isn't accurate
					std::string collected = fmt::format("Collected: {}/{}", plushies, 3 * (total_plushies + total_charas.size()));
					std::cout << collected.c_str() << "\n";
					discordPresence.state = collected.c_str();
					should_update = true;
				}
				// Run has ended
				if (data.contains("single_mode_factor_select_common"))
					should_update = true;
				// Inside a training run
				if (data.contains("chara_info"))
				{
					auto& info = data["chara_info"];
					// TODO
					int scenario_id = info["scenario_id"];
					int card_id = info["card_id"];
					// annoyingly, converting chara_key to a c string does not make this work properly
					// no seriously. wdym converting a c++ string "chara_2042" to a c string is NOT the same???
					// the command line says theres no difference. what is discord replying with that says its not
					// so sprintf shit it is
					char key[64] = {0};
					int char_id = card_id / 100; // actual character id, without the outfit id
					sprintf(key, "chara_%i", char_id);
					discordPresence.largeImageKey = key;
					std::string name = chara_dict.find(char_id) != chara_dict.end() ? chara_dict[char_id] : "";
					if (outfit_dict.find(card_id) != outfit_dict.end())
						name += "\n" + outfit_dict[card_id];
					discordPresence.largeImageText = name.c_str();
					discordPresence.smallImageText = SCENARIO_DICT.find(scenario_id) != SCENARIO_DICT.end() ? SCENARIO_DICT[scenario_id].c_str() : "";
					discordPresence.smallImageKey = "umaicon";
					// sometimes things might override what im doing at the top for some reason
					discordPresence.details = fmt::format("Training - {}", turn_to_string(info["turn"])).c_str();
					//if (data.contains("race_start_info")) // TODO
					//	discordPresence.state = "In race";
					//else
					{
						// Annoyingly having to do this
						int speed = info["speed"];
						int stamina = info["stamina"];
						int power = info["power"];
						int guts = info["guts"];
						int wisdom = info["wiz"];
						int skill_pts = info["skill_point"];
						discordPresence.state = fmt::format("{} {} {} {} {} | {}", speed, stamina, power, guts, wisdom, skill_pts).c_str();
						//std::cout << discordPresence.state << "\n";
					}
					should_update = true;
				}
				/*
				else if (data.contains("unchecked_event_array"))
				{
					// In single mode.
					if (data.contains("team_data_set") &&
						static_cast<int>(data.at("chara_info").at("turn")) <=
						config::get().aoharu_print_team_average_status_max_turn)
					{
						print_aoharu_team_average_status(data);
					}

					auto& unchecked_event_array = data.at("unchecked_event_array");

					bool should_print_chara_info = false;
					for (const auto& e : unchecked_event_array)
					{
						should_print_chara_info = print_event_data(e) || should_print_chara_info;
					}
					if (should_print_chara_info)
					{
						print_single_mode_chara_info(data.at("chara_info"));
					}

					if (unchecked_event_array.empty()
						&& data.contains("free_data_set") && data.at("free_data_set").contains(
							"pick_up_item_info_array")
						&& config::get().climax_print_shop_items)
					{
						print_climax_shop_items(data);
					}
				}
				else if (data.contains("event_contents_info"))
				{
					// In gallery/play_event.
					print_event_data(data);
				}
				else if (data.contains("team_data_set") && !data.contains("home_info"))
				{
					// single_mode_team/team_edit, team_race_*
					print_aoharu_team_info(data);
				}
				*/
				if (should_update)
				{
					Discord_UpdatePresence(&discordPresence);
				}
			}
			catch (const json::out_of_range& e)
			{
				std::cout << "json: out_of_range: " << e.what() << "\n";
			}
			catch (const json::type_error& e)
			{
				std::cout << "json: type_error: " << e.what() << "\n";
			}
		}
		catch (...)
		{
			std::cout << "Uncaught exception!\n";
		}
	}
}