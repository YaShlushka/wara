#include "transport_catalogue.h"

#include <algorithm>

namespace catalogue {
void TransportCatalogue::AddStop(const std::string_view name, location::Coordinates coord) {
	if(std::find(stops_.begin(), stops_.end(), detail::Stop{std::string(name), coord}) != stops_.end()) {
		return;
	}

	stops_.push_back(detail::Stop{std::string(name), coord});
	detail::Stop &stop = stops_.back();
	stops_ptr_.insert({stop.name, &stop});
	stop_buses_.insert({stop.name, {}});
}

void TransportCatalogue::AddBus(const std::string_view name, const std::vector<const detail::Stop*> &stops_list) {
	if(std::find(buses_.begin(), buses_.end(), detail::Bus{std::string(name), stops_list}) != buses_.end()) {
		return;
	}

	buses_.push_back(detail::Bus{std::string(name), stops_list});
	detail::Bus &bus = buses_.back();
	buses_ptr_.insert({bus.number, &bus});

	for(const detail::Stop *i : stops_list) {
		std::unordered_set<std::string_view> &buses = stop_buses_[i->name];

		buses.insert(bus.number);
	}
}

const detail::Stop* TransportCatalogue::FindStop(std::string_view str) const {
	auto it = stops_ptr_.find(str);
	return it != stops_ptr_.end() ? it->second : nullptr;
}

const detail::Bus* TransportCatalogue::FindBus(std::string_view num) const {
	auto it = buses_ptr_.find(num);
	return it != buses_ptr_.end() ? it->second : nullptr;
}

detail::BusInfo TransportCatalogue::GetBusInfo(const detail::Bus &bus) const {
	int stops = bus.stop_list.size();
	std::vector<std::string_view> unique_stops;

	for(const detail::Stop *i : bus.stop_list) {
		if(std::find(unique_stops.begin(), unique_stops.end(), i->name) == unique_stops.end()){
			unique_stops.push_back(i->name);
		}
	}

	int length = 0;
	double temp = 0;

	detail::Stop prev = *bus.stop_list[0];
	for(size_t i = 1; i < bus.stop_list.size(); ++i) {
		detail::Stop cur = *bus.stop_list[i];

		temp += ComputeDistance(prev.coordinates, cur.coordinates);
		length += GetDistance(prev.name, cur.name);
		prev = cur;
	}

	double curvature = length / temp;

	detail::BusInfo info{stops, static_cast<int> (unique_stops.size()), length, curvature};
	return info;
}

std::vector<std::string_view> TransportCatalogue::GetStopInfo(const detail::Stop &stop) const {
	auto it = stop_buses_.find(stop.name);

	if(it != stop_buses_.end()){
		return {it->second.begin(), it->second.end()};
	}

	return {};
}

void TransportCatalogue::AddDistance(const std::string_view from, const std::string_view to, int distance) {
	auto stop1 = FindStop(from);
	auto stop2 = FindStop(to);

	auto p = std::make_pair(stop1, stop2);
	distances_.insert({p, distance});
}

int TransportCatalogue::GetDistance(const std::string_view from, const std::string_view to) const {
	auto stop1 = FindStop(from);
	auto stop2 = FindStop(to);

	auto it = distances_.find(std::make_pair(stop1, stop2));
	if(it != distances_.end()) {
		return it->second;
	}

	it = distances_.find(std::make_pair(stop2, stop1));
	if(it != distances_.end()) {
		return it->second;
	}

	return 0;
}
}