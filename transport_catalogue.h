#pragma once
#include "geo.h"

#include <string>
#include <utility>
#include <string_view>
#include <deque>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace catalogue {
namespace detail {
struct Stop {
	bool operator==(const Stop &stop) const {
		return name == stop.name && coordinates == stop.coordinates;
	}

	std::string name;
	location::Coordinates coordinates;
};

struct Bus {
	bool operator==(const Bus &bus) const {
		return number == bus.number && stop_list == bus.stop_list;
	}

	std::string number;
	std::vector<const Stop*> stop_list;
};

struct BusInfo {
	int stops;
	int unique_stops;
	int length;
	double curvature;
};

struct PairStopHasher {
	size_t operator()(const std::pair<const detail::Stop*, const detail::Stop*> &pair) const {
		auto hash1 = std::hash<const void*>{}(pair.first);
		auto hash2 = std::hash<const void*>{}(pair.second);
		return hash1 + hash2 * 37;
	}
};
}

class TransportCatalogue {
public:
	TransportCatalogue() = default;
	void AddStop(const std::string_view name, location::Coordinates coord);
	void AddBus(const std::string_view name, const std::vector<const detail::Stop*> &stops_list);
	const detail::Stop* FindStop(std::string_view str) const;
	const detail::Bus* FindBus(std::string_view num) const;
	detail::BusInfo GetBusInfo(const detail::Bus &bus) const;
	std::vector<std::string_view> GetStopInfo(const detail::Stop &stop) const;
	void AddDistance(const std::string_view from, const std::string_view to, int distance);
	int GetDistance(const std::string_view from, const std::string_view to) const;

private:
	std::deque<detail::Stop> stops_;
	std::unordered_map<std::string_view, const detail::Stop *> stops_ptr_;

	std::deque<detail::Bus> buses_;
	std::unordered_map<std::string_view, const detail::Bus *> buses_ptr_;

	std::unordered_map<std::string_view, std::vector<std::string_view>> stop_buses_;
	std::unordered_map<std::pair<const detail::Stop *, const detail::Stop *>, int, detail::PairStopHasher> distances_;
};
}