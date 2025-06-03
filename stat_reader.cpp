#include "stat_reader.h"
#include <ostream>

namespace catalogue::output {
namespace request {
struct Request {
	std::string_view type;
	std::string_view description;
};

Request ParseRequest(std::string_view req) {
	size_t space_pos = req.find(' ');
	std::string_view type = req.substr(0, space_pos);
	std::string_view description = req.substr(space_pos + 1);

	return Request{type, description};
}
}

namespace print {
	void BusInfo(const detail::Bus *bus, const TransportCatalogue &catalogue, std::ostream &out) {
		if(!bus) {
			out << "not found" << std::endl;
			return;	
		}

		detail::BusInfo info = catalogue.GetBusInfo(*bus);
		out << info.stops << " stops on route, " << info.unique_stops << " unique stops, "
				<< info.length << " route length, " << info.curvature << " curvature" << std::endl;
	}

	void StopInfo(const detail::Stop *stop, const TransportCatalogue &catalogue, std::ostream &out) {
		if(!stop) {
			out << "not found" << std::endl;
			return;
		}

		auto buses = catalogue.GetStopInfo(*stop);

		if(buses.empty()) {
			out << "no buses" << std::endl;
			return;
		}

		out << "buses";

		for(auto i : buses) {
			out << " " << i;
		}

		out << std::endl;
	}
}

void ParseAndPrintStat(const TransportCatalogue& catalogue, std::string_view request,
							  std::ostream& output) {
	request::Request req = request::ParseRequest(request);
	output << req.type << " " << req.description << ": ";
	
	if(req.type == "Bus") {
		const detail::Bus *bus = catalogue.FindBus(req.description);
		print::BusInfo(bus, catalogue, output);

		return;
	}
	
	if(req.type == "Stop") {
		const detail::Stop *stop = catalogue.FindStop(req.description);
		print::StopInfo(stop, catalogue, output);
	}
}
}