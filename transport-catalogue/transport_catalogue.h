#pragma once
#include "geo.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <optional>
#include <deque>
#include <set>

namespace catalogue
{
	class Transport {
	public:
		struct Stop {
			std::string name;
			utils::Coordinates coord;
		};

		struct Route {
			std::string name;
			std::vector<std::string> stops;
		};

		struct RouteInfo {
			int stops_count = 0;
			int unique_stops_count = 0;
			double length = 0.0;
			double curvature = 1.0;
		};

		struct StopInfo {
			std::string name;
			std::vector<std::string>* buses;
		};

		struct PairHash {
			size_t operator()(const std::pair<const Stop*, const Stop*>& p) const noexcept {
					size_t h1 = std::hash<const Stop*>{}(p.first);
					size_t h2 = std::hash<const Stop*>{}(p.second);
					return h1 * 37 + h2;
				}
		};

		void AddRoute(std::string_view id,  const std::vector<std::string_view>& route);
		void AddBusStop(std::string_view name, utils::Coordinates coord);
		void AddDistance(std::string_view a, std::string_view b, double distance);
		
		const Route* SearchRoute(std::string_view id) const;
		const Stop* SearchBusStop(std::string_view name) const;
		std::optional<RouteInfo> GetRouteInfo(const std::string& name) const;
		std::optional<std::set<std::string>> GetBusesInfo(const std::string& stop) const;
		double GetDistance(std::string_view from, std::string_view to) const;

	private:
		std::deque<Stop> stops_;
		std::deque<Route> routes_;

		std::unordered_map<std::string_view, const Route*> ref_routes_;
		std::unordered_map<std::string_view, const Stop*> ref_stops_;
		std::unordered_map<std::string_view, std::vector<std::string_view>> stop_to_routes_;
		std::unordered_map<std::pair<const Stop*, const Stop*>, double, PairHash> distances_;
	};
}