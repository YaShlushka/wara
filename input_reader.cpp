#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <iterator>

/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
namespace catalogue::input {
namespace parse {
location::Coordinates ParseCoordinates(std::string_view str) {
	static const double nan = std::nan("");

	auto not_space = str.find_first_not_of(' ');
	auto comma = str.find(',');

	if (comma == str.npos) {
		return {nan, nan};
	}

	auto not_space2 = str.find_first_not_of(' ', comma + 1);

	double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
	double lng = std::stod(std::string(str.substr(not_space2)));

	return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
	const auto start = string.find_first_not_of(' ');
	if (start == string.npos) {
		return {};
	}
	return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
	std::vector<std::string_view> result;

	size_t pos = 0;
	while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
		auto delim_pos = string.find(delim, pos);
		if (delim_pos == string.npos) {
			 delim_pos = string.size();
		}
		if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
			result.push_back(substr);
		}
		pos = delim_pos + 1;
	}

	return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
	if (route.find('>') != route.npos) {
		return Split(route, '>');
	}

	auto stops = Split(route, '-');
	std::vector<std::string_view> results(stops.begin(), stops.end());
	results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

	return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
	auto colon_pos = line.find(':');
	if (colon_pos == line.npos) {
		return {};
	}

	auto space_pos = line.find(' ');
	if (space_pos >= colon_pos) {
		return {};
	}

	auto not_space = line.find_first_not_of(' ', space_pos);
	if (not_space >= colon_pos) {
		return {};
	}

	return {std::string(line.substr(0, space_pos)),
			  std::string(line.substr(not_space, colon_pos - not_space)),
			  std::string(line.substr(colon_pos + 1))};
}

void ApplyDistance(std::string_view stop_name, std::string_view str, TransportCatalogue &catalogue) {
	auto args = Split(str, ',');

	for(int i = 2; i < args.size(); ++i) {
		std::string_view &cur = args[i];
		size_t distance_end = cur.find('m');
		size_t to_start = cur.find_first_not_of(' ', distance_end + 1);
		size_t stop_start = cur.find_first_not_of(' ', to_start + 3);

		catalogue.AddDistance(stop_name, cur.substr(stop_start),
									std::stoi(std::string(cur.substr(0, distance_end))));
	}
}
}

void InputReader::ParseLine(std::string_view line) {
	auto command_description = parse::ParseCommandDescription(line);

	if (command_description) {
		commands_.push_back(std::move(command_description));
	}
}

void InputReader::ApplyCommands([[maybe_unused]] TransportCatalogue& catalogue) const {
	std::unordered_map<std::string_view, std::string_view> distances;

	for(auto &command : commands_) {
		if(command.command != "Stop") {
			continue;
		}

		location::Coordinates coord = parse::ParseCoordinates(command.description);
		catalogue.AddStop(std::move(command.id), std::move(coord));
		distances.insert({command.id, command.description});
	}

	for(auto i : distances) {
		parse::ApplyDistance(i.first, i.second, catalogue);
	}

	for(auto &command : commands_) {
		if(command.command != "Bus") {
			continue;
		}

		std::vector<std::string_view> stops = parse::ParseRoute(command.description);
		std::vector<const detail::Stop *> stop_names;
		for(auto stop : stops) {
			const detail::Stop *temp = catalogue.FindStop(stop);
			stop_names.push_back(temp);
		}
		catalogue.AddBus(command.id, stop_names);
	}
}
}