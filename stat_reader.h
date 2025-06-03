#pragma once

#include <iosfwd>
#include <string_view>

#include "transport_catalogue.h"

namespace catalogue::output {
	void ParseAndPrintStat(const TransportCatalogue& catalogue, std::string_view request,
							  	  std::ostream& output);
}