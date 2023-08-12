#pragma once
#include <string>
#include "transport_catalogue.h"
namespace output {
std::string ParseStop(std::string& line);
std::string ParseBus(std::string& line);
void PrintBus(std::ostream& os, transport::BusPrintInfo* info);
void PrintStop(std::ostream& os, std::string &  name, std::pair<bool,std::set<std::string_view>*> st);
} //output
