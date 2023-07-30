#pragma once
#include <string>
#include "transport_catalogue.h"
namespace output {
std::string ParseBus(std::string& line);
void PrintBus(std::ostream& os, std::string &  name, transport::Bus *bus);

} //output
