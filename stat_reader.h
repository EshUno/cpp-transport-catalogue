#pragma once
#include <string>
#include "transport_catalogue.h"
namespace output {
void LoadQueries(std::istream& in, transport::TransportCatalogue &catalogue);
std::string ParseStop(std::string& line);
std::string ParseBus(std::string& line);
void PrintBus(std::ostream& os, transport::BusPrintInfo* info);
void PrintStop(std::ostream& os, const std::string &name, std::optional<const std::set<std::string_view>*> st);
} //output
