#pragma once
#include <discord_rpc.h>
#include <string>

namespace discord 
{
	void init();
	void deinit();
	void update_presence_by_data(const std::string& data);
	void update_presence_by_data_request(const std::string& data);
	void get_main_menu_screenshot();
}