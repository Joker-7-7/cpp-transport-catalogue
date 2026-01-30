#pragma once
#include "geo.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <optional>

namespace catalogue
{
	class Transport {
	public:
		void AddRoute(const std::string& id,  const std::vector<std::string_view>& route);
		void AddBusStop(const std::string& name, utils::Coordinates coord);
		std::vector<std::string> SearchRoute(const std::string& id) const;
		std::optional<utils::Coordinates> SearchBusStop(const std::string& name) const;
		std::string GetRouteInfo(const std::string& name) const;
		std::string GetBusesInfo(const std::string& stop) const;
	private:
		std::unordered_map<std::string, std::vector<std::string>> routes_;
		std::unordered_map<std::string, utils::Coordinates> stops_;
	};
}
