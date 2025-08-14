#pragma once
#include <discord_rpc.h>
#include <string>

namespace discord 
{
	void init();
	void deinit();
	void update_presence_by_data(const std::string& data);
}